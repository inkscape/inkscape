#ifndef __SP_ITEM_H__
#define __SP_ITEM_H__

/** \file
 * Some things pertinent to all visible shapes: SPItem, SPItemView, SPItemCtx, SPItemClass, SPEvent.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
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
#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include <2geom/forward.h>
#include <libnr/nr-convert2geom.h>

class SPGuideConstraint;
struct SPClipPathReference;
struct SPMaskReference;
struct SPAvoidRef;
struct SPPrintContext;
namespace Inkscape { class URIReference; }
 
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
struct SPEvent {
    unsigned int type;
    gpointer data;
};

class SPItemView;

/// SPItemView
struct SPItemView {
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

class SPItemCtx;

/// Contains transformations to document/viewport and the viewport size.
struct SPItemCtx {
    SPCtx ctx;
    /** Item to document transformation */
    Geom::Matrix i2doc;
    /** Viewport size */
    NRRect vp;
    /** Item to viewport transformation */
    Geom::Matrix i2vp;
};

/** Abstract base class for all visible shapes. */
struct SPItem : public SPObject {
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
    
    sigc::signal<void, NR::Matrix const *, SPItem *> _transformed_signal;

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

    void setCenter(NR::Point object_centre);
    void unsetCenter();
    bool isCenterSet();
    Geom::Point getCenter() const;

    bool isVisibleAndUnlocked() const;
    
    bool isVisibleAndUnlocked(unsigned display_key) const;
    
    NR::Matrix getRelativeTransform(SPObject const *obj) const;
    
    void raiseOne();
    void lowerOne();
    void raiseToTop();
    void lowerToBottom();

    boost::optional<NR::Rect> getBounds(NR::Matrix const &transform, BBoxType type=APPROXIMATE_BBOX, unsigned int dkey=0) const;

    sigc::connection _clip_ref_connection;
    sigc::connection _mask_ref_connection;

    sigc::connection connectTransformed(sigc::slot<void, NR::Matrix const *, SPItem *> slot)  {
        return _transformed_signal.connect(slot);
    }

private:
    enum EvaluatedStatus
    {
        StatusUnknown, StatusCalculated, StatusSet
    };

    mutable bool _is_evaluated;
    mutable EvaluatedStatus _evaluated_status;
};

typedef std::back_insert_iterator<std::vector<NR::Point> > SnapPointsIter;

/// The SPItem vtable.
struct SPItemClass {
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
    void (* snappoints) (SPItem const *item, SnapPointsIter p);
    
    /** Apply the transform optimally, and return any residual transformation */
    NR::Matrix (* set_transform)(SPItem *item, NR::Matrix const &transform);

    /** Convert the item to guidelines */
    void (* convert_to_guides)(SPItem *item);
    
    /** Emit event, if applicable */
    gint (* event) (SPItem *item, SPEvent *event);
};

/* Flag testing macros */

#define SP_ITEM_STOP_PAINT(i) (SP_ITEM (i)->stop_paint)

/* Methods */

void sp_item_invoke_bbox(SPItem const *item, boost::optional<NR::Rect> &bbox, NR::Matrix const &transform, unsigned const clear, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX);
void sp_item_invoke_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const clear, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) __attribute__ ((deprecated));
void sp_item_invoke_bbox_full(SPItem const *item, boost::optional<NR::Rect> &bbox, NR::Matrix const &transform, unsigned const flags, unsigned const clear);
void sp_item_invoke_bbox_full(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags, unsigned const clear) __attribute__ ((deprecated));

unsigned sp_item_pos_in_parent(SPItem *item);

gchar *sp_item_description(SPItem * item);
void sp_item_invoke_print(SPItem *item, SPPrintContext *ctx);

/** Shows/Hides item on given arena display list */
unsigned int sp_item_display_key_new(unsigned int numkeys);
NRArenaItem *sp_item_invoke_show(SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
void sp_item_invoke_hide(SPItem *item, unsigned int key);

void sp_item_snappoints(SPItem const *item, bool includeItemCenter, SnapPointsIter p);

void sp_item_adjust_pattern(SPItem *item, /* NR::Matrix const &premul, */ NR::Matrix const &postmul, bool set = false);
void sp_item_adjust_gradient(SPItem *item, /* NR::Matrix const &premul, */ NR::Matrix const &postmul, bool set = false);
void sp_item_adjust_stroke(SPItem *item, gdouble ex);
void sp_item_adjust_stroke_width_recursive(SPItem *item, gdouble ex);
void sp_item_adjust_paint_recursive(SPItem *item, NR::Matrix advertized_transform, NR::Matrix t_ancestors, bool is_pattern);
void sp_item_adjust_livepatheffect(SPItem *item, NR::Matrix const &postmul, bool set = false);

void sp_item_write_transform(SPItem *item, Inkscape::XML::Node *repr, NR::Matrix const *transform, NR::Matrix const *adv = NULL);
void sp_item_write_transform(SPItem *item, Inkscape::XML::Node *repr, NR::Matrix const &transform, NR::Matrix const *adv = NULL, bool compensate = true);

void sp_item_set_item_transform(SPItem *item, NR::Matrix const &transform);

void sp_item_convert_item_to_guides(SPItem *item);

gint sp_item_event (SPItem *item, SPEvent *event);

/* Utility */

NRArenaItem *sp_item_get_arenaitem(SPItem *item, unsigned int key);

void sp_item_bbox_desktop(SPItem *item, NRRect *bbox, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) __attribute__ ((deprecated));
boost::optional<NR::Rect> sp_item_bbox_desktop(SPItem *item, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX);

Geom::Matrix i2anc_affine(SPObject const *item, SPObject const *ancestor);
Geom::Matrix i2i_affine(SPObject const *src, SPObject const *dest);

Geom::Matrix sp_item_i2doc_affine(SPItem const *item);
Geom::Matrix sp_item_i2root_affine(SPItem const *item);

Geom::Matrix matrix_to_desktop (Geom::Matrix m, SPItem const *item);
Geom::Matrix matrix_from_desktop (Geom::Matrix m, SPItem const *item);

/* fixme: - these are evil, but OK */

/* Fill *TRANSFORM with the item-to-desktop transform.  See doc/coordinates.txt
 * for a description of `Desktop coordinates'; though see also mental's comment
 * at the top of that file.
 *
 * \return TRANSFORM.
 */
Geom::Matrix sp_item_i2d_affine(SPItem const *item);
Geom::Matrix sp_item_i2r_affine(SPItem const *item);
void sp_item_set_i2d_affine(SPItem *item, Geom::Matrix const &transform);
Geom::Matrix sp_item_dt2i_affine(SPItem const *item);
int sp_item_repr_compare_position(SPItem *first, SPItem *second);
SPItem *sp_item_first_item_child (SPObject *obj);

void sp_item_convert_to_guides(SPItem *item);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
