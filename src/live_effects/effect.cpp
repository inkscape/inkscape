#define INKSCAPE_LIVEPATHEFFECT_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"

#include "display/display-forward.h"
#include "xml/node-event-vector.h"
#include "sp-object.h"
#include "attributes.h"
#include "message-stack.h"
#include "desktop.h"
#include "inkscape.h"
#include "document.h"
#include "document-private.h"
#include "xml/document.h"
#include <glibmm/i18n.h>
#include "pen-context.h"
#include "tools-switch.h"
#include "message-stack.h"
#include "desktop.h"
#include "nodepath.h"

#include "live_effects/lpeobject.h"
#include "live_effects/parameter/parameter.h"
#include <glibmm/ustring.h>
#include "libnr/n-art-bpath-2geom.h"
#include "display/curve.h"
#include <gtkmm.h>

#include <exception>

#include <2geom/sbasis-to-bezier.h>
#include <2geom/matrix.h>


// include effects:
#include "live_effects/lpe-patternalongpath.h"
#include "live_effects/lpe-bendpath.h"
#include "live_effects/lpe-sketch.h"
#include "live_effects/lpe-vonkoch.h"
#include "live_effects/lpe-knot.h"
#include "live_effects/lpe-test-doEffect-stack.h"
#include "live_effects/lpe-gears.h"
#include "live_effects/lpe-curvestitch.h"
#include "live_effects/lpe-circle_with_radius.h"
#include "live_effects/lpe-perspective_path.h"
#include "live_effects/lpe-spiro.h"
#include "live_effects/lpe-lattice.h"
#include "live_effects/lpe-envelope.h"
#include "live_effects/lpe-constructgrid.h"
#include "live_effects/lpe-perp_bisector.h"
#include "live_effects/lpe-tangent_to_curve.h"
#include "live_effects/lpe-mirror_reflect.h"
#include "live_effects/lpe-circle_3pts.h"
#include "live_effects/lpe-angle_bisector.h"
#include "live_effects/lpe-parallel.h"
// end of includes

namespace Inkscape {

namespace LivePathEffect {

const Util::EnumData<EffectType> LPETypeData[INVALID_LPE] = {
    // {constant defined in effect.h, N_("name of your effect"), "name of your effect in SVG"}
    {BEND_PATH,             N_("Bend"),                  "bend_path"},
    {PATTERN_ALONG_PATH,    N_("Pattern Along Path"),    "skeletal"},   // for historic reasons, this effect is called skeletal(strokes) in Inkscape:SVG
    {SKETCH,                N_("Sketch"),                "sketch"},
    {VONKOCH,               N_("VonKoch"),               "vonkoch"},
    {KNOT,                  N_("Knot"),                  "knot"},
#ifdef LPE_ENABLE_TEST_EFFECTS
    {DOEFFECTSTACK_TEST,    N_("doEffect stack test"),   "doeffectstacktest"},
#endif
    {GEARS,                 N_("Gears"),                 "gears"},
    {CURVE_STITCH,          N_("Stitch Sub-Paths"),       "curvestitching"},
    {CIRCLE_WITH_RADIUS,    N_("Circle (center+radius)"), "circle_with_radius"},
    {PERSPECTIVE_PATH,      N_("Perspective path"),      "perspective_path"},
    {SPIRO,      N_("Spiro spline"),      "spiro"},
    {LATTICE,               N_("Lattice Deformation"),   "lattice"},
    {ENVELOPE,              N_("Envelope Deformation"),  "envelope"},
    {CONSTRUCT_GRID,        N_("Construct grid"),        "construct_grid"},
    {PERP_BISECTOR, N_("Perpendicular bisector"), "perp_bisector"},
    {TANGENT_TO_CURVE, N_("Tangent to curve"), "tangent_to_curve"},
    {MIRROR_REFLECT, N_("Mirror reflection"), "mirror_reflect"},
    {CIRCLE_3PTS, N_("Circle through 3 points"), "circle_3pts"},
    {ANGLE_BISECTOR, N_("Angle bisector"), "angle_bisector"},
    {PARALLEL, N_("Parallel"), "parallel"},
};
const Util::EnumDataConverter<EffectType> LPETypeConverter(LPETypeData, INVALID_LPE);

Effect*
Effect::New(EffectType lpenr, LivePathEffectObject *lpeobj)
{
    Effect* neweffect = NULL;
    switch (lpenr) {
        case PATTERN_ALONG_PATH:
            neweffect = static_cast<Effect*> ( new LPEPatternAlongPath(lpeobj) );
            break;
        case BEND_PATH:
            neweffect = static_cast<Effect*> ( new LPEBendPath(lpeobj) );
            break;
        case SKETCH:
            neweffect = static_cast<Effect*> ( new LPESketch(lpeobj) );
            break;
        case VONKOCH:
            neweffect = static_cast<Effect*> ( new LPEVonKoch(lpeobj) );
            break;
        case KNOT:
            neweffect = static_cast<Effect*> ( new LPEKnot(lpeobj) );
            break;
#ifdef LPE_ENABLE_TEST_EFFECTS
        case DOEFFECTSTACK_TEST:
            neweffect = static_cast<Effect*> ( new LPEdoEffectStackTest(lpeobj) );
            break;
#endif
        case GEARS:
            neweffect = static_cast<Effect*> ( new LPEGears(lpeobj) );
            break;
        case CURVE_STITCH:
            neweffect = static_cast<Effect*> ( new LPECurveStitch(lpeobj) );
            break;
        case LATTICE:
            neweffect = static_cast<Effect*> ( new LPELattice(lpeobj) );
            break;
        case ENVELOPE:
            neweffect = static_cast<Effect*> ( new LPEEnvelope(lpeobj) );
            break;
        case CIRCLE_WITH_RADIUS:
            neweffect = static_cast<Effect*> ( new LPECircleWithRadius(lpeobj) );
            break;
        case PERSPECTIVE_PATH:
            neweffect = static_cast<Effect*> ( new LPEPerspectivePath(lpeobj) );
            break;
        case SPIRO:
            neweffect = static_cast<Effect*> ( new LPESpiro(lpeobj) );
            break;
        case CONSTRUCT_GRID:
            neweffect = static_cast<Effect*> ( new LPEConstructGrid(lpeobj) );
            break;
        case PERP_BISECTOR:
            neweffect = static_cast<Effect*> ( new LPEPerpBisector(lpeobj) );
            break;
        case TANGENT_TO_CURVE:
            neweffect = static_cast<Effect*> ( new LPETangentToCurve(lpeobj) );
            break;
        case MIRROR_REFLECT:
            neweffect = static_cast<Effect*> ( new LPEMirrorReflect(lpeobj) );
            break;
        case CIRCLE_3PTS:
            neweffect = static_cast<Effect*> ( new LPECircle3Pts(lpeobj) );
            break;
        case ANGLE_BISECTOR:
            neweffect = static_cast<Effect*> ( new LPEAngleBisector(lpeobj) );
            break;
        case PARALLEL:
            neweffect = static_cast<Effect*> ( new LPEParallel(lpeobj) );
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

void
Effect::createAndApply(const char* name, SPDocument *doc, SPItem *item)
{
    // Path effect definition
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
    Inkscape::XML::Node *repr = xml_doc->createElement("inkscape:path-effect");
    repr->setAttribute("effect", name);

    SP_OBJECT_REPR(SP_DOCUMENT_DEFS(doc))->addChild(repr, NULL); // adds to <defs> and assigns the 'id' attribute
    const gchar * repr_id = repr->attribute("id");
    Inkscape::GC::release(repr);

    gchar *href = g_strdup_printf("#%s", repr_id);
    sp_lpe_item_add_path_effect(SP_LPE_ITEM(item), href, true);
    g_free(href);

    sp_document_done(doc, SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                     _("Create and apply path effect"));
}

void
Effect::createAndApply(EffectType type, SPDocument *doc, SPItem *item)
{
    createAndApply(LPETypeConverter.get_key(type).c_str(), doc, item);
}

Effect::Effect(LivePathEffectObject *lpeobject)
    : oncanvasedit_it(0),
      is_visible(_("Is visible?"), _("If unchecked, the effect remains applied to the object but is temporarily disabled on canvas"), "is_visible", &wr, this, true),
      done_pathparam_set(false),
      show_orig_path(false),
      lpeobj(lpeobject),
      concatenate_before_pwd2(false),
      provides_own_flash_paths(true) // is automatically set to false if providesOwnFlashPaths() is not overridden
{
    registerParameter( dynamic_cast<Parameter *>(&is_visible) );
}

Effect::~Effect()
{
}

Glib::ustring
Effect::getName()
{
    if (lpeobj->effecttype_set && lpeobj->effecttype < INVALID_LPE)
        return Glib::ustring( _(LPETypeConverter.get_label(lpeobj->effecttype).c_str()) );
    else
        return Glib::ustring( _("No effect") );
}

EffectType
Effect::effectType() {
    return lpeobj->effecttype;
}

/**
 * Is performed a single time when the effect is freshly applied to a path
 */
void
Effect::doOnApply (SPLPEItem */*lpeitem*/)
{
}

/**
 * Is performed each time before the effect is updated.
 */
void
Effect::doBeforeEffect (SPLPEItem */*lpeitem*/)
{
    //Do nothing for simple effects
}

/**
 * Effects have a parameter path set before they are applied by accepting a nonzero number of mouse
 * clicks. This method activates the pen context, which waits for the specified number of clicks.
 * Override Effect::acceptsNumParams() to set the number of expected mouse clicks.
 */
void
Effect::doAcceptPathPreparations(SPLPEItem *lpeitem)
{
    // switch to pen context
    SPDesktop *desktop = inkscape_active_desktop(); // TODO: Is there a better method to find the item's desktop?
    if (!tools_isactive(desktop, TOOLS_FREEHAND_PEN)) {
        tools_switch(desktop, TOOLS_FREEHAND_PEN);
    }

    SPEventContext *ec = desktop->event_context;
    SPPenContext *pc = SP_PEN_CONTEXT(ec);
    pc->expecting_clicks_for_LPE = this->acceptsNumParams();
    pc->waiting_LPE = this;
    pc->waiting_item = lpeitem;
    pc->polylines_only = true;

    ec->desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE,
        g_strdup_printf(_("Please specify a parameter path for the LPE '%s' with %d mouse clicks"),
                        getName().c_str(), acceptsNumParams()));
}

void
Effect::writeParamsToSVG() {
    std::vector<Inkscape::LivePathEffect::Parameter *>::iterator p;
    for (p = param_vector.begin(); p != param_vector.end(); ++p) {
        (*p)->write_to_SVG();
    }
}

/**
 * If the effect expects a path parameter (specified by a number of mouse clicks) before it is
 * applied, this is the method that processes the resulting path. Override it to customize it for
 * your LPE. But don't forget to call the parent method so that done_pathparam_set is set to true!
 */
void
Effect::acceptParamPath (SPPath *param_path) {
    done_pathparam_set = true;
}

/*
 *  Here be the doEffect function chain:
 */
void
Effect::doEffect (SPCurve * curve)
{
    std::vector<Geom::Path> orig_pathv = curve->get_pathvector();

    std::vector<Geom::Path> result_pathv = doEffect_path(orig_pathv);

    curve->set_pathv(result_pathv);
}

std::vector<Geom::Path>
Effect::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    std::vector<Geom::Path> path_out;

    if ( !concatenate_before_pwd2 ) {
        // default behavior
        for (unsigned int i=0; i < path_in.size(); i++) {
            Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in = path_in[i].toPwSb();
            Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_out = doEffect_pwd2(pwd2_in);
            std::vector<Geom::Path> path = Geom::path_from_piecewise( pwd2_out, LPE_CONVERSION_TOLERANCE);
            // add the output path vector to the already accumulated vector:
            for (unsigned int j=0; j < path.size(); j++) {
                path_out.push_back(path[j]);
            }
        }
    } else {
      // concatenate the path into possibly discontinuous pwd2
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in;
        for (unsigned int i=0; i < path_in.size(); i++) {
            pwd2_in.concat( path_in[i].toPwSb() );
        }
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_out = doEffect_pwd2(pwd2_in);
        path_out = Geom::path_from_piecewise( pwd2_out, LPE_CONVERSION_TOLERANCE);
    }

    return path_out;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
Effect::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    g_warning("Effect has no doEffect implementation");
    return pwd2_in;
}

void
Effect::readallParameters(Inkscape::XML::Node * repr)
{
    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        Parameter * param = *it;
        const gchar * key = param->param_key.c_str();
        const gchar * value = repr->attribute(key);
        if (value) {
            bool accepted = param->param_readSVGValue(value);
            if (!accepted) {
                g_warning("Effect::readallParameters - '%s' not accepted for %s", value, key);
            }
        } else {
            // set default value
            param->param_set_default();
        }

        it++;
    }
}

/* This function does not and SHOULD NOT write to XML */
void
Effect::setParameter(const gchar * key, const gchar * new_value)
{
    Parameter * param = getParameter(key);
    if (param) {
        if (new_value) {
            bool accepted = param->param_readSVGValue(new_value);
            if (!accepted) {
                g_warning("Effect::setParameter - '%s' not accepted for %s", new_value, key);
            }
        } else {
            // set default value
            param->param_set_default();
        }
    }
}

void
Effect::registerParameter(Parameter * param)
{
    param_vector.push_back(param);
}

// TODO: should we provide a way to alter the handle's appearance?
void
Effect::registerKnotHolderHandle(KnotHolderEntity* entity, const char* descr)
{
    kh_entity_vector.push_back(std::make_pair(entity, descr));
}

/**
 * Add all registered LPE knotholder handles to the knotholder
 */
void
Effect::addHandles(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    std::vector<std::pair<KnotHolderEntity*, const char*> >::iterator i;
    for (i = kh_entity_vector.begin(); i != kh_entity_vector.end(); ++i) {
        KnotHolderEntity *entity = i->first;
        const char *descr = i->second;

        entity->create(desktop, item, knotholder, descr);
        knotholder->add(entity);
    }
}

void
Effect::addPointParamHandles(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    using namespace std;
    for (std::vector<Parameter *>::iterator p = param_vector.begin(); p != param_vector.end(); ++p) {
        if ((*p)->paramType() == Inkscape::LivePathEffect::POINT_PARAM) {
            KnotHolderEntity *e = dynamic_cast<KnotHolderEntity *>(*p);
            e->create(desktop, item, knotholder);
            knotholder->add(e);
        }
    }
}

void
Effect::addHelperPaths(SPLPEItem *lpeitem, SPDesktop *desktop)
{
    g_return_if_fail(SP_IS_PATH(lpeitem));

    if (providesKnotholder() && showOrigPath()) {
        // TODO: we assume that if the LPE provides its own knotholder, there is no nodepath so we
        // must create the helper curve for the original path manually; when we allow nodepaths and
        // knotholders alongside each other, this needs to be rethought!
        SPCanvasItem *canvasitem = sp_nodepath_generate_helperpath(desktop, SP_PATH(lpeitem));
        // TODO: Make sure the tempitem doesn't get destroyed when the mouse leaves the item
        Inkscape::Display::TemporaryItem* tmpitem = desktop->add_temporary_canvasitem (canvasitem, 0);
        lpeitem->lpe_helperpaths.push_back(tmpitem);
    }

    addHelperPathsImpl(lpeitem, desktop);
}

void
Effect::addHelperPathsImpl(SPLPEItem *lpeitem, SPDesktop *desktop)
{
    // if this method is overloaded in derived classes, provides_own_flash_paths will be true
    provides_own_flash_paths = false;
}

/**
 * This *creates* a new widget, management of deletion should be done by the caller
 */
Gtk::Widget *
Effect::newWidget(Gtk::Tooltips * tooltips)
{
    // use manage here, because after deletion of Effect object, others might still be pointing to this widget.
    Gtk::VBox * vbox = Gtk::manage( new Gtk::VBox() );

    vbox->set_border_width(5);

    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        Parameter * param = *it;
        Gtk::Widget * widg = param->param_newWidget(tooltips);
        Glib::ustring * tip = param->param_getTooltip();
        if (widg) {
           vbox->pack_start(*widg, true, true, 2);
            if (tip != NULL) {
                tooltips->set_tip(*widg, *tip);
            }
        }

        it++;
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

Parameter *
Effect::getParameter(const char * key)
{
    Glib::ustring stringkey(key);

    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        Parameter * param = *it;
        if ( param->param_key == key) {
            return param;
        }

        it++;
    }

    return NULL;
}

Parameter *
Effect::getNextOncanvasEditableParam()
{
    if (param_vector.size() == 0) // no parameters
        return NULL;

    oncanvasedit_it++;
    if (oncanvasedit_it >= static_cast<int>(param_vector.size())) {
        oncanvasedit_it = 0;
    }
    int old_it = oncanvasedit_it;

    do {
        Parameter * param = param_vector[oncanvasedit_it];
        if(param && param->oncanvas_editable) {
            return param;
        } else {
            oncanvasedit_it++;
            if (oncanvasedit_it == static_cast<int>(param_vector.size())) {  // loop round the map
                oncanvasedit_it = 0;
            }
        }
    } while (oncanvasedit_it != old_it); // iterate until complete loop through map has been made

    return NULL;
}

void
Effect::editNextParamOncanvas(SPItem * item, SPDesktop * desktop)
{
    if (!desktop) return;

    Parameter * param = getNextOncanvasEditableParam();
    if (param) {
        param->param_editOncanvas(item, desktop);
        gchar *message = g_strdup_printf(_("Editing parameter <b>%s</b>."), param->param_label.c_str());
        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, message);
        g_free(message);
    } else {
        desktop->messageStack()->flash( Inkscape::WARNING_MESSAGE,
                                        _("None of the applied path effect's parameters can be edited on-canvas.") );
    }
}

/* This function should reset the defaults and is used for example to initialize an effect right after it has been applied to a path
* The nice thing about this is that this function can use knowledge of the original path and set things accordingly for example to the size or origin of the original path!
*/
void
Effect::resetDefaults(SPItem * /*item*/)
{
    // do nothing for simple effects
}

void
Effect::setup_nodepath(Inkscape::NodePath::Path *np)
{
    np->helperpath_rgba = 0xff0000ff;
    np->helperpath_width = 1.0;
}

void
Effect::transform_multiply(Geom::Matrix const& postmul, bool set)
{
    // cycle through all parameters. Most parameters will not need transformation, but path and point params do.
    for (std::vector<Parameter *>::iterator it = param_vector.begin(); it != param_vector.end(); it++) {
        Parameter * param = *it;
        param->param_transform_multiply(postmul, set);
    }
}

bool
Effect::providesKnotholder()
{
    // does the effect actively provide any knotholder entities of its own?
    if (kh_entity_vector.size() > 0)
        return true;

    // otherwise: are there any PointParams?
    for (std::vector<Parameter *>::iterator p = param_vector.begin(); p != param_vector.end(); ++p) {
        if ((*p)->paramType() == Inkscape::LivePathEffect::POINT_PARAM) {
            return true;
        }
    }

    return false;
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
