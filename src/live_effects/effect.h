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
    SKELETAL_STROKES,
    SKETCH,
    VONKOCH,
    KNOT,
#ifdef LPE_ENABLE_TEST_EFFECTS
    SLANT,
    DOEFFECTSTACK_TEST,
#endif
    GEARS,
    CURVE_STITCH,
    CIRCLE_WITH_RADIUS,
    PERSPECTIVE_PATH,
    INVALID_LPE // This must be last
};

extern const Util::EnumData<EffectType> LPETypeData[INVALID_LPE];
extern const Util::EnumDataConverter<EffectType> LPETypeConverter;

class Parameter;

class Effect {
public:
    static Effect* New(EffectType lpenr, LivePathEffectObject *lpeobj);

    virtual ~Effect();

    virtual void doBeforeEffect (SPLPEItem *lpeitem);

    virtual void doEffect (SPCurve * curve);

    virtual Gtk::Widget * newWidget(Gtk::Tooltips * tooltips);

    virtual void resetDefaults(SPItem * item);

    virtual void setup_nodepath(Inkscape::NodePath::Path *np);

    virtual void transform_multiply(Geom::Matrix const& postmul, bool set);

    Glib::ustring          getName();
    Inkscape::XML::Node *  getRepr();
    SPDocument *           getSPDoc();
    LivePathEffectObject * getLPEObj() {return lpeobj;};
    Parameter *            getParameter(const char * key);

    void readallParameters(Inkscape::XML::Node * repr);
    void setParameter(const gchar * key, const gchar * new_value);

    void editNextParamOncanvas(SPItem * item, SPDesktop * desktop);

protected:
    Effect(LivePathEffectObject *lpeobject);

    // provide a set of doEffect functions so the developer has a choice
    // of what kind of input/output parameters he desires.
    // the order in which they appear is the order in which they are
    // called by this base class. (i.e. doEffect(SPCurve * curve) defaults to calling
    // doEffect(std::vector<Geom::Path> )
    virtual NArtBpath *
            doEffect_nartbpath (NArtBpath * path_in);
    virtual std::vector<Geom::Path>
            doEffect_path (std::vector<Geom::Path> & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
            doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

    void registerParameter(Parameter * param);
    Parameter * getNextOncanvasEditableParam();

    std::vector<Parameter *> param_vector;
    int oncanvasedit_it;

    Inkscape::UI::Widget::Registry wr;

    LivePathEffectObject *lpeobj;

private:
    Effect(const Effect&);
    Effect& operator=(const Effect&);
};


} //namespace LivePathEffect
} //namespace Inkscape

#endif
