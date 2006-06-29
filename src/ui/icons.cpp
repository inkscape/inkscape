/**
 * \brief Stock icons for Inkscape-specific menu items and buttons.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/iconfactory.h>
#include <gtkmm/stock.h>

#include "stock.h"
#include "path-prefix.h"

namespace Inkscape {
namespace UI {
namespace Icons {

static Glib::ustring const get_icon_path(char const *utf8_basename);

void
init()
{
    Glib::RefPtr<Gtk::IconFactory> icons = Gtk::IconFactory::create();
    Gtk::IconSource src;

    // repeat, for every Inkscape stock.h entry that needs an icon.
    //Gtk::IconSet ;
    //src.set_icon_name("");
    //src.set_filename(get_icon_path(".svg"));
    //.add_source(src);
    //icons->add(Stock::, );

    // File menu
    //  Open Recent
    Gtk::IconSet _open_recent;
    src.set_icon_name("OpenRecent");
    src.set_filename(get_icon_path("open-recent.svg"));
    _open_recent.add_source(src);
    icons->add(Stock::OPEN_RECENT, _open_recent);
    //  Import
    Gtk::IconSet _import;
    src.set_icon_name("Import");
    src.set_filename(get_icon_path("import.svg"));
    _import.add_source(src);
    icons->add(Stock::IMPORT, _import);
    //  Export
    Gtk::IconSet _export;
    src.set_icon_name("Export");
    src.set_filename(get_icon_path("export.svg"));
    _export.add_source(src);
    icons->add(Stock::EXPORT, _export);
    //  Vacuum Defs
    Gtk::IconSet _vacuum_defs;
    src.set_icon_name("VacuumDefs");
    src.set_filename(get_icon_path("vacuum-defs.svg"));
    _vacuum_defs.add_source(src);
    icons->add(Stock::VACUUM_DEFS, _vacuum_defs);

    // Edit menu
    //  Undo History
    Gtk::IconSet _undo_history;
    src.set_icon_name("UndoHistory");
    src.set_filename(get_icon_path("undo-history.svg"));
    _undo_history.add_source(src);
    icons->add(Stock::UNDO_HISTORY, _undo_history);
    //  Paste In Place
    Gtk::IconSet _paste_in_place;
    src.set_icon_name("PasteInPlace");
    src.set_filename(get_icon_path("paste-in-place.svg"));
    _paste_in_place.add_source(src);
    icons->add(Stock::PASTE_IN_PLACE, _paste_in_place);
    //  Paste Style
    Gtk::IconSet _paste_style;
    src.set_icon_name("PasteStyle");
    src.set_filename(get_icon_path("paste-style.svg"));
    _paste_style.add_source(src);
    icons->add(Stock::PASTE_STYLE, _paste_style);
    //  Duplicate
    Gtk::IconSet _duplicate;
    src.set_icon_name("Duplicate");
    src.set_filename(get_icon_path("duplicate.svg"));
    _duplicate.add_source(src);
    icons->add(Stock::DUPLICATE, _duplicate);
    //  Clone
    Gtk::IconSet _clone;
    src.set_icon_name("Clone");
    src.set_filename(get_icon_path("clone.svg"));
    _clone.add_source(src);
    icons->add(Stock::CLONE, _clone);
    //  Unlink Clone
    Gtk::IconSet _clone_unlink;
    src.set_icon_name("CloneUnlink");
    src.set_filename(get_icon_path("clone-unlink.svg"));
    _clone_unlink.add_source(src);
    icons->add(Stock::CLONE_UNLINK, _clone_unlink);
    //  Make Bitmap
    Gtk::IconSet _make_bitmap;
    src.set_icon_name("MakeBitmap");
    src.set_filename(get_icon_path("make-bitmap.svg"));
    _make_bitmap.add_source(src);
    icons->add(Stock::MAKE_BITMAP, _make_bitmap);
    //  Select All
    Gtk::IconSet _select_all;
    src.set_icon_name("SelectAll");
    src.set_filename(get_icon_path("select-all.svg"));
    _select_all.add_source(src);
    icons->add(Stock::SELECT_ALL, _select_all);
    //  XML Editor
    Gtk::IconSet _xml_editor;
    src.set_icon_name("XmlEditor");
    src.set_filename(get_icon_path("xml-editor.svg"));
    _xml_editor.add_source(src);
    icons->add(Stock::XML_EDITOR, _xml_editor);

    // View menu
    //  Zoom
    Gtk::IconSet _zoom;
    src.set_icon_name("Zoom");
    src.set_filename(get_icon_path("zoom.svg"));
    _zoom.add_source(src);
    icons->add(Stock::ZOOM, _zoom);
    //  Zoom In (use Inkscape icon for consistency over Gtk::Stock::ZOOM_IN)
    Gtk::IconSet _zoom_in;
    src.set_icon_name("ZoomIn");
    src.set_filename(get_icon_path("zoom-in.svg"));
    _zoom_in.add_source(src);
    icons->add(Stock::ZOOM_IN, _zoom_in);
    //  Zoom Out (use Inkscape icon for consistency over Gtk::Stock::ZOOM_OUT)
    Gtk::IconSet _zoom_out;
    src.set_icon_name("ZoomOut");
    src.set_filename(get_icon_path("zoom-out.svg"));
    _zoom_out.add_source(src);
    icons->add(Stock::ZOOM_OUT, _zoom_out);
    //  Zoom 100% (use Inkscape icon for consistency over Gtk::Stock::ZOOM_100)
    Gtk::IconSet _zoom_100;
    src.set_icon_name("Zoom100");
    src.set_filename(get_icon_path("zoom-100.svg"));
    _zoom_100.add_source(src);
    icons->add(Stock::ZOOM_100, _zoom_100);
    //  Zoom 50%
    Gtk::IconSet _zoom_50;
    src.set_icon_name("Zoom50");
    src.set_filename(get_icon_path("zoom-50.svg"));
    _zoom_50.add_source(src);
    icons->add(Stock::ZOOM_50, _zoom_50);
    //  Zoom 200%
    Gtk::IconSet _zoom_200;
    src.set_icon_name("Zoom200");
    src.set_filename(get_icon_path("zoom-200.svg"));
    _zoom_200.add_source(src);
    icons->add(Stock::ZOOM_200, _zoom_200);
    //  Zoom Selection
    Gtk::IconSet _zoom_selection;
    src.set_icon_name("ZoomSelection");
    src.set_filename(get_icon_path("zoom-selection.svg"));
    _zoom_selection.add_source(src);
    icons->add(Stock::ZOOM_SELECTION, _zoom_selection);
    //  Zoom Drawing
    Gtk::IconSet _zoom_drawing;
    src.set_icon_name("ZoomDrawing");
    src.set_filename(get_icon_path("zoom-drawing.svg"));
    _zoom_drawing.add_source(src);
    icons->add(Stock::ZOOM_DRAWING, _zoom_drawing);
    //  Zoom Page (use Inkscape icon for consistency over Gtk::Stock::ZOOM_FIT)
    Gtk::IconSet _zoom_page;
    src.set_icon_name("ZoomPage");
    src.set_filename(get_icon_path("zoom-page.svg"));
    _zoom_page.add_source(src);
    icons->add(Stock::ZOOM_PAGE, _zoom_page);
    //  Zoom Width
    Gtk::IconSet _zoom_width;
    src.set_icon_name("ZoomWidth");
    src.set_filename(get_icon_path("zoom-width.svg"));
    _zoom_width.add_source(src);
    icons->add(Stock::ZOOM_WIDTH, _zoom_width);
    //  Zoom Previous
    Gtk::IconSet _zoom_prev;
    src.set_icon_name("ZoomPrev");
    src.set_filename(get_icon_path("zoom-prev.svg"));
    _zoom_prev.add_source(src);
    icons->add(Stock::ZOOM_PREV, _zoom_prev);
    //  Zoom Next
    Gtk::IconSet _zoom_next;
    src.set_icon_name("ZoomNext");
    src.set_filename(get_icon_path("zoom-next.svg"));
    _zoom_next.add_source(src);
    icons->add(Stock::ZOOM_NEXT, _zoom_next);
    //  Show/Hide Dialogs
    Gtk::IconSet _show_hide_dialogs;
    src.set_icon_name("ShowHideDialogs");
    src.set_filename(get_icon_path("show-hide-dialogs.svg"));
    _show_hide_dialogs.add_source(src);
    icons->add(Stock::SHOW_HIDE_DIALOGS, _show_hide_dialogs);
    //  Grid
    Gtk::IconSet _grid;
    src.set_icon_name("Grid");
    src.set_filename(get_icon_path("grid.svg"));
    _grid.add_source(src);
    icons->add(Stock::GRID, _grid);
    //  Guides
    Gtk::IconSet _guides;
    src.set_icon_name("Guides");
    src.set_filename(get_icon_path("guides.svg"));
    _guides.add_source(src);
    icons->add(Stock::GUIDES, _guides);
    //  Fullscreen
    Gtk::IconSet _fullscreen;
    src.set_icon_name("Fullscreen");
    src.set_filename(get_icon_path("fullscreen.svg"));
    _fullscreen.add_source(src);
    icons->add(Stock::FULLSCREEN, _fullscreen);
    //  Previous Window
    Gtk::IconSet _window_prev;
    src.set_icon_name("WindowPrev");
    src.set_filename(get_icon_path("window-prev.svg"));
    _window_prev.add_source(src);
    icons->add(Stock::WINDOW_PREV, _window_prev);
    //  Next Window
    Gtk::IconSet _window_next;
    src.set_icon_name("WindowNext");
    src.set_filename(get_icon_path("window-next.svg"));
    _window_next.add_source(src);
    icons->add(Stock::WINDOW_NEXT, _window_next);
    //  Duplicate Window
    Gtk::IconSet _window_duplicate;
    src.set_icon_name("WindowDuplicate");
    src.set_filename(get_icon_path("window-duplicate.svg"));
    _window_duplicate.add_source(src);
    icons->add(Stock::WINDOW_DUPLICATE, _window_duplicate);

    // Layer menu
    //  New Layer
    icons->add(Stock::LAYER_NEW,
               Gtk::IconSet::lookup_default(Gtk::Stock::NEW));
    //  Delete Layer
    icons->add(Stock::LAYER_DELETE,
               Gtk::IconSet::lookup_default(Gtk::Stock::DELETE));
    //  Raise Layer
    icons->add(Stock::LAYER_RAISE,
               Gtk::IconSet::lookup_default(Gtk::Stock::GO_UP));
    //  Lower Layer
    icons->add(Stock::LAYER_LOWER,
               Gtk::IconSet::lookup_default(Gtk::Stock::GO_DOWN));
    //  Layer to Top
    icons->add(Stock::LAYER_TO_TOP,
               Gtk::IconSet::lookup_default(Gtk::Stock::GOTO_TOP));
    //  Layer to Bottom
    icons->add(Stock::LAYER_TO_BOTTOM,
               Gtk::IconSet::lookup_default(Gtk::Stock::GOTO_BOTTOM));

    // Object menu
    //  Fill and Stoke
    Gtk::IconSet _fill_stroke;
    src.set_icon_name("FillAndStroke");
    src.set_filename(get_icon_path("fill-stroke.svg"));
    _fill_stroke.add_source(src);
    icons->add(Stock::FILL_STROKE, _fill_stroke);
    //  Object Properties
    Gtk::IconSet _object_properties;
    src.set_icon_name("ObjectProperties");
    src.set_filename(get_icon_path("object-properties.svg"));
    _object_properties.add_source(src);
    icons->add(Stock::OBJECT_PROPERTIES, _object_properties);
    //  Group
    Gtk::IconSet _group;
    src.set_icon_name("Group");
    src.set_filename(get_icon_path("group.svg"));
    _group.add_source(src);
    icons->add(Stock::GROUP, _group);
    //  Ungroup
    Gtk::IconSet _ungroup;
    src.set_icon_name("Ungroup");
    src.set_filename(get_icon_path("ungroup.svg"));
    _ungroup.add_source(src);
    icons->add(Stock::UNGROUP, _ungroup);
    //  Raise
    Gtk::IconSet _raise;
    src.set_icon_name("Raise");
    src.set_filename(get_icon_path("raise.svg"));
    _raise.add_source(src);
    icons->add(Stock::RAISE, _raise);
    //  Lower
    Gtk::IconSet _lower;
    src.set_icon_name("Lower");
    src.set_filename(get_icon_path("lower.svg"));
    _lower.add_source(src);
    icons->add(Stock::LOWER, _lower);
    //  Raise to Top
    Gtk::IconSet _raise_to_top;
    src.set_icon_name("RaiseToTop");
    src.set_filename(get_icon_path("raise-to-top.svg"));
    _raise_to_top.add_source(src);
    icons->add(Stock::RAISE_TO_TOP, _raise_to_top);
    //  Lower to Bottom
    Gtk::IconSet _lower_to_bottom;
    src.set_icon_name("LowerToBottom");
    src.set_filename(get_icon_path("lower-to-bottom.svg"));
    _lower_to_bottom.add_source(src);
    icons->add(Stock::LOWER_TO_BOTTOM, _lower_to_bottom);
    //  Move to Next Layer
    Gtk::IconSet _move_to_next_layer;
    src.set_icon_name("MoveToNextLayer");
    src.set_filename(get_icon_path("move-to-next-layer.svg"));
    _move_to_next_layer.add_source(src);
    icons->add(Stock::MOVE_TO_NEXT_LAYER, _move_to_next_layer);
    //  Move to Previous Layer
    Gtk::IconSet _move_to_prev_layer;
    src.set_icon_name("MoveToPrevLayer");
    src.set_filename(get_icon_path("move-to-prev-layer.svg"));
    _move_to_prev_layer.add_source(src);
    icons->add(Stock::MOVE_TO_PREV_LAYER, _move_to_prev_layer);
    //  Move to Top Layer
    Gtk::IconSet _move_to_top_layer;
    src.set_icon_name("MoveToTopLayer");
    src.set_filename(get_icon_path("move-to-top-layer.svg"));
    _move_to_top_layer.add_source(src);
    icons->add(Stock::MOVE_TO_TOP_LAYER, _move_to_top_layer);
    //  Move to Bottom Layer
    Gtk::IconSet _move_to_bottom_layer;
    src.set_icon_name("MoveToBottomLayer");
    src.set_filename(get_icon_path("move-to-bottom-layer.svg"));
    _move_to_bottom_layer.add_source(src);
    icons->add(Stock::MOVE_TO_BOTTOM_LAYER, _move_to_bottom_layer);
    //  Rotate 90 CW
    Gtk::IconSet _rotate_90_cw;
    src.set_icon_name("Rotate90CW");
    src.set_filename(get_icon_path("rotate-90-cw.svg"));
    _rotate_90_cw.add_source(src);
    icons->add(Stock::ROTATE_90_CW, _rotate_90_cw);
    //  Rotate 90 CCW
    Gtk::IconSet _rotate_90_ccw;
    src.set_icon_name("Rotate90CCW");
    src.set_filename(get_icon_path("rotate-90-ccw.svg"));
    _rotate_90_ccw.add_source(src);
    icons->add(Stock::ROTATE_90_CCW, _rotate_90_ccw);
    //  Flip Horizontal
    Gtk::IconSet _flip_horiz;
    src.set_icon_name("FlipHoriz");
    src.set_filename(get_icon_path("flip-horiz.svg"));
    _flip_horiz.add_source(src);
    icons->add(Stock::FLIP_HORIZ, _flip_horiz);
    //  Flip Vertical
    Gtk::IconSet _flip_vert;
    src.set_icon_name("FlipVert");
    src.set_filename(get_icon_path("flip-vert.svg"));
    _flip_vert.add_source(src);
    icons->add(Stock::FLIP_VERT, _flip_vert);
    //  Transform
    Gtk::IconSet _transform;
    src.set_icon_name("Transform");
    src.set_filename(get_icon_path("transform.svg"));
    _transform.add_source(src);
    icons->add(Stock::TRANSFORM, _transform);
    //  Transformation
    Gtk::IconSet _transformation;
    src.set_icon_name("Transformation");
    src.set_filename(get_icon_path("transform.svg"));
    _transformation.add_source(src);
    icons->add(Stock::TRANSFORMATION, _transformation);
    //  Align and Distribute
    Gtk::IconSet _align_distribute;
    src.set_icon_name("AlignAndDistribute");
    src.set_filename(get_icon_path("align-distribute.svg"));
    _align_distribute.add_source(src);
    icons->add(Stock::ALIGN_DISTRIBUTE, _align_distribute);

    // Path menu
    //  Object to Path
    Gtk::IconSet _object_to_path;
    src.set_icon_name("ObjectToPath");
    src.set_filename(get_icon_path("object-to-path.svg"));
    _object_to_path.add_source(src);
    icons->add(Stock::OBJECT_TO_PATH, _object_to_path);
    //  Stroke to Path
    Gtk::IconSet _stroke_to_path;
    src.set_icon_name("StrokeToPath");
    src.set_filename(get_icon_path("stroke-to-path.svg"));
    _stroke_to_path.add_source(src);
    icons->add(Stock::STROKE_TO_PATH, _stroke_to_path);
    //  Trace
    Gtk::IconSet _trace;
    src.set_icon_name("Trace");
    src.set_filename(get_icon_path("trace.svg"));
    _trace.add_source(src);
    icons->add(Stock::TRACE, _trace);
    //  Union 
    Gtk::IconSet _union;
    src.set_icon_name("Union");
    src.set_filename(get_icon_path("union.svg"));
    _union.add_source(src);
    icons->add(Stock::UNION, _union);
    //  Difference
    Gtk::IconSet _difference;
    src.set_icon_name("Difference");
    src.set_filename(get_icon_path("difference.svg"));
    _difference.add_source(src);
    icons->add(Stock::DIFFERENCE, _difference);
    //  Intersection
    Gtk::IconSet _intersection;
    src.set_icon_name("Intersection");
    src.set_filename(get_icon_path("intersection.svg"));
    _intersection.add_source(src);
    icons->add(Stock::INTERSECTION, _intersection);
    //  Exclusion
    Gtk::IconSet _exclusion;
    src.set_icon_name("Exclusion");
    src.set_filename(get_icon_path("exclusion.svg"));
    _exclusion.add_source(src);
    icons->add(Stock::EXCLUSION, _exclusion);
    //  Division
    Gtk::IconSet _division;
    src.set_icon_name("Division");
    src.set_filename(get_icon_path("division.svg"));
    _division.add_source(src);
    icons->add(Stock::DIVISION, _division);
    //  Cut Path
    Gtk::IconSet _cut_path;
    src.set_icon_name("CutPath");
    src.set_filename(get_icon_path("cut-path.svg"));
    _cut_path.add_source(src);
    icons->add(Stock::CUT_PATH, _cut_path);
    //  Combine
    Gtk::IconSet _combine;
    src.set_icon_name("Combine");
    src.set_filename(get_icon_path("combine.svg"));
    _combine.add_source(src);
    icons->add(Stock::COMBINE, _combine);
    //  Break Apart
    Gtk::IconSet _break_apart;
    src.set_icon_name("BreakApart");
    src.set_filename(get_icon_path("break-apart.svg"));
    _break_apart.add_source(src);
    icons->add(Stock::BREAK_APART, _break_apart);
    //  Inset
    Gtk::IconSet _inset;
    src.set_icon_name("Inset");
    src.set_filename(get_icon_path("inset.svg"));
    _inset.add_source(src);
    icons->add(Stock::INSET, _inset);
    //  Outset
    Gtk::IconSet _outset;
    src.set_icon_name("Outset");
    src.set_filename(get_icon_path("outset.svg"));
    _outset.add_source(src);
    icons->add(Stock::OUTSET, _outset);
    //  Dynamic Offset
    Gtk::IconSet _offset_dynamic;
    src.set_icon_name("OffsetDynamic");
    src.set_filename(get_icon_path("offset-dynamic.svg"));
    _offset_dynamic.add_source(src);
    icons->add(Stock::OFFSET_DYNAMIC, _offset_dynamic);
    //  Linked Offset
    Gtk::IconSet _offset_linked;
    src.set_icon_name("OffsetLinked");
    src.set_filename(get_icon_path("offset-linked.svg"));
    _offset_linked.add_source(src);
    icons->add(Stock::OFFSET_LINKED, _offset_linked);
    //  Simplify
    Gtk::IconSet _simplify;
    src.set_icon_name("Simplify");
    src.set_filename(get_icon_path("simplify.svg"));
    _simplify.add_source(src);
    icons->add(Stock::SIMPLIFY, _simplify);
    //  Reverse
    Gtk::IconSet _reverse;
    src.set_icon_name("Reverse");
    src.set_filename(get_icon_path("reverse.svg"));
    _reverse.add_source(src);
    icons->add(Stock::REVERSE, _reverse);

    // Help menu
    //  Keys and Mouse
    Gtk::IconSet _keys_mouse;
    src.set_icon_name("KeysAndMouse");
    src.set_filename(get_icon_path("keys-mouse.svg"));
    _keys_mouse.add_source(src);
    icons->add(Stock::KEYS_MOUSE, _keys_mouse);
    //  Tutorials
    Gtk::IconSet _tutorials;
    src.set_icon_name("Tutorials");
    src.set_filename(get_icon_path("tutorials.svg"));
    _tutorials.add_source(src);
    icons->add(Stock::TUTORIALS, _tutorials);
    //  About Inkscape
    Gtk::IconSet _about;
    src.set_icon_name("About");
    src.set_filename(get_icon_path("inkscape.svg"));
    _about.add_source(src);
    icons->add(Stock::ABOUT, _about);
    //  About Splash
    Gtk::IconSet _about_splash;
    src.set_filename(get_icon_path("about41.svg"));
    src.set_size(Gtk::IconSize::register_new("about", 750, 625));
    _about_splash.add_source(src);
    icons->add(Stock::ABOUT_SPLASH, _about_splash);
    //  42 Don't Panic!
    Gtk::IconSet _about_42;
    src.set_filename(get_icon_path("dontpanic.svg"));
    src.set_size(Gtk::IconSize::register_new("dontpanic", 250, 172));
    _about_42.add_source(src);
    icons->add(Stock::ABOUT_42, _about_42);
    //  Quick Help
    Gtk::IconSet _about_quick_help;
    src.set_filename(get_icon_path("quick-help.svg"));
    src.set_size(Gtk::IconSize::from_name("about"));
    _about_quick_help.add_source(src);
    icons->add(Stock::ABOUT_QUICK_HELP, _about_quick_help);

    // Tools toolbar
    //  Select tool
    Gtk::IconSet _tool_select;
    src.set_icon_name("ToolSelect");
    src.set_filename(get_icon_path("tool-select.svg"));
    _tool_select.add_source(src);
    icons->add(Stock::TOOL_SELECT, _tool_select);
    //  Node tool
    Gtk::IconSet _tool_node;
    src.set_icon_name("ToolNode");
    src.set_filename(get_icon_path("tool-node.svg"));
    _tool_node.add_source(src);
    icons->add(Stock::TOOL_NODE, _tool_node);
    //  Zoom tool
    Gtk::IconSet _tool_zoom;
    src.set_icon_name("ToolZoom");
    src.set_filename(get_icon_path("tool-zoom.svg"));
    _tool_zoom.add_source(src);
    icons->add(Stock::TOOL_ZOOM, _tool_zoom);
    //  Rect tool
    Gtk::IconSet _tool_rect;
    src.set_icon_name("ToolRect");
    src.set_filename(get_icon_path("tool-rect.svg"));
    _tool_rect.add_source(src);
    icons->add(Stock::TOOL_RECT, _tool_rect);
    //  Arc tool
    Gtk::IconSet _tool_arc;
    src.set_icon_name("ToolArc");
    src.set_filename(get_icon_path("tool-arc.svg"));
    _tool_arc.add_source(src);
    icons->add(Stock::TOOL_ARC, _tool_arc);
    //  Star tool
    Gtk::IconSet _tool_star;
    src.set_icon_name("ToolStar");
    src.set_filename(get_icon_path("tool-star.svg"));
    _tool_star.add_source(src);
    icons->add(Stock::TOOL_STAR, _tool_star);
    //  Spiral tool
    Gtk::IconSet _tool_spiral;
    src.set_icon_name("ToolSpiral");
    src.set_filename(get_icon_path("tool-spiral.svg"));
    _tool_spiral.add_source(src);
    icons->add(Stock::TOOL_SPIRAL, _tool_spiral);
    //  Freehand tool
    Gtk::IconSet _tool_freehand;
    src.set_icon_name("ToolFreehand");
    src.set_filename(get_icon_path("tool-freehand.svg"));
    _tool_freehand.add_source(src);
    icons->add(Stock::TOOL_FREEHAND, _tool_freehand);
    //  Pen tool
    Gtk::IconSet _tool_pen;
    src.set_icon_name("ToolPen");
    src.set_filename(get_icon_path("tool-pen.svg"));
    _tool_pen.add_source(src);
    icons->add(Stock::TOOL_PEN, _tool_pen);
    //  DynaDraw tool
    Gtk::IconSet _tool_dynadraw;
    src.set_icon_name("ToolDynaDraw");
    src.set_filename(get_icon_path("tool-dynadraw.svg"));
    _tool_dynadraw.add_source(src);
    icons->add(Stock::TOOL_DYNADRAW, _tool_dynadraw);
    //  Text tool
    Gtk::IconSet _tool_text;
    src.set_icon_name("ToolText");
    src.set_filename(get_icon_path("tool-text.svg"));
    _tool_text.add_source(src);
    icons->add(Stock::TOOL_TEXT, _tool_text);
    //  Dropper tool
    Gtk::IconSet _tool_dropper;
    src.set_icon_name("ToolDropper");
    src.set_filename(get_icon_path("tool-dropper.svg"));
    _tool_dropper.add_source(src);
    icons->add(Stock::TOOL_DROPPER, _tool_dropper);

    // Select Tool controls
    Gtk::IconSource small;
    small.set_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
    //  Transform Stroke
    Gtk::IconSet _transform_stroke;
    small.set_icon_name("TransformStroke");
    small.set_filename(get_icon_path("transform-stroke.svg"));
    _transform_stroke.add_source(small);
    icons->add(Stock::TRANSFORM_STROKE, _transform_stroke);
    //  Transform Corners
    Gtk::IconSet _transform_corners;
    small.set_icon_name("TransformCorners");
    small.set_filename(get_icon_path("transform-corners.svg"));
    _transform_corners.add_source(small);
    icons->add(Stock::TRANSFORM_CORNERS, _transform_corners);
    //  Transform Grandient
    Gtk::IconSet _transform_gradient;
    small.set_icon_name("TransformGrandient");
    small.set_filename(get_icon_path("transform-gradient.svg"));
    _transform_gradient.add_source(small);
    icons->add(Stock::TRANSFORM_GRADIENT, _transform_gradient);
    //  Transform Pattern
    Gtk::IconSet _transform_pattern;
    small.set_icon_name("TransformPattern");
    small.set_filename(get_icon_path("transform-pattern.svg"));
    _transform_pattern.add_source(small);
    icons->add(Stock::TRANSFORM_PATTERN, _transform_pattern);

    // Calligraphy Tool controls
    //  Use Pressure
    Gtk::IconSet _use_pressure;
    small.set_icon_name("UsePressure");
    small.set_filename(get_icon_path("use-pressure.svg"));
    _use_pressure.add_source(small);
    icons->add(Stock::USE_PRESSURE, _use_pressure);
    //  Use Tilt
    Gtk::IconSet _use_tilt;
    small.set_icon_name("UseTilt");
    small.set_filename(get_icon_path("use-tilt.svg"));
    _use_tilt.add_source(small);
    icons->add(Stock::USE_TILT, _use_tilt);

	// Session playback dialog
	// Rewind
    Gtk::IconSet _session_rewind;
    src.set_icon_name("Rewind");
    src.set_filename(get_icon_path("session-rew.svg"));
    _session_rewind.add_source(src);
    icons->add(Stock::SESSION_PLAYBACK_REW, _session_rewind);
	// Step backwards
    Gtk::IconSet _session_stepback;
    src.set_icon_name("StepBackward");
    src.set_filename(get_icon_path("session-back1.svg"));
    _session_stepback.add_source(src);
    icons->add(Stock::SESSION_PLAYBACK_STEPBACK, _session_stepback);
	// Pause
    Gtk::IconSet _session_pause;
    src.set_icon_name("Pause");
    src.set_filename(get_icon_path("session-pause.svg"));
    _session_pause.add_source(src);
    icons->add(Stock::SESSION_PLAYBACK_PAUSE, _session_pause);
	// Step forwards
    Gtk::IconSet _session_stepforward;
    src.set_icon_name("StepForward");
    src.set_filename(get_icon_path("session-adv1.svg"));
    _session_stepforward.add_source(src);
    icons->add(Stock::SESSION_PLAYBACK_STEPFORWARD, _session_stepforward);
	// Play
    Gtk::IconSet _session_play;
    src.set_icon_name("Play");
    src.set_filename(get_icon_path("session-play.svg"));
    _session_play.add_source(src);
    icons->add(Stock::SESSION_PLAYBACK_PLAY, _session_play);

    icons->add_default();
}

/** Returns the icon filename (in the operating system encoding used for filenames) whose basename
    in utf8 encoding is \a utf8_basename.
**/
static Glib::ustring const
get_icon_path(char const *const utf8_basename)
{
    /* Given that INKSCAPE_PIXMAPDIR is often a compiled constant, I suppose we should
       interpret it as utf8: under windows, the encoding for filenames can change from
       day to day even for a given file. */
    static char *const opsys_iconsdir = g_filename_from_utf8(INKSCAPE_PIXMAPDIR, -1,
                                                             NULL, NULL, NULL);

    g_assert(g_utf8_validate(utf8_basename, -1, NULL));
    char *const opsys_basename = g_filename_from_utf8(utf8_basename, -1, NULL, NULL, NULL);
    char *const ret_cstr = g_build_filename(opsys_iconsdir, opsys_basename, NULL);
    Glib::ustring const ret(ret_cstr);
    g_free(ret_cstr);
    g_free(opsys_basename);
    return ret;
}

} // namespace Icon
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
