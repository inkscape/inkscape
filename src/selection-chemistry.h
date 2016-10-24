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

namespace Inkscape {

class Selection;
class ObjectSet;

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

void sp_edit_clear_all(Inkscape::Selection *selection);

void sp_edit_select_all(SPDesktop *desktop);
void sp_edit_select_all_in_all_layers (SPDesktop *desktop);
void sp_edit_invert (SPDesktop *desktop);
void sp_edit_invert_in_all_layers (SPDesktop *desktop);


SPCSSAttr *take_style_from_item (SPObject *object);

void sp_selection_paste(SPDesktop *desktop, bool in_place);

void sp_set_style_clipboard (SPCSSAttr *css);


void sp_selection_item_next (SPDesktop *desktop);
void sp_selection_item_prev (SPDesktop *desktop);

void sp_selection_next_patheffect_param(SPDesktop * dt);

//void sp_selection_edit_clip_or_mask(SPDesktop * dt, bool clip);

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

void sp_document_get_export_hints (SPDocument * doc, Glib::ustring &filename, float *xdpi, float *ydpi);

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
