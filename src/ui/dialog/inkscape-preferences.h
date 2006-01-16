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

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class InkscapePreferences : public Dialog {
public:
    virtual ~InkscapePreferences();

    static InkscapePreferences *create() {return new InkscapePreferences(); }

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
        { Gtk::TreeModelColumnRecord::add(_col_id); Gtk::TreeModelColumnRecord::add(_col_page); }
        Gtk::TreeModelColumn<Glib::ustring> _col_id;
        Gtk::TreeModelColumn<DialogPage*> _col_page;
    };
    PageListModelColumns _page_list_columns;

    DialogPage _page_mouse, _page_scrolling, _page_steps, _page_tools, _page_windows,
               _page_clones, _page_transforms, _page_select, _page_misc;
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
    PrefCheckButton _win_save_geom, _win_hide_task, _win_zoom_resize;

    PrefRadioButton _clone_option_parallel, _clone_option_stay, _clone_option_transform,
                    _clone_option_unlink, _clone_option_delete;

    PrefCheckButton _trans_scale_stroke, _trans_scale_corner, _trans_gradient,_trans_pattern;
    PrefRadioButton _trans_optimized, _trans_preserved;

    PrefCheckButton _sel_current, _sel_hidden, _sel_locked;

    PrefSpinButton  _misc_export, _misc_recent, _misc_simpl;
    PrefCheckButton _misc_imp_bitmap, _misc_comment, _misc_scripts;
    PrefCombo       _misc_overs_bitmap;

    int _max_dialog_width;
    int _max_dialog_height;
    int _sb_width;
    DialogPage* _current_page;

    Gtk::TreeModel::iterator AddPage(DialogPage& p, Glib::ustring title);
    Gtk::TreeModel::iterator AddPage(DialogPage& p, Glib::ustring title, Gtk::TreeModel::iterator parent);
    bool SetMaxDialogSize(const Gtk::TreeModel::iterator& iter);

    static void AddSelcueCheckbox(DialogPage& p, const std::string& prefs_path, bool def_value);
    static void AddGradientCheckbox(DialogPage& p, const std::string& prefs_path, bool def_value);
    static void AddNewObjectsStyle(DialogPage& p, const std::string& prefs_path);
    static void StyleFromSelectionToTool(gchar const *prefs_path);

    void on_pagelist_selection_changed();
    void initPageMouse();
    void initPageScrolling();
    void initPageSteps();
    void initPageTools();
    void initPageWindows();
    void initPageClones();
    void initPageTransforms();
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
