/**
 * \brief StockItems for Inkscape-specific menu/button labels and key
 *        accelerators.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <gtkmm/stock.h>
#include <glibmm/i18n.h>

#include "stock.h"

namespace Inkscape {
namespace UI {
namespace Stock {

void
init()
{
    using namespace Gtk::Stock;

// FIXME: strings are replaced by placeholders, NOT to be translated until the code is enabled
// See http://sourceforge.net/mailarchive/message.php?msg_id=11746016 for details

    // File menu
    add(Gtk::StockItem(OPEN, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(OPEN_RECENT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SAVE_AS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(IMPORT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(EXPORT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(IMPORTFROMOCAL, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(EXPORTTOOCAL, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(PRINT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(PRINT_PREVIEW, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(VACUUM_DEFS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(PROPERTIES, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(PREFERENCES, _("PLACEHOLDER, do not translate")));

    // Edit menu
    add(Gtk::StockItem(PASTE_IN_PLACE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(PASTE_STYLE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(DUPLICATE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(CLONE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(CLONE_UNLINK, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(CLONE_SELECT_ORIG, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MAKE_BITMAP, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TILE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(UNTILE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SELECT_ALL, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SELECT_ALL_IN_ALL_LAYERS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SELECT_INVERT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SELECT_NONE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(XML_EDITOR, _("PLACEHOLDER, do not translate")));

    // View menu
    add(Gtk::StockItem(ZOOM, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_IN, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_OUT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_100, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_50, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_200, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_SELECTION, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_DRAWING, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_PAGE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_WIDTH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_PREV, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ZOOM_NEXT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_COMMANDS_BAR, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_TOOL_CONTROLS_BAR, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_TOOLS_BAR, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_RULERS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_SCROLLBARS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_STATUSBAR, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SHOW_HIDE_DIALOGS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(GRID, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(GUIDES, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(FULLSCREEN, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MESSAGES, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SCRIPTS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(WINDOW_PREV, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(WINDOW_NEXT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(WINDOW_DUPLICATE, _("PLACEHOLDER, do not translate")));

    // Layer menu
    add(Gtk::StockItem(LAYER_NEW, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_RENAME, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_DUPLICATE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_ANCHOR, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_MERGE_DOWN, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_DELETE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_SELECT_NEXT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_SELECT_PREV, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_SELECT_TOP, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_SELECT_BOTTOM, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_RAISE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_LOWER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_TO_TOP, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LAYER_TO_BOTTOM, _("PLACEHOLDER, do not translate")));

    // Object menu
    add(Gtk::StockItem(FILL_STROKE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(OBJECT_PROPERTIES, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(GROUP, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(UNGROUP, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(RAISE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LOWER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(RAISE_TO_TOP, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(LOWER_TO_BOTTOM, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MOVE_TO_NEW_LAYER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MOVE_TO_NEXT_LAYER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MOVE_TO_PREV_LAYER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MOVE_TO_TOP_LAYER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(MOVE_TO_BOTTOM_LAYER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ROTATE_90_CW, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ROTATE_90_CCW, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(FLIP_HORIZ, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(FLIP_VERT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TRANSFORM, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TRANSFORMATION, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ALIGN_DISTRIBUTE, _("PLACEHOLDER, do not translate")));

    // Path menu
    add(Gtk::StockItem(OBJECT_TO_PATH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(STROKE_TO_PATH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TRACE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(UNION, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(DIFFERENCE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(INTERSECTION, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(EXCLUSION, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(DIVISION, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(CUT_PATH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(COMBINE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(BREAK_APART, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(INSET, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(OUTSET, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(OFFSET_DYNAMIC, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(OFFSET_LINKED, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(SIMPLIFY, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(REVERSE, _("PLACEHOLDER, do not translate")));
    //add(Gtk::StockItem(CLEANUP, _("PLACEHOLDER, do not translate"))); (using Gtk::Stock::CLEAR)

    // Text menu
    add(Gtk::StockItem(SELECT_FONT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(PUT_ON_PATH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(REMOVE_FROM_PATH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(REMOVE_MANUAL_KERNS, _("PLACEHOLDER, do not translate")));

    // About menu
    add(Gtk::StockItem(KEYS_MOUSE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TUTORIALS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(ABOUT, _("PLACEHOLDER, do not translate")));

    // Tools toolbox
    add(Gtk::StockItem(TOOL_SELECT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_NODE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_ZOOM, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_RECT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_ARC, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_STAR, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_SPIRAL, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_FREEHAND, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_PEN, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_DYNADRAW, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_TEXT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TOOL_DROPPER, _("PLACEHOLDER, do not translate")));

    // Select Tool controls
    add(Gtk::StockItem(TRANSFORM_STROKE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TRANSFORM_CORNERS, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TRANSFORM_GRADIENT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(TRANSFORM_PATTERN, _("PLACEHOLDER, do not translate")));

    // Node Tool controls
    add(Gtk::StockItem(NODE_INSERT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_DELETE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_JOIN, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_JOIN_SEGMENT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_DELETE_SEGMENT, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_BREAK, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_CORNER, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_SMOOTH, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_SYMMETRIC, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_LINE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(NODE_CURVE, _("PLACEHOLDER, do not translate")));

    // Calligraphy Tool controls
    add(Gtk::StockItem(USE_PRESSURE, _("PLACEHOLDER, do not translate")));
    add(Gtk::StockItem(USE_TILT, _("PLACEHOLDER, do not translate")));

	// Session playback controls
	add(Gtk::StockItem(SESSION_PLAYBACK_REW, _("PLACEHOLDER, do not translate")));
	add(Gtk::StockItem(SESSION_PLAYBACK_STEPBACK, _("PLACEHOLDER, do not translate")));
	add(Gtk::StockItem(SESSION_PLAYBACK_PAUSE, _("PLACEHOLDER, do not translate")));
	add(Gtk::StockItem(SESSION_PLAYBACK_STEPFORWARD, _("PLACEHOLDER, do not translate")));
	add(Gtk::StockItem(SESSION_PLAYBACK_PLAY, _("PLACEHOLDER, do not translate")));
}

} // namespace Stock
} // namespace UI
} // namespace Inkscape

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
