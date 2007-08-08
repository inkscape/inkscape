/**
 * \brief StockIDs for Inkscape-specific stock items and icons.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_STOCK_H
#define INKSCAPE_UI_STOCK_H

#include <gtkmm/stockid.h>

namespace Inkscape {
namespace UI {
namespace Stock {

// File menu
extern Gtk::StockID const OPEN_RECENT;
extern Gtk::StockID const IMPORT;
extern Gtk::StockID const EXPORT;
extern Gtk::StockID const IMPORTFROMOCAL;
extern Gtk::StockID const EXPORTTOOCAL;
extern Gtk::StockID const VACUUM_DEFS;
// Edit menu
extern Gtk::StockID const UNDO_HISTORY;
extern Gtk::StockID const PASTE_IN_PLACE;
extern Gtk::StockID const PASTE_STYLE;
extern Gtk::StockID const DUPLICATE;
extern Gtk::StockID const CLONE;
extern Gtk::StockID const CLONE_UNLINK;
extern Gtk::StockID const CLONE_SELECT_ORIG;
extern Gtk::StockID const MAKE_BITMAP;
extern Gtk::StockID const TILE;
extern Gtk::StockID const UNTILE;
extern Gtk::StockID const SELECT_ALL;
extern Gtk::StockID const SELECT_ALL_IN_ALL_LAYERS;
extern Gtk::StockID const SELECT_INVERT;
extern Gtk::StockID const SELECT_NONE;
extern Gtk::StockID const XML_EDITOR;
// View menu
extern Gtk::StockID const ZOOM;
extern Gtk::StockID const ZOOM_IN;
extern Gtk::StockID const ZOOM_OUT;
extern Gtk::StockID const ZOOM_100;
extern Gtk::StockID const ZOOM_50;
extern Gtk::StockID const ZOOM_200;
extern Gtk::StockID const ZOOM_SELECTION;
extern Gtk::StockID const ZOOM_DRAWING;
extern Gtk::StockID const ZOOM_PAGE;
extern Gtk::StockID const ZOOM_WIDTH;
extern Gtk::StockID const ZOOM_PREV;
extern Gtk::StockID const ZOOM_NEXT;
extern Gtk::StockID const SHOW_HIDE;
extern Gtk::StockID const SHOW_HIDE_COMMANDS_BAR;
extern Gtk::StockID const SHOW_HIDE_TOOL_CONTROLS_BAR;
extern Gtk::StockID const SHOW_HIDE_TOOLS_BAR;
extern Gtk::StockID const SHOW_HIDE_RULERS;
extern Gtk::StockID const SHOW_HIDE_SCROLLBARS;
extern Gtk::StockID const SHOW_HIDE_STATUSBAR;
extern Gtk::StockID const SHOW_HIDE_DIALOGS;
extern Gtk::StockID const GRID;
extern Gtk::StockID const GUIDES;
extern Gtk::StockID const FULLSCREEN;
extern Gtk::StockID const MESSAGES;
extern Gtk::StockID const SCRIPTS;
extern Gtk::StockID const WINDOW_PREV;
extern Gtk::StockID const WINDOW_NEXT;
extern Gtk::StockID const WINDOW_DUPLICATE;
// Layer menu
extern Gtk::StockID const LAYER_NEW;
extern Gtk::StockID const LAYER_RENAME;
extern Gtk::StockID const LAYER_DUPLICATE;
extern Gtk::StockID const LAYER_ANCHOR;
extern Gtk::StockID const LAYER_MERGE_DOWN;
extern Gtk::StockID const LAYER_DELETE;
extern Gtk::StockID const LAYER_SELECT_NEXT;
extern Gtk::StockID const LAYER_SELECT_PREV;
extern Gtk::StockID const LAYER_SELECT_TOP;
extern Gtk::StockID const LAYER_SELECT_BOTTOM;
extern Gtk::StockID const LAYER_RAISE;
extern Gtk::StockID const LAYER_LOWER;
extern Gtk::StockID const LAYER_TO_TOP;
extern Gtk::StockID const LAYER_TO_BOTTOM;
// Object menu
extern Gtk::StockID const FILL_STROKE;
extern Gtk::StockID const OBJECT_PROPERTIES;
extern Gtk::StockID const FILTER_EFFECTS;
extern Gtk::StockID const GROUP;
extern Gtk::StockID const UNGROUP;
extern Gtk::StockID const RAISE;
extern Gtk::StockID const LOWER;
extern Gtk::StockID const RAISE_TO_TOP;
extern Gtk::StockID const LOWER_TO_BOTTOM;
extern Gtk::StockID const MOVE_TO_NEW_LAYER;
extern Gtk::StockID const MOVE_TO_NEXT_LAYER;
extern Gtk::StockID const MOVE_TO_PREV_LAYER;
extern Gtk::StockID const MOVE_TO_TOP_LAYER;
extern Gtk::StockID const MOVE_TO_BOTTOM_LAYER;
extern Gtk::StockID const ROTATE_90_CW;
extern Gtk::StockID const ROTATE_90_CCW;
extern Gtk::StockID const FLIP_HORIZ;
extern Gtk::StockID const FLIP_VERT;
extern Gtk::StockID const TRANSFORM;
extern Gtk::StockID const TRANSFORMATION;
extern Gtk::StockID const ALIGN_DISTRIBUTE;
// Path menu
extern Gtk::StockID const OBJECT_TO_PATH;
extern Gtk::StockID const STROKE_TO_PATH;
extern Gtk::StockID const TRACE;
extern Gtk::StockID const UNION;
extern Gtk::StockID const DIFFERENCE;
extern Gtk::StockID const INTERSECTION;
extern Gtk::StockID const EXCLUSION;
extern Gtk::StockID const DIVISION;
extern Gtk::StockID const CUT_PATH;
extern Gtk::StockID const COMBINE;
extern Gtk::StockID const BREAK_APART;
extern Gtk::StockID const INSET;
extern Gtk::StockID const OUTSET;
extern Gtk::StockID const OFFSET_DYNAMIC;
extern Gtk::StockID const OFFSET_LINKED;
extern Gtk::StockID const SIMPLIFY;
extern Gtk::StockID const REVERSE;
//extern Gtk::StockID const CLEANUP; (using Gtk::Stock::CLEAR)
// Text menu
extern Gtk::StockID const TEXT_PROPERTIES;
extern Gtk::StockID const PUT_ON_PATH;
extern Gtk::StockID const REMOVE_FROM_PATH;
extern Gtk::StockID const REMOVE_MANUAL_KERNS;
// About menu
extern Gtk::StockID const KEYS_MOUSE;
extern Gtk::StockID const TUTORIALS;
extern Gtk::StockID const ABOUT;
extern Gtk::StockID const ABOUT_SPLASH;
extern Gtk::StockID const ABOUT_42;
extern Gtk::StockID const ABOUT_QUICK_HELP;
// Tools bar
extern Gtk::StockID const TOOL_SELECT;
extern Gtk::StockID const TOOL_NODE;
extern Gtk::StockID const TOOL_ZOOM;
extern Gtk::StockID const TOOL_RECT;
extern Gtk::StockID const TOOL_ARC;
extern Gtk::StockID const TOOL_STAR;
extern Gtk::StockID const TOOL_SPIRAL;
extern Gtk::StockID const TOOL_FREEHAND;
extern Gtk::StockID const TOOL_PEN;
extern Gtk::StockID const TOOL_DYNADRAW;
extern Gtk::StockID const TOOL_TEXT;
extern Gtk::StockID const TOOL_DROPPER;
// Select Tool controls
extern Gtk::StockID const TRANSFORM_STROKE;
extern Gtk::StockID const TRANSFORM_CORNERS;
extern Gtk::StockID const TRANSFORM_GRADIENT;
extern Gtk::StockID const TRANSFORM_PATTERN;
// Node Tool controls
extern Gtk::StockID const NODE_INSERT;
extern Gtk::StockID const NODE_DELETE;
extern Gtk::StockID const NODE_JOIN;
extern Gtk::StockID const NODE_JOIN_SEGMENT;
extern Gtk::StockID const NODE_DELETE_SEGMENT;
extern Gtk::StockID const NODE_BREAK;
extern Gtk::StockID const NODE_CORNER;
extern Gtk::StockID const NODE_SMOOTH;
extern Gtk::StockID const NODE_SYMMETRIC;
extern Gtk::StockID const NODE_LINE;
extern Gtk::StockID const NODE_CURVE;
// Calligraphy Tool controls
extern Gtk::StockID const USE_PRESSURE;
extern Gtk::StockID const USE_TILT;
// Session playback dialog controls
extern Gtk::StockID const SESSION_PLAYBACK_REW;
extern Gtk::StockID const SESSION_PLAYBACK_STEPBACK;
extern Gtk::StockID const SESSION_PLAYBACK_PAUSE;
extern Gtk::StockID const SESSION_PLAYBACK_STEPFORWARD;
extern Gtk::StockID const SESSION_PLAYBACK_PLAY;

} // namespace Stock
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_STOCK_H

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
