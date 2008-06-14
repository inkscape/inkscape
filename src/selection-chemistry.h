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
#include "libnr/nr-forward.h"

namespace Inkscape { class Selection; }

namespace Inkscape {
namespace LivePathEffect {
    class PathParam;
}
}

class SPCSSAttr;

void sp_selection_delete();
void sp_selection_duplicate(bool suppressDone = false);
void sp_edit_clear_all();

void sp_edit_select_all();
void sp_edit_select_all_in_all_layers ();
void sp_edit_invert ();
void sp_edit_invert_in_all_layers ();

void sp_selection_clone();
void sp_selection_unlink();
void sp_select_clone_original ();

void sp_selection_to_marker(bool apply = true);
void sp_selection_to_guides();

void sp_selection_tile(bool apply = true);
void sp_selection_untile();

void sp_selection_group();
void sp_selection_ungroup();

void sp_selection_raise();
void sp_selection_raise_to_top();
void sp_selection_lower();
void sp_selection_lower_to_bottom();

SPCSSAttr *take_style_from_item (SPItem *item);

void sp_selection_cut();
void sp_selection_copy();
void sp_selection_paste(bool in_place);
void sp_selection_paste_style();
void sp_selection_paste_livepatheffect();
void sp_selection_remove_livepatheffect();

void sp_selection_remove_filter();

void sp_set_style_clipboard (SPCSSAttr *css);

void sp_selection_paste_size(bool apply_x, bool apply_y);
void sp_selection_paste_size_separately(bool apply_x, bool apply_y);

void sp_selection_to_next_layer( bool suppressDone = false );
void sp_selection_to_prev_layer( bool suppressDone = false );

void sp_selection_apply_affine(Inkscape::Selection *selection, NR::Matrix const &affine, bool set_i2d = true);
void sp_selection_remove_transform (void);
void sp_selection_scale_absolute (Inkscape::Selection *selection, double x0, double x1, double y0, double y1);
void sp_selection_scale_relative(Inkscape::Selection *selection, NR::Point const &align, NR::scale const &scale);
void sp_selection_rotate_relative (Inkscape::Selection *selection, NR::Point const &center, gdouble angle);
void sp_selection_skew_relative (Inkscape::Selection *selection, NR::Point const &align, double dx, double dy);
void sp_selection_move_relative (Inkscape::Selection *selection, NR::Point const &move);
void sp_selection_move_relative (Inkscape::Selection *selection, double dx, double dy);

void sp_selection_rotate_90_cw (void);
void sp_selection_rotate_90_ccw (void);
void sp_selection_rotate (Inkscape::Selection *selection, gdouble angle);
void sp_selection_rotate_screen (Inkscape::Selection *selection, gdouble angle);

void sp_selection_scale (Inkscape::Selection *selection, gdouble grow);
void sp_selection_scale_screen (Inkscape::Selection *selection, gdouble grow_pixels);
void sp_selection_scale_times (Inkscape::Selection *selection, gdouble times);

void sp_selection_move (gdouble dx, gdouble dy);
void sp_selection_move_screen (gdouble dx, gdouble dy);

void sp_selection_item_next (void);
void sp_selection_item_prev (void);

void sp_selection_next_patheffect_param(SPDesktop * dt);

void sp_selection_edit_clip_or_mask(SPDesktop * dt, bool clip);

void scroll_to_show_item(SPDesktop *desktop, SPItem *item);

void sp_undo (SPDesktop *desktop, SPDocument *doc);
void sp_redo (SPDesktop *desktop, SPDocument *doc);

void sp_selection_get_export_hints (Inkscape::Selection *selection, const char **filename, float *xdpi, float *ydpi);
void sp_document_get_export_hints (SPDocument * doc, const char **filename, float *xdpi, float *ydpi);

void sp_selection_create_bitmap_copy ();

void sp_selection_set_mask(bool apply_clip_path, bool apply_to_layer);
void sp_selection_unset_mask(bool apply_clip_path);

void fit_canvas_to_selection(SPDesktop *desktop);
void fit_canvas_to_drawing(SPDocument *doc);
void fit_canvas_to_selection_or_drawing(SPDesktop *desktop);

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



