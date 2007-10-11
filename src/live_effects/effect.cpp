#define INKSCAPE_LIVEPATHEFFECT_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"
#include "xml/node-event-vector.h"
#include "sp-object.h"
#include "attributes.h"
#include "message-stack.h"
#include "desktop.h"
#include "inkscape.h"
#include "document.h"
#include <glibmm/i18n.h>

#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/parameter/parameter.h"
#include <glibmm/ustring.h>
#include "live_effects/n-art-bpath-2geom.h"
#include "display/curve.h"
#include <2geom/sbasis-to-bezier.h>
#include <gtkmm.h>

#include <exception>

// include effects:
#include "live_effects/lpe-skeletalstrokes.h"
#include "live_effects/lpe-slant.h"
#include "live_effects/lpe-test-doEffect-stack.h"
#include "live_effects/lpe-gears.h"
#include "live_effects/lpe-curvestitch.h"

namespace Inkscape {

namespace LivePathEffect {

const Util::EnumData<EffectType> LPETypeData[INVALID_LPE] = {
    // {constant defined in effect.h, N_("name of your effect"), "name of your effect in SVG"}
    {SKELETAL_STROKES,      N_("Path along path"),      "skeletal"},
#ifdef LPE_ENABLE_TEST_EFFECTS
    {SLANT,                 N_("Slant"),                 "slant"},
    {DOEFFECTSTACK_TEST,    N_("doEffect stack test"),   "doeffectstacktest"},
#endif
    {GEARS,                 N_("Gears"),                 "gears"},
    {CURVE_STITCH,          N_("Curve stitching"),       "curvestitching"},
};
const Util::EnumDataConverter<EffectType> LPETypeConverter(LPETypeData, INVALID_LPE);

Effect*
Effect::New(EffectType lpenr, LivePathEffectObject *lpeobj)
{
    Effect* neweffect = NULL;
    switch (lpenr) {
        case SKELETAL_STROKES:
            neweffect = (Effect*) new LPESkeletalStrokes(lpeobj);
            break;
#ifdef LPE_ENABLE_TEST_EFFECTS
            case SLANT:
            neweffect = (Effect*) new LPESlant(lpeobj);
            break;
        case DOEFFECTSTACK_TEST:
            neweffect = (Effect*) new LPEdoEffectStackTest(lpeobj);
            break;
#endif
        case GEARS:
            neweffect = (Effect*) new LPEGears(lpeobj);
            break;
        case CURVE_STITCH:
            neweffect = (Effect*) new LPECurveStitch(lpeobj);
            break;
        default:
            g_warning("LivePathEffect::Effect::New   called with invalid patheffect type (%d)", lpenr);
            neweffect = NULL;
            break;
    }

    if (neweffect) {
        neweffect->readallParameters(SP_OBJECT_REPR(lpeobj));
    }

    return neweffect;
}

Effect::Effect(LivePathEffectObject *lpeobject)
{
    vbox = NULL;
    tooltips = NULL;
    lpeobj = lpeobject;
}

Effect::~Effect()
{
    if (tooltips) {
        delete tooltips;
    }
}

Glib::ustring 
Effect::getName()
{
    if (lpeobj->effecttype_set && lpeobj->effecttype < INVALID_LPE)
        return Glib::ustring( _(LPETypeConverter.get_label(lpeobj->effecttype).c_str()) );
    else
        return Glib::ustring( _("No effect") );
}

/*
 *  Here be the doEffect function chain:
 */
void
Effect::doEffect (SPCurve * curve)
{
    NArtBpath *new_bpath = doEffect(SP_CURVE_BPATH(curve));

    if (new_bpath && new_bpath != SP_CURVE_BPATH(curve)) {        // FIXME, add function to SPCurve to change bpath? or a copy function?
        if (curve->_bpath) {
            g_free(curve->_bpath); //delete old bpath
        }
        curve->_bpath = new_bpath;
    }
}

NArtBpath *
Effect::doEffect (NArtBpath * path_in)
{
    try {
        std::vector<Geom::Path> orig_pathv = BPath_to_2GeomPath(path_in);

        std::vector<Geom::Path> result_pathv = doEffect(orig_pathv);

        NArtBpath *new_bpath = BPath_from_2GeomPath(result_pathv);

        return new_bpath;
    }
    catch (std::exception e) {
        g_warning("An exception occurred during execution of an LPE - %s", e.what());
        SP_ACTIVE_DESKTOP->messageStack()->flash( Inkscape::WARNING_MESSAGE,
            _("An exception occurred during execution of a Path Effect.") );

        NArtBpath *path_out;

        unsigned ret = 0;
        while ( path_in[ret].code != NR_END ) {
            ++ret;
        }
        unsigned len = ++ret;

        path_out = g_new(NArtBpath, len);
        memcpy(path_out, path_in, len * sizeof(NArtBpath));
        return path_out;
    }
}

std::vector<Geom::Path>
Effect::doEffect (std::vector<Geom::Path> & path_in)
{
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in;

    for (unsigned int i=0; i < path_in.size(); i++) {
        pwd2_in.concat( path_in[i].toPwSb() );
    }

    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_out = doEffect(pwd2_in);

    std::vector<Geom::Path> path_out = Geom::path_from_piecewise( pwd2_out, LPE_CONVERSION_TOLERANCE);

    return path_out;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
Effect::doEffect (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in)
{
    g_warning("Effect has no doEffect implementation");
    return pwd2_in;
}

void
Effect::readallParameters(Inkscape::XML::Node * repr)
{
    param_map_type::iterator it = param_map.begin();
    while (it != param_map.end()) {
        const gchar * key = (*it).first.c_str();
        const gchar * value = repr->attribute(key);
        if(value) {
            setParameter(repr, key, NULL, value);
        }
        it++;
    }
}

void
Effect::setParameter(Inkscape::XML::Node * repr, const gchar * key, const gchar * old_value, const gchar * new_value)
{
    Glib::ustring stringkey(key);

    param_map_type::iterator it = param_map.find(stringkey);
    if (it != param_map.end()) {
        if (new_value) {
            bool accepted = it->second->param_readSVGValue(new_value);
            if (!accepted) { 
                g_warning("Effect::setParameter - '%s' not accepted for %s", new_value, key);
                // change was not accepted, so change it back.
                // think: can this backfire and create infinite loop when started with unacceptable old_value?
                // repr->setAttribute(key, old_value);
            }
        } else {
            // set default value
            it->second->param_set_default();
        }
    }
}

void
Effect::registerParameter(Parameter * param)
{
    param_map[param->param_key] = param; // inserts or updates
}

Gtk::Widget *
Effect::getWidget()
{
    if (!vbox) {
        vbox = Gtk::manage( new Gtk::VBox() ); // use manage here, because after deletion of Effect object, others might still be pointing to this widget.
        //if (!tooltips)
            tooltips = new Gtk::Tooltips();

        vbox->set_border_width(5);

        param_map_type::iterator it = param_map.begin();
        while (it != param_map.end()) {
            Parameter * param = it->second;
            Gtk::Widget * widg = param->param_getWidget();
            Glib::ustring * tip = param->param_getTooltip();
            if (widg) {
               vbox->pack_start(*widg, true, true, 2);
                if (tip != NULL) {
                    tooltips->set_tip(*widg, *tip);
                }
            }

            it++;
        }
    }

    return dynamic_cast<Gtk::Widget *>(vbox);
}


Inkscape::XML::Node * 
Effect::getRepr()
{
    return SP_OBJECT_REPR(lpeobj);
}

SPDocument * 
Effect::getSPDoc()
{
    if (SP_OBJECT_DOCUMENT(lpeobj) == NULL) g_message("Effect::getSPDoc() returns NULL");
    return SP_OBJECT_DOCUMENT(lpeobj);
}


} /* namespace LivePathEffect */

} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
