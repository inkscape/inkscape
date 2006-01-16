/**
 * \brief Inkscape Preferences dialog
 *
 * Authors:
 *   Carl Hetherington
 *   Marco Scholten
 *
 * Copyright (C) 2004, 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */ 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>

#include "prefs-utils.h"
#include "inkscape-preferences.h"
#include "verbs.h"
#include "selcue.h"
#include <iostream>
#include "enums.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "message-stack.h"
#include "style.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "xml/repr.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

InkscapePreferences::InkscapePreferences()
    : Dialog ("dialogs.preferences", SP_VERB_DIALOG_DISPLAY),
      _max_dialog_width(0), 
      _max_dialog_height(0),
      _current_page(0)
{ 
    //get the width of a spinbutton
    Gtk::SpinButton* sb = new Gtk::SpinButton;
    sb->set_width_chars(6);
    this->get_vbox()->add(*sb);
    this->show_all_children();
    _sb_width = sb->size_request().width;
    this->get_vbox()->remove(*sb);
    delete sb;

    //Main HBox
    Gtk::HBox* hbox_list_page = Gtk::manage(new Gtk::HBox());
    hbox_list_page->set_border_width(12);
    hbox_list_page->set_spacing(12);
    this->get_vbox()->add(*hbox_list_page);


    //Pagelist
    Gtk::Frame* list_frame = Gtk::manage(new Gtk::Frame());
    Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
    hbox_list_page->pack_start(*list_frame, false, true, 0);
    _page_list.set_headers_visible(false);
    scrolled_window->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scrolled_window->add(_page_list);
    list_frame->set_shadow_type(Gtk::SHADOW_IN);
    list_frame->add(*scrolled_window);
    _page_list_model = Gtk::TreeStore::create(_page_list_columns);
    _page_list.set_model(_page_list_model);
    _page_list.append_column("id",_page_list_columns._col_id);
	Glib::RefPtr<Gtk::TreeSelection> _page_list_selection = _page_list.get_selection();
	_page_list_selection->signal_changed().connect(sigc::mem_fun(*this, &InkscapePreferences::on_pagelist_selection_changed));
	_page_list_selection->set_mode(Gtk::SELECTION_BROWSE);
    
    //Pages
    Gtk::VBox* vbox_page = Gtk::manage(new Gtk::VBox());
    Gtk::Frame* title_frame = Gtk::manage(new Gtk::Frame());
    hbox_list_page->pack_start(*vbox_page, true, true, 0);
    title_frame->add(_page_title);
    vbox_page->pack_start(*title_frame, false, false, 0);
    vbox_page->pack_start(_page_frame, true, true, 0);
    _page_frame.set_shadow_type(Gtk::SHADOW_IN);
    title_frame->set_shadow_type(Gtk::SHADOW_IN);

    initPageMouse();
    initPageScrolling();
    initPageSteps();
    initPageTools();
    initPageWindows();
    initPageClones();
    initPageTransforms();
    initPageSelecting();
    initPageMisc();

    //calculate the size request for this dialog
    this->show_all_children();
    _page_list.expand_all();
    _page_list_model->foreach_iter(sigc::mem_fun(*this, &InkscapePreferences::SetMaxDialogSize)); 
    this->set_size_request(_max_dialog_width, _max_dialog_height);
    _page_list.collapse_all();

    //Select page todo: select last 
    Gtk::TreeModel::iterator iter = _page_list_model->children().begin();
    if(iter)
      _page_list_selection->select(iter);
}

InkscapePreferences::~InkscapePreferences()
{
}

Gtk::TreeModel::iterator InkscapePreferences::AddPage(DialogPage& p, Glib::ustring title)
{
    Gtk::TreeModel::iterator iter = _page_list_model->append();
    Gtk::TreeModel::Row row = *iter;
    row[_page_list_columns._col_id] = title;
    row[_page_list_columns._col_page] = &p;
    return iter;
}

Gtk::TreeModel::iterator InkscapePreferences::AddPage(DialogPage& p, Glib::ustring title, Gtk::TreeModel::iterator parent)
{
    Gtk::TreeModel::iterator iter = _page_list_model->append((*parent).children());
    Gtk::TreeModel::Row row = *iter;
    row[_page_list_columns._col_id] = title;
    row[_page_list_columns._col_page] = &p;
    return iter;
}

void InkscapePreferences::initPageMouse()
{
    this->AddPage(_page_mouse, _("Mouse"));
    _mouse_sens.init ( "options.cursortolerance", "value", 0.0, 30.0, 1.0, 1.0, 8.0, true, false);
    _page_mouse.add_line( false, _("Grab sensitivity:"), _mouse_sens, _("pixels"), 
                           _("How close on the screen you need to be to an object to be able to grab it with mouse (in screen pixels)"), false);
    _mouse_thres.init ( "options.dragtolerance", "value", 0.0, 20.0, 1.0, 1.0, 4.0, true, false);
    _page_mouse.add_line( false, _("Click/drag threshold:"), _mouse_thres, _("pixels"), 
                           _("Maximum mouse drag (in screen pixels) which is considered a click, not a drag"), false);
}

void InkscapePreferences::initPageScrolling()
{
    this->AddPage(_page_scrolling, _("Scrolling"));
    _scroll_wheel.init ( "options.wheelscroll", "value", 0.0, 1000.0, 1.0, 1.0, 40.0, true, false);
    _page_scrolling.add_line( false, _("Mouse wheel scrolls by:"), _scroll_wheel, _("pixels"), 
                           _("One mouse wheel notch scrolls by this distance in screen pixels (horizontally with Shift)"), false);
    _page_scrolling.add_group_header( _("Ctrl+arrows"));
    _scroll_arrow_px.init ( "options.keyscroll", "value", 0.0, 1000.0, 1.0, 1.0, 10.0, true, false);
    _page_scrolling.add_line( true, _("Scroll by:"), _scroll_arrow_px, _("pixels"), 
                           _("Pressing Ctrl+arrow key scrolls by this distance (in screen pixels)"), false);
    _scroll_arrow_acc.init ( "options.scrollingacceleration", "value", 0.0, 5.0, 0.01, 1.0, 0.35, false, false);
    _page_scrolling.add_line( true, _("Acceleration:"), _scroll_arrow_acc, "", 
                           _("Pressing and holding Ctrl+arrow will gradually speed up scrolling (0 for no acceleration)"), false);
    _page_scrolling.add_group_header( _("Autoscrolling"));
    _scroll_auto_speed.init ( "options.autoscrollspeed", "value", 0.0, 5.0, 0.01, 1.0, 0.7, false, false);
    _page_scrolling.add_line( true, _("Speed:"), _scroll_auto_speed, "", 
                           _("How fast the canvas autoscrolls when you drag beyond canvas edge (0 to turn autoscroll off)"), false);
    _scroll_auto_thres.init ( "options.autoscrolldistance", "value", -600.0, 600.0, 1.0, 1.0, -10.0, true, false);
    _page_scrolling.add_line( true, _("Threshold:"), _scroll_auto_thres, _("pixels"), 
                           _("How far (in screen pixels) you need to be from the canvas edge to trigger autoscroll; positive is outside the canvas, negative is within the canvas"), false);
}

void InkscapePreferences::initPageSteps()
{
    this->AddPage(_page_steps, _("Steps"));

    _steps_arrow.init ( "options.nudgedistance", "value", 0.0, 3000.0, 0.01, 1.0, 2.0, false, false);
    _page_steps.add_line( false, _("Arrow keys move by:"), _steps_arrow, _("px"), 
                          _("Pressing an arrow key moves selected object(s) or node(s) by this distance (in px units)"), false);
    _steps_scale.init ( "options.defaultscale", "value", 0.0, 3000.0, 0.01, 1.0, 2.0, false, false);
    _page_steps.add_line( false, _("> and < scale by:"), _steps_scale, _("px"), 
                          _("Pressing > or < scales selection up or down by this increment (in px units)"), false);
    _steps_inset.init ( "options.defaultoffsetwidth", "value", 0.0, 3000.0, 0.01, 1.0, 2.0, false, false);
    _page_steps.add_line( false, _("Inset/Outset by:"), _steps_inset, _("px"), 
                          _("Inset and Outset commands displace the path by this distance (in px units)"), false);
    _steps_compass.init ( _("Compass-like display of angles"), "options.compassangledisplay", "value", true);
    _page_windows.add_line( false, "", _steps_compass, "", 
                            _("When on, angles are displayed with 0 at north, 0 to 360 range, positive clockwise; otherwise with 0 at east, -180 to 180 range, positive counterclockwise"));
    int const num_items = 12;
    Glib::ustring labels[num_items] = {"90", "60", "45", "30", "15", "10", "7.5", "6", "3", "2", "1", _("None")};
    int values[num_items] = {2, 3, 4, 6, 12, 18, 24, 30, 60, 90, 180, 0};
    _steps_rot_snap.set_size_request(_sb_width);
    _steps_rot_snap.init("options.rotationsnapsperpi", "value", labels, values, num_items, 12);
    _page_steps.add_line( false, _("Rotation snaps every:"), _steps_rot_snap, _("degrees"), 
                           _("Rotating with Ctrl pressed snaps every that much degrees; also, pressing [ or ] rotates by this amount"), false);
    _steps_zoom.init ( "options.zoomincrement", "value", 101.0, 500.0, 1.0, 1.0, 1.414213562, true, true);
    _page_steps.add_line( false, _("Zoom in/out by:"), _steps_zoom, _("%"), 
                          _("Zoom tool click, +/- keys, and middle click zoom in and out by this multiplier"), false);
}

void InkscapePreferences::AddSelcueCheckbox(DialogPage& p, const std::string& prefs_path, bool def_value)
{
    PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
    cb->init ( _("Show selection cue"), prefs_path, "selcue", def_value);
    p.add_line( false, "", *cb, "", _("Whether selected objects display a selection cue (the same as in selector)"));
}

void InkscapePreferences::AddGradientCheckbox(DialogPage& p, const std::string& prefs_path, bool def_value)
{
    PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
    cb->init ( _("Enable gradient editing"), prefs_path, "gradientdrag", def_value);
    p.add_line( false, "", *cb, "", _("Whether selected objects display gradient editing controls"));
}

void InkscapePreferences::AddNewObjectsStyle(DialogPage& p, const std::string& prefs_path)
{
    PrefRadioButton* current = Gtk::manage( new PrefRadioButton);
    PrefRadioButton* own = Gtk::manage( new PrefRadioButton);
    Gtk::Button* button = Gtk::manage( new Gtk::Button(_("Take from selection"),true));

    own->changed_signal.connect( sigc::mem_fun(*button, &Gtk::Button::set_sensitive) );
    button->signal_clicked().connect(sigc::bind( sigc::ptr_fun(InkscapePreferences::StyleFromSelectionToTool), prefs_path.c_str() ) );

    current->init ( _("Last used style"), prefs_path, "usecurrent", 1, true, 0);
    own->init ( _("This tool's own style:"), prefs_path, "usecurrent", 0, false, current);

    p.add_group_header( _("Create new objects with:"));
    p.add_line( true, "", *current, "",
                _("Apply the style you last set on an object"));
    p.add_line( true, "", *own, "",
                _("Each tool may store its own style to apply to the newly created objects. Use the button below to set it."));
    p.add_line( true, "", *button, "",
                _("Remember the style of the (first) selected object as this tool's style"));
}

void InkscapePreferences::StyleFromSelectionToTool(gchar const *prefs_path)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        SP_DT_MSGSTACK(desktop)->flash(Inkscape::ERROR_MESSAGE,
                                       _("<b>No objects selected</b> to take the style from."));
        return;
    }
    SPItem *item = selection->singleItem();
    if (!item) {
        /* TODO: If each item in the selection has the same style then don't consider it an error.
         * Maybe we should try to handle multiple selections anyway, e.g. the intersection of the
         * style attributes for the selected items. */
        SP_DT_MSGSTACK(desktop)->flash(Inkscape::ERROR_MESSAGE,
                                       _("<b>More than one object selected.</b>  Cannot take style from multiple objects."));
        return;
    }

    SPCSSAttr *css = take_style_from_item (item);

    if (!css) return;

    // only store text style for the text tool
    if (!g_strrstr ((const gchar *) prefs_path, "text")) {
        css = sp_css_attr_unset_text (css);
    }

    // we cannot store properties with uris - they will be invalid in other documents
    css = sp_css_attr_unset_uris (css);

    sp_repr_css_change (inkscape_get_repr (INKSCAPE, prefs_path), css, "style");
    sp_repr_css_attr_unref (css);
}

void InkscapePreferences::initPageTools()
{
    Gtk::TreeModel::iterator iter_tools = this->AddPage(_page_tools, _("Tools"));    

    //Selector
    this->AddPage(_page_selector, _("Selector"), iter_tools);

    AddSelcueCheckbox(_page_selector, "tools.select", false);
    _page_selector.add_group_header( _("When transforming, show:"));
    _t_sel_trans_obj.init ( _("Objects"), "tools.select", "show", "content", true, 0);
    _page_selector.add_line( true, "", _t_sel_trans_obj, "", 
                            _("Show the actual objects when moving or transforming"));
    _t_sel_trans_outl.init ( _("Box outline"), "tools.select", "show", "outline", false, &_t_sel_trans_obj);
    _page_selector.add_line( true, "", _t_sel_trans_outl, "", 
                            _("Show only a box outline of the objects when moving or transforming"));
    _page_selector.add_group_header( _("Per-object selection cue:"));
    _t_sel_cue_none.init ( _("None"), "options.selcue", "value", Inkscape::SelCue::NONE, false, 0);
    _page_selector.add_line( true, "", _t_sel_cue_none, "", 
                            _("No per-object selection indication"));
    _t_sel_cue_mark.init ( _("Mark"), "options.selcue", "value", Inkscape::SelCue::MARK, true, &_t_sel_cue_none);
    _page_selector.add_line( true, "", _t_sel_cue_mark, "", 
                            _("Each selected object has a diamond mark in the top left corner"));
    _t_sel_cue_box.init ( _("Box"), "options.selcue", "value", Inkscape::SelCue::BBOX, false, &_t_sel_cue_none);
    _page_selector.add_line( true, "", _t_sel_cue_box, "", 
                            _("Each selected object displays its bounding box"));
    _page_selector.add_group_header( _("Default scale origin:"));
    _t_sel_org_edge.init ( _("Opposite bounding box edge"), "tools.select", "scale_origin", "bbox", true, 0);
    _page_selector.add_line( true, "", _t_sel_org_edge, "", 
                            _("Default scale origin will be on the bounding box of the item"));
    _t_sel_org_node.init ( _("Farthest opposite node"), "tools.select", "scale_origin", "points", false, &_t_sel_org_edge);
    _page_selector.add_line( true, "", _t_sel_org_node, "", 
                            _("Default scale origin will be on the bounding box of the item's points"));
    //Node
    this->AddPage(_page_node, _("Node"), iter_tools);
    AddSelcueCheckbox(_page_node, "tools.nodes", true);
    AddGradientCheckbox(_page_node, "tools.nodes", true);
    //Zoom
    this->AddPage(_page_zoom, _("Zoom"), iter_tools);
    AddSelcueCheckbox(_page_zoom, "tools.zoom", true);
    AddGradientCheckbox(_page_zoom, "tools.zoom", false);
    //Shapes
    Gtk::TreeModel::iterator iter_shapes = this->AddPage(_page_shapes, _("Shapes"), iter_tools);
    this->AddSelcueCheckbox(_page_shapes, "tools.shapes", true);
    this->AddGradientCheckbox(_page_shapes, "tools.shapes", true);
    //Rectangle
    this->AddPage(_page_rectangle, _("Rectangle"), iter_shapes);
    this->AddNewObjectsStyle(_page_rectangle, "tools.shapes.rect");
    //ellipse
    this->AddPage(_page_ellipse, _("Ellipse"), iter_shapes);
    this->AddNewObjectsStyle(_page_ellipse, "tools.shapes.arc");
    //star
    this->AddPage(_page_star, _("Star"), iter_shapes);
    this->AddNewObjectsStyle(_page_star, "tools.shapes.star");
    //spiral
    this->AddPage(_page_spiral, _("Spiral"), iter_shapes);
    this->AddNewObjectsStyle(_page_spiral, "tools.shapes.spiral");
    //Pencil
    this->AddPage(_page_pencil, _("Pencil"), iter_tools);
    this->AddSelcueCheckbox(_page_pencil, "tools.freehand.pencil", true);
    _t_pencil_tolerance.init ( "tools.freehand.pencil", "tolerance", 0.0, 100.0, 0.5, 1.0, 10.0, false, false);
    _page_pencil.add_line( false, _("Tolerance:"), _t_pencil_tolerance, "", 
                           _("This value affects the amount of smoothing applied to freehand lines; lower values produce more uneven paths with more nodes"),
                           false );
    this->AddNewObjectsStyle(_page_pencil, "tools.freehand.pencil");
    //Pen
    this->AddPage(_page_pen, _("Pen"), iter_tools);
    this->AddSelcueCheckbox(_page_pen, "tools.freehand.pen", true);
    this->AddNewObjectsStyle(_page_pen, "tools.freehand.pen");
    //Calligraphy
    this->AddPage(_page_calligraphy, _("Calligraphy"), iter_tools);
    this->AddNewObjectsStyle(_page_calligraphy, "tools.calligraphic");
    //Text
    this->AddPage(_page_text, _("Text"), iter_tools);
    this->AddSelcueCheckbox(_page_text, "tools.text", true);
    this->AddGradientCheckbox(_page_text, "tools.text", true);
    this->AddNewObjectsStyle(_page_text, "tools.text");
    //Gradient
    this->AddPage(_page_gradient, _("Gradient"), iter_tools);
    this->AddSelcueCheckbox(_page_gradient, "tools.gradient", true);
    //Connector
    this->AddPage(_page_connector, _("Connector"), iter_tools);
    this->AddSelcueCheckbox(_page_connector, "tools.connector", true);
    //Dropper
    this->AddPage(_page_dropper, _("Dropper"), iter_tools);
    this->AddSelcueCheckbox(_page_dropper, "tools.dropper", true);
    this->AddGradientCheckbox(_page_dropper, "tools.dropper", true);
}

void InkscapePreferences::initPageWindows()
{
    _win_save_geom.init ( _("Save window geometry"), "options.savewindowgeometry", "value", true);
    _win_hide_task.init ( _("Dialogs are hidden in taskbar"), "options.dialogsskiptaskbar", "value", true);
    _win_zoom_resize.init ( _("Zoom when window is resized"), "options.stickyzoom", "value", false);
    _win_ontop_none.init ( _("None"), "options.transientpolicy", "value", 0, false, 0);
    _win_ontop_normal.init ( _("Normal"), "options.transientpolicy", "value", 1, true, &_win_ontop_none);
    _win_ontop_agressive.init ( _("Aggressive"), "options.transientpolicy", "value", 2, false, &_win_ontop_none);

    _page_windows.add_line( false, "", _win_save_geom, "", 
                            _("Save the window size and position with each document (only for Inkscape SVG format)"));
    _page_windows.add_line( false, "", _win_hide_task, "", 
                            _("Whether dialog windows are to be hidden in the window manager taskbar"));
    _page_windows.add_line( false, "", _win_zoom_resize, "", 
                            _("Zoom drawing when document window is resized, to keep the same area visible (this is the default which can be changed in any window using the button above the right scrollbar)"));
    _page_windows.add_group_header( _("Dialogs on top:"));
    _page_windows.add_line( true, "", _win_ontop_none, "", 
                            _("Dialogs are treated as regular windows"));
    _page_windows.add_line( true, "", _win_ontop_normal, "", 
                            _("Dialogs stay on top of document windows"));
    _page_windows.add_line( true, "", _win_ontop_agressive, "", 
                            _("Same as Normal but may work better with some window managers"));

    this->AddPage(_page_windows, _("Windows"));
}

void InkscapePreferences::initPageClones()
{
    _clone_option_parallel.init ( _("Move in parallel"), "options.clonecompensation", "value", 
                                  SP_CLONE_COMPENSATION_PARALLEL, true, 0);
    _clone_option_stay.init ( _("Stay unmoved"), "options.clonecompensation", "value", 
                                  SP_CLONE_COMPENSATION_UNMOVED, false, &_clone_option_parallel);
    _clone_option_transform.init ( _("Move according to transform"), "options.clonecompensation", "value", 
                                  SP_CLONE_COMPENSATION_NONE, false, &_clone_option_parallel);
    _clone_option_unlink.init ( _("Are unlinked"), "options.cloneorphans", "value", 
                                  SP_CLONE_ORPHANS_UNLINK, true, 0);
    _clone_option_delete.init ( _("Are deleted"), "options.cloneorphans", "value", 
                                  SP_CLONE_ORPHANS_DELETE, false, &_clone_option_unlink);

    _page_clones.add_group_header( _("When the original moves, its clones and linked offsets:"));
    _page_clones.add_line( true, "", _clone_option_parallel, "", 
                           _("Clones are translated by the same vector as their original."));
    _page_clones.add_line( true, "", _clone_option_stay, "", 
                           _("Clones preserve their positions when their original is moved."));
    _page_clones.add_line( true, "", _clone_option_transform, "", 
                           _("Each clone moves according to the value of its transform= attribute. For example, a rotated clone will move in a different direction than its original."));
    _page_clones.add_group_header( _("When the original is deleted, its clones:"));
    _page_clones.add_line( true, "", _clone_option_unlink, "", 
                           _("Orphaned clones are converted to regular objects."));
    _page_clones.add_line( true, "", _clone_option_delete, "", 
                           _("Orphaned clones are deleted along with their original."));

    this->AddPage(_page_clones, _("Clones"));
}

void InkscapePreferences::initPageTransforms()
{
    _trans_scale_stroke.init ( _("Scale stroke width"), "options.transform", "stroke", true);
    _trans_scale_corner.init ( _("Scale rounded corners in rectangles"), "options.transform", "rectcorners", false);
    _trans_gradient.init ( _("Transform gradients"), "options.transform", "gradient", true);
    _trans_pattern.init ( _("Transform patterns"), "options.transform", "pattern", false);
    _trans_optimized.init ( _("Normal"), "options.preservetransform", "value", 0, true, 0);
    _trans_preserved.init ( _("Aggressive"), "options.preservetransform", "value", 1, false, &_trans_optimized);

    _page_transforms.add_line( false, "", _trans_scale_stroke, "", 
                               _("When scaling objects, scale the stroke width by the same proportion"));
    _page_transforms.add_line( false, "", _trans_scale_corner, "", 
                               _("When scaling rectangles, scale the radii of rounded corners"));
    _page_transforms.add_line( false, "", _trans_gradient, "", 
                               _("Transform gradients (in fill or stroke) along with the objects"));
    _page_transforms.add_line( false, "", _trans_pattern, "", 
                               _("Transform patterns (in fill or stroke) along with the objects"));
    _page_transforms.add_group_header( _("Store transformation:"));
    _page_transforms.add_line( true, "", _trans_optimized, "", 
                               _("If possible, apply transformation to objects without adding a transform= attribute"));
    _page_transforms.add_line( true, "", _trans_preserved, "", 
                               _("Always store transformation as a transform= attribute on objects"));

    this->AddPage(_page_transforms, _("Transforms"));
}

void InkscapePreferences::initPageSelecting()
{
    _sel_current.init ( _("Select only within current layer"), "options.kbselection", "inlayer", true);
    _sel_hidden.init ( _("Ignore hidden objects"), "options.kbselection", "onlyvisible", true);
    _sel_locked.init ( _("Ignore locked objects"), "options.kbselection", "onlysensitive", true);

    _page_select.add_group_header( _("Ctrl+A, Tab, Shift+Tab:"));
    _page_select.add_line( true, "", _sel_current, "", 
                           _("Uncheck this to make keyboard selection commands work on objects in all layers"));
    _page_select.add_line( true, "", _sel_hidden, "", 
                           _("Uncheck this to be able to select objects that are hidden (either by themselves or by being in a hidden group or layer)"));
    _page_select.add_line( true, "", _sel_locked, "", 
                           _("Uncheck this to be able to select objects that are locked (either by themselves or by being in a locked group or layer)"));

    this->AddPage(_page_select, _("Selecting"));
}


void InkscapePreferences::initPageMisc()
{
    _misc_export.init("dialogs.export.defaultxdpi", "value", 0.0, 6000.0, 1.0, 1.0, 1.0, true, false);
    _page_misc.add_line( false, _("Default export resolution:"), _misc_export, _("dpi"), 
                           _("Default bitmap resolution (in dots per inch) in the Export dialog"), false);
    _misc_imp_bitmap.init( _("Import bitmap as <image>"), "options.importbitmapsasimages", "value", true);
    _page_misc.add_line( false, "", _misc_imp_bitmap, "", 
                           _("When on, an imported bitmap creates an <image> element; otherwise it is a rectangle with bitmap fill"), true);
    _misc_comment.init( _("Add label comments to printing output"), "printing.debug", "show-label-comments", false);
    _page_misc.add_line( false, "", _misc_comment, "", 
                           _("When on, a comment will be added to the raw print output, marking the rendered output for an object with its label"), true);
    _misc_scripts.init( _("Enable script effects (requires restart) - EXPERIMENTAL"), "extensions", "show-effects-menu", false);
    _page_misc.add_line( false, "", _misc_scripts, "", 
                           _("When on, the effects menu is enabled, allowing external effect scripts to be called, requires restart before effective - EXPERIMENTAL"), true);
    _misc_recent.init("options.maxrecentdocuments", "value", 0.0, 1000.0, 1.0, 1.0, 1.0, true, false);
    _page_misc.add_line( false, _("Max recent documents:"), _misc_recent, "", 
                           _("The maximum length of the Open Recent list in the File menu"), false);
    _misc_simpl.init("options.simplifythreshold", "value", 0.0, 1.0, 0.001, 0.01, 0.002, false, false);
    _page_misc.add_line( false, _("Simplification threshold:"), _misc_simpl, "", 
                           _("How strong is the Simplify command by default. If you invoke this command several times in quick succession, it will act more and more aggressively; invoking it again after a pause restores the default threshold."), false);
    int const num_items = 5;
    Glib::ustring labels[num_items] = {_("None"), _("2x2"), _("4x4"), _("8x8"), _("16x16")};
    int values[num_items] = {0, 1, 2, 3, 4};
    _misc_overs_bitmap.set_size_request(_sb_width);
    _misc_overs_bitmap.init("options.bitmapoversample", "value", labels, values, num_items, 1);
    _page_misc.add_line( false, _("Oversample bitmaps:"), _misc_overs_bitmap, "", "", false);

    this->AddPage(_page_misc, _("Misc"));
}

bool InkscapePreferences::SetMaxDialogSize(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    DialogPage* page = row[_page_list_columns._col_page];
    _page_frame.add(*page);
    this->show_all_children();
    _max_dialog_width=std::max(_max_dialog_width, this->size_request().width);
    _max_dialog_height=std::max(_max_dialog_height, this->size_request().height);
    _page_frame.remove();
    return false;
}

void InkscapePreferences::on_pagelist_selection_changed()
{
    // show new selection
    Glib::RefPtr<Gtk::TreeSelection> selection = _page_list.get_selection();
    Gtk::TreeModel::iterator iter = selection->get_selected();
    if(iter)
    {
        if (_current_page) 
            _page_frame.remove();
        Gtk::TreeModel::Row row = *iter;
        _current_page = row[_page_list_columns._col_page];
        _page_title.set_markup("<span size='large'><b>" + row[_page_list_columns._col_id] + "</b></span>");
        _page_frame.add(*_current_page);
        _current_page->show();
    }
}

} // namespace Dialog
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
