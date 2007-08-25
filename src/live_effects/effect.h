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

struct SPShape;
struct SPDocument;
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

namespace LivePathEffect {

enum EffectType {
    SKELETAL_STROKES = 0,
#ifdef LPE_ENABLE_TEST_EFFECTS
    SLANT,
    DOEFFECTSTACK_TEST,
#endif
    GEARS,
    INVALID_LPE // This must be last
};

extern const Util::EnumData<EffectType> LPETypeData[INVALID_LPE];
extern const Util::EnumDataConverter<EffectType> LPETypeConverter;

class Parameter;

class Effect {
public:
    virtual ~Effect();

    Glib::ustring getName();

    virtual void doEffect (SPCurve * curve);

    static Effect* New(EffectType lpenr, LivePathEffectObject *lpeobj);

    virtual Gtk::Widget * getWidget();

    Inkscape::XML::Node * getRepr();
    SPDocument * getSPDoc();

    void readallParameters(Inkscape::XML::Node * repr);
    void setParameter(Inkscape::XML::Node * repr, const gchar * key, const gchar * old_value, const gchar * new_value);

protected:
    Effect(LivePathEffectObject *lpeobject);

    // provide a set of doEffect functions so the developer has a choice 
    // of what kind of input/output parameters he desires.
    // the order in which they appear is the order in which they are 
    // called by this base class. (i.e. doEffect(SPCurve * curve) defaults to calling
    // doEffect(std::vector<Geom::Path> )
    virtual NArtBpath *
            doEffect (NArtBpath * path_in);
    virtual std::vector<Geom::Path> 
            doEffect (std::vector<Geom::Path> & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > 
            doEffect (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

    void registerParameter(Parameter * param);

    typedef std::map<Glib::ustring, Parameter *> param_map_type;
    param_map_type param_map;

    Inkscape::UI::Widget::Registry wr; 
    Gtk::VBox * vbox;
    Gtk::Tooltips * tooltips;

    LivePathEffectObject *lpeobj;

private:
    Effect(const Effect&);
    Effect& operator=(const Effect&);
};


} //namespace LivePathEffect
} //namespace Inkscape

#endif
