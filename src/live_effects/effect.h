#ifndef INKSCAPE_LIVEPATHEFFECT_H
#define INKSCAPE_LIVEPATHEFFECT_H

/*
 * Inkscape::LivePathEffect
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "display/display-forward.h"
#include <map>
#include <glibmm/ustring.h>
#include <2geom/path.h>
#include "ui/widget/registry.h"
#include "util/enums.h"

#define  LPE_CONVERSION_TOLERANCE 0.01    // FIXME: find good solution for this.

//#define LPE_ENABLE_TEST_EFFECTS

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

namespace Inkscape {

namespace XML {
    class Node;
}

namespace NodePath {
    class Path ;
}

namespace LivePathEffect {

enum EffectType {
    PATH_ALONG_PATH = 0,
    SKELETAL_STROKES,
#ifdef LPE_ENABLE_TEST_EFFECTS
    SLANT,
    DOEFFECTSTACK_TEST,
#endif
    GEARS,
    CURVE_STITCH,
    INVALID_LPE // This must be last
};

extern const Util::EnumData<EffectType> LPETypeData[INVALID_LPE];
extern const Util::EnumDataConverter<EffectType> LPETypeConverter;

class Parameter;

class Effect {
public:
    static Effect* New(EffectType lpenr, LivePathEffectObject *lpeobj);

    virtual ~Effect();

    virtual void doEffect (SPCurve * curve);

    virtual Gtk::Widget * getWidget();

    virtual void resetDefaults(SPItem * item);

    Glib::ustring          getName();
    Inkscape::XML::Node *  getRepr();
    SPDocument *           getSPDoc();
    LivePathEffectObject * getLPEObj() {return lpeobj;};
    Parameter *            getParameter(const char * key);

    void readallParameters(Inkscape::XML::Node * repr);
    void setParameter(const gchar * key, const gchar * new_value);

    void editNextParamOncanvas(SPItem * item, SPDesktop * desktop);

    virtual void setup_notepath(Inkscape::NodePath::Path *np);

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

    typedef std::map<Glib::ustring, Parameter *> param_map_type;
    param_map_type param_map;

    Inkscape::UI::Widget::Registry wr; 
    Gtk::VBox * vbox;
    Gtk::Tooltips * tooltips;

    LivePathEffectObject *lpeobj;

    param_map_type::iterator oncanvasedit_it;

private:
    Effect(const Effect&);
    Effect& operator=(const Effect&);
};


} //namespace LivePathEffect
} //namespace Inkscape

#endif
