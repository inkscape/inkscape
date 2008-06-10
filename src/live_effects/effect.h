#ifndef INKSCAPE_LIVEPATHEFFECT_H
#define INKSCAPE_LIVEPATHEFFECT_H

/*
 * Inkscape::LivePathEffect
 *
* Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "display/display-forward.h"
#include <map>
#include <glibmm/ustring.h>
#include <2geom/path.h>
#include "ui/widget/registry.h"
#include "util/enums.h"
#include "sp-lpe-item.h"
#include "knotholder.h"
#include "parameter/bool.h"

#define  LPE_CONVERSION_TOLERANCE 0.01    // FIXME: find good solution for this.

#define LPE_ENABLE_TEST_EFFECTS

struct SPDocument;
struct SPDesktop;
struct SPItem;
class NArtBpath;
struct LivePathEffectObject;

namespace Gtk {
    class Widget;
    class VBox;
    class Tooltips;
}

namespace Geom {
    class Matrix;
}

namespace Inkscape {

namespace XML {
    class Node;
}

namespace NodePath {
    class Path ;
}

namespace LivePathEffect {

enum EffectType {
    BEND_PATH = 0,
    PATTERN_ALONG_PATH,
    SKETCH,
    VONKOCH,
    KNOT,
#ifdef LPE_ENABLE_TEST_EFFECTS
    DOEFFECTSTACK_TEST,
#endif
    GEARS,
    CURVE_STITCH,
    CIRCLE_WITH_RADIUS,
    PERSPECTIVE_PATH,
    SPIRO,
    LATTICE,
    ENVELOPE,
    CONSTRUCT_GRID,
    PERP_BISECTOR,
    TANGENT_TO_CURVE,
    MIRROR_REFLECT,
    INVALID_LPE // This must be last
};

extern const Util::EnumData<EffectType> LPETypeData[INVALID_LPE];
extern const Util::EnumDataConverter<EffectType> LPETypeConverter;

enum LPEPathFlashType {
    SUPPRESS_FLASH,
    PERMANENT_FLASH,
    DEFAULT
};

class Effect {
public:
    static Effect* New(EffectType lpenr, LivePathEffectObject *lpeobj);
    static void createAndApply(const char* name, SPDocument *doc, SPItem *item);
    static void createAndApply(EffectType type, SPDocument *doc, SPItem *item);

    virtual ~Effect();

    EffectType effectType ();

    virtual void doOnApply (SPLPEItem *lpeitem);

    virtual void doBeforeEffect (SPLPEItem *lpeitem);
    void writeParamsToSVG();

    virtual void doEffect (SPCurve * curve);

    virtual Gtk::Widget * newWidget(Gtk::Tooltips * tooltips);

    virtual void resetDefaults(SPItem * item);

    virtual void setup_nodepath(Inkscape::NodePath::Path *np);

    virtual void transform_multiply(Geom::Matrix const& postmul, bool set);

    bool providesKnotholder() { return (kh_entity_vector.size() > 0); }
    virtual LPEPathFlashType pathFlashType() { return DEFAULT; }
    void addHandles(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

    Glib::ustring          getName();
    Inkscape::XML::Node *  getRepr();
    SPDocument *           getSPDoc();
    LivePathEffectObject * getLPEObj() {return lpeobj;};
    Parameter *            getParameter(const char * key);

    void readallParameters(Inkscape::XML::Node * repr);
    void setParameter(const gchar * key, const gchar * new_value);

    inline bool isVisible() { return is_visible; }

    void editNextParamOncanvas(SPItem * item, SPDesktop * desktop);

protected:
    Effect(LivePathEffectObject *lpeobject);

    // provide a set of doEffect functions so the developer has a choice
    // of what kind of input/output parameters he desires.
    // the order in which they appear is the order in which they are
    // called by this base class. (i.e. doEffect(SPCurve * curve) defaults to calling
    // doEffect(std::vector<Geom::Path> )
    virtual std::vector<Geom::Path>
            doEffect_path (std::vector<Geom::Path> const & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
            doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    void registerParameter(Parameter * param);
    void registerKnotHolderHandle(KnotHolderEntity* entity, const char* descr);
    void addPointParamHandles(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);
    Parameter * getNextOncanvasEditableParam();

    std::vector<Parameter *> param_vector;
    std::vector<std::pair<KnotHolderEntity*, const char*> > kh_entity_vector;
    int oncanvasedit_it;
    BoolParam is_visible;

    Inkscape::UI::Widget::Registry wr;

    LivePathEffectObject *lpeobj;

    // this boolean defaults to false, it concatenates the input path to one pwd2,
    // instead of normally 'splitting' the path into continuous pwd2 paths.
    bool concatenate_before_pwd2;

private:
    Effect(const Effect&);
    Effect& operator=(const Effect&);
};


} //namespace LivePathEffect
} //namespace Inkscape

#endif

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
