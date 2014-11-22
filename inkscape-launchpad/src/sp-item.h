#ifndef SEEN_SP_ITEM_H
#define SEEN_SP_ITEM_H

/**
 * @file
 * Some things pertinent to all visible shapes: SPItem, SPItemView, SPItemCtx, SPItemClass, SPEvent.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2006 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/forward.h>
#include <2geom/affine.h>
#include <2geom/rect.h>
#include <vector>

#include "sp-object.h"
#include "snap-preferences.h"
#include "snap-candidate.h"

//class SPGuideConstraint;
#include "sp-guide-constraint.h"

class SPClipPathReference;
class SPMaskReference;
class SPAvoidRef;
class SPPattern;
struct SPPrintContext;
typedef unsigned int guint32;

namespace Inkscape {

class Drawing;
class DrawingItem;
class URIReference;

}

enum {
    SP_EVENT_INVALID,
    SP_EVENT_NONE,
    SP_EVENT_ACTIVATE,
    SP_EVENT_MOUSEOVER,
    SP_EVENT_MOUSEOUT
};

// TODO make a completely new function that transforms either the fill or
// stroke of any SPItem  without adding an extra parameter to adjust_pattern.
enum PatternTransform {
    TRANSFORM_BOTH,
    TRANSFORM_FILL,
    TRANSFORM_STROKE
};

/**
 * Event structure.
 *
 * @todo This is just placeholder. Plan:
 * We do extensible event structure, that hold applicable (ui, non-ui)
 * data pointers. So it is up to given object/arena implementation
 * to process correct ones in meaningful way.
 * Also, this probably goes to SPObject base class.
 *
 */
class SPEvent {
public:
    unsigned int type;
    void* data;
};

class SPItemView {
public:
    SPItemView *next;
    unsigned int flags;
    unsigned int key;
    Inkscape::DrawingItem *arenaitem;
};

/* flags */

#define SP_ITEM_BBOX_VISUAL 1

#define SP_ITEM_SHOW_DISPLAY (1 << 0)

/**
 * Flag for referenced views (i.e. markers, clippaths, masks and patterns);
 * currently unused, does the same as DISPLAY
 */
#define SP_ITEM_REFERENCE_FLAGS (1 << 1)

/**
 * Contains transformations to document/viewport and the viewport size.
 */
class SPItemCtx : public SPCtx {
public:
    /** Item to document transformation */
    Geom::Affine i2doc;

    /** Viewport size */
    Geom::Rect viewport;

    /** Item to viewport transformation */
    Geom::Affine i2vp;
};

#define SP_ITEM(obj) (dynamic_cast<SPItem*>((SPObject*)obj))
#define SP_IS_ITEM(obj) (dynamic_cast<const SPItem*>((SPObject*)obj) != NULL)

/**
 * Base class for visual SVG elements.
 * SPItem is an abstract base class for all graphic (visible) SVG nodes. It
 * is a subclass of SPObject, with great deal of specific functionality.
 */
class SPItem : public SPObject {
public:
    enum BBoxType {
        // legacy behavior: includes crude stroke, markers; excludes long miters, blur margin; is known to be wrong for caps
        APPROXIMATE_BBOX,
        // includes only the bare path bbox, no stroke, no nothing
        GEOMETRIC_BBOX,
        // includes everything: correctly done stroke (with proper miters and caps), markers, filter margins (e.g. blur)
        VISUAL_BBOX
    };

    SPItem();
    virtual ~SPItem();

    unsigned int sensitive : 1;
    unsigned int stop_paint: 1;
    mutable unsigned bbox_valid : 1;
    double transform_center_x;
    double transform_center_y;
    bool freeze_stroke_width;

    Geom::Affine transform;
    mutable Geom::OptRect doc_bbox;

    SPClipPathReference *clip_ref;
    SPMaskReference *mask_ref;

    // Used for object-avoiding connectors
    SPAvoidRef *avoidRef;

    SPItemView *display;

    std::vector<SPGuideConstraint> constraints;

    sigc::signal<void, Geom::Affine const *, SPItem *> _transformed_signal;

    bool isLocked() const;
    void setLocked(bool lock);

    bool isHidden() const;
    void setHidden(bool hidden);

    // Objects dialogue
    bool isSensitive() const {
        return sensitive;
    };

    bool isHighlightSet() const;
    guint32 highlight_color() const;
    
    void setHighlightColor(guint32 color);
    
    void unsetHighlightColor();
    //====================

    bool isEvaluated() const;
    void setEvaluated(bool visible);
    void resetEvaluated();

    bool isHidden(unsigned display_key) const;

    /**
     * Returns something suitable for the `Hide' checkbox in the Object Properties dialog box.
     *  Corresponds to setExplicitlyHidden.
     */
    bool isExplicitlyHidden() const;

    /**
     * Sets the display CSS property to `hidden' if \a val is true,
     * otherwise makes it unset.
     */
    void setExplicitlyHidden(bool val);

    /**
     * Sets the transform_center_x and transform_center_y properties to retain the rotation center
     */
    void setCenter(Geom::Point const &object_centre);

    void unsetCenter();
    bool isCenterSet() const;
    Geom::Point getCenter() const;
    void scaleCenter(Geom::Scale const &sc);

    bool isVisibleAndUnlocked() const;

    bool isVisibleAndUnlocked(unsigned display_key) const;

    Geom::Affine getRelativeTransform(SPObject const *obj) const;

    void raiseOne();
    void lowerOne();
    void raiseToTop();
    void lowerToBottom();

    /**
     * Move this SPItem into or after another SPItem in the doc.
     *
     * @param target the SPItem to move into or after.
     * @param intoafter move to after the target (false), move inside (sublayer) of the target (true).
     */
    void moveTo(SPItem *target, bool intoafter);

    sigc::connection connectTransformed(sigc::slot<void, Geom::Affine const *, SPItem *> slot)  {
        return _transformed_signal.connect(slot);
    }

    /**
     * Get item's geometric bounding box in this item's coordinate system.
     *
     * The geometric bounding box includes only the path, disregarding all style attributes.
     */
    Geom::OptRect geometricBounds(Geom::Affine const &transform = Geom::identity()) const;

    /**
     * Get item's visual bounding box in this item's coordinate system.
     *
     * The visual bounding box includes the stroke and the filter region.
     */
    Geom::OptRect visualBounds(Geom::Affine const &transform = Geom::identity()) const;

    Geom::OptRect bounds(BBoxType type, Geom::Affine const &transform = Geom::identity()) const;

    /**
     * Get item's geometric bbox in document coordinate system.
     * Document coordinates are the default coordinates of the root element:
     * the origin is at the top left, X grows to the right and Y grows downwards.
     */
    Geom::OptRect documentGeometricBounds() const;

    /**
     * Get item's visual bbox in document coordinate system.
     */
    Geom::OptRect documentVisualBounds() const;

    Geom::OptRect documentBounds(BBoxType type) const;

    /**
     * Get item's geometric bbox in desktop coordinate system.
     * Desktop coordinates should be user defined. Currently they are hardcoded:
     * origin is at bottom left, X grows to the right and Y grows upwards.
     */
    Geom::OptRect desktopGeometricBounds() const;

    /**
     * Get item's visual bbox in desktop coordinate system.
     */
    Geom::OptRect desktopVisualBounds() const;

    Geom::OptRect desktopPreferredBounds() const;
    Geom::OptRect desktopBounds(BBoxType type) const;

    unsigned int pos_in_parent() const;

    /**
     * Returns a string suitable for status bar, formatted in pango markup language.
     *
     * Must be freed by caller.
     */
    char *detailedDescription() const;

    /**
     * Returns true if the item is filtered, false otherwise.
     * Used with groups/lists to determine how many, or if any, are filtered.
     */
    bool isFiltered() const;

    void invoke_print(SPPrintContext *ctx);

    /**
     * Allocates unique integer keys.
     * 
     * @param numkeys Number of keys required.
     * @return First allocated key; hence if the returned key is n
     * you can use n, n + 1, ..., n + (numkeys - 1)
     */
    static unsigned int display_key_new(unsigned int numkeys);

    Inkscape::DrawingItem *invoke_show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
    void invoke_hide(unsigned int key);
    void getSnappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs=0) const;
    void adjust_pattern(/* Geom::Affine const &premul, */ Geom::Affine const &postmul, bool set = false, PatternTransform = TRANSFORM_BOTH);
    void adjust_gradient(/* Geom::Affine const &premul, */ Geom::Affine const &postmul, bool set = false);
    void adjust_stroke(double ex);

    /**
     * Recursively scale stroke width in \a item and its children by \a expansion.
     */
    void adjust_stroke_width_recursive(double ex);

    void freeze_stroke_width_recursive(bool freeze);

    /**
     * Recursively compensate pattern or gradient transform.
     */
    void adjust_paint_recursive(Geom::Affine advertized_transform, Geom::Affine t_ancestors, bool is_pattern);

    void adjust_livepatheffect(Geom::Affine const &postmul, bool set = false);

    /**
     * Set a new transform on an object.
     *
     * Compensate for stroke scaling and gradient/pattern fill transform, if
     * necessary. Call the object's set_transform method if transforms are
     * stored optimized. Send _transformed_signal. Invoke _write method so that
     * the repr is updated with the new transform.
     */
    void doWriteTransform(Inkscape::XML::Node *repr, Geom::Affine const &transform, Geom::Affine const *adv = NULL, bool compensate = true);

    /**
     * Sets item private transform (not propagated to repr), without compensating stroke widths,
     * gradients, patterns as sp_item_write_transform does.
     */
    void set_item_transform(Geom::Affine const &transform_matrix);

    int emitEvent (SPEvent &event);

    /**
     * Return the arenaitem corresponding to the given item in the display
     * with the given key
     */
    Inkscape::DrawingItem *get_arenaitem(unsigned int key);

    /**
     * Returns the accumulated transformation of the item and all its ancestors, including root's viewport.
     * @pre (item != NULL) and SP_IS_ITEM(item).
     */
    Geom::Affine i2doc_affine() const;

    /**
     * Returns the transformation from item to desktop coords
     */
    Geom::Affine i2dt_affine() const;

    void set_i2d_affine(Geom::Affine const &transform);

    /**
     * should rather be named "sp_item_d2i_affine" to match "sp_item_i2d_affine" (or vice versa).
     */
    Geom::Affine dt2i_affine() const;

    char *_highlightColor;

private:
    enum EvaluatedStatus
    {
        StatusUnknown, StatusCalculated, StatusSet
    };

    mutable bool _is_evaluated;
    mutable EvaluatedStatus _evaluated_status;

    static SPItemView *sp_item_view_new_prepend(SPItemView *list, SPItem *item, unsigned flags, unsigned key, Inkscape::DrawingItem *arenaitem);
    static void clip_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);
    static void mask_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);
    static void fill_ps_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);
    static void stroke_ps_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);

public:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void set(unsigned int key, char const* value);
	virtual void update(SPCtx *ctx, unsigned int flags);
        virtual void modified(unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type) const;
	virtual void print(SPPrintContext *ctx);
    virtual const char* displayName() const;
	virtual char* description() const;
	virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
	virtual void hide(unsigned int key);
    virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;
    virtual Geom::Affine set_transform(Geom::Affine const &transform);

    virtual void convert_to_guides() const;

    virtual int event(SPEvent *event);
};


// Utility

/**
 * @pre \a ancestor really is an ancestor (\>=) of \a object, or NULL.
 *   ("Ancestor (\>=)" here includes as far as \a object itself.)
 */
Geom::Affine i2anc_affine(SPObject const *item, SPObject const *ancestor);

Geom::Affine i2i_affine(SPObject const *src, SPObject const *dest);

/* fixme: - these are evil, but OK */

int sp_item_repr_compare_position(SPItem const *first, SPItem const *second);
SPItem *sp_item_first_item_child (SPObject *obj);
SPItem const *sp_item_first_item_child (SPObject const *obj);

#endif // SEEN_SP_ITEM_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
