#ifndef __SP_SELECTION_CHEMISTRY_H__
#define __SP_SELECTION_CHEMISTRY_H__

/*
 * Miscellanous operations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "2geom/forward.h"

namespace Inkscape { class Selection; }

namespace Inkscape {
namespace LivePathEffect {
    class PathParam;
}
}

class SPCSSAttr;

void sp_selection_delete(SPDesktop *desktop);
void sp_selection_duplicate(SPDesktop *desktop, bool suppressDone = false);
void sp_edit_clear_all(SPDesktop *desktop);

void sp_edit_select_all(SPDesktop *desktop);
void sp_edit_select_all_in_all_layers (SPDesktop *desktop);
void sp_edit_invert (SPDesktop *desktop);
void sp_edit_invert_in_all_layers (SPDesktop *desktop);

void sp_selection_clone(SPDesktop *desktop);
void sp_selection_unlink(SPDesktop *desktop);
void sp_selection_relink(SPDesktop *desktop);
void sp_select_clone_original(SPDesktop *desktop);

void sp_selection_to_marker(SPDesktop *desktop, bool apply = true);
void sp_selection_to_guides(SPDesktop *desktop);

void sp_selection_tile(SPDesktop *desktop, bool apply = true);
void sp_selection_untile(SPDesktop *desktop);

void sp_selection_group(SPDesktop *desktop);
void sp_selection_ungroup(SPDesktop *desktop);

void sp_selection_raise(SPDesktop *desktop);
void sp_selection_raise_to_top(SPDesktop *desktop);
void sp_selection_lower(SPDesktop *desktop);
void sp_selection_lower_to_bottom(SPDesktop *desktop);

SPCSSAttr *take_style_from_item (SPItem *item);

void sp_selection_cut(SPDesktop *desktop);
void sp_selection_copy();
void sp_selection_paste(SPDesktop *desktop, bool in_place);
void sp_selection_paste_style(SPDesktop *desktop);
void sp_selection_paste_livepatheffect(SPDesktop *desktop);
void sp_selection_remove_livepatheffect(SPDesktop *desktop);

void sp_selection_remove_filter(SPDesktop *desktop);

void sp_set_style_clipboard (SPCSSAttr *css);

void sp_selection_paste_size(SPDesktop *desktop, bool apply_x, bool apply_y);
void sp_selection_paste_size_separately(SPDesktop *desktop, bool apply_x, bool apply_y);

void sp_selection_to_next_layer( SPDesktop *desktop, bool suppressDone = false );
void sp_selection_to_prev_layer( SPDesktop *desktop, bool suppressDone = false );

void sp_selection_apply_affine(Inkscape::Selection *selection, Geom::Matrix const &affine, bool set_i2d = true);
void sp_selection_remove_transform (SPDesktop *desktop);
void sp_selection_scale_absolute (Inkscape::Selection *selection, double x0, double x1, double y0, double y1);
void sp_selection_scale_relative(Inkscape::Selection *selection, Geom::Point const &align, Geom::Scale const &scale);
void sp_selection_rotate_relative (Inkscape::Selection *selection, Geom::Point const &center, gdouble angle);
void sp_selection_skew_relative (Inkscape::Selection *selection, Geom::Point const &align, double dx, double dy);
void sp_selection_move_relative (Inkscape::Selection *selection, Geom::Point const &move);
void sp_selection_move_relative (Inkscape::Selection *selection, double dx, double dy);

void sp_selection_rotate_90 (SPDesktop *desktop, bool ccw);
void sp_selection_rotate (Inkscape::Selection *selection, gdouble angle);
void sp_selection_rotate_screen (Inkscape::Selection *selection, gdouble angle);

void sp_selection_scale (Inkscape::Selection *selection, gdouble grow);
void sp_selection_scale_screen (Inkscape::Selection *selection, gdouble grow_pixels);
void sp_selection_scale_times (Inkscape::Selection *selection, gdouble times);

void sp_selection_move (SPDesktop *desktop, gdouble dx, gdouble dy);
void sp_selection_move_screen (SPDesktop *desktop, gdouble dx, gdouble dy);

void sp_selection_item_next (SPDesktop *desktop);
void sp_selection_item_prev (SPDesktop *desktop);

void sp_selection_next_patheffect_param(SPDesktop * dt);

void sp_selection_edit_clip_or_mask(SPDesktop * dt, bool clip);

void scroll_to_show_item(SPDesktop *desktop, SPItem *item);

void sp_undo (SPDesktop *desktop, SPDocument *doc);
void sp_redo (SPDesktop *desktop, SPDocument *doc);

void sp_selection_get_export_hints (Inkscape::Selection *selection, const char **filename, float *xdpi, float *ydpi);
void sp_document_get_export_hints (SPDocument * doc, const char **filename, float *xdpi, float *ydpi);

void sp_selection_create_bitmap_copy (SPDesktop *desktop);

void sp_selection_set_mask(SPDesktop *desktop, bool apply_clip_path, bool apply_to_layer);
void sp_selection_unset_mask(SPDesktop *desktop, bool apply_clip_path);

bool fit_canvas_to_selection(SPDesktop *);
void verb_fit_canvas_to_selection(SPDesktop *);
bool fit_canvas_to_drawing(SPDocument *);
void verb_fit_canvas_to_drawing(SPDesktop *);
void fit_canvas_to_selection_or_drawing(SPDesktop *);

void unlock_all(SPDesktop *dt);
void unlock_all_in_all_layers(SPDesktop *dt);
void unhide_all(SPDesktop *dt);
void unhide_all_in_all_layers(SPDesktop *dt);

/* selection cycling */

typedef enum
{
	SP_CYCLE_SIMPLE,
	SP_CYCLE_VISIBLE, /* cycle only visible items */
	SP_CYCLE_FOCUS /* readjust visible area to view selected item */
} SPCycleType;

/* fixme: This should be moved into preference repr */
#ifndef __SP_SELECTION_CHEMISTRY_C__
extern SPCycleType SP_CYCLING;
#else
SPCycleType SP_CYCLING = SP_CYCLE_FOCUS;
#endif

#endif



