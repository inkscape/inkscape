/**
 * brief Inkscape Preferences dialog
 *
 * Authors:
 *   Carl Hetherington
 *   Marco Scholten
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Copyright (C) 2004-2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

#include <iostream>
#include <vector>
#include <gtkmm/base.h>
#include <gtkmm/table.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/notebook.h>
#include "ui/widget/preferences-widget.h"
#include <sigc++/sigc++.h>
#include <glibmm/i18n.h>

#include "ui/widget/panel.h"

// UPDATE THIS IF YOU'RE ADDING PREFS PAGES.
// Otherwise the commands that open the dialog with the new page will fail.

enum {
    PREFS_PAGE_TOOLS,
    PREFS_PAGE_TOOLS_SELECTOR,
    PREFS_PAGE_TOOLS_NODE,
    PREFS_PAGE_TOOLS_TWEAK,
    PREFS_PAGE_TOOLS_ZOOM,
    PREFS_PAGE_TOOLS_SHAPES,
    PREFS_PAGE_TOOLS_SHAPES_RECT,
    PREFS_PAGE_TOOLS_SHAPES_3DBOX,
    PREFS_PAGE_TOOLS_SHAPES_ELLIPSE,
    PREFS_PAGE_TOOLS_SHAPES_STAR,
    PREFS_PAGE_TOOLS_SHAPES_SPIRAL,
    PREFS_PAGE_TOOLS_PENCIL,
    PREFS_PAGE_TOOLS_PEN,
    PREFS_PAGE_TOOLS_CALLIGRAPHY,
    PREFS_PAGE_TOOLS_PAINTBUCKET,
    PREFS_PAGE_TOOLS_ERASER,
    PREFS_PAGE_TOOLS_LPETOOL,
    PREFS_PAGE_TOOLS_TEXT,
    PREFS_PAGE_TOOLS_GRADIENT,
    PREFS_PAGE_TOOLS_CONNECTOR,
    PREFS_PAGE_TOOLS_DROPPER,
    PREFS_PAGE_SELECTING,
    PREFS_PAGE_TRANSFORMS,
    PREFS_PAGE_CLONES,
    PREFS_PAGE_MASKS,
    PREFS_PAGE_FILTERS,
    PREFS_PAGE_BITMAPS,
    PREFS_PAGE_CMS,
    PREFS_PAGE_GRIDS,
    PREFS_PAGE_SVGOUTPUT,
    PREFS_PAGE_AUTOSAVE,
    PREFS_PAGE_IMPORTEXPORT,
    PREFS_PAGE_UI,
    PREFS_PAGE_MOUSE,
    PREFS_PAGE_SCROLLING,
    PREFS_PAGE_STEPS,
    PREFS_PAGE_WINDOWS,
    PREFS_PAGE_MISC
};

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class InkscapePreferences : public UI::Widget::Panel {
public:
    virtual ~InkscapePreferences();

    static InkscapePreferences &getInstance() { return *new InkscapePreferences(); }

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
        _page_clones, _page_mask, _page_transforms, _page_filters, _page_select,
        _page_importexport, _page_cms, _page_grids, _page_svgoutput, _page_misc,
        _page_ui, _page_autosave, _page_bitmaps;
    DialogPage _page_selector, _page_node, _page_tweak, _page_zoom, _page_shapes, _page_pencil, _page_pen,
               _page_calligraphy, _page_text, _page_gradient, _page_connector, _page_dropper, _page_lpetool;
    DialogPage _page_rectangle, _page_3dbox, _page_ellipse, _page_star, _page_spiral, _page_paintbucket, _page_eraser;

    PrefSpinButton _mouse_sens, _mouse_thres;
    PrefCheckButton _mouse_use_ext_input;
    PrefCheckButton _mouse_switch_on_ext_input;

    PrefSpinButton _scroll_wheel, _scroll_arrow_px, _scroll_arrow_acc, _scroll_auto_speed, _scroll_auto_thres;
    PrefCheckButton _scroll_space;
    PrefCheckButton _wheel_zoom;

    PrefCombo       _steps_rot_snap;
    PrefCheckButton _steps_compass;
    PrefSpinButton  _steps_arrow, _steps_scale, _steps_inset, _steps_zoom;

    PrefRadioButton _t_sel_trans_obj, _t_sel_trans_outl, _t_sel_cue_none, _t_sel_cue_mark,
                    _t_sel_cue_box, _t_bbox_visual, _t_bbox_geometric;
    PrefCheckButton _t_cvg_keep_objects, _t_cvg_convert_whole_groups;
    PrefCheckButton _t_node_pathflash_enabled;
    PrefSpinButton  _t_node_pathflash_timeout;
    PrefColorPicker _t_node_pathoutline_color;

    PrefRadioButton _win_dockable, _win_floating;
    PrefRadioButton _win_ontop_none, _win_ontop_normal, _win_ontop_agressive;
    PrefRadioButton _win_save_geom_off, _win_save_geom, _win_save_geom_prefs;
    PrefCheckButton _win_hide_task, _win_zoom_resize , _win_show_close;
	PrefSpinButton _win_trans_focus; /**< The dialog transparency setting for when the dialog is focused. */
	PrefSpinButton _win_trans_blur;  /**< The dialog transparency setting for when the dialog is out of focus. */
	PrefSpinButton _win_trans_time;  /**< How much time to go from one transparency setting to another */

    PrefCheckButton _calligrapy_use_abs_size;
    PrefCheckButton _calligrapy_keep_selected;

    PrefCheckButton _connector_ignore_text;

    PrefRadioButton _clone_option_parallel, _clone_option_stay, _clone_option_transform,
                    _clone_option_unlink, _clone_option_delete;
    PrefCheckButton _clone_relink_on_duplicate;

    PrefCheckButton _mask_mask_on_top;
    PrefCheckButton _mask_mask_remove;

    PrefRadioButton _blur_quality_best, _blur_quality_better, _blur_quality_normal, _blur_quality_worse, _blur_quality_worst;
    PrefCheckButton _show_filters_info_box;

    PrefCheckButton _trans_scale_stroke, _trans_scale_corner, _trans_gradient,_trans_pattern;
    PrefRadioButton _trans_optimized, _trans_preserved;

    PrefRadioButton _sel_all;
    PrefRadioButton _sel_current;
    PrefRadioButton _sel_recursive;
    PrefCheckButton _sel_hidden, _sel_locked;
    PrefCheckButton _sel_layer_deselects;

    PrefSpinButton  _importexport_export, _misc_recent, _misc_simpl;
    ZoomCorrRulerSlider _ui_zoom_correction;
    PrefSpinButton  _misc_latency_skew;
    PrefCheckButton _misc_comment, _misc_forkvectors, _misc_scripts, _misc_namedicon_delay;
    PrefCombo       _misc_small_toolbar;
    PrefCombo       _misc_small_secondary;
    PrefCombo       _misc_small_tools;
    PrefCombo       _misc_overs_bitmap;
    PrefCombo       _misc_bitmap_editor;
    PrefCheckButton _misc_bitmap_autoreload;
    PrefSpinButton  _bitmap_copy_res;

    PrefCheckButton _autosave_autosave_enable;
    PrefSpinButton  _autosave_autosave_interval;
    PrefEntry       _autosave_autosave_path;
    PrefSpinButton  _autosave_autosave_max;

    Gtk::ComboBoxText   _cms_display_profile;
    PrefCheckButton     _cms_from_display;
    PrefCombo           _cms_intent;

    PrefCheckButton     _cms_softproof;
    PrefCheckButton     _cms_gamutwarn;
    Gtk::ColorButton    _cms_gamutcolor;
    Gtk::ComboBoxText   _cms_proof_profile;
    PrefCombo           _cms_proof_intent;
    PrefCheckButton     _cms_proof_blackpoint;
    PrefCheckButton     _cms_proof_preserveblack;

    Gtk::Notebook       _grids_notebook;
    PrefCheckButton     _grids_no_emphasize_on_zoom;
    DialogPage          _grids_xy, _grids_axonom;
    // CanvasXYGrid properties:
        PrefUnit            _grids_xy_units;
        PrefSpinButton      _grids_xy_origin_x;
        PrefSpinButton      _grids_xy_origin_y;
        PrefSpinButton      _grids_xy_spacing_x;
        PrefSpinButton      _grids_xy_spacing_y;
        PrefColorPicker     _grids_xy_color;
        PrefColorPicker     _grids_xy_empcolor;
        PrefSpinButton      _grids_xy_empspacing;
        PrefCheckButton     _grids_xy_dotted;
    // CanvasAxonomGrid properties:
        PrefUnit            _grids_axonom_units;
        PrefSpinButton      _grids_axonom_origin_x;
        PrefSpinButton      _grids_axonom_origin_y;
        PrefSpinButton      _grids_axonom_spacing_y;
        PrefSpinButton      _grids_axonom_angle_x;
        PrefSpinButton      _grids_axonom_angle_z;
        PrefColorPicker     _grids_axonom_color;
        PrefColorPicker     _grids_axonom_empcolor;
        PrefSpinButton      _grids_axonom_empspacing;

    // SVG Output page:
    PrefCheckButton   _svgoutput_usenamedcolors;
    PrefSpinButton    _svgoutput_numericprecision;
    PrefSpinButton    _svgoutput_minimumexponent;
    PrefCheckButton   _svgoutput_inlineattrs;
    PrefSpinButton    _svgoutput_indent;
    PrefCheckButton   _svgoutput_allowrelativecoordinates;
    PrefCheckButton   _svgoutput_forcerepeatcommands;

    PrefEntryButtonHBox _importexport_ocal_url;
    PrefEntry       _importexport_ocal_username;
    PrefEntry       _importexport_ocal_password;

    int _max_dialog_width;
    int _max_dialog_height;
    int _sb_width;
    DialogPage* _current_page;

    Gtk::TreeModel::iterator AddPage(DialogPage& p, Glib::ustring title, int id);
    Gtk::TreeModel::iterator AddPage(DialogPage& p, Glib::ustring title, Gtk::TreeModel::iterator parent, int id);
    bool SetMaxDialogSize(const Gtk::TreeModel::iterator& iter);
    bool PresentPage(const Gtk::TreeModel::iterator& iter);

    static void AddSelcueCheckbox(DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddGradientCheckbox(DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddConvertGuidesCheckbox(DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddDotSizeSpinbutton(DialogPage& p, Glib::ustring const &prefs_path, double def_value);
    static void AddNewObjectsStyle(DialogPage& p, Glib::ustring const &prefs_path, const gchar* banner = NULL);

    void on_pagelist_selection_changed();
    void initPageMouse();
    void initPageScrolling();
    void initPageSteps();
    void initPageTools();
    void initPageWindows();
    void initPageClones();
    void initPageMasks();
    void initPageTransforms();
    void initPageFilters();
    void initPageSelecting();
    void initPageImportExport();
    void initPageCMS();
    void initPageGrids();
    void initPageSVGOutput();
    void initPageUI();
    void initPageAutosave();
    void initPageBitmaps();
    void initPageMisc();

    void _presentPages();

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
