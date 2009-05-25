/**
 * \brief This class implements the functionality of the window layout, menus,
 *        and signals.
 *
 * This is a reimplementation into C++/Gtkmm of Sodipodi's SPDesktopWidget class.
 * Both SPDesktopWidget and EditWidget adhere to the EditWidgetInterface, so
 * they both can serve as widget for the same SPDesktop/Edit class.
 *
 * Ideally, this class should only contain the handling of the Window (i.e.,
 * content construction and window signals) and implement its
 * EditWidgetInterface.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   John Bintz <jcoswell@coswellproductions.org>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 1999-2005 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtkwindow.h>
#include <gtk/gtkversion.h>
#include <gtk/gtklabel.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/menubar.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/accelmap.h>
#include <gtkmm/separator.h>
#include <gtkmm/base.h>

#include <sigc++/functors/mem_fun.h>

#include "macros.h"
#include "path-prefix.h"
#include "preferences.h"
#include "file.h"
#include "application/editor.h"
#include "edit-widget.h"

#include "display/sodipodi-ctrlrect.h"
#include "helper/units.h"
#include "shortcuts.h"
#include "widgets/spw-utilities.h"
#include "event-context.h"
#include "document.h"
#include "sp-namedview.h"
#include "sp-item.h"
#include "interface.h"
#include "extension/db.h"

#include "ui/dialog/dialog-manager.h"

using namespace Inkscape::UI;
using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace View {

EditWidget::EditWidget (SPDocument *doc)
    : _main_window_table(4),
      _viewport_table(3,3),
      _act_grp(Gtk::ActionGroup::create()),
      _ui_mgr(Gtk::UIManager::create()),
      _update_s_f(false),
      _update_a_f(false),
      _interaction_disabled_counter(0)
{
    g_warning("Creating new EditWidget");

    _desktop = 0;
    initActions();
    initAccelMap();
    initUIManager();
    initLayout();
    initEdit (doc);
    g_warning("Done creating new EditWidget");
}

EditWidget::~EditWidget()
{
    destroyEdit();
}

void
EditWidget::initActions()
{
    initMenuActions();
    initToolbarActions();
}

void
EditWidget::initUIManager()
{
    _ui_mgr->insert_action_group(_act_grp);
    add_accel_group(_ui_mgr->get_accel_group());

    gchar *filename_utf8 = g_build_filename(INKSCAPE_UIDIR, "menus-bars.xml", NULL);
    if (_ui_mgr->add_ui_from_file(filename_utf8) == 0) {
        g_warning("Error merging ui from file '%s'", filename_utf8);
        // fixme-charset: What charset should we pass to g_warning?
    }
    g_free(filename_utf8);
}

void
EditWidget::initLayout()
{
    set_title("New document 1 - Inkscape");
    set_resizable();
    set_default_size(640, 480);

    // top level window into which all other portions of the UI get inserted
    add(_main_window_table);
    // attach box for horizontal toolbars
    _main_window_table.attach(_toolbars_vbox, 0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
    // attach sub-window for viewport and vertical toolbars
    _main_window_table.attach(_sub_window_hbox, 0, 1, 2, 3);
    // viewport table with 3 rows by 3 columns
    _sub_window_hbox.pack_end(_viewport_table);

    // Menus and Bars
    initMenuBar();
    initCommandsBar();
    initToolControlsBar();
    initUriBar();
    initToolsBar();

    // Canvas Viewport
    initLeftRuler();
    initTopRuler();
    initStickyZoom();
    initBottomScrollbar();
    initRightScrollbar();
    _viewport_table.attach(_svg_canvas.widget(), 1, 2, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
    _svg_canvas.widget().show_all();

    // The statusbar comes last and appears at the bottom of _main_window_table
    initStatusbar();

    signal_size_allocate().connect (sigc::mem_fun (*this, &EditWidget::onWindowSizeAllocate), false);
    signal_realize().connect (sigc::mem_fun (*this, &EditWidget::onWindowRealize));
    show_all_children();
}

void
EditWidget::onMenuItem()
{
    g_warning("onMenuItem called");
}

void
EditWidget::onActionFileNew()
{
//    g_warning("onActionFileNew called");
    sp_file_new_default();
}

void
EditWidget::onActionFileOpen()
{
//    g_warning("onActionFileOpen called");
    sp_file_open_dialog (*this, NULL, NULL);
}

void
EditWidget::onActionFileQuit()
{
    g_warning("onActionFileQuit");
    sp_ui_close_all();
}

void
EditWidget::onActionFilePrint()
{
    g_warning("onActionFilePrint");
}

void
EditWidget::onToolbarItem()
{
    g_warning("onToolbarItem called");
}

void
EditWidget::onSelectTool()
{
    _tool_ctrl->remove();
    _tool_ctrl->add(*_select_ctrl);
}

void
EditWidget::onNodeTool()
{
    _tool_ctrl->remove();
    _tool_ctrl->add(*_node_ctrl);
}

void
EditWidget::onDialogInkscapePreferences()
{
    _dlg_mgr.showDialog("InkscapePreferences");
}

void
EditWidget::onDialogAbout()
{
}

void
EditWidget::onDialogAlignAndDistribute()
{
    _dlg_mgr.showDialog("AlignAndDistribute");
}

void
EditWidget::onDialogDocumentProperties()
{
//    manage (Inkscape::UI::Dialog::DocumentPreferences::create());
    _dlg_mgr.showDialog("DocumentPreferences");
}

void
EditWidget::onDialogExport()
{
    _dlg_mgr.showDialog("Export");
}

void
EditWidget::onDialogExtensionEditor()
{
    _dlg_mgr.showDialog("ExtensionEditor");
}

void
EditWidget::onDialogFillAndStroke()
{
    _dlg_mgr.showDialog("FillAndStroke");
}

void
EditWidget::onDialogFind()
{
    _dlg_mgr.showDialog("Find");
}

void
EditWidget::onDialogLayerEditor()
{
    _dlg_mgr.showDialog("LayerEditor");
}

void
EditWidget::onDialogMessages()
{
    _dlg_mgr.showDialog("Messages");
}

void
EditWidget::onDialogObjectProperties()
{
    _dlg_mgr.showDialog("ObjectProperties");
}

void
EditWidget::onDialogTextProperties()
{
    _dlg_mgr.showDialog("TextProperties");
}

void
EditWidget::onDialogTrace()
{
}

void
EditWidget::onDialogTransformation()
{
    _dlg_mgr.showDialog("Transformation");
}

void
EditWidget::onDialogXmlEditor()
{
    _dlg_mgr.showDialog("XmlEditor");
}

void
EditWidget::onUriChanged()
{
    g_message("onUriChanged called");

}

// FIXME: strings are replaced by placeholders, NOT to be translated until the code is enabled
// See http://sourceforge.net/mailarchive/message.php?msg_id=11746016 for details

void
EditWidget::initMenuActions()
{
// This has no chance of working right now.
// Has to be converted to Gtk::Action::create_with_icon_name.

    _act_grp->add(Gtk::Action::create("MenuFile",   "File"));
    _act_grp->add(Gtk::Action::create("MenuEdit",   "Edit"));
    _act_grp->add(Gtk::Action::create("MenuView",   "View"));
    _act_grp->add(Gtk::Action::create("MenuLayer",  "Layer"));
    _act_grp->add(Gtk::Action::create("MenuObject", "Object"));
    _act_grp->add(Gtk::Action::create("MenuPath",   "Path"));
    _act_grp->add(Gtk::Action::create("MenuText",   "Text"));
    _act_grp->add(Gtk::Action::create("MenuHelp",   "Help"));

    // File menu
    _act_grp->add(Gtk::Action::create("New",
                                      Gtk::Stock::NEW, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFileNew));

    _act_grp->add(Gtk::Action::create("Open",
                                      Gtk::Stock::OPEN, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));
/*
    _act_grp->add(Gtk::Action::create("OpenRecent",
                                      Stock::OPEN_RECENT));*/

    _act_grp->add(Gtk::Action::create("Revert",
                                      Gtk::Stock::REVERT_TO_SAVED, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Save",
                                      Gtk::Stock::SAVE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("SaveAs",
                                      Gtk::Stock::SAVE_AS, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));
/*
    _act_grp->add(Gtk::Action::create("Import",
                                      Stock::IMPORT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Export",
                                      Stock::EXPORT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogExport));*/

    _act_grp->add(Gtk::Action::create("Print",
                                      Gtk::Stock::PRINT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onActionFilePrint));

    _act_grp->add(Gtk::Action::create("PrintPreview",
                                      Gtk::Stock::PRINT_PREVIEW),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));
/*
    _act_grp->add(Gtk::Action::create("VacuumDefs",
                                      Stock::VACUUM_DEFS),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));*/

    _act_grp->add(Gtk::Action::create("DocumentProperties",
                                      Gtk::Stock::PROPERTIES, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogDocumentProperties));

    _act_grp->add(Gtk::Action::create("InkscapePreferences",
                                      Gtk::Stock::PREFERENCES, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogInkscapePreferences));

    _act_grp->add(Gtk::Action::create("Close",
                                      Gtk::Stock::CLOSE),
                  sigc::mem_fun(*this, &EditWidget::onActionFileOpen));

    _act_grp->add(Gtk::Action::create("Quit",
                                      Gtk::Stock::QUIT),
                  sigc::mem_fun(*this, &EditWidget::onActionFileQuit));

    // EditWidget menu
    _act_grp->add(Gtk::Action::create("Undo",
                                      Gtk::Stock::UNDO, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Redo",
                                      Gtk::Stock::REDO, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));
/*
    _act_grp->add(Gtk::Action::create("UndoHistory",
                                      Stock::UNDO_HISTORY, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));*/

    _act_grp->add(Gtk::Action::create("Cut",
                                      Gtk::Stock::CUT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Copy",
                                      Gtk::Stock::COPY, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Paste",
                                      Gtk::Stock::PASTE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));
/*
    _act_grp->add(Gtk::Action::create("PasteInPlace",
                                      Stock::PASTE_IN_PLACE));

    _act_grp->add(Gtk::Action::create("PasteStyle",
                                      Stock::PASTE_STYLE));*/

    _act_grp->add(Gtk::Action::create("Find",
                                      Gtk::Stock::FIND),
                  sigc::mem_fun(*this, &EditWidget::onDialogFind));
/*
    _act_grp->add(Gtk::Action::create("Duplicate",
                                      Stock::DUPLICATE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Clone",
                                      Stock::CLONE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("CloneUnlink",
                                      Stock::CLONE_UNLINK, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("CloneSelectOrig",
                                      Stock::CLONE_SELECT_ORIG));

    _act_grp->add(Gtk::Action::create("MakeBitmap",
                                      Stock::MAKE_BITMAP));

    _act_grp->add(Gtk::Action::create("Tile",
                                     Stock::TILE));

    _act_grp->add(Gtk::Action::create("Untile",
                                      Stock::UNTILE));

    _act_grp->add(Gtk::Action::create("Delete",
                                      Gtk::Stock::DELETE));

    _act_grp->add(Gtk::Action::create("SelectAll",
                                      Stock::SELECT_ALL));

    _act_grp->add(Gtk::Action::create("SelectAllInAllLayers",
                                      Stock::SELECT_ALL_IN_ALL_LAYERS));

    _act_grp->add(Gtk::Action::create("SelectInvert",
                                      Stock::SELECT_INVERT));

    _act_grp->add(Gtk::Action::create("SelectNone",
                                      Stock::SELECT_NONE));

    _act_grp->add(Gtk::Action::create("XmlEditor",
                                      Stock::XML_EDITOR, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogXmlEditor));

    // View menu
    _act_grp->add(Gtk::Action::create("Zoom",
                                      Stock::ZOOM));

    _act_grp->add(Gtk::Action::create("ZoomIn",
                                      Stock::ZOOM_IN, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomOut",
                                      Stock::ZOOM_OUT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Zoom100",
                                      Stock::ZOOM_100, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Zoom50",
                                      Stock::ZOOM_50, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Zoom200",
                                      Stock::ZOOM_200, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomSelection",
                                      Stock::ZOOM_SELECTION, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomDrawing",
                                      Stock::ZOOM_DRAWING, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomPage",
                                      Stock::ZOOM_PAGE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomWidth",
                                      Stock::ZOOM_WIDTH, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomPrev",
                                      Stock::ZOOM_PREV, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ZoomNext",
                                      Stock::ZOOM_NEXT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("ShowHide",
                                      Stock::SHOW_HIDE));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideCommandsBar",
                                            Stock::SHOW_HIDE_COMMANDS_BAR));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideToolControlsBar",
                                            Stock::SHOW_HIDE_TOOL_CONTROLS_BAR));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideToolsBar",
                                            Stock::SHOW_HIDE_TOOLS_BAR));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideRulers",
                                            Stock::SHOW_HIDE_RULERS));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideScrollbars",
                                            Stock::SHOW_HIDE_SCROLLBARS));

    _act_grp->add(Gtk::ToggleAction::create("ShowHideStatusbar",
                                            Stock::SHOW_HIDE_STATUSBAR));

    _act_grp->add(Gtk::Action::create("ShowHideDialogs",
                                      Stock::SHOW_HIDE_DIALOGS));

    _act_grp->add(Gtk::Action::create("Grid",
                                      Stock::GRID));

    _act_grp->add(Gtk::Action::create("Guides",
                                      Stock::GUIDES));

    _act_grp->add(Gtk::Action::create("Fullscreen",
                                      Stock::FULLSCREEN));

    _act_grp->add(Gtk::Action::create("Messages",
                                      Stock::MESSAGES),
                  sigc::mem_fun(*this, &EditWidget::onDialogMessages));

    _act_grp->add(Gtk::Action::create("Scripts",
                                      Stock::SCRIPTS));

    _act_grp->add(Gtk::Action::create("WindowPrev",
                                      Stock::WINDOW_PREV));

    _act_grp->add(Gtk::Action::create("WindowNext",
                                      Stock::WINDOW_NEXT));

    _act_grp->add(Gtk::Action::create("WindowDuplicate",
                                      Stock::WINDOW_DUPLICATE));

    // Layer menu
    _act_grp->add(Gtk::Action::create("LayerNew",
                                      Stock::LAYER_NEW));

    _act_grp->add(Gtk::Action::create("LayerRename",
                                      Stock::LAYER_RENAME));

    _act_grp->add(Gtk::Action::create("LayerDuplicate",
                                      Stock::LAYER_DUPLICATE));

    _act_grp->add(Gtk::Action::create("LayerAnchor",
                                      Stock::LAYER_ANCHOR));

    _act_grp->add(Gtk::Action::create("LayerMergeDown",
                                      Stock::LAYER_MERGE_DOWN));

    _act_grp->add(Gtk::Action::create("LayerDelete",
                                      Stock::LAYER_DELETE));

    _act_grp->add(Gtk::Action::create("LayerSelectNext",
                                      Stock::LAYER_SELECT_NEXT));

    _act_grp->add(Gtk::Action::create("LayerSelectPrev",
                                      Stock::LAYER_SELECT_PREV));

    _act_grp->add(Gtk::Action::create("LayerSelectTop",
                                      Stock::LAYER_SELECT_TOP));

    _act_grp->add(Gtk::Action::create("LayerSelectBottom",
                                      Stock::LAYER_SELECT_BOTTOM));

    _act_grp->add(Gtk::Action::create("LayerRaise",
                                      Stock::LAYER_RAISE));

    _act_grp->add(Gtk::Action::create("LayerLower",
                                      Stock::LAYER_LOWER));

    _act_grp->add(Gtk::Action::create("LayerToTop",
                                      Stock::LAYER_TO_TOP));

    _act_grp->add(Gtk::Action::create("LayerToBottom",
                                      Stock::LAYER_TO_BOTTOM));

    // Object menu
    _act_grp->add(Gtk::Action::create("FillAndStroke",
                                      Stock::FILL_STROKE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogFillAndStroke));

    _act_grp->add(Gtk::Action::create("ObjectProperties",
                                      Stock::OBJECT_PROPERTIES),
                  sigc::mem_fun(*this, &EditWidget::onDialogObjectProperties));

    _act_grp->add(Gtk::Action::create("FilterEffects",
                                      Stock::FILTER_EFFECTS));

    _act_grp->add(Gtk::Action::create("Group",
                                      Stock::GROUP, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Ungroup",
                                      Stock::UNGROUP, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Raise",
                                      Stock::RAISE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Lower",
                                      Stock::LOWER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("RaiseToTop",
                                      Stock::RAISE_TO_TOP, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("LowerToBottom",
                                      Stock::LOWER_TO_BOTTOM, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("MoveToNewLayer",
                                      Stock::MOVE_TO_NEW_LAYER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("MoveToNextLayer",
                                      Stock::MOVE_TO_NEXT_LAYER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("MoveToPrevLayer",
                                      Stock::MOVE_TO_PREV_LAYER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("MoveToTopLayer",
                                      Stock::MOVE_TO_TOP_LAYER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("MoveToBottomLayer",
                                      Stock::MOVE_TO_BOTTOM_LAYER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Rotate90CW",
                                      Stock::ROTATE_90_CW, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Rotate90CCW",
                                      Stock::ROTATE_90_CCW, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("FlipHoriz",
                                      Stock::FLIP_HORIZ, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("FlipVert",
                                      Stock::FLIP_VERT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Transformation",
                                      Stock::TRANSFORMATION, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogTransformation));

    _act_grp->add(Gtk::Action::create("AlignAndDistribute",
                                      Stock::ALIGN_DISTRIBUTE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogAlignAndDistribute));

    // Path menu
    _act_grp->add(Gtk::Action::create("ObjectToPath",
                                      Stock::OBJECT_TO_PATH, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("StrokeToPath",
                                      Stock::STROKE_TO_PATH, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("Trace",
                                      Stock::TRACE),
                  sigc::mem_fun(*this, &EditWidget::onDialogTrace));

    _act_grp->add(Gtk::Action::create("Union",
                                      Stock::UNION));

    _act_grp->add(Gtk::Action::create("Difference",
                                      Stock::DIFFERENCE));

    _act_grp->add(Gtk::Action::create("Intersection",
                                      Stock::INTERSECTION));

    _act_grp->add(Gtk::Action::create("Exclusion",
                                      Stock::EXCLUSION));

    _act_grp->add(Gtk::Action::create("Division",
                                      Stock::DIVISION));

    _act_grp->add(Gtk::Action::create("CutPath",
                                      Stock::CUT_PATH));

    _act_grp->add(Gtk::Action::create("Combine",
                                      Stock::COMBINE));

    _act_grp->add(Gtk::Action::create("BreakApart",
                                      Stock::BREAK_APART));

    _act_grp->add(Gtk::Action::create("Inset",
                                      Stock::INSET));

    _act_grp->add(Gtk::Action::create("Outset",
                                      Stock::OUTSET));

    _act_grp->add(Gtk::Action::create("OffsetDynamic",
                                      Stock::OFFSET_DYNAMIC));

    _act_grp->add(Gtk::Action::create("OffsetLinked",
                                      Stock::OFFSET_LINKED));

    _act_grp->add(Gtk::Action::create("Simplify",
                                      Stock::SIMPLIFY));

    _act_grp->add(Gtk::Action::create("Reverse",
                                      Stock::REVERSE));*/

    _act_grp->add(Gtk::Action::create("Cleanup",
                                      Gtk::Stock::CLEAR,
                                      _("PLACEHOLDER, do not translate")));

    // Text menu
    _act_grp->add(Gtk::Action::create("TextProperties",
                                      Gtk::Stock::SELECT_FONT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onDialogTextProperties));
/*
    _act_grp->add(Gtk::Action::create("PutOnPath",
                                      Stock::PUT_ON_PATH));

    _act_grp->add(Gtk::Action::create("RemoveFromPath",
                                      Stock::REMOVE_FROM_PATH));

    _act_grp->add(Gtk::Action::create("RemoveManualKerns",
                                      Stock::REMOVE_MANUAL_KERNS));*/

	// Whiteboard menu
#ifdef WITH_INKBOARD
#endif

    // About menu
/*
    _act_grp->add(Gtk::Action::create("KeysAndMouse",
                                      Stock::KEYS_MOUSE));

    _act_grp->add(Gtk::Action::create("Tutorials",
                                      Stock::TUTORIALS));

    _act_grp->add(Gtk::Action::create("About",
                                      Stock::ABOUT),
                  sigc::mem_fun(*this, &EditWidget::onDialogAbout));*/
}

void
EditWidget::initToolbarActions()
{
    // Tools bar
    // This has zero chance to work, and has to be converted to create_with_icon_name
    Gtk::RadioAction::Group tools;
/*
    _act_grp->add(Gtk::RadioAction::create(tools, "ToolSelect",
                                           Stock::TOOL_SELECT, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onSelectTool));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolNode",
                                           Stock::TOOL_NODE, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")),
                  sigc::mem_fun(*this, &EditWidget::onNodeTool));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolZoom",
                                           Stock::TOOL_ZOOM, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolRect",
                                           Stock::TOOL_RECT, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolArc",
                                           Stock::TOOL_ARC, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolStar",
                                           Stock::TOOL_STAR, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolSpiral",
                                           Stock::TOOL_SPIRAL, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolFreehand",
                                           Stock::TOOL_FREEHAND, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolPen",
                                           Stock::TOOL_PEN, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolDynaDraw",
                                           Stock::TOOL_DYNADRAW, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolText",
                                           Stock::TOOL_TEXT, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::RadioAction::create(tools, "ToolDropper",
                                           Stock::TOOL_DROPPER, Glib::ustring(),
                                           _("PLACEHOLDER, do not translate")));

    // Select Controls bar
    _act_grp->add(Gtk::ToggleAction::create("TransformStroke",
                                            Stock::TRANSFORM_STROKE, Glib::ustring(),
                                            _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::ToggleAction::create("TransformCorners",
                                            Stock::TRANSFORM_CORNERS, Glib::ustring(),
                                            _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::ToggleAction::create("TransformGradient",
                                            Stock::TRANSFORM_GRADIENT, Glib::ustring(),
                                            _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::ToggleAction::create("TransformPattern",
                                            Stock::TRANSFORM_PATTERN, Glib::ustring(),
                                            _("PLACEHOLDER, do not translate")));*/

    // Node Controls bar
    _act_grp->add(Gtk::Action::create("NodeInsert",
                                      Gtk::Stock::ADD, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeDelete",
                                      Gtk::Stock::REMOVE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));
/*
    _act_grp->add(Gtk::Action::create("NodeJoin",
                                      Stock::NODE_JOIN, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeJoinSegment",
                                      Stock::NODE_JOIN_SEGMENT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeDeleteSegment",
                                      Stock::NODE_DELETE_SEGMENT, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeBreak",
                                      Stock::NODE_BREAK, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeCorner",
                                      Stock::NODE_CORNER, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeSmooth",
                                      Stock::NODE_SMOOTH, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeSymmetric",
                                      Stock::NODE_SYMMETRIC, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeLine",
                                      Stock::NODE_LINE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));

    _act_grp->add(Gtk::Action::create("NodeCurve",
                                      Stock::NODE_CURVE, Glib::ustring(),
                                      _("PLACEHOLDER, do not translate")));*/
}

void
EditWidget::initAccelMap()
{
    gchar *filename = g_build_filename(INKSCAPE_UIDIR, "keybindings.rc", NULL);
    Gtk::AccelMap::load(filename);
    g_free(filename);

    // One problem is that the keys 1-6 are zoom accelerators which get
    // caught as accelerator _before_ any Entry input handler receives them,
    // for example the zoom status. At the moment, the best way seems to
    // disable them as accelerators when the Entry gets focus, and enable
    // them when focus goes elsewhere. The code for this belongs here,
    // and not in zoom-status.cpp .

    _zoom_status.signal_focus_in_event().connect (sigc::mem_fun (*this, &EditWidget::onEntryFocusIn));
    _zoom_status.signal_focus_out_event().connect (sigc::mem_fun (*this, &EditWidget::onEntryFocusOut));
}

bool
EditWidget::onEntryFocusIn (GdkEventFocus* /*ev*/)
{
    Gdk::ModifierType m = static_cast<Gdk::ModifierType>(0);
    Gtk::AccelMap::change_entry ("<Actions>//Zoom100", 0, m, false);
    Gtk::AccelMap::change_entry ("<Actions>//Zoom50", 0, m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomSelection", 0, m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomDrawing", 0, m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomPage", 0, m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomWidth", 0, m, false);
    return false;
}

bool
EditWidget::onEntryFocusOut (GdkEventFocus* /*ev*/)
{
    Gdk::ModifierType m = static_cast<Gdk::ModifierType>(0);
    Gtk::AccelMap::change_entry ("<Actions>//Zoom100", '1', m, false);
    Gtk::AccelMap::change_entry ("<Actions>//Zoom50", '2', m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomSelection", '3', m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomDrawing", '4', m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomPage", '5', m, false);
    Gtk::AccelMap::change_entry ("<Actions>//ZoomWidth", '6', m, false);
    return false;
}

void
EditWidget::initMenuBar()
{
    g_assert(_ui_mgr);
    Gtk::MenuBar *menu = static_cast<Gtk::MenuBar*>(_ui_mgr->get_widget("/MenuBar"));
    g_assert(menu != NULL);
    _main_window_table.attach(*Gtk::manage(menu), 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
}

void
EditWidget::initCommandsBar()
{
    g_assert(_ui_mgr);
    Toolbox *bar = new Toolbox(static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/CommandsBar")),
                               Gtk::TOOLBAR_ICONS);
    g_assert(bar != NULL);
    _toolbars_vbox.pack_start(*Gtk::manage(bar), Gtk::PACK_SHRINK);
}

void
EditWidget::initToolControlsBar()
{
    // TODO: Do UIManager controlled widgets need to be deleted?
    _select_ctrl = static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/SelectControlsBar"));
    _node_ctrl = static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/NodeControlsBar"));

    _tool_ctrl = new Toolbox(_select_ctrl, Gtk::TOOLBAR_ICONS);

    _toolbars_vbox.pack_start(*Gtk::manage(_tool_ctrl), Gtk::PACK_SHRINK);
}

void
EditWidget::initUriBar()
{
    /// \todo  Create an Inkscape::UI::Widget::UriBar class (?)

    _uri_ctrl = new Gtk::Toolbar();

    _uri_label.set_label(_("PLACEHOLDER, do not translate"));
    _uri_ctrl->add(_uri_label);
    _uri_ctrl->add(_uri_entry);

    _uri_entry.signal_activate()
        .connect_notify(sigc::mem_fun(*this, &EditWidget::onUriChanged));

    _toolbars_vbox.pack_start(*Gtk::manage(_uri_ctrl), Gtk::PACK_SHRINK);
}

void
EditWidget::initToolsBar()
{
    Toolbox *bar = new Toolbox(static_cast<Gtk::Toolbar*>(_ui_mgr->get_widget("/ToolsBar")),
                               Gtk::TOOLBAR_ICONS,
                               Gtk::ORIENTATION_VERTICAL);
    g_assert(bar != NULL);
    _sub_window_hbox.pack_start(*Gtk::manage(bar), Gtk::PACK_SHRINK);
}

void
EditWidget::initTopRuler()
{
    _viewport_table.attach(_top_ruler,  1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);

    _tooltips.set_tip (_top_ruler, _top_ruler.get_tip());
}

void
EditWidget::initLeftRuler()
{
    _viewport_table.attach(_left_ruler, 0, 1, 1, 2, Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);

    _tooltips.set_tip (_left_ruler, _left_ruler.get_tip());
}

void
EditWidget::initBottomScrollbar()
{
    _viewport_table.attach(_bottom_scrollbar, 1, 2, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
    _bottom_scrollbar.signal_value_changed().connect (sigc::mem_fun (*this, &EditWidget::onAdjValueChanged));
    _bottom_scrollbar.property_adjustment() = new Gtk::Adjustment (0.0, -4000.0, 4000.0, 10.0, 100.0, 4.0);
}

void
EditWidget::initRightScrollbar()
{
    _viewport_table.attach(_right_scrollbar, 2, 3, 1, 2, Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);

    _right_scrollbar.signal_value_changed().connect (sigc::mem_fun (*this, &EditWidget::onAdjValueChanged));
    _right_scrollbar.property_adjustment() = new Gtk::Adjustment (0.0, -4000.0, 4000.0, 10.0, 100.0, 4.0);
}

void
EditWidget::initStickyZoom()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _viewport_table.attach(_sticky_zoom, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _sticky_zoom.set_active (prefs->getBool("/options/stickyzoom/value"));
    _tooltips.set_tip (_sticky_zoom, _("Zoom drawing if window size changes"));

    /// \todo icon not implemented
}

void
EditWidget::initStatusbar()
{
    _statusbar.pack_start (_selected_style_status, false, false, 1);
    _statusbar.pack_start (*new Gtk::VSeparator(), false, false, 0);

    _tooltips.set_tip (_zoom_status, _("Zoom"));

    _layer_selector.reference();
    _statusbar.pack_start (_layer_selector, false, false, 1);

    _coord_status.property_n_rows() = 2;
    _coord_status.property_n_columns() = 5;
    _coord_status.property_row_spacing() = 0;
    _coord_status.property_column_spacing() = 2;
    _coord_eventbox.add (_coord_status);
    _tooltips.set_tip (_coord_eventbox, _("Cursor coordinates"));
    _coord_status.attach (*new Gtk::VSeparator(), 0,1, 0,2, Gtk::FILL,Gtk::FILL, 0,0);
    _coord_status.attach (*new Gtk::Label("X:", 0.0, 0.5), 1,2, 0,1, Gtk::FILL,Gtk::FILL, 0,0);
    _coord_status.attach (*new Gtk::Label("Y:", 0.0, 0.5), 1,2, 1,2, Gtk::FILL,Gtk::FILL, 0,0);
    _coord_status_x.set_text ("0.0");
    _coord_status_x.set_alignment (0.0, 0.5);
    _coord_status_y.set_text ("0.0");
    _coord_status_y.set_alignment (0.0, 0.5);
    _coord_status.attach (_coord_status_x, 2,3, 0,1, Gtk::FILL,Gtk::FILL, 0,0);
    _coord_status.attach (_coord_status_y, 2,3, 1,2, Gtk::FILL,Gtk::FILL, 0,0);
    _coord_status.attach (*new Gtk::Label("Z:", 0.0, 0.5), 3,4, 0,2, Gtk::FILL,Gtk::FILL, 0,0);
    _coord_status.attach (_zoom_status, 4,5, 0,2, Gtk::FILL,Gtk::FILL, 0,0);
    sp_set_font_size_smaller (static_cast<GtkWidget*>((void*)_coord_status.gobj()));
    _statusbar.pack_end (_coord_eventbox, false, false, 1);

    _select_status.property_xalign() = 0.0;
    _select_status.property_yalign() = 0.5;
    _select_status.set_markup (_("<b>Welcome to Inkscape!</b> Use shape or drawing tools to create objects; use selector (arrow) to move or transform them."));
    // include this again with Gtk+-2.6
#if GTK_VERSION_GE(2,6)
     gtk_label_set_ellipsize (GTK_LABEL(_select_status.gobj()), PANGO_ELLIPSIZE_END);
#endif
    _select_status.set_size_request (1, -1);
    _statusbar.pack_start (_select_status, true, true, 0);

    _main_window_table.attach(_statusbar, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK);
}

//========================================
//----------implements EditWidgetInterface

Gtk::Window *
EditWidget::getWindow()
{
    return this;
}

void
EditWidget::setTitle (gchar const* new_title)
{
    set_title (new_title);
}

void
EditWidget::layout()
{
   show_all_children();
}

void
EditWidget::present()
{
    this->Gtk::Window::present();
}

void
EditWidget::getGeometry (gint &x, gint &y, gint &w, gint &h)
{
    get_position (x, y);
    get_size (w, h);
}

void
EditWidget::setSize (gint w, gint h)
{
    resize (w, h);
}

void
EditWidget::setPosition (Geom::Point p)
{
    move (int(p[Geom::X]), int(p[Geom::Y]));
}

/// \param p is already gobj()!
void
EditWidget::setTransient (void* p, int i)
{
    gtk_window_set_transient_for (static_cast<GtkWindow*>(p), this->gobj());
    if (i==2)
        this->Gtk::Window::present();
}

Geom::Point
EditWidget::getPointer()
{
    int x, y;
    get_pointer (x, y);
    return Geom::Point (x, y);
}

void
EditWidget::setIconified()
{
    iconify();
}

void
EditWidget::setMaximized()
{
    maximize();
}

void
EditWidget::setFullscreen()
{
    fullscreen();
}

/**
 *  Shuts down the desktop object for the view being closed.  It checks
 *  to see if the document has been edited, and if so prompts the user
 *  to save, discard, or cancel.  Returns TRUE if the shutdown operation
 *  is cancelled or if the save is cancelled or fails, FALSE otherwise.
 */
bool
EditWidget::shutdown()
{
    g_assert (_desktop != NULL);
    if (Inkscape::NSApplication::Editor::isDuplicatedView (_desktop))
        return false;

    SPDocument *doc = _desktop->doc();
    if (doc->isModifiedSinceSave()) {
        gchar *markup;
        /// \todo FIXME !!! obviously this will have problems if the document
        /// name contains markup characters
        markup = g_strdup_printf(
                _("<span weight=\"bold\" size=\"larger\">Save changes to document \"%s\" before closing?</span>\n\n"
                  "If you close without saving, your changes will be discarded."),
                SP_DOCUMENT_NAME(doc));

        Gtk::MessageDialog dlg (*this,
                       markup,
                       true,
                       Gtk::MESSAGE_WARNING,
                       Gtk::BUTTONS_NONE,
                       true);
        g_free(markup);
        Gtk::Button close_button (_("Close _without saving"), true);
        dlg.add_action_widget (close_button, Gtk::RESPONSE_NO);
        close_button.show();
        dlg.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        dlg.add_button (Gtk::Stock::SAVE, Gtk::RESPONSE_YES);
        dlg.set_default_response (Gtk::RESPONSE_YES);

        int response = dlg.run();
        switch (response)
        {
            case Gtk::RESPONSE_YES:
                sp_document_ref(doc);
                sp_namedview_document_from_window(_desktop);
                if (sp_file_save_document(*this, doc)) {
                    sp_document_unref(doc);
                } else { // save dialog cancelled or save failed
                    sp_document_unref(doc);
                    return TRUE;
                }
                break;
            case Gtk::RESPONSE_NO:
                break;
            default: // cancel pressed, or dialog was closed
                return TRUE;
                break;
        }
    }

    /* Code to check data loss */
    bool allow_data_loss = FALSE;
    while (sp_document_repr_root(doc)->attribute("inkscape:dataloss") != NULL && allow_data_loss == FALSE)
    {
        gchar *markup;
        /// \todo FIXME !!! obviously this will have problems if the document
        /// name contains markup characters
        markup = g_strdup_printf(
                _("<span weight=\"bold\" size=\"larger\">The file \"%s\" was saved with a format (%s) that may cause data loss!</span>\n\n"
                  "Do you want to save this file as an Inkscape SVG?"),
                SP_DOCUMENT_NAME(doc),
                Inkscape::Extension::db.get(sp_document_repr_root(doc)->attribute("inkscape:output_extension"))->get_name());

        Gtk::MessageDialog dlg (*this,
                       markup,
                       true,
                       Gtk::MESSAGE_WARNING,
                       Gtk::BUTTONS_NONE,
                       true);
        g_free(markup);
        Gtk::Button close_button (_("Close _without saving"), true);
        dlg.add_action_widget (close_button, Gtk::RESPONSE_NO);
        close_button.show();
        Gtk::Button save_button (_("_Save as SVG"), true);
        dlg.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        dlg.add_action_widget (save_button, Gtk::RESPONSE_YES);
        save_button.show();
        dlg.set_default_response (Gtk::RESPONSE_YES);

        int response = dlg.run();

        switch (response)
        {
            case Gtk::RESPONSE_YES:
                sp_document_ref(doc);
                if (sp_file_save_document(*this, doc)) {
                    sp_document_unref(doc);
                } else { // save dialog cancelled or save failed
                    sp_document_unref(doc);
                    return TRUE;
                }
                break;
            case Gtk::RESPONSE_NO:
                allow_data_loss = TRUE;
                break;
            default: // cancel pressed, or dialog was closed
                return TRUE;
                break;
        }
    }

    return false;
}


void
EditWidget::destroy()
{
    delete this;
}

void
EditWidget::requestCanvasUpdate()
{
    _svg_canvas.widget().queue_draw();
}

void
EditWidget::requestCanvasUpdateAndWait()
{
    requestCanvasUpdate();

    while (gtk_events_pending())
      gtk_main_iteration_do(FALSE);
}

void
EditWidget::enableInteraction()
{
  g_return_if_fail(_interaction_disabled_counter > 0);

  _interaction_disabled_counter--;

  if (_interaction_disabled_counter == 0) {
    this->set_sensitive(true);
  }
}

void
EditWidget::disableInteraction()
{
  if (_interaction_disabled_counter == 0) {
    this->set_sensitive(false);
  }

  _interaction_disabled_counter++;
}

void
EditWidget::activateDesktop()
{
    /// \todo active_desktop_indicator not implemented
}

void
EditWidget::deactivateDesktop()
{
    /// \todo active_desktop_indicator not implemented
}

void
EditWidget::viewSetPosition (Geom::Point p)
{
    // p -= _namedview->gridorigin;
    /// \todo Why was the origin corrected for the grid origin? (johan)

    double lo, up, pos, max;
    _top_ruler.get_range (lo, up, pos, max);
    _top_ruler.set_range (lo, up, p[Geom::X], max);
    _left_ruler.get_range (lo, up, pos, max);
    _left_ruler.set_range (lo, up, p[Geom::Y], max);
}

void
EditWidget::updateRulers()
{
    //Geom::Point gridorigin = _namedview->gridorigin;
    /// \todo Why was the origin corrected for the grid origin? (johan)

    Geom::Rect const viewbox = _svg_canvas.spobj()->getViewbox();
    double lo, up, pos, max;
    double const scale = _desktop->current_zoom();
    double s = viewbox.min()[Geom::X] / scale; //- gridorigin[Geom::X];
    double e = viewbox.max()[Geom::X] / scale; //- gridorigin[Geom::X];
    _top_ruler.get_range(lo, up, pos, max);
    _top_ruler.set_range(s, e, pos, e);
    s = viewbox.min()[Geom::Y] / -scale; //- gridorigin[Geom::Y];
    e = viewbox.max()[Geom::Y] / -scale; //- gridorigin[Geom::Y];
    _left_ruler.set_range(s, e, 0 /*gridorigin[Geom::Y]*/, e);
    /// \todo is that correct?
}

void
EditWidget::updateScrollbars (double scale)
{
    // do not call this function before canvas has its size allocated
    if (!is_realized() || _update_s_f) {
        return;
    }

    _update_s_f = true;

    /* The desktop region we always show unconditionally */
    SPDocument *doc = _desktop->doc();
    Geom::Rect darea ( Geom::Point(-sp_document_width(doc), -sp_document_height(doc)),
                     Geom::Point(2 * sp_document_width(doc), 2 * sp_document_height(doc))  );
    SPObject* root = doc->root;
    SPItem* item = SP_ITEM(root);
    Geom::OptRect deskarea = Geom::unify(darea, sp_item_bbox_desktop(item));

    /* Canvas region we always show unconditionally */
    Geom::Rect carea( Geom::Point(deskarea->min()[Geom::X] * scale - 64, deskarea->max()[Geom::Y] * -scale - 64),
                    Geom::Point(deskarea->max()[Geom::X] * scale + 64, deskarea->min()[Geom::Y] * -scale + 64)  );

    Geom::Rect const viewbox = _svg_canvas.spobj()->getViewbox();

    /* Viewbox is always included into scrollable region */
    carea = Geom::unify(carea, viewbox);

    Gtk::Adjustment *adj = _bottom_scrollbar.get_adjustment();
    adj->set_value(viewbox.min()[Geom::X]);
    adj->set_lower(carea.min()[Geom::X]);
    adj->set_upper(carea.max()[Geom::X]);
    adj->set_page_increment(viewbox.dimensions()[Geom::X]);
    adj->set_step_increment(0.1 * (viewbox.dimensions()[Geom::X]));
    adj->set_page_size(viewbox.dimensions()[Geom::X]);

    adj = _right_scrollbar.get_adjustment();
    adj->set_value(viewbox.min()[Geom::Y]);
    adj->set_lower(carea.min()[Geom::Y]);
    adj->set_upper(carea.max()[Geom::Y]);
    adj->set_page_increment(viewbox.dimensions()[Geom::Y]);
    adj->set_step_increment(0.1 * viewbox.dimensions()[Geom::Y]);
    adj->set_page_size(viewbox.dimensions()[Geom::Y]);

    _update_s_f = false;
}

void
EditWidget::toggleRulers()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (_top_ruler.is_visible())
    {
        _top_ruler.hide_all();
        _left_ruler.hide_all();
        prefs->setBool(_desktop->is_fullscreen() ? "/fullscreen/rulers/state" : "/window/rulers/state", false);
    } else {
        _top_ruler.show_all();
        _left_ruler.show_all();
        prefs->setBool(_desktop->is_fullscreen() ? "/fullscreen/rulers/state" : "/window/rulers/state", true);
    }
}

void
EditWidget::toggleScrollbars()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (_bottom_scrollbar.is_visible())
    {
        _bottom_scrollbar.hide_all();
        _right_scrollbar.hide_all();
        prefs->setBool(_desktop->is_fullscreen() ? "/fullscreen/scrollbars/state" : "/window/scrollbars/state", false);
    } else {
        _bottom_scrollbar.show_all();
        _right_scrollbar.show_all();
        prefs->setBool(_desktop->is_fullscreen() ? "/fullscreen/scrollbars/state" : "/window/scrollbars/state", true);
    }
}

void EditWidget::toggleColorProfAdjust()
{
    // TODO implement
}

void
EditWidget::updateZoom()
{
    _zoom_status.update();
}

void
EditWidget::letZoomGrabFocus()
{
    _zoom_status.grab_focus();
}

void
EditWidget::setToolboxFocusTo (const gchar *)
{
    /// \todo not implemented
}

void
EditWidget::setToolboxAdjustmentValue (const gchar *, double)
{
    /// \todo not implemented
}

void
EditWidget::setToolboxSelectOneValue (const gchar *, gint)
{
    /// \todo not implemented
}

bool
EditWidget::isToolboxButtonActive (gchar const*)
{
    /// \todo not implemented
    return true;
}

void
EditWidget::setCoordinateStatus (Geom::Point p)
{
    gchar *cstr = g_strdup_printf ("%6.2f", _dt2r * p[Geom::X]);
    _coord_status_x.property_label() = cstr;
    g_free (cstr);
    cstr = g_strdup_printf ("%6.2f", _dt2r * p[Geom::Y]);
    _coord_status_y.property_label() = cstr;
    g_free (cstr);
}

void
EditWidget::setMessage (Inkscape::MessageType /*type*/, gchar const* msg)
{
    _select_status.set_markup (msg? msg : "");
}

bool
EditWidget::warnDialog (gchar* msg)
{
    Gtk::MessageDialog dlg (*this,
                       msg,
                       true,
                       Gtk::MESSAGE_WARNING,
                       Gtk::BUTTONS_YES_NO,
                       true);
    int r = dlg.run();
    return r == Gtk::RESPONSE_YES;
}


Inkscape::UI::Widget::Dock*
EditWidget::getDock ()
{
    return &_dock;
}

void EditWidget::_namedview_modified (SPObject *obj, guint flags) {
    SPNamedView *nv = static_cast<SPNamedView *>(obj);
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        this->_dt2r = 1.0 / nv->doc_units->unittobase;
        this->_top_ruler.update_metric();
        this->_left_ruler.update_metric();
        this->_tooltips.set_tip(this->_top_ruler, this->_top_ruler.get_tip());
        this->_tooltips.set_tip(this->_left_ruler, this->_left_ruler.get_tip());
        this->updateRulers();
    }
}

void
EditWidget::initEdit (SPDocument *doc)
{
    _desktop = new SPDesktop();
    _desktop->registerEditWidget (this);

    _namedview = sp_document_namedview (doc, 0);
    _svg_canvas.init (_desktop);
    _desktop->init (_namedview, _svg_canvas.spobj());
    sp_namedview_window_from_document (_desktop);
    sp_namedview_update_layers_from_document (_desktop);
    _dt2r = 1.0 / _namedview->doc_units->unittobase;

    /// \todo convert to sigc++ when SPObject hierarchy gets converted
    /* Listen on namedview modification */
    _namedview_modified_connection = _desktop->namedview->connectModified(sigc::mem_fun(*this, &EditWidget::_namedview_modified));
    _layer_selector.setDesktop (_desktop);
    _selected_style_status.setDesktop (_desktop);

    Inkscape::NSApplication::Editor::addDesktop (_desktop);

    _zoom_status.init (_desktop);
    _top_ruler.init (_desktop, _svg_canvas.widget());
    _left_ruler.init (_desktop, _svg_canvas.widget());
    updateRulers();
}

void
EditWidget::destroyEdit()
{
    if (_desktop) {
        _layer_selector.unreference();
        Inkscape::NSApplication::Editor::removeDesktop (_desktop); // clears selection too
        _namedview_modified_connection.disconnect();
        _desktop->destroy();
        Inkscape::GC::release (_desktop);
        _desktop = 0;
    }
}

//----------end of EditWidgetInterface implementation

//----------start of other callbacks

bool
EditWidget::on_key_press_event (GdkEventKey* event)
{
    // this is the original code from helper/window.cpp

    unsigned int shortcut;
    shortcut = get_group0_keyval (event) |
	           ( event->state & GDK_SHIFT_MASK ?
	             SP_SHORTCUT_SHIFT_MASK : 0 ) |
	           ( event->state & GDK_CONTROL_MASK ?
	             SP_SHORTCUT_CONTROL_MASK : 0 ) |
	           ( event->state & GDK_MOD1_MASK ?
	             SP_SHORTCUT_ALT_MASK : 0 );
    return sp_shortcut_invoke (shortcut,
                             Inkscape::NSApplication::Editor::getActiveDesktop());
}

bool
EditWidget::on_delete_event (GdkEventAny*)
{
    return shutdown();
}

bool
EditWidget::on_focus_in_event (GdkEventFocus*)
{
    Inkscape::NSApplication::Editor::activateDesktop (_desktop);
    _svg_canvas.widget().grab_focus();

    return false;
}

void
EditWidget::onWindowSizeAllocate (Gtk::Allocation &newall)
{
    if (!is_realized()) return;

    const Gtk::Allocation& all = get_allocation();
    if ((newall.get_x() == all.get_x()) &&
        (newall.get_y() == all.get_y()) &&
        (newall.get_width() == all.get_width()) &&
        (newall.get_height() == all.get_height())) {
        return;
    }

    Geom::Rect const area = _desktop->get_display_area();
    double zoom = _desktop->current_zoom();

    if (_sticky_zoom.get_active()) {
        /* Calculate zoom per pixel */
        double const zpsp = zoom / hypot(area.dimensions()[Geom::X], area.dimensions()[Geom::Y]);
        /* Find new visible area */
        Geom::Rect const newarea = _desktop->get_display_area();
        /* Calculate adjusted zoom */
        zoom = zpsp * hypot(newarea.dimensions()[Geom::X], newarea.dimensions()[Geom::Y]);
    }

    _desktop->zoom_absolute(area.midpoint()[Geom::X], area.midpoint()[Geom::Y], zoom);
}

void
EditWidget::onWindowRealize()
{

    if ( (sp_document_width(_desktop->doc()) < 1.0) || (sp_document_height(_desktop->doc()) < 1.0) ) {
        return;
    }

    Geom::Rect d( Geom::Point(0, 0),
                  Geom::Point(sp_document_width(_desktop->doc()), sp_document_height(_desktop->doc())) );

    _desktop->set_display_area(d.min()[Geom::X], d.min()[Geom::Y], d.max()[Geom::X], d.max()[Geom::Y], 10);
    _namedview_modified(_desktop->namedview, SP_OBJECT_MODIFIED_FLAG);
    setTitle (SP_DOCUMENT_NAME(_desktop->doc()));
}

void
EditWidget::onAdjValueChanged()
{
    if (_update_a_f) return;
    _update_a_f = true;

    sp_canvas_scroll_to (_svg_canvas.spobj(),
                         _bottom_scrollbar.get_value(),
                         _right_scrollbar.get_value(),
                         false);
    updateRulers();

    _update_a_f = false;
}


} // namespace View
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
