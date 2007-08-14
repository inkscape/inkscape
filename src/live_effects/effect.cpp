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

#include "desktop.h"

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

// include effects:
#include "live_effects/lpe-skeletalstrokes.h"
#include "live_effects/lpe-slant.h"
#include "live_effects/lpe-test-doEffect-stack.h"
#include "live_effects/lpe-gears.h"

namespace Inkscape {

namespace LivePathEffect {

const Util::EnumData<EffectType> LPETypeData[ENDTYPE_LPE] = {
    {INVALID_LPE,           _("Invalid effect"),        "invalid"},
    {SKELETAL_STROKES,      _("Skeletal Strokes"),      "skeletal"},
    {SLANT,                 _("Slant"),                 "slant"},
    {DOEFFECTSTACK_TEST,    _("doEffect stack test"),   "doeffectstacktest"},
    {GEARS,                 _("Gears"),                 "gears"}
};
const Util::EnumDataConverter<EffectType> LPETypeConverter(LPETypeData, ENDTYPE_LPE);

Effect*
Effect::New(EffectType lpenr, LivePathEffectObject *lpeobj)
{
    switch (lpenr) {
        case INVALID_LPE:
            g_warning("LivePathEffect::Effect::New   called with invalid patheffect type");
            return NULL;
        case SKELETAL_STROKES:
            return (Effect*) new LPESkeletalStrokes(lpeobj);
        case SLANT:
            return (Effect*) new LPESlant(lpeobj);
        case DOEFFECTSTACK_TEST:
            return (Effect*) new LPEdoEffectStackTest(lpeobj);
        case GEARS:
            return (Effect*) new LPEGears(lpeobj);
        case ENDTYPE_LPE:
            return NULL;
    }

    return NULL;
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
    return Glib::ustring( LPETypeConverter.get_label(lpeobj->effecttype) );
}

/*
 *  Here be the doEffect function chain:
 */
void
Effect::doEffect (SPCurve * curve)
{
    NArtBpath *new_bpath = doEffect(SP_CURVE_BPATH(curve));

    if (new_bpath) {        // FIXME, add function to SPCurve to change bpath? or a copy function?
        if (curve->_bpath) {
            g_free(curve->_bpath); //delete old bpath
        }
        curve->_bpath = new_bpath;
    }
}

NArtBpath *
Effect::doEffect (NArtBpath * path_in)
{
    std::vector<Geom::Path> orig_pathv = BPath_to_2GeomPath(path_in);

    std::vector<Geom::Path> result_pathv = doEffect(orig_pathv);

    NArtBpath *new_bpath = BPath_from_2GeomPath(result_pathv);

    return new_bpath;
}

std::vector<Geom::Path>
Effect::doEffect (std::vector<Geom::Path> & path_in)
{
    Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in;
    // FIXME: find standard function to convert std::vector<Geom::Path> ==> Piecewise< D2<SBasis> >
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
        bool accepted = it->second->param_readSVGValue(new_value);
        /* think: can this backfire and create infinite loop when started with unacceptable old_value?
        if (!accepted) { // change was not accepted, so change it back.
            repr->setAttribute(key, old_value);
        } */
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
    if (SP_OBJECT_DOCUMENT(lpeobj) == NULL) g_message("oh crap");
    return SP_OBJECT_DOCUMENT(lpeobj);
}


}; /* namespace LivePathEffect */

}; /* namespace Inkscape */

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
