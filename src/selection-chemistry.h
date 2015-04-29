#ifndef SEEN_SELECTION_CHEMISTRY_H
#define SEEN_SELECTION_CHEMISTRY_H

/*
 * Miscellanous operations on selected items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2012 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/forward.h>
#include "sp-item.h"

class SPCSSAttr;
class SPDesktop;
typedef struct _GSList GSList;

namespace Inkscape {

class Selection;

namespace LivePathEffect {
    class PathParam;
}

    class SelectionHelper {
    public:
        static void selectAll(SPDesktop *desktop);
        static void selectAllInAll(SPDesktop *desktop);
        static void selectNone(SPDesktop *desktop);
        static void selectSameFillStroke(SPDesktop *dt);
        static void selectSameFillColor(SPDesktop *dt);
        static void selectSameStrokeColor(SPDesktop *dt);
        static void selectSameStrokeStyle(SPDesktop *dt);
        static void selectSameObjectType(SPDesktop *dt);
        static void invert(SPDesktop *desktop);
        static void invertAllInAll(SPDesktop *desktop);
        static void reverse(SPDesktop *dt);
        static void selectNext(SPDesktop *desktop);
        static void selectPrev(SPDesktop *desktop);
        static void fixSelection(SPDesktop *desktop);
    };
} // namespace Inkscape

void sp_selection_delete(SPDesktop *desktop);
void sp_selection_duplicate(SPDesktop *desktop, bool suppressDone = false);
void sp_edit_clear_all(Inkscape::Selection *selection);

void sp_edit_select_all(SPDesktop *desktop);
void sp_edit_select_all_in_all_layers (SPDesktop *desktop);
void sp_edit_invert (SPDesktop *desktop);
void sp_edit_invert_in_all_layers (SPDesktop *desktop);

void sp_selection_clone(SPDesktop *desktop);
void sp_selection_unlink(SPDesktop *desktop);
void sp_selection_relink(SPDesktop *desktop);
void sp_select_clone_original(SPDesktop *desktop);
void sp_selection_clone_original_path_lpe(SPDesktop *desktop);

void sp_selection_to_marker(SPDesktop *desktop, bool apply = true);
void sp_selection_to_guides(SPDesktop *desktop);

void sp_selection_symbol(SPDesktop *desktop, bool apply = true);
void sp_selection_unsymbol(SPDesktop *desktop);

void sp_selection_tile(SPDesktop *desktop, bool apply = true);
void sp_selection_untile(SPDesktop *desktop);

//void sp_selection_group_impl(GSList const *reprs_to_group, Inkscape::XML::Node *group, Inkscape::XML::Document *xml_doc, SPDocument *doc);
void sp_selection_group(Inkscape::Selection *selection, SPDesktop *desktop);
void sp_selection_ungroup(Inkscape::Selection *selection, SPDesktop *desktop);

void sp_selection_raise(Inkscape::Selection *selection, SPDesktop *desktop);
void sp_selection_raise_to_top(Inkscape::Selection *selection, SPDesktop *desktop);
void sp_selection_lower(Inkscape::Selection *selection, SPDesktop *desktop);
void sp_selection_lower_to_bottom(Inkscape::Selection *selection, SPDesktop *desktop);

SPCSSAttr *take_style_from_item (SPObject *object);

void sp_selection_cut(SPDesktop *desktop);
void sp_selection_copy(SPDesktop *desktop);
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
void sp_selection_to_layer( SPDesktop *desktop, SPObject *layer, bool suppressDone = false );

void sp_selection_apply_affine(Inkscape::Selection *selection, Geom::Affine const &affine, bool set_i2d = true, bool compensate = true, bool adjust_transf_center = true);
void sp_selection_remove_transform (SPDesktop *desktop);
void sp_selection_scale_absolute (Inkscape::Selection *selection, double x0, double x1, double y0, double y1);
void sp_selection_scale_relative(Inkscape::Selection *selection, Geom::Point const &align, Geom::Scale const &scale);
void sp_selection_rotate_relative (Inkscape::Selection *selection, Geom::Point const &center, double angle);
void sp_selection_skew_relative (Inkscape::Selection *selection, Geom::Point const &align, double dx, double dy);
void sp_selection_move_relative (Inkscape::Selection *selection, Geom::Point const &move, bool compensate = true);
void sp_selection_move_relative (Inkscape::Selection *selection, double dx, double dy);

void sp_selection_rotate_90 (SPDesktop *desktop, bool ccw);
void sp_selection_rotate (Inkscape::Selection *selection, double angle);
void sp_selection_rotate_screen (Inkscape::Selection *selection, double angle);

void sp_selection_scale (Inkscape::Selection *selection, double grow);
void sp_selection_scale_screen (Inkscape::Selection *selection, double grow_pixels);
void sp_selection_scale_times (Inkscape::Selection *selection, double times);

void sp_selection_move (Inkscape::Selection *selection, double dx, double dy);
void sp_selection_move_screen (Inkscape::Selection *selection, double dx, double dy);

void sp_selection_item_next (SPDesktop *desktop);
void sp_selection_item_prev (SPDesktop *desktop);

void sp_selection_next_patheffect_param(SPDesktop * dt);

void sp_selection_edit_clip_or_mask(SPDesktop * dt, bool clip);

enum SPSelectStrokeStyleType {
    SP_FILL_COLOR  = 0,
    SP_STROKE_COLOR  = 1,
    SP_STROKE_STYLE_WIDTH = 2,
    SP_STROKE_STYLE_DASHES = 3,
    SP_STROKE_STYLE_MARKERS = 4,
    SP_STROKE_STYLE_ALL = 5,
    SP_STYLE_ALL = 6
};

void sp_select_same_fill_stroke_style(SPDesktop *desktop, gboolean fill, gboolean strok, gboolean style);
void sp_select_same_object_type(SPDesktop *desktop);

std::vector<SPItem*> sp_get_same_style(SPItem *sel, std::vector<SPItem*> &src, SPSelectStrokeStyleType type=SP_STYLE_ALL);
std::vector<SPItem*> sp_get_same_object_type(SPItem *sel, std::vector<SPItem*> &src);

void scroll_to_show_item(SPDesktop *desktop, SPItem *item);

void sp_undo (SPDesktop *desktop, SPDocument *doc);
void sp_redo (SPDesktop *desktop, SPDocument *doc);

void sp_selection_get_export_hints (Inkscape::Selection *selection, Glib::ustring &filename, float *xdpi, float *ydpi);
void sp_document_get_export_hints (SPDocument * doc, Glib::ustring &filename, float *xdpi, float *ydpi);

void sp_selection_create_bitmap_copy (SPDesktop *desktop);

void sp_selection_set_clipgroup(SPDesktop *desktop);
void sp_selection_set_mask(SPDesktop *desktop, bool apply_clip_path, bool apply_to_layer);
void sp_selection_unset_mask(SPDesktop *desktop, bool apply_clip_path);

bool fit_canvas_to_selection(SPDesktop *, bool with_margins = false);
void verb_fit_canvas_to_selection(SPDesktop *);
bool fit_canvas_to_drawing(SPDocument *, bool with_margins = false);
void verb_fit_canvas_to_drawing(SPDesktop *);
void fit_canvas_to_selection_or_drawing(SPDesktop *);

void unlock_all(SPDesktop *dt);
void unlock_all_in_all_layers(SPDesktop *dt);
void unhide_all(SPDesktop *dt);
void unhide_all_in_all_layers(SPDesktop *dt);

std::vector<SPItem*> &get_all_items(std::vector<SPItem*> &list, SPObject *from, SPDesktop *desktop, bool onlyvisible, bool onlysensitive, bool ingroups, std::vector<SPItem*> const &exclude);

std::vector<SPItem*> sp_degroup_list (std::vector<SPItem*> &items);

/* selection cycling */
typedef enum
{
    SP_CYCLE_SIMPLE,
    SP_CYCLE_VISIBLE, // cycle only visible items
    SP_CYCLE_FOCUS // readjust visible area to view selected item
} SPCycleType;



// TOOD fixme: This should be moved into preference repr
extern SPCycleType SP_CYCLING;

#endif // SEEN_SELECTION_CHEMISTRY_H
