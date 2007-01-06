/**
 * brief Inkscape Preferences dialog
 *
 * Authors:
 *   Carl Hetherington
 *   Marco Scholten
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2004-2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

#include <iostream>
#include <vector>
#include <gtkmm/table.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/frame.h>
#include "ui/widget/preferences-widget.h"
#include <sigc++/sigc++.h>
#include <glibmm/i18n.h>

#include "dialog.h"

// UPDATE THIS IF YOU'RE ADDING PREFS PAGES.
// Otherwise the commands that open the dialog with the new page will fail.

enum {
    PREFS_PAGE_MOUSE,
    PREFS_PAGE_SCROLLING,
    PREFS_PAGE_STEPS,
    PREFS_PAGE_TOOLS,
    PREFS_PAGE_TOOLS_SELECTOR,
    PREFS_PAGE_TOOLS_NODE,
    PREFS_PAGE_TOOLS_ZOOM,
    PREFS_PAGE_TOOLS_SHAPES,
    PREFS_PAGE_TOOLS_SHAPES_RECT,
    PREFS_PAGE_TOOLS_SHAPES_ELLIPSE,
    PREFS_PAGE_TOOLS_SHAPES_STAR,
    PREFS_PAGE_TOOLS_SHAPES_SPIRAL,
    PREFS_PAGE_TOOLS_PENCIL,
    PREFS_PAGE_TOOLS_PEN,
    PREFS_PAGE_TOOLS_CALLIGRAPHY,
    PREFS_PAGE_TOOLS_TEXT,
    PREFS_PAGE_TOOLS_GRADIENT,
    PREFS_PAGE_TOOLS_CONNECTOR,
    PREFS_PAGE_TOOLS_DROPPER,
    PREFS_PAGE_WINDOWS,
    PREFS_PAGE_CLONES,
    PREFS_PAGE_FILTERS,
    PREFS_PAGE_TRANSFORMS,
    PREFS_PAGE_SELECTING,
    PREFS_PAGE_MISC
};

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class InkscapePreferences : public Dialog {
public:
    virtual ~InkscapePreferences();

    static InkscapePreferences *create() {return new InkscapePreferences(); }
    void present();

protected:
    Gtk::Frame _page_frame;
    Gtk::Label _page_title;
    Gtk::TreeView _page_list;  
    Glib::RefPtr<Gtk::TreeStore> _page_list_model;

    //Pagelist model columns:
    class PageListModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        PageListModelColumns()
        { Gtk::TreeModelColumnRecord::add(_col_name); Gtk::TreeModelColumnRecord::add(_col_page); Gtk::TreeModelColumnRecord::add(_col_id); }
        Gtk::TreeModelColumn<Glib::ustring> _col_name;
        Gtk::TreeModelColumn<int> _col_id;
        Gtk::TreeModelColumn<DialogPage*> _col_page;
    };
    PageListModelColumns _page_list_columns;

    Gtk::TreeModel::Path _path_tools;
    Gtk::TreeModel::Path _path_shapes;

    DialogPage _page_mouse, _page_scrolling, _page_steps, _page_tools, _page_windows,
               _page_clones, _page_transforms, _page_filters, _page_select, _page_misc;
    DialogPage _page_selector, _page_node, _page_zoom, _page_shapes, _page_pencil, _page_pen,
               _page_calligraphy, _page_text, _page_gradient, _page_connector, _page_dropper;
    DialogPage _page_rectangle, _page_ellipse, _page_star, _page_spiral;

    PrefSpinButton _mouse_sens, _mouse_thres;

    PrefSpinButton _scroll_wheel, _scroll_arrow_px, _scroll_arrow_acc, _scroll_auto_speed, _scroll_auto_thres;

    PrefCombo       _steps_rot_snap;
    PrefCheckButton _steps_compass;
    PrefSpinButton  _steps_arrow, _steps_scale, _steps_inset, _steps_zoom;

    PrefRadioButton _t_sel_trans_obj, _t_sel_trans_outl, _t_sel_cue_none, _t_sel_cue_mark,
                    _t_sel_cue_box, _t_sel_org_edge, _t_sel_org_node;

    PrefSpinButton  _t_pencil_tolerance;

    PrefRadioButton _win_ontop_none, _win_ontop_normal, _win_ontop_agressive;
    PrefCheckButton _win_save_geom, _win_hide_task, _win_zoom_resize , _win_show_close;

// FIXME: Temporary Win32 special code to enable transient dialogs
#ifdef WIN32 
    PrefCheckButton _win_ontop_win32;   
#endif    

    PrefCheckButton _calligrapy_use_abs_size;
    PrefCheckButton _calligrapy_keep_selected;

    PrefCheckButton _connector_ignore_text;
    
    PrefRadioButton _clone_option_parallel, _clone_option_stay, _clone_option_transform,
                    _clone_option_unlink, _clone_option_delete;

    PrefRadioButton _blur_quality_best, _blur_quality_better, _blur_quality_normal, _blur_quality_worse, _blur_quality_worst;

    PrefCheckButton _trans_scale_stroke, _trans_scale_corner, _trans_gradient,_trans_pattern;
    PrefRadioButton _trans_optimized, _trans_preserved;

    PrefRadioButton _sel_all;
    PrefRadioButton _sel_current;
    PrefRadioButton _sel_recursive;
    PrefCheckButton _sel_hidden, _sel_locked;
    PrefCheckButton _sel_layer_deselects;

    PrefSpinButton  _misc_export, _misc_recent, _misc_simpl;
    PrefCheckButton _misc_imp_bitmap, _misc_comment, _misc_scripts;
    PrefCombo       _misc_overs_bitmap;
    PrefCheckButton _misc_mask_on_top;
    PrefCheckButton _misc_mask_remove;
    PrefCheckButton _misc_use_ext_input;

    int _max_dialog_width;
    int _max_dialog_height;
    int _sb_width;
    DialogPage* _current_page;

    Gtk::TreeModel::iterator AddPage(DialogPage& p, Glib::ustring title, int id);
    Gtk::TreeModel::iterator AddPage(DialogPage& p, Glib::ustring title, Gtk::TreeModel::iterator parent, int id);
    bool SetMaxDialogSize(const Gtk::TreeModel::iterator& iter);
    bool PresentPage(const Gtk::TreeModel::iterator& iter);

    static void AddSelcueCheckbox(DialogPage& p, const std::string& prefs_path, bool def_value);
    static void AddGradientCheckbox(DialogPage& p, const std::string& prefs_path, bool def_value);
    static void AddNewObjectsStyle(DialogPage& p, const std::string& prefs_path);

    void on_pagelist_selection_changed();
    void initPageMouse();
    void initPageScrolling();
    void initPageSteps();
    void initPageTools();
    void initPageWindows();
    void initPageClones();
    void initPageTransforms();
    void initPageFilters();
    void initPageSelecting();
    void initPageMisc();

private:
    InkscapePreferences();
    InkscapePreferences(InkscapePreferences const &d);
    InkscapePreferences operator=(InkscapePreferences const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
