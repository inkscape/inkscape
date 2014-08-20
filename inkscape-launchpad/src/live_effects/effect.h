#ifndef INKSCAPE_LIVEPATHEFFECT_H
#define INKSCAPE_LIVEPATHEFFECT_H

/*
 * Copyright (C) Johan Engelen 2007-2012 <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include <2geom/forward.h>
#include "ui/widget/registry.h"
#include "parameter/bool.h"
#include "effect-enum.h"

#define  LPE_CONVERSION_TOLERANCE 0.01    // FIXME: find good solution for this.

class  SPDocument;
class  SPDesktop;
class  SPItem;
class LivePathEffectObject;
class  SPLPEItem;
class  KnotHolder;
class  KnotHolderEntity;
class  SPPath;
class  SPCurve;

namespace Gtk {
    class Widget;
}

namespace Inkscape {

namespace XML {
    class Node;
}

namespace LivePathEffect {

enum LPEPathFlashType {
    SUPPRESS_FLASH,
//    PERMANENT_FLASH,
    DEFAULT
};

class Effect {
public:
    static Effect* New(EffectType lpenr, LivePathEffectObject *lpeobj);
    static void createAndApply(const char* name, SPDocument *doc, SPItem *item);
    static void createAndApply(EffectType type, SPDocument *doc, SPItem *item);

    virtual ~Effect();

    EffectType effectType() const;

    virtual void doOnApply (SPLPEItem const* lpeitem);
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    void writeParamsToSVG();

    virtual void acceptParamPath (SPPath const* param_path);
    static int acceptsNumClicks(EffectType type);
    int acceptsNumClicks() const { return acceptsNumClicks(effectType()); }
    void doAcceptPathPreparations(SPLPEItem *lpeitem);

    /*
     * isReady() indicates whether all preparations which are necessary to apply the LPE are done,
     * e.g., waiting for a parameter path either before the effect is created or when it needs a
     * path as argument. This is set in SPLPEItem::addPathEffect().
     */
    inline bool isReady() const { return is_ready; }
    inline void setReady(bool ready = true) { is_ready = ready; }

    virtual void doEffect (SPCurve * curve);

    virtual Gtk::Widget * newWidget();

    /**
     * Sets all parameters to their default values and writes them to SVG.
     */
    virtual void resetDefaults(SPItem const* item);

    /// /todo: is this method really necessary? it causes UI inconsistensies... (johan)
    virtual void transform_multiply(Geom::Affine const& postmul, bool set);

    // /TODO: providesKnotholder() is currently used as an indicator of whether a nodepath is
    // created for an item or not. When we allow both at the same time, this needs rethinking!
    bool providesKnotholder() const;
    // /TODO: in view of providesOwnFlashPaths() below, this is somewhat redundant
    //       (but spiro lpe still needs it!)
    virtual LPEPathFlashType pathFlashType() const { return DEFAULT; }
    void addHandles(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);
    std::vector<Geom::PathVector> getCanvasIndicators(SPLPEItem const* lpeitem);

    inline bool providesOwnFlashPaths() const {
        return provides_own_flash_paths || show_orig_path;
    }
    inline bool showOrigPath() const { return show_orig_path; }

    Glib::ustring          getName() const;
    Inkscape::XML::Node *  getRepr();
    SPDocument *           getSPDoc();
    LivePathEffectObject * getLPEObj() {return lpeobj;};
    Parameter *            getParameter(const char * key);

    void readallParameters(Inkscape::XML::Node const* repr);
    void setParameter(const gchar * key, const gchar * new_value);

    inline bool isVisible() const { return is_visible; }

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
    Parameter * getNextOncanvasEditableParam();

    virtual void addKnotHolderEntities(KnotHolder * /*knotholder*/, SPDesktop * /*desktop*/, SPItem * /*item*/) {};

    virtual void addCanvasIndicators(SPLPEItem const* lpeitem, std::vector<Geom::PathVector> &hp_vec);

    std::vector<Parameter *> param_vector;
    bool _provides_knotholder_entities;
    int oncanvasedit_it;
    BoolParam is_visible;

    bool show_orig_path; // set this to true in derived effects to automatically have the original
                         // path displayed as helperpath

    Inkscape::UI::Widget::Registry wr;

    LivePathEffectObject *lpeobj;

    // this boolean defaults to false, it concatenates the input path to one pwd2,
    // instead of normally 'splitting' the path into continuous pwd2 paths and calling doEffect_pwd2 for each.
    bool concatenate_before_pwd2;

private:
    bool provides_own_flash_paths; // if true, the standard flash path is suppressed

    bool is_ready;

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
