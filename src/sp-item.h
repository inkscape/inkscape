#ifndef SEEN_SP_ITEM_H
#define SEEN_SP_ITEM_H

/** \file
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
#include <vector>

#include "display/nr-arena-forward.h"
#include "sp-object.h"
#include <2geom/matrix.h>
#include <libnr/nr-rect.h>
#include <2geom/forward.h>
#include <libnr/nr-convert2geom.h>
#include <snap-preferences.h>
#include "snap-candidate.h"

class SPGuideConstraint;
struct SPClipPathReference;
struct SPMaskReference;
struct SPAvoidRef;
struct SPPrintContext;
namespace Inkscape { class URIReference;}

enum {
    SP_EVENT_INVALID,
    SP_EVENT_NONE,
    SP_EVENT_ACTIVATE,
    SP_EVENT_MOUSEOVER,
    SP_EVENT_MOUSEOUT
};

/**
 * Event structure.
 *
 * \todo This is just placeholder. Plan:
 * We do extensible event structure, that hold applicable (ui, non-ui)
 * data pointers. So it is up to given object/arena implementation
 * to process correct ones in meaningful way.
 * Also, this probably goes to SPObject base class.
 *
 */
class SPEvent {
public:
    unsigned int type;
    gpointer data;
};

/// SPItemView
class SPItemView {
public:
    SPItemView *next;
    unsigned int flags;
    unsigned int key;
    NRArenaItem *arenaitem;
};

/* flags */

#define SP_ITEM_BBOX_VISUAL 1

#define SP_ITEM_SHOW_DISPLAY (1 << 0)

/**
 * Flag for referenced views (i.e. markers, clippaths, masks and patterns);
   currently unused, does the same as DISPLAY
 */
#define SP_ITEM_REFERENCE_FLAGS (1 << 1)

/// Contains transformations to document/viewport and the viewport size.
class SPItemCtx {
public:
    SPCtx ctx;
    /** Item to document transformation */
    Geom::Matrix i2doc;
    /** Viewport size */
    NRRect vp;
    /** Item to viewport transformation */
    Geom::Matrix i2vp;
};

class SPItem;
class SPItemClass;

#define SP_TYPE_ITEM (SPItem::getType ())
#define SP_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_ITEM, SPItem))
#define SP_ITEM_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST((clazz), SP_TYPE_ITEM, SPItemClass))
#define SP_IS_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_ITEM))


/** Abstract base class for all visible shapes. */
class SPItem : public SPObject {
public:
    static GType getType();
    enum BBoxType {
        // legacy behavior: includes crude stroke, markers; excludes long miters, blur margin; is known to be wrong for caps
        APPROXIMATE_BBOX,
        // includes only the bare path bbox, no stroke, no nothing
        GEOMETRIC_BBOX,
        // includes everything: correctly done stroke (with proper miters and caps), markers, filter margins (e.g. blur)
        RENDERING_BBOX
    };

    unsigned int sensitive : 1;
    unsigned int stop_paint: 1;
    double transform_center_x;
    double transform_center_y;

    Geom::Matrix transform;

    SPClipPathReference *clip_ref;
    SPMaskReference *mask_ref;

    // Used for object-avoiding connectors
    SPAvoidRef *avoidRef;

    SPItemView *display;

    std::vector<SPGuideConstraint> constraints;

    sigc::signal<void, Geom::Matrix const *, SPItem *> _transformed_signal;

    void init();
    bool isLocked() const;
    void setLocked(bool lock);

    bool isHidden() const;
    void setHidden(bool hidden);

    bool isEvaluated() const;
    void setEvaluated(bool visible);
    void resetEvaluated();

    bool isHidden(unsigned display_key) const;

    bool isExplicitlyHidden() const;

    void setExplicitlyHidden(bool val);

    void setCenter(Geom::Point object_centre);
    void unsetCenter();
    bool isCenterSet();
    Geom::Point getCenter() const;

    bool isVisibleAndUnlocked() const;

    bool isVisibleAndUnlocked(unsigned display_key) const;

    Geom::Matrix getRelativeTransform(SPObject const *obj) const;

    void raiseOne();
    void lowerOne();
    void raiseToTop();
    void lowerToBottom();

    Geom::OptRect getBounds(Geom::Matrix const &transform, BBoxType type=APPROXIMATE_BBOX, unsigned int dkey=0) const;

    sigc::connection _clip_ref_connection;
    sigc::connection _mask_ref_connection;

    sigc::connection connectTransformed(sigc::slot<void, Geom::Matrix const *, SPItem *> slot)  {
        return _transformed_signal.connect(slot);
    }
    void invoke_bbox( Geom::OptRect &bbox, Geom::Matrix const &transform, unsigned const clear, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX);
    void invoke_bbox( NRRect *bbox, Geom::Matrix const &transform, unsigned const clear, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) __attribute__ ((deprecated));
    void invoke_bbox_full( Geom::OptRect &bbox, Geom::Matrix const &transform, unsigned const flags, unsigned const clear) const;
    void invoke_bbox_full( NRRect *bbox, Geom::Matrix const &transform, unsigned const flags, unsigned const clear) __attribute__ ((deprecated));

    unsigned pos_in_parent();
    gchar *description();
    void invoke_print(SPPrintContext *ctx);
    static unsigned int display_key_new(unsigned int numkeys);
    NRArenaItem *invoke_show(NRArena *arena, unsigned int key, unsigned int flags);
    void invoke_hide(unsigned int key);
    void getSnappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs=0) const;
    void adjust_pattern(/* Geom::Matrix const &premul, */ Geom::Matrix const &postmul, bool set = false);
    void adjust_gradient(/* Geom::Matrix const &premul, */ Geom::Matrix const &postmul, bool set = false);
    void adjust_stroke(gdouble ex);
    void adjust_stroke_width_recursive(gdouble ex);
    void adjust_paint_recursive(Geom::Matrix advertized_transform, Geom::Matrix t_ancestors, bool is_pattern);
    void adjust_livepatheffect(Geom::Matrix const &postmul, bool set = false);
    void doWriteTransform(Inkscape::XML::Node *repr, Geom::Matrix const &transform, Geom::Matrix const *adv = NULL, bool compensate = true);
    void set_item_transform(Geom::Matrix const &transform_matrix);
    void convert_item_to_guides();
    gint emitEvent (SPEvent &event);
    NRArenaItem *get_arenaitem(unsigned int key);
    void getBboxDesktop(NRRect *bbox, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) __attribute__ ((deprecated));
    Geom::OptRect getBboxDesktop(SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX);
    Geom::Matrix i2doc_affine() const;
    Geom::Matrix i2d_affine() const;
    void set_i2d_affine(Geom::Matrix const &transform);
    Geom::Matrix dt2i_affine() const;
    void convert_to_guides();

private:
    enum EvaluatedStatus
    {
        StatusUnknown, StatusCalculated, StatusSet
    };

    mutable bool _is_evaluated;
    mutable EvaluatedStatus _evaluated_status;

    static void sp_item_init(SPItem *item);

    static void sp_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
    static void sp_item_release(SPObject *object);
    static void sp_item_set(SPObject *object, unsigned key, gchar const *value);
    static void sp_item_update(SPObject *object, SPCtx *ctx, guint flags);
    static Inkscape::XML::Node *sp_item_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

    static gchar *sp_item_private_description(SPItem *item);
    static void sp_item_private_snappoints(SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs);

    static SPItemView *sp_item_view_new_prepend(SPItemView *list, SPItem *item, unsigned flags, unsigned key, NRArenaItem *arenaitem);
    static SPItemView *sp_item_view_list_remove(SPItemView *list, SPItemView *view);
    static void clip_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);
    static void mask_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);

    friend class SPItemClass;
};

/// The SPItem vtable.
class SPItemClass {
public:
    SPObjectClass parent_class;

    /** BBox union in given coordinate system */
    void (* bbox) (SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);

    /** Printing method. Assumes ctm is set to item affine matrix */
    /* \todo Think about it, and maybe implement generic export method instead (Lauris) */
    void (* print) (SPItem *item, SPPrintContext *ctx);

    /** Give short description of item (for status display) */
    gchar * (* description) (SPItem * item);

    NRArenaItem * (* show) (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
    void (* hide) (SPItem *item, unsigned int key);

    /** Write to an iterator the points that should be considered for snapping
     * as the item's `nodes'.
     */
    void (* snappoints) (SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs);

    /** Apply the transform optimally, and return any residual transformation */
    Geom::Matrix (* set_transform)(SPItem *item, Geom::Matrix const &transform);

    /** Convert the item to guidelines */
    void (* convert_to_guides)(SPItem *item);

    /** Emit event, if applicable */
    gint (* event) (SPItem *item, SPEvent *event);

	private:
	static SPObjectClass *static_parent_class;
	static void sp_item_class_init(SPItemClass *klass);

	friend class SPItem;
};

// Utility

Geom::Matrix i2anc_affine(SPObject const *item, SPObject const *ancestor);
Geom::Matrix i2i_affine(SPObject const *src, SPObject const *dest);

/* fixme: - these are evil, but OK */

/* Fill *TRANSFORM with the item-to-desktop transform.  See doc/coordinates.txt
 * for a description of `Desktop coordinates'; though see also mental's comment
 * at the top of that file.
 *
 * \return TRANSFORM.
 */
int sp_item_repr_compare_position(SPItem *first, SPItem *second);
SPItem *sp_item_first_item_child (SPObject *obj);

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
