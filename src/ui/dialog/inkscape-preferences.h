/** @file
 * @brief Inkscape Preferences dialog
 */
/* Authors:
 *   Carl Hetherington
 *   Marco Scholten
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Copyright (C) 2004-2013 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

#include <iostream>
#include <vector>
#include "ui/widget/preferences-widget.h"
#include "ui/widget/button.h"
#include <stddef.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/frame.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelfilter.h>

#include "ui/widget/panel.h"

// UPDATE THIS IF YOU'RE ADDING PREFS PAGES.
// Otherwise the commands that open the dialog with the new page will fail.

enum {
    PREFS_PAGE_TOOLS,
    PREFS_PAGE_TOOLS_SELECTOR,
    PREFS_PAGE_TOOLS_NODE,
    PREFS_PAGE_TOOLS_TWEAK,
    PREFS_PAGE_TOOLS_ZOOM,
    PREFS_PAGE_TOOLS_MEASURE,
    PREFS_PAGE_TOOLS_SHAPES,
    PREFS_PAGE_TOOLS_SHAPES_RECT,
    PREFS_PAGE_TOOLS_SHAPES_3DBOX,
    PREFS_PAGE_TOOLS_SHAPES_ELLIPSE,
    PREFS_PAGE_TOOLS_SHAPES_STAR,
    PREFS_PAGE_TOOLS_SHAPES_SPIRAL,
    PREFS_PAGE_TOOLS_PENCIL,
    PREFS_PAGE_TOOLS_PEN,
    PREFS_PAGE_TOOLS_CALLIGRAPHY,
    PREFS_PAGE_TOOLS_TEXT,
    PREFS_PAGE_TOOLS_SPRAY,
    PREFS_PAGE_TOOLS_ERASER,
    PREFS_PAGE_TOOLS_PAINTBUCKET,
    PREFS_PAGE_TOOLS_GRADIENT,
    PREFS_PAGE_TOOLS_DROPPER,
    PREFS_PAGE_TOOLS_CONNECTOR,
    PREFS_PAGE_TOOLS_LPETOOL,
    PREFS_PAGE_UI,
    PREFS_PAGE_UI_WINDOWS,
    PREFS_PAGE_UI_GRIDS,
    PREFS_PAGE_UI_KEYBOARD_SHORTCUTS,
    PREFS_PAGE_BEHAVIOR,
    PREFS_PAGE_BEHAVIOR_SELECTING,
    PREFS_PAGE_BEHAVIOR_TRANSFORMS,
    PREFS_PAGE_BEHAVIOR_SCROLLING,
    PREFS_PAGE_BEHAVIOR_SNAPPING,
    PREFS_PAGE_BEHAVIOR_STEPS,
    PREFS_PAGE_BEHAVIOR_CLONES,
    PREFS_PAGE_BEHAVIOR_MASKS,
    PREFS_PAGE_BEHAVIOR_MARKERS,
    PREFS_PAGE_BEHAVIOR_CLEANUP,
    PREFS_PAGE_IO,
    PREFS_PAGE_IO_MOUSE,
    PREFS_PAGE_IO_SVGOUTPUT,
    PREFS_PAGE_IO_CMS,
    PREFS_PAGE_IO_AUTOSAVE,
    PREFS_PAGE_IO_OPENCLIPART,
    PREFS_PAGE_SYSTEM,
    PREFS_PAGE_BITMAPS,
    PREFS_PAGE_RENDERING,
    PREFS_PAGE_SPELLCHECK

};

namespace Gtk {
#if WITH_GTKMM_3_0
class Scale;
#else
class HScale;
#endif
}

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
        Gtk::TreeModelColumn<UI::Widget::DialogPage*> _col_page;
    };
    PageListModelColumns _page_list_columns;

    Gtk::TreeModel::Path _path_tools;
    Gtk::TreeModel::Path _path_shapes;
    Gtk::TreeModel::Path _path_ui;
    Gtk::TreeModel::Path _path_behavior;
    Gtk::TreeModel::Path _path_io;

    UI::Widget::DialogPage _page_tools;
    UI::Widget::DialogPage _page_selector;
    UI::Widget::DialogPage _page_node;
    UI::Widget::DialogPage _page_tweak;
    UI::Widget::DialogPage _page_spray;
    UI::Widget::DialogPage _page_zoom;
    UI::Widget::DialogPage _page_measure;
    UI::Widget::DialogPage _page_shapes;
    UI::Widget::DialogPage _page_pencil;
    UI::Widget::DialogPage _page_pen;
    UI::Widget::DialogPage _page_calligraphy;
    UI::Widget::DialogPage _page_text;
    UI::Widget::DialogPage _page_gradient;
    UI::Widget::DialogPage _page_connector;
    UI::Widget::DialogPage _page_dropper;
    UI::Widget::DialogPage _page_lpetool;

    UI::Widget::DialogPage _page_rectangle;
    UI::Widget::DialogPage _page_3dbox;
    UI::Widget::DialogPage _page_ellipse;
    UI::Widget::DialogPage _page_star;
    UI::Widget::DialogPage _page_spiral;
    UI::Widget::DialogPage _page_paintbucket;
    UI::Widget::DialogPage _page_eraser;

    UI::Widget::DialogPage _page_ui;
    UI::Widget::DialogPage _page_windows;
    UI::Widget::DialogPage _page_grids;

    UI::Widget::DialogPage _page_behavior;
    UI::Widget::DialogPage _page_select;
    UI::Widget::DialogPage _page_transforms;
    UI::Widget::DialogPage _page_scrolling;
    UI::Widget::DialogPage _page_snapping;
    UI::Widget::DialogPage _page_steps;
    UI::Widget::DialogPage _page_clones;
    UI::Widget::DialogPage _page_mask;
    UI::Widget::DialogPage _page_markers;
    UI::Widget::DialogPage _page_cleanup;

    UI::Widget::DialogPage _page_io;
    UI::Widget::DialogPage _page_mouse;
    UI::Widget::DialogPage _page_svgoutput;
    UI::Widget::DialogPage _page_cms;
    UI::Widget::DialogPage _page_autosave;
    UI::Widget::DialogPage _page_openclipart;

    UI::Widget::DialogPage _page_rendering;
    UI::Widget::DialogPage _page_system;
    UI::Widget::DialogPage _page_bitmaps;
    UI::Widget::DialogPage _page_spellcheck;

    UI::Widget::DialogPage _page_keyshortcuts;

    UI::Widget::PrefSpinButton _mouse_sens;
    UI::Widget::PrefSpinButton _mouse_thres;
    UI::Widget::PrefSlider      _mouse_grabsize;
    UI::Widget::PrefCheckButton _mouse_use_ext_input;
    UI::Widget::PrefCheckButton _mouse_switch_on_ext_input;

    UI::Widget::PrefSpinButton _scroll_wheel;
    UI::Widget::PrefSpinButton _scroll_arrow_px;
    UI::Widget::PrefSpinButton _scroll_arrow_acc;
    UI::Widget::PrefSpinButton _scroll_auto_speed;
    UI::Widget::PrefSpinButton _scroll_auto_thres;
    UI::Widget::PrefCheckButton _scroll_space;
    UI::Widget::PrefCheckButton _wheel_zoom;

#if WITH_GTKMM_3_0
    Gtk::Scale      *_slider_snapping_delay;
#else
    Gtk::HScale     *_slider_snapping_delay;
#endif

    UI::Widget::PrefCheckButton _snap_indicator;
    UI::Widget::PrefCheckButton _snap_closest_only;
    UI::Widget::PrefCheckButton _snap_mouse_pointer;

    UI::Widget::PrefCombo       _steps_rot_snap;
    UI::Widget::PrefCheckButton _steps_rot_relative;
    UI::Widget::PrefCheckButton _steps_compass;
    UI::Widget::PrefSpinUnit    _steps_arrow;
    UI::Widget::PrefSpinUnit    _steps_scale;
    UI::Widget::PrefSpinUnit    _steps_inset;
    UI::Widget::PrefSpinButton  _steps_zoom;

    UI::Widget::PrefRadioButton _t_sel_trans_obj;
    UI::Widget::PrefRadioButton _t_sel_trans_outl;
    UI::Widget::PrefRadioButton _t_sel_cue_none;
    UI::Widget::PrefRadioButton _t_sel_cue_mark;
    UI::Widget::PrefRadioButton _t_sel_cue_box;
    UI::Widget::PrefRadioButton _t_bbox_visual;
    UI::Widget::PrefRadioButton _t_bbox_geometric;

    UI::Widget::PrefCheckButton _t_cvg_keep_objects;
    UI::Widget::PrefCheckButton _t_cvg_convert_whole_groups;
    UI::Widget::PrefCheckButton _t_node_show_outline;
    UI::Widget::PrefCheckButton _t_node_live_outline;
    UI::Widget::PrefCheckButton _t_node_live_objects;
    UI::Widget::PrefCheckButton _t_node_pathflash_enabled;
    UI::Widget::PrefCheckButton _t_node_pathflash_selected;
    UI::Widget::PrefSpinButton  _t_node_pathflash_timeout;
    UI::Widget::PrefCheckButton _t_node_show_path_direction;
    UI::Widget::PrefCheckButton _t_node_single_node_transform_handles;
    UI::Widget::PrefCheckButton _t_node_delete_preserves_shape;
    UI::Widget::PrefColorPicker _t_node_pathoutline_color;

    UI::Widget::PrefRadioButton _win_dockable;
    UI::Widget::PrefRadioButton _win_floating;
    UI::Widget::PrefRadioButton _win_native;
    UI::Widget::PrefRadioButton _win_gtk;
    UI::Widget::PrefRadioButton _win_save_dialog_pos_on;
    UI::Widget::PrefRadioButton _win_save_dialog_pos_off;
    UI::Widget::PrefCombo       _win_default_size;
    UI::Widget::PrefRadioButton _win_ontop_none;
    UI::Widget::PrefRadioButton _win_ontop_normal;
    UI::Widget::PrefRadioButton _win_ontop_agressive;
    UI::Widget::PrefRadioButton _win_save_geom_off;
    UI::Widget::PrefRadioButton _win_save_geom;
    UI::Widget::PrefRadioButton _win_save_geom_prefs;
    UI::Widget::PrefCheckButton _win_hide_task;
    UI::Widget::PrefCheckButton _win_save_viewport;
    UI::Widget::PrefCheckButton _win_zoom_resize;
    UI::Widget::PrefCheckButton _win_show_close;
    UI::Widget::PrefSpinButton _win_trans_focus; /**< The dialog transparency setting for when the dialog is focused. */
    UI::Widget::PrefSpinButton _win_trans_blur;  /**< The dialog transparency setting for when the dialog is out of focus. */
    UI::Widget::PrefSpinButton _win_trans_time;  /**< How much time to go from one transparency setting to another */

    UI::Widget::PrefCheckButton _pencil_average_all_sketches;

    UI::Widget::PrefCheckButton _calligrapy_use_abs_size;
    UI::Widget::PrefCheckButton _calligrapy_keep_selected;

    UI::Widget::PrefCheckButton _connector_ignore_text;

    UI::Widget::PrefRadioButton _clone_option_parallel;
    UI::Widget::PrefRadioButton _clone_option_stay;
    UI::Widget::PrefRadioButton _clone_option_transform;
    UI::Widget::PrefRadioButton _clone_option_unlink;
    UI::Widget::PrefRadioButton _clone_option_delete;
    UI::Widget::PrefCheckButton _clone_relink_on_duplicate;

    UI::Widget::PrefCheckButton _mask_mask_on_top;
    UI::Widget::PrefCheckButton _mask_mask_remove;
    UI::Widget::PrefRadioButton _mask_grouping_none;
    UI::Widget::PrefRadioButton _mask_grouping_separate;
    UI::Widget::PrefRadioButton _mask_grouping_all;
    UI::Widget::PrefCheckButton _mask_ungrouping;

    UI::Widget::PrefRadioButton _blur_quality_best;
    UI::Widget::PrefRadioButton _blur_quality_better;
    UI::Widget::PrefRadioButton _blur_quality_normal;
    UI::Widget::PrefRadioButton _blur_quality_worse;
    UI::Widget::PrefRadioButton _blur_quality_worst;
    UI::Widget::PrefRadioButton _filter_quality_best;
    UI::Widget::PrefRadioButton _filter_quality_better;
    UI::Widget::PrefRadioButton _filter_quality_normal;
    UI::Widget::PrefRadioButton _filter_quality_worse;
    UI::Widget::PrefRadioButton _filter_quality_worst;
    UI::Widget::PrefCheckButton _show_filters_info_box;
    UI::Widget::PrefCombo       _dockbar_style;
    UI::Widget::PrefCombo       _switcher_style;
    UI::Widget::PrefCheckButton _rendering_image_outline;
    UI::Widget::PrefSpinButton  _rendering_cache_size;
    UI::Widget::PrefSpinButton  _filter_multi_threaded;

    UI::Widget::PrefCheckButton _trans_scale_stroke;
    UI::Widget::PrefCheckButton _trans_scale_corner;
    UI::Widget::PrefCheckButton _trans_gradient;
    UI::Widget::PrefCheckButton _trans_pattern;
    UI::Widget::PrefRadioButton _trans_optimized;
    UI::Widget::PrefRadioButton _trans_preserved;

    UI::Widget::PrefRadioButton _sel_all;
    UI::Widget::PrefRadioButton _sel_current;
    UI::Widget::PrefRadioButton _sel_recursive;
    UI::Widget::PrefCheckButton _sel_hidden;
    UI::Widget::PrefCheckButton _sel_locked;
    UI::Widget::PrefCheckButton _sel_layer_deselects;
    UI::Widget::PrefCheckButton _sel_cycle;

    UI::Widget::PrefCheckButton _markers_color_stock;
    UI::Widget::PrefCheckButton _markers_color_custom;
    UI::Widget::PrefCheckButton _markers_color_update;
    
    UI::Widget::PrefCheckButton _cleanup_swatches;

    UI::Widget::PrefSpinButton  _importexport_export_res;
    UI::Widget::PrefSpinButton  _importexport_import_res;
    UI::Widget::PrefCheckButton _importexport_import_res_override;
    UI::Widget::PrefSlider      _snap_delay;
    UI::Widget::PrefSlider      _snap_weight;
    UI::Widget::PrefSlider      _snap_persistence;
    UI::Widget::PrefCheckButton _font_dialog;
    UI::Widget::PrefCombo       _font_unit_type;
    UI::Widget::PrefCheckButton _font_output_px;

    UI::Widget::PrefCheckButton _misc_comment;
    UI::Widget::PrefCheckButton _misc_default_metadata;
    UI::Widget::PrefCheckButton _misc_forkvectors;
    UI::Widget::PrefCheckButton _misc_gradienteditor;
    UI::Widget::PrefSpinButton  _misc_gradientangle;
    UI::Widget::PrefCheckButton _misc_scripts;
    UI::Widget::PrefCheckButton _misc_namedicon_delay;

    // System page
    UI::Widget::PrefSpinButton  _misc_latency_skew;
    UI::Widget::PrefSpinButton  _misc_simpl;
    Gtk::Entry                  _sys_user_prefs;
    Gtk::Entry                  _sys_tmp_files;
    Gtk::Entry                  _sys_extension_dir;
    Gtk::Entry                  _sys_user_extension_dir;
    Gtk::Entry                  _sys_user_config;
    Gtk::Entry                  _sys_user_cache;
    Gtk::Entry                  _sys_data;
    Gtk::TextView               _sys_icon;
    Gtk::ScrolledWindow         _sys_icon_scroll;
    Gtk::TextView               _sys_systemdata;
    Gtk::ScrolledWindow         _sys_systemdata_scroll;

    // UI page
    UI::Widget::PrefCombo       _ui_languages;
    UI::Widget::PrefCombo       _misc_small_toolbar;
    UI::Widget::PrefCombo       _misc_small_secondary;
    UI::Widget::PrefCombo       _misc_small_tools;
    UI::Widget::PrefCheckButton _ui_colorsliders_top;
    UI::Widget::PrefSpinButton  _misc_recent;
    UI::Widget::PrefCheckButton _ui_partialdynamic;
    UI::Widget::ZoomCorrRulerSlider _ui_zoom_correction;

    //Spellcheck
    UI::Widget::PrefCombo       _spell_language;
    UI::Widget::PrefCombo       _spell_language2;
    UI::Widget::PrefCombo       _spell_language3;
    UI::Widget::PrefCheckButton _spell_ignorenumbers;
    UI::Widget::PrefCheckButton _spell_ignoreallcaps;

    // Bitmaps
    UI::Widget::PrefCombo       _misc_overs_bitmap;
    UI::Widget::PrefEntryFileButtonHBox       _misc_bitmap_editor;
    UI::Widget::PrefCheckButton _misc_bitmap_autoreload;
    UI::Widget::PrefSpinButton  _bitmap_copy_res;
    UI::Widget::PrefCheckButton _bitmap_ask;
    UI::Widget::PrefCombo       _bitmap_link;
    UI::Widget::PrefCombo       _bitmap_scale;
    UI::Widget::PrefSpinButton  _bitmap_import_quality;

    UI::Widget::PrefEntry       _kb_search;
    UI::Widget::PrefCombo       _kb_filelist;

    UI::Widget::PrefCheckButton _save_use_current_dir;
    UI::Widget::PrefCheckButton _save_autosave_enable;
    UI::Widget::PrefSpinButton  _save_autosave_interval;
    UI::Widget::PrefEntry       _save_autosave_path;
    UI::Widget::PrefSpinButton  _save_autosave_max;

    Gtk::ComboBoxText   _cms_display_profile;
    UI::Widget::PrefCheckButton     _cms_from_display;
    UI::Widget::PrefCombo           _cms_intent;

    UI::Widget::PrefCheckButton     _cms_softproof;
    UI::Widget::PrefCheckButton     _cms_gamutwarn;
    Gtk::ColorButton    _cms_gamutcolor;
    Gtk::ComboBoxText   _cms_proof_profile;
    UI::Widget::PrefCombo           _cms_proof_intent;
    UI::Widget::PrefCheckButton     _cms_proof_blackpoint;
    UI::Widget::PrefCheckButton     _cms_proof_preserveblack;

    Gtk::Notebook       _grids_notebook;
    UI::Widget::PrefRadioButton     _grids_no_emphasize_on_zoom;
    UI::Widget::PrefRadioButton     _grids_emphasize_on_zoom;
    UI::Widget::DialogPage          _grids_xy;
    UI::Widget::DialogPage          _grids_axonom;
    // CanvasXYGrid properties:
        UI::Widget::PrefUnit            _grids_xy_units;
        UI::Widget::PrefSpinButton      _grids_xy_origin_x;
        UI::Widget::PrefSpinButton      _grids_xy_origin_y;
        UI::Widget::PrefSpinButton      _grids_xy_spacing_x;
        UI::Widget::PrefSpinButton      _grids_xy_spacing_y;
        UI::Widget::PrefColorPicker     _grids_xy_color;
        UI::Widget::PrefColorPicker     _grids_xy_empcolor;
        UI::Widget::PrefSpinButton      _grids_xy_empspacing;
        UI::Widget::PrefCheckButton     _grids_xy_dotted;
    // CanvasAxonomGrid properties:
        UI::Widget::PrefUnit            _grids_axonom_units;
        UI::Widget::PrefSpinButton      _grids_axonom_origin_x;
        UI::Widget::PrefSpinButton      _grids_axonom_origin_y;
        UI::Widget::PrefSpinButton      _grids_axonom_spacing_y;
        UI::Widget::PrefSpinButton      _grids_axonom_angle_x;
        UI::Widget::PrefSpinButton      _grids_axonom_angle_z;
        UI::Widget::PrefColorPicker     _grids_axonom_color;
        UI::Widget::PrefColorPicker     _grids_axonom_empcolor;
        UI::Widget::PrefSpinButton      _grids_axonom_empspacing;

    // SVG Output page:
    UI::Widget::PrefCheckButton   _svgoutput_usenamedcolors;
    UI::Widget::PrefSpinButton    _svgoutput_numericprecision;
    UI::Widget::PrefSpinButton    _svgoutput_minimumexponent;
    UI::Widget::PrefCheckButton   _svgoutput_inlineattrs;
    UI::Widget::PrefSpinButton    _svgoutput_indent;
    UI::Widget::PrefCombo         _svgoutput_pathformat;
    UI::Widget::PrefCheckButton   _svgoutput_forcerepeatcommands;

    // Attribute Checking controls for SVG Output page:
    UI::Widget::PrefCheckButton   _svgoutput_attrwarn;
    UI::Widget::PrefCheckButton   _svgoutput_attrremove;
    UI::Widget::PrefCheckButton   _svgoutput_stylepropwarn;
    UI::Widget::PrefCheckButton   _svgoutput_stylepropremove;
    UI::Widget::PrefCheckButton   _svgoutput_styledefaultswarn;
    UI::Widget::PrefCheckButton   _svgoutput_styledefaultsremove;
    UI::Widget::PrefCheckButton   _svgoutput_check_reading;
    UI::Widget::PrefCheckButton   _svgoutput_check_editing;
    UI::Widget::PrefCheckButton   _svgoutput_check_writing;

    UI::Widget::PrefEntryButtonHBox _importexport_ocal_url;
    UI::Widget::PrefEntry       _importexport_ocal_username;
    UI::Widget::PrefEntry       _importexport_ocal_password;

    /*
     * Keyboard shortcut members
     */
    class ModelColumns: public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
            add(name);
            add(id);
            add(shortcut);
            add(description);
            add(shortcutid);
            add(user_set);
        }
        virtual ~ModelColumns() {
        }

        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> id;
        Gtk::TreeModelColumn<Glib::ustring> shortcut;
        Gtk::TreeModelColumn<Glib::ustring> description;
        Gtk::TreeModelColumn<unsigned int> shortcutid;
        Gtk::TreeModelColumn<unsigned int> user_set;
    };
    ModelColumns _kb_columns;
    static ModelColumns &onKBGetCols();
    Glib::RefPtr<Gtk::TreeStore> _kb_store;
    Gtk::TreeView _kb_tree;
    Gtk::CellRendererAccel _kb_shortcut_renderer;
    Glib::RefPtr<Gtk::TreeModelFilter> _kb_filter;
    gboolean _kb_shortcuts_loaded;

    int _max_dialog_width;
    int _max_dialog_height;
    int _sb_width;
    UI::Widget::DialogPage* _current_page;

    Gtk::TreeModel::iterator AddPage(UI::Widget::DialogPage& p, Glib::ustring title, int id);
    Gtk::TreeModel::iterator AddPage(UI::Widget::DialogPage& p, Glib::ustring title, Gtk::TreeModel::iterator parent, int id);
    bool SetMaxDialogSize(const Gtk::TreeModel::iterator& iter);
    bool PresentPage(const Gtk::TreeModel::iterator& iter);

    static void AddSelcueCheckbox(UI::Widget::DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddGradientCheckbox(UI::Widget::DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddConvertGuidesCheckbox(UI::Widget::DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddFirstAndLastCheckbox(UI::Widget::DialogPage& p, Glib::ustring const &prefs_path, bool def_value);
    static void AddDotSizeSpinbutton(UI::Widget::DialogPage& p, Glib::ustring const &prefs_path, double def_value);
    static void AddNewObjectsStyle(UI::Widget::DialogPage& p, Glib::ustring const &prefs_path, const gchar* banner = NULL);

    void on_pagelist_selection_changed();
    void on_reset_open_recent_clicked();

    void initPageTools();
    void initPageUI();
    void initPageBehavior();
    void initPageIO();

    void initPageRendering();
    void initPageSpellcheck();
    void initPageBitmaps();
    void initPageSystem();
    void initPageI18n(); // Do we still need it?
    void initKeyboardShortcuts(Gtk::TreeModel::iterator iter_ui);

    void _presentPages();

    /*
     * Functions for the Keyboard shortcut editor panel
     */
    void onKBReset();
    void onKBImport();
    void onKBExport();
    void onKBList();
    void onKBRealize();
    void onKBListKeyboardShortcuts();
    void onKBTreeEdited (const Glib::ustring& path, guint accel_key, Gdk::ModifierType accel_mods, guint hardware_keycode);
    void onKBTreeCleared(const Glib::ustring& path_string);
    bool onKBSearchKeyEvent(GdkEventKey *event);
    bool onKBSearchFilter(const Gtk::TreeModel::const_iterator& iter);
    static void onKBShortcutRenderer(Gtk::CellRenderer *rndr, Gtk::TreeIter const &iter);


private:
    InkscapePreferences();
    InkscapePreferences(InkscapePreferences const &d);
    InkscapePreferences operator=(InkscapePreferences const &d);
    bool _init;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_INKSCAPE_PREFERENCES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
