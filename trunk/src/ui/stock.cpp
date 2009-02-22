/**
 * \brief StockIDs for Inkscape-specific stock menu/toolbar items and icons.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "stock.h"

namespace Inkscape {
namespace UI {
namespace Stock {

// File menu
Gtk::StockID const OPEN_RECENT("open-recent");
Gtk::StockID const IMPORT("import");
Gtk::StockID const EXPORT("export");
Gtk::StockID const IMPORTFROMOCAL("import_from_ocal");
Gtk::StockID const EXPORTTOOCAL("export_to_ocal");
Gtk::StockID const VACUUM_DEFS("vacuum-defs");
// Edit menu
Gtk::StockID const UNDO_HISTORY("undo-history");
Gtk::StockID const PASTE_IN_PLACE("paste-in-place");
Gtk::StockID const PASTE_STYLE("paste-style");
Gtk::StockID const DUPLICATE("duplicate");
Gtk::StockID const CLONE("clone");
Gtk::StockID const CLONE_UNLINK("clone-unlink");
Gtk::StockID const CLONE_SELECT_ORIG("clone-select-orig");
Gtk::StockID const MAKE_BITMAP("make-bitmap");
Gtk::StockID const TILE("tile");
Gtk::StockID const UNTILE("untile");
Gtk::StockID const SELECT_ALL("select-all");
Gtk::StockID const SELECT_ALL_IN_ALL_LAYERS("select-all-in-all-layers");
Gtk::StockID const SELECT_INVERT("select-invert");
Gtk::StockID const SELECT_NONE("select-none");
Gtk::StockID const XML_EDITOR("xml-editor");
// View menu
Gtk::StockID const ZOOM("zoom");
Gtk::StockID const ZOOM_IN("zoom-in");
Gtk::StockID const ZOOM_OUT("zoom-out");
Gtk::StockID const ZOOM_100("zoom-100");
Gtk::StockID const ZOOM_50("zoom-50");
Gtk::StockID const ZOOM_200("zoom-200");
Gtk::StockID const ZOOM_SELECTION("zoom-selection");
Gtk::StockID const ZOOM_DRAWING("zoom-drawing");
Gtk::StockID const ZOOM_PAGE("zoom-page");
Gtk::StockID const ZOOM_WIDTH("zoom-width");
Gtk::StockID const ZOOM_PREV("zoom-prev");
Gtk::StockID const ZOOM_NEXT("zoom-next");
Gtk::StockID const SHOW_HIDE("show-hide");
Gtk::StockID const SHOW_HIDE_COMMANDS_BAR("show-hide-commands-bar");
Gtk::StockID const SHOW_HIDE_TOOL_CONTROLS_BAR("show-hide-tool-controls-bar");
Gtk::StockID const SHOW_HIDE_TOOLS_BAR("show-hide-tools-bar");
Gtk::StockID const SHOW_HIDE_RULERS("show-hide-rulers");
Gtk::StockID const SHOW_HIDE_SCROLLBARS("show-hide-scrollbars");
Gtk::StockID const SHOW_HIDE_STATUSBAR("show-hide-statusbar");
Gtk::StockID const SHOW_HIDE_DIALOGS("show-hide-dialogs");
Gtk::StockID const GRID("grid");
Gtk::StockID const GUIDES("guides");
Gtk::StockID const FULLSCREEN("fullscreen");
Gtk::StockID const MESSAGES("messages");
Gtk::StockID const SCRIPTS("scripts");
Gtk::StockID const WINDOW_PREV("window-prev");
Gtk::StockID const WINDOW_NEXT("window-next");
Gtk::StockID const WINDOW_DUPLICATE("window-duplicate");
// Layer menu
Gtk::StockID const LAYER_NEW("layer-new");
Gtk::StockID const LAYER_RENAME("layer-rename");
Gtk::StockID const LAYER_DUPLICATE("layer-duplicate");
Gtk::StockID const LAYER_ANCHOR("layer-anchor");
Gtk::StockID const LAYER_MERGE_DOWN("layer-merge-down");
Gtk::StockID const LAYER_DELETE("layer-delete");
Gtk::StockID const LAYER_SELECT_NEXT("layer-select-next");
Gtk::StockID const LAYER_SELECT_PREV("layer-select-prev");
Gtk::StockID const LAYER_SELECT_TOP("layer-select-top");
Gtk::StockID const LAYER_SELECT_BOTTOM("layer-select-bottom");
Gtk::StockID const LAYER_RAISE("layer-raise");
Gtk::StockID const LAYER_LOWER("layer-lower");
Gtk::StockID const LAYER_TO_TOP("layer-to-top");
Gtk::StockID const LAYER_TO_BOTTOM("layer-to-bottom");
// Object menu
Gtk::StockID const FILL_STROKE("fill-stroke");
Gtk::StockID const OBJECT_PROPERTIES("object-properties");
Gtk::StockID const FILTER_EFFECTS("filter-effects");
Gtk::StockID const GROUP("group");
Gtk::StockID const UNGROUP("upgroup");
Gtk::StockID const RAISE("raise");
Gtk::StockID const LOWER("lower");
Gtk::StockID const RAISE_TO_TOP("raise-to-top");
Gtk::StockID const LOWER_TO_BOTTOM("lower-to-bottom");
Gtk::StockID const MOVE_TO_NEW_LAYER("move-to-new-layer");
Gtk::StockID const MOVE_TO_NEXT_LAYER("move-to-next-layer");
Gtk::StockID const MOVE_TO_PREV_LAYER("move-to-prev-layer");
Gtk::StockID const MOVE_TO_TOP_LAYER("move-to-top-layer");
Gtk::StockID const MOVE_TO_BOTTOM_LAYER("move-to-bottom-layer");
Gtk::StockID const ROTATE_90_CW("rotate-90-cw");
Gtk::StockID const ROTATE_90_CCW("rotate-90-ccw");
Gtk::StockID const FLIP_HORIZ("flip-horiz");
Gtk::StockID const FLIP_VERT("flip-vert");
Gtk::StockID const TRANSFORM("transform");
Gtk::StockID const TRANSFORMATION("transformation");
Gtk::StockID const ALIGN_DISTRIBUTE("align-distribute");
// Path menu
Gtk::StockID const OBJECT_TO_PATH("object-to-path");
Gtk::StockID const STROKE_TO_PATH("stroke-to-path");
Gtk::StockID const TRACE("trace");
Gtk::StockID const UNION("union");
Gtk::StockID const DIFFERENCE("difference");
Gtk::StockID const INTERSECTION("intersection");
Gtk::StockID const EXCLUSION("exclusion");
Gtk::StockID const DIVISION("division");
Gtk::StockID const CUT_PATH("cut-path");
Gtk::StockID const COMBINE("combine");
Gtk::StockID const BREAK_APART("break-apart");
Gtk::StockID const INSET("inset");
Gtk::StockID const OUTSET("outset");
Gtk::StockID const OFFSET_DYNAMIC("offset-dynamic");
Gtk::StockID const OFFSET_LINKED("offset-linked");
Gtk::StockID const SIMPLIFY("simplify");
Gtk::StockID const REVERSE("reverse");
//Gtk::StockID const CLEANUP("cleanup"); (using Gtk::Stock::CLEAR)
// Text menu
Gtk::StockID const TEXT_PROPERTIES("text-properties");
Gtk::StockID const PUT_ON_PATH("put-on-path");
Gtk::StockID const REMOVE_FROM_PATH("remove-from-path");
Gtk::StockID const REMOVE_MANUAL_KERNS("remove-manual-kerns");
// About menu
Gtk::StockID const KEYS_MOUSE("keys-mouse");
Gtk::StockID const TUTORIALS("tutorials");
Gtk::StockID const ABOUT("about");
Gtk::StockID const ABOUT_SPLASH("about-splash");
Gtk::StockID const ABOUT_42("about-42");
Gtk::StockID const ABOUT_QUICK_HELP("about-quick-help");
// Tools bar
Gtk::StockID const TOOL_SELECT("tool-select");
Gtk::StockID const TOOL_NODE("tool-node");
Gtk::StockID const TOOL_ZOOM("tool-zoom");
Gtk::StockID const TOOL_RECT("tool-rect");
Gtk::StockID const TOOL_ARC("tool-arc");
Gtk::StockID const TOOL_STAR("tool-star");
Gtk::StockID const TOOL_SPIRAL("tool-spiral");
Gtk::StockID const TOOL_FREEHAND("tool-freehand");
Gtk::StockID const TOOL_PEN("tool-pen");
Gtk::StockID const TOOL_DYNADRAW("tool-dynadraw");
Gtk::StockID const TOOL_TEXT("tool-text");
Gtk::StockID const TOOL_DROPPER("tool-dropper");
// Select Tool controls
Gtk::StockID const TRANSFORM_STROKE("transform-stroke");
Gtk::StockID const TRANSFORM_CORNERS("transform-corners");
Gtk::StockID const TRANSFORM_GRADIENT("transform-gradient");
Gtk::StockID const TRANSFORM_PATTERN("transform-pattern");
// Node Tool controls
Gtk::StockID const NODE_INSERT("node-insert");
Gtk::StockID const NODE_DELETE("node-delete");
Gtk::StockID const NODE_JOIN("node-join");
Gtk::StockID const NODE_JOIN_SEGMENT("node-join-segment");
Gtk::StockID const NODE_DELETE_SEGMENT("node-delete-segment");
Gtk::StockID const NODE_BREAK("node-break");
Gtk::StockID const NODE_CORNER("node-corner");
Gtk::StockID const NODE_SMOOTH("node-smooth");
Gtk::StockID const NODE_SYMMETRIC("node-symmetric");
Gtk::StockID const NODE_LINE("node-line");
Gtk::StockID const NODE_CURVE("node-curve");
// Calligraphy Tool controls
Gtk::StockID const USE_PRESSURE("use-pressure");
Gtk::StockID const USE_TILT("use-tilt");

Gtk::StockID const SESSION_PLAYBACK_REW("session-rewind");
Gtk::StockID const SESSION_PLAYBACK_STEPBACK("session-stepback");
Gtk::StockID const SESSION_PLAYBACK_PAUSE("session-pause");
Gtk::StockID const SESSION_PLAYBACK_STEPFORWARD("session-stepforward");
Gtk::StockID const SESSION_PLAYBACK_PLAY("session-play");

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
