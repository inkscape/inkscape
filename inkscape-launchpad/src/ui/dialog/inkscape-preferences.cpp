/**
 * @file
 * Inkscape Preferences dialog - implementation.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "inkscape-preferences.h"
#include <glibmm/i18n.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>
#include <gtkmm/main.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/alignment.h>

#include "preferences.h"
#include "verbs.h"
#include "selcue.h"
#include "util/units.h"
#include <iostream>
#include "enums.h"

#include "extension/internal/gdkpixbuf-input.h"
#include "message-stack.h"
#include "style.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "xml/repr.h"
#include "ui/widget/style-swatch.h"
#include "ui/widget/spinbutton.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"
#include "cms-system.h"
#include "color-profile.h"
#include "display/canvas-grid.h"
#include "path-prefix.h"
#include "io/resource.h"
#include "io/sys.h"
#include "inkscape.h"
#include "shortcuts.h"
#include "document.h"


#ifdef HAVE_ASPELL
# include <aspell.h>
# ifdef WIN32
#  include <windows.h>
# endif
#endif

namespace Inkscape {
namespace UI {
namespace Dialog {

using Inkscape::UI::Widget::DialogPage;
using Inkscape::UI::Widget::PrefCheckButton;
using Inkscape::UI::Widget::PrefRadioButton;
using Inkscape::UI::Widget::PrefSpinButton;
using Inkscape::UI::Widget::StyleSwatch;
using Inkscape::CMSSystem;


InkscapePreferences::InkscapePreferences()
    : UI::Widget::Panel ("", "/dialogs/preferences", SP_VERB_DIALOG_DISPLAY),
      _max_dialog_width(0),
      _max_dialog_height(0),
      _current_page(0),
      _init(true)
{
    //get the width of a spinbutton
    Inkscape::UI::Widget::SpinButton* sb = new Inkscape::UI::Widget::SpinButton;
    sb->set_width_chars(6);
    _getContents()->add(*sb);
    show_all_children();
    Gtk::Requisition sreq;
#if WITH_GTKMM_3_0
    Gtk::Requisition sreq_natural;
    sb->get_preferred_size(sreq_natural, sreq);
#else
    sreq = sb->size_request();
#endif
    _sb_width = sreq.width;
    _getContents()->remove(*sb);
    delete sb;

    //Main HBox
    Gtk::HBox* hbox_list_page = Gtk::manage(new Gtk::HBox());
    hbox_list_page->set_border_width(12);
    hbox_list_page->set_spacing(12);
    _getContents()->add(*hbox_list_page);

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
    _page_list.append_column("name",_page_list_columns._col_name);
    Glib::RefPtr<Gtk::TreeSelection> page_list_selection = _page_list.get_selection();
    page_list_selection->signal_changed().connect(sigc::mem_fun(*this, &InkscapePreferences::on_pagelist_selection_changed));
    page_list_selection->set_mode(Gtk::SELECTION_BROWSE);

    //Pages
    Gtk::VBox* vbox_page = Gtk::manage(new Gtk::VBox());
    Gtk::Frame* title_frame = Gtk::manage(new Gtk::Frame());

    Gtk::ScrolledWindow* pageScroller = Gtk::manage(new Gtk::ScrolledWindow());
    pageScroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    pageScroller->add(*vbox_page);
    hbox_list_page->pack_start(*pageScroller, true, true, 0);

    title_frame->add(_page_title);
    vbox_page->pack_start(*title_frame, false, false, 0);
    vbox_page->pack_start(_page_frame, true, true, 0);
    _page_frame.set_shadow_type(Gtk::SHADOW_IN);
    title_frame->set_shadow_type(Gtk::SHADOW_IN);

    initPageTools();
    initPageUI();
    initPageBehavior();
    initPageIO();

    initPageSystem();
    initPageBitmaps();
    initPageRendering();
    initPageSpellcheck();


    signalPresent().connect(sigc::mem_fun(*this, &InkscapePreferences::_presentPages));

    //calculate the size request for this dialog
    this->show_all_children();
    _page_list.expand_all();
    _page_list_model->foreach_iter(sigc::mem_fun(*this, &InkscapePreferences::SetMaxDialogSize));
    _getContents()->set_size_request(_max_dialog_width, _max_dialog_height);
    _page_list.collapse_all();
}

InkscapePreferences::~InkscapePreferences()
{
}

Gtk::TreeModel::iterator InkscapePreferences::AddPage(DialogPage& p, Glib::ustring title, int id)
{
    return AddPage(p, title, Gtk::TreeModel::iterator() , id);
}

Gtk::TreeModel::iterator InkscapePreferences::AddPage(DialogPage& p, Glib::ustring title, Gtk::TreeModel::iterator parent, int id)
{
    Gtk::TreeModel::iterator iter;
    if (parent)
       iter = _page_list_model->append((*parent).children());
    else
       iter = _page_list_model->append();
    Gtk::TreeModel::Row row = *iter;
    row[_page_list_columns._col_name] = title;
    row[_page_list_columns._col_id] = id;
    row[_page_list_columns._col_page] = &p;
    return iter;
}

void InkscapePreferences::AddSelcueCheckbox(DialogPage &p, Glib::ustring const &prefs_path, bool def_value)
{
    PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
    cb->init ( _("Show selection cue"), prefs_path + "/selcue", def_value);
    p.add_line( false, "", *cb, "", _("Whether selected objects display a selection cue (the same as in selector)"));
}

void InkscapePreferences::AddGradientCheckbox(DialogPage &p, Glib::ustring const &prefs_path, bool def_value)
{
    PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
    cb->init ( _("Enable gradient editing"), prefs_path + "/gradientdrag", def_value);
    p.add_line( false, "", *cb, "", _("Whether selected objects display gradient editing controls"));
}

void InkscapePreferences::AddConvertGuidesCheckbox(DialogPage &p, Glib::ustring const &prefs_path, bool def_value) {
    PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
    cb->init ( _("Conversion to guides uses edges instead of bounding box"), prefs_path + "/convertguides", def_value);
    p.add_line( false, "", *cb, "", _("Converting an object to guides places these along the object's true edges (imitating the object's shape), not along the bounding box"));
}

void InkscapePreferences::AddDotSizeSpinbutton(DialogPage &p, Glib::ustring const &prefs_path, double def_value)
{
    PrefSpinButton* sb = Gtk::manage( new PrefSpinButton);
    sb->init ( prefs_path + "/dot-size", 0.0, 1000.0, 0.1, 10.0, def_value, false, false);
    p.add_line( false, _("Ctrl+click _dot size:"), *sb, _("times current stroke width"),
                       _("Size of dots created with Ctrl+click (relative to current stroke width)"),
                       false );
}


static void StyleFromSelectionToTool(Glib::ustring const &prefs_path, StyleSwatch *swatch)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty()) {
        desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE,
                                       _("<b>No objects selected</b> to take the style from."));
        return;
    }
    SPItem *item = selection->singleItem();
    if (!item) {
        /* TODO: If each item in the selection has the same style then don't consider it an error.
         * Maybe we should try to handle multiple selections anyway, e.g. the intersection of the
         * style attributes for the selected items. */
        desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE,
                                       _("<b>More than one object selected.</b>  Cannot take style from multiple objects."));
        return;
    }

    SPCSSAttr *css = take_style_from_item (item);

    if (!css) return;

    // remove black-listed properties
    css = sp_css_attr_unset_blacklist (css);

    // only store text style for the text tool
    if (prefs_path != "/tools/text") {
        css = sp_css_attr_unset_text (css);
    }

    // we cannot store properties with uris - they will be invalid in other documents
    css = sp_css_attr_unset_uris (css);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setStyle(prefs_path + "/style", css);
    sp_repr_css_attr_unref (css);

    // update the swatch
    if (swatch) {
        SPCSSAttr *css = prefs->getInheritedStyle(prefs_path + "/style");
        swatch->setStyle (css);
        sp_repr_css_attr_unref(css);
    }
}

void InkscapePreferences::AddNewObjectsStyle(DialogPage &p, Glib::ustring const &prefs_path, const gchar *banner)
{
    if (banner)
        p.add_group_header(banner);
    else
        p.add_group_header( _("Style of new objects"));
    PrefRadioButton* current = Gtk::manage( new PrefRadioButton);
    current->init ( _("Last used style"), prefs_path + "/usecurrent", 1, true, 0);
    p.add_line( true, "", *current, "",
                _("Apply the style you last set on an object"));

    PrefRadioButton* own = Gtk::manage( new PrefRadioButton);
    Gtk::HBox* hb = Gtk::manage( new Gtk::HBox);
    Gtk::Alignment* align = Gtk::manage( new Gtk::Alignment);
    own->init ( _("This tool's own style:"), prefs_path + "/usecurrent", 0, false, current);
    align->set(0,0,0,0);
    align->add(*own);
    hb->add(*align);
    p.set_tip( *own, _("Each tool may store its own style to apply to the newly created objects. Use the button below to set it."));
    p.add_line( true, "", *hb, "", "");

    // style swatch
    Gtk::Button* button = Gtk::manage( new Gtk::Button(_("Take from selection"), true));
    StyleSwatch *swatch = 0;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getInt(prefs_path + "/usecurrent")) {
        button->set_sensitive(false);
    }

    SPCSSAttr *css = prefs->getStyle(prefs_path + "/style");
    swatch = new StyleSwatch(css, _("This tool's style of new objects"));
    hb->add(*swatch);
    sp_repr_css_attr_unref(css);

    button->signal_clicked().connect( sigc::bind( sigc::ptr_fun(StyleFromSelectionToTool), prefs_path, swatch)  );
    own->changed_signal.connect( sigc::mem_fun(*button, &Gtk::Button::set_sensitive) );
    p.add_line( true, "", *button, "",
                _("Remember the style of the (first) selected object as this tool's style"));
}

void InkscapePreferences::initPageTools()
{
    Gtk::TreeModel::iterator iter_tools = this->AddPage(_page_tools, _("Tools"), PREFS_PAGE_TOOLS);
    _path_tools = _page_list.get_model()->get_path(iter_tools);

    _page_tools.add_group_header( _("Bounding box to use"));
    _t_bbox_visual.init ( _("Visual bounding box"), "/tools/bounding_box", 0, false, 0); // 0 means visual
    _page_tools.add_line( true, "", _t_bbox_visual, "",
                            _("This bounding box includes stroke width, markers, filter margins, etc."));
    _t_bbox_geometric.init ( _("Geometric bounding box"), "/tools/bounding_box", 1, true, &_t_bbox_visual); // 1 means geometric
    _page_tools.add_line( true, "", _t_bbox_geometric, "",
                            _("This bounding box includes only the bare path"));

    _page_tools.add_group_header( _("Conversion to guides"));
    _t_cvg_keep_objects.init ( _("Keep objects after conversion to guides"), "/tools/cvg_keep_objects", false);
    _page_tools.add_line( true, "", _t_cvg_keep_objects, "",
                            _("When converting an object to guides, don't delete the object after the conversion"));
    _t_cvg_convert_whole_groups.init ( _("Treat groups as a single object"), "/tools/cvg_convert_whole_groups", false);
    _page_tools.add_line( true, "", _t_cvg_convert_whole_groups, "",
                            _("Treat groups as a single object during conversion to guides rather than converting each child separately"));

    _pencil_average_all_sketches.init ( _("Average all sketches"), "/tools/freehand/pencil/average_all_sketches", false);
    _calligrapy_use_abs_size.init ( _("Width is in absolute units"), "/tools/calligraphic/abs_width", false);
    _calligrapy_keep_selected.init ( _("Select new path"), "/tools/calligraphic/keep_selected", true);
    _connector_ignore_text.init( _("Don't attach connectors to text objects"), "/tools/connector/ignoretext", true);

    //Selector
    this->AddPage(_page_selector, _("Selector"), iter_tools, PREFS_PAGE_TOOLS_SELECTOR);


    AddSelcueCheckbox(_page_selector, "/tools/select", false);
    AddGradientCheckbox(_page_selector, "/tools/select", false);
    _page_selector.add_group_header( _("When transforming, show"));
    _t_sel_trans_obj.init ( _("Objects"), "/tools/select/show", "content", true, 0);
    _page_selector.add_line( true, "", _t_sel_trans_obj, "",
                            _("Show the actual objects when moving or transforming"));
    _t_sel_trans_outl.init ( _("Box outline"), "/tools/select/show", "outline", false, &_t_sel_trans_obj);
    _page_selector.add_line( true, "", _t_sel_trans_outl, "",
                            _("Show only a box outline of the objects when moving or transforming"));
    _page_selector.add_group_header( _("Per-object selection cue"));
    _t_sel_cue_none.init ( C_("Selection cue", "None"), "/options/selcue/value", Inkscape::SelCue::NONE, false, 0);
    _page_selector.add_line( true, "", _t_sel_cue_none, "",
                            _("No per-object selection indication"));
    _t_sel_cue_mark.init ( _("Mark"), "/options/selcue/value", Inkscape::SelCue::MARK, true, &_t_sel_cue_none);
    _page_selector.add_line( true, "", _t_sel_cue_mark, "",
                            _("Each selected object has a diamond mark in the top left corner"));
    _t_sel_cue_box.init ( _("Box"), "/options/selcue/value", Inkscape::SelCue::BBOX, false, &_t_sel_cue_none);
    _page_selector.add_line( true, "", _t_sel_cue_box, "",
                            _("Each selected object displays its bounding box"));

    //Node
    this->AddPage(_page_node, _("Node"), iter_tools, PREFS_PAGE_TOOLS_NODE);
    AddSelcueCheckbox(_page_node, "/tools/nodes", true);
    AddGradientCheckbox(_page_node, "/tools/nodes", true);
    _page_node.add_group_header( _("Path outline"));
    _t_node_pathoutline_color.init(_("Path outline color"), "/tools/nodes/highlight_color", 0xff0000ff);
    _page_node.add_line( false, "", _t_node_pathoutline_color, "", _("Selects the color used for showing the path outline"), false);
    _t_node_show_outline.init(_("Always show outline"), "/tools/nodes/show_outline", false);
    _page_node.add_line( true, "", _t_node_show_outline, "", _("Show outlines for all paths, not only invisible paths"));
    _t_node_live_outline.init(_("Update outline when dragging nodes"), "/tools/nodes/live_outline", false);
    _page_node.add_line( true, "", _t_node_live_outline, "", _("Update the outline when dragging or transforming nodes; if this is off, the outline will only update when completing a drag"));
    _t_node_live_objects.init(_("Update paths when dragging nodes"), "/tools/nodes/live_objects", false);
    _page_node.add_line( true, "", _t_node_live_objects, "", _("Update paths when dragging or transforming nodes; if this is off, paths will only be updated when completing a drag"));
    _t_node_show_path_direction.init(_("Show path direction on outlines"), "/tools/nodes/show_path_direction", false);
    _page_node.add_line( true, "", _t_node_show_path_direction, "", _("Visualize the direction of selected paths by drawing small arrows in the middle of each outline segment"));
    _t_node_pathflash_enabled.init ( _("Show temporary path outline"), "/tools/nodes/pathflash_enabled", false);
    _page_node.add_line( true, "", _t_node_pathflash_enabled, "", _("When hovering over a path, briefly flash its outline"));
    _t_node_pathflash_selected.init ( _("Show temporary outline for selected paths"), "/tools/nodes/pathflash_selected", false);
    _page_node.add_line( true, "", _t_node_pathflash_selected, "", _("Show temporary outline even when a path is selected for editing"));
    _t_node_pathflash_timeout.init("/tools/nodes/pathflash_timeout", 0, 10000.0, 100.0, 100.0, 1000.0, true, false);
    _page_node.add_line( false, _("_Flash time:"), _t_node_pathflash_timeout, "ms", _("Specifies how long the path outline will be visible after a mouse-over (in milliseconds); specify 0 to have the outline shown until mouse leaves the path"), false);
    _page_node.add_group_header(_("Editing preferences"));
    _t_node_single_node_transform_handles.init(_("Show transform handles for single nodes"), "/tools/nodes/single_node_transform_handles", false);
    _page_node.add_line( true, "", _t_node_single_node_transform_handles, "", _("Show transform handles even when only a single node is selected"));
    _t_node_delete_preserves_shape.init(_("Deleting nodes preserves shape"), "/tools/nodes/delete_preserves_shape", true);
    _page_node.add_line( true, "", _t_node_delete_preserves_shape, "", _("Move handles next to deleted nodes to resemble original shape; hold Ctrl to get the other behavior"));

    //Tweak
    this->AddPage(_page_tweak, _("Tweak"), iter_tools, PREFS_PAGE_TOOLS_TWEAK);
    this->AddNewObjectsStyle(_page_tweak, "/tools/tweak", _("Object paint style"));
    AddSelcueCheckbox(_page_tweak, "/tools/tweak", true);
    AddGradientCheckbox(_page_tweak, "/tools/tweak", false);

    //Zoom
    this->AddPage(_page_zoom, _("Zoom"), iter_tools, PREFS_PAGE_TOOLS_ZOOM);
    AddSelcueCheckbox(_page_zoom, "/tools/zoom", true);
    AddGradientCheckbox(_page_zoom, "/tools/zoom", false);

    //Measure
    this->AddPage(_page_measure, C_("ContextVerb", "Measure"), iter_tools, PREFS_PAGE_TOOLS_MEASURE);
    PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
    cb->init ( _("Ignore first and last points"), "/tools/measure/ignore_1st_and_last", true);
    _page_measure.add_line( false, "", *cb, "", _("The start and end of the measurement tool's control line will not be considered for calculating lengths. Only lengths between actual curve intersections will be displayed."));

    //Shapes
    Gtk::TreeModel::iterator iter_shapes = this->AddPage(_page_shapes, _("Shapes"), iter_tools, PREFS_PAGE_TOOLS_SHAPES);
    _path_shapes = _page_list.get_model()->get_path(iter_shapes);
    this->AddSelcueCheckbox(_page_shapes, "/tools/shapes", true);
    this->AddGradientCheckbox(_page_shapes, "/tools/shapes", true);

    //Rectangle
    this->AddPage(_page_rectangle, _("Rectangle"), iter_shapes, PREFS_PAGE_TOOLS_SHAPES_RECT);
    this->AddNewObjectsStyle(_page_rectangle, "/tools/shapes/rect");
    this->AddConvertGuidesCheckbox(_page_rectangle, "/tools/shapes/rect", true);

    //3D box
    this->AddPage(_page_3dbox, _("3D Box"), iter_shapes, PREFS_PAGE_TOOLS_SHAPES_3DBOX);
    this->AddNewObjectsStyle(_page_3dbox, "/tools/shapes/3dbox");
    this->AddConvertGuidesCheckbox(_page_3dbox, "/tools/shapes/3dbox", true);

    //Ellipse
    this->AddPage(_page_ellipse, _("Ellipse"), iter_shapes, PREFS_PAGE_TOOLS_SHAPES_ELLIPSE);
    this->AddNewObjectsStyle(_page_ellipse, "/tools/shapes/arc");

    //Star
    this->AddPage(_page_star, _("Star"), iter_shapes, PREFS_PAGE_TOOLS_SHAPES_STAR);
    this->AddNewObjectsStyle(_page_star, "/tools/shapes/star");

    //Spiral
    this->AddPage(_page_spiral, _("Spiral"), iter_shapes, PREFS_PAGE_TOOLS_SHAPES_SPIRAL);
    this->AddNewObjectsStyle(_page_spiral, "/tools/shapes/spiral");

    //Pencil
    this->AddPage(_page_pencil, _("Pencil"), iter_tools, PREFS_PAGE_TOOLS_PENCIL);
    this->AddSelcueCheckbox(_page_pencil, "/tools/freehand/pencil", true);
    this->AddNewObjectsStyle(_page_pencil, "/tools/freehand/pencil");
    this->AddDotSizeSpinbutton(_page_pencil, "/tools/freehand/pencil", 3.0);
    _page_pencil.add_group_header( _("Sketch mode"));
    _page_pencil.add_line( true, "", _pencil_average_all_sketches, "",
                            _("If on, the sketch result will be the normal average of all sketches made, instead of averaging the old result with the new sketch"));

    //Pen
    this->AddPage(_page_pen, _("Pen"), iter_tools, PREFS_PAGE_TOOLS_PEN);
    this->AddSelcueCheckbox(_page_pen, "/tools/freehand/pen", true);
    this->AddNewObjectsStyle(_page_pen, "/tools/freehand/pen");
    this->AddDotSizeSpinbutton(_page_pen, "/tools/freehand/pen", 3.0);

    //Calligraphy
    this->AddPage(_page_calligraphy, _("Calligraphy"), iter_tools, PREFS_PAGE_TOOLS_CALLIGRAPHY);
    this->AddSelcueCheckbox(_page_calligraphy, "/tools/calligraphic", false);
    this->AddNewObjectsStyle(_page_calligraphy, "/tools/calligraphic");
    _page_calligraphy.add_line( false, "", _calligrapy_use_abs_size, "",
                            _("If on, pen width is in absolute units (px) independent of zoom; otherwise pen width depends on zoom so that it looks the same at any zoom"));
    _page_calligraphy.add_line( false, "", _calligrapy_keep_selected, "",
                            _("If on, each newly created object will be selected (deselecting previous selection)"));

    //Text
    this->AddPage(_page_text, C_("ContextVerb", "Text"), iter_tools, PREFS_PAGE_TOOLS_TEXT);
    this->AddSelcueCheckbox(_page_text, "/tools/text", true);
    this->AddGradientCheckbox(_page_text, "/tools/text", true);
    {
        PrefCheckButton* cb = Gtk::manage( new PrefCheckButton);
        cb->init ( _("Show font samples in the drop-down list"), "/tools/text/show_sample_in_list", 1);
        _page_text.add_line( false, "", *cb, "", _("Show font samples alongside font names in the drop-down list in Text bar"));

        _font_dialog.init ( _("Show font substitution warning dialog"), "/options/font/substitutedlg", false);
        _page_text.add_line( false, "", _font_dialog, "", _("Show font substitution warning dialog when requested fonts are not available on the system"));
    }

    Glib::ustring sizeLabels[] = {_("Pixel"), _("Point"), _("Pica"), _("Millimeter"), _("Centimeter"), _("Inch"), _("Em square")/*, _("Ex square"), _("Percent")*/};
    int sizeValues[] = {SP_CSS_UNIT_PX, SP_CSS_UNIT_PT, SP_CSS_UNIT_PC, SP_CSS_UNIT_MM, SP_CSS_UNIT_CM, SP_CSS_UNIT_IN, SP_CSS_UNIT_EM/*, SP_CSS_UNIT_EX, SP_CSS_UNIT_PERCENT*/};

    _page_text.add_group_header( _("Text units"));
    _font_unit_type.init( "/options/font/unitType", sizeLabels, sizeValues, G_N_ELEMENTS(sizeLabels), SP_CSS_UNIT_PT );
    _page_text.add_line( false, _("Text size unit type:"), _font_unit_type, "",
                       _("Set the type of unit used in the text toolbar and text dialogs"), false);
    _font_output_px.init ( _("Always output text size in pixels (px)"), "/options/font/textOutputPx", true);
//    _page_text.add_line( false, "", _font_output_px, "", _("Always convert the text size units above into pixels (px) before saving to file"));

    this->AddNewObjectsStyle(_page_text, "/tools/text");

    //Spray
    this->AddPage(_page_spray, _("Spray"), iter_tools, PREFS_PAGE_TOOLS_SPRAY);
    AddSelcueCheckbox(_page_spray, "/tools/spray", true);
    AddGradientCheckbox(_page_spray, "/tools/spray", false);

    //Eraser
    this->AddPage(_page_eraser, _("Eraser"), iter_tools, PREFS_PAGE_TOOLS_ERASER);
    this->AddNewObjectsStyle(_page_eraser, "/tools/eraser");

    //Paint Bucket
    this->AddPage(_page_paintbucket, _("Paint Bucket"), iter_tools, PREFS_PAGE_TOOLS_PAINTBUCKET);
    this->AddSelcueCheckbox(_page_paintbucket, "/tools/paintbucket", false);
    this->AddNewObjectsStyle(_page_paintbucket, "/tools/paintbucket");

    //Gradient
    this->AddPage(_page_gradient, _("Gradient"), iter_tools, PREFS_PAGE_TOOLS_GRADIENT);
    this->AddSelcueCheckbox(_page_gradient, "/tools/gradient", true);
    _misc_forkvectors.init( _("Prevent sharing of gradient definitions"), "/options/forkgradientvectors/value", true);
    _page_gradient.add_line( false, "", _misc_forkvectors, "",
                           _("When on, shared gradient definitions are automatically forked on change; uncheck to allow sharing of gradient definitions so that editing one object may affect other objects using the same gradient"), true);
    _misc_gradienteditor.init( _("Use legacy Gradient Editor"), "/dialogs/gradienteditor/showlegacy", false);
    _page_gradient.add_line( false, "", _misc_gradienteditor, "",
                           _("When on, the Gradient Edit button in the Fill & Stroke dialog will show the legacy Gradient Editor dialog, when off the Gradient Tool will be used"), true);

    _misc_gradientangle.init("/dialogs/gradienteditor/angle", -359, 359, 1, 90, 0, false, false);
    _page_gradient.add_line( false, _("Linear gradient _angle:"), _misc_gradientangle, "",
                           _("Default angle of new linear gradients in degrees (clockwise from horizontal)"), false);


    //Dropper
    this->AddPage(_page_dropper, _("Dropper"), iter_tools, PREFS_PAGE_TOOLS_DROPPER);
    this->AddSelcueCheckbox(_page_dropper, "/tools/dropper", true);
    this->AddGradientCheckbox(_page_dropper, "/tools/dropper", true);
    
    //Connector
    this->AddPage(_page_connector, _("Connector"), iter_tools, PREFS_PAGE_TOOLS_CONNECTOR);
    this->AddSelcueCheckbox(_page_connector, "/tools/connector", true);
    _page_connector.add_line(false, "", _connector_ignore_text, "",
            _("If on, connector attachment points will not be shown for text objects"));

#ifdef WITH_LPETOOL
    //LPETool
    //disabled, because the LPETool is not finished yet.
    this->AddPage(_page_lpetool, _("LPE Tool"), iter_tools, PREFS_PAGE_TOOLS_LPETOOL);
    this->AddNewObjectsStyle(_page_lpetool, "/tools/lpetool");
#endif // WITH_LPETOOL
}

void InkscapePreferences::initPageUI()
{
    Gtk::TreeModel::iterator iter_ui = this->AddPage(_page_ui, _("Interface"), PREFS_PAGE_UI);
    _path_ui = _page_list.get_model()->get_path(iter_ui);

    Glib::ustring languages[] = {_("System default"), _("Albanian (sq)"), _("Amharic (am)"), _("Arabic (ar)"), _("Armenian (hy)"),_("Azerbaijani (az)"), _("Basque (eu)"), _("Belarusian (be)"),
        _("Bulgarian (bg)"), _("Bengali (bn)"), _("Bengali/Bangladesh (bn_BD)"), _("Breton (br)"), _("Catalan (ca)"), _("Valencian Catalan (ca@valencia)"), _("Chinese/China (zh_CN)"),
        _("Chinese/Taiwan (zh_TW)"), _("Croatian (hr)"), _("Czech (cs)"),
        _("Danish (da)"), _("Dutch (nl)"), _("Dzongkha (dz)"), _("German (de)"), _("Greek (el)"), _("English (en)"), _("English/Australia (en_AU)"),
        _("English/Canada (en_CA)"), _("English/Great Britain (en_GB)"), _("Pig Latin (en_US@piglatin)"),
        _("Esperanto (eo)"), _("Estonian (et)"), _("Farsi (fa)"), _("Finnish (fi)"),
        _("French (fr)"), _("Irish (ga)"), _("Galician (gl)"), _("Hebrew (he)"), _("Hungarian (hu)"),
        _("Indonesian (id)"), _("Italian (it)"), _("Japanese (ja)"), _("Khmer (km)"), _("Kinyarwanda (rw)"), _("Korean (ko)"), _("Lithuanian (lt)"), _("Latvian (lv)"), _("Macedonian (mk)"),
        _("Mongolian (mn)"), _("Nepali (ne)"), _("Norwegian Bokmål (nb)"), _("Norwegian Nynorsk (nn)"), _("Panjabi (pa)"),
        _("Polish (pl)"), _("Portuguese (pt)"), _("Portuguese/Brazil (pt_BR)"), _("Romanian (ro)"), _("Russian (ru)"),
        _("Serbian (sr)"), _("Serbian in Latin script (sr@latin)"), _("Slovak (sk)"), _("Slovenian (sl)"),  _("Spanish (es)"), _("Spanish/Mexico (es_MX)"),
        _("Swedish (sv)"),_("Telugu (te)"), _("Thai (th)"), _("Turkish (tr)"), _("Ukrainian (uk)"), _("Vietnamese (vi)")};
    Glib::ustring langValues[] = {"", "sq", "am", "ar", "hy", "az", "eu", "be", "bg", "bn", "bn_BD", "br", "ca", "ca@valencia", "zh_CN", "zh_TW", "hr", "cs", "da", "nl",
        "dz", "de", "el", "en", "en_AU", "en_CA", "en_GB", "en_US@piglatin", "eo", "et", "fa", "fi", "fr", "ga",
        "gl", "he", "hu", "id", "it", "ja", "km", "rw", "ko", "lt", "lv", "mk", "mn", "ne", "nb", "nn", "pa",
        "pl", "pt", "pt_BR", "ro", "ru", "sr", "sr@latin", "sk", "sl", "es", "es_MX", "sv", "te", "th", "tr", "uk", "vi" };

    {
        // sorting languages according to translated name
        int i = 0;
        int j = 0;
        int n = sizeof( languages ) / sizeof( Glib::ustring );
        Glib::ustring key_language;
        Glib::ustring key_langValue;
        for ( j = 1 ; j < n ; j++ ) {
            key_language = languages[j];
            key_langValue = langValues[j];
            i = j-1;
            while ( i >= 0
                    && ( ( languages[i] > key_language
                         && langValues[i] != "" )
                       || key_langValue == "" ) )
            {
                languages[i+1] = languages[i];
                langValues[i+1] = langValues[i];
                i--;
            }
            languages[i+1] = key_language;
            langValues[i+1] = key_langValue;
        }
    }

    _ui_languages.init( "/ui/language", languages, langValues, G_N_ELEMENTS(languages), languages[0]);
    _page_ui.add_line( false, _("Language (requires restart):"), _ui_languages, "",
                              _("Set the language for menus and number formats"), false);

    {
        Glib::ustring sizeLabels[] = {_("Large"), _("Small"), _("Smaller")};
        int sizeValues[] = {0, 1, 2};

        _misc_small_tools.init( "/toolbox/tools/small", sizeLabels, sizeValues, G_N_ELEMENTS(sizeLabels), 0 );
        _page_ui.add_line( false, _("Toolbox icon size:"), _misc_small_tools, "",
                           _("Set the size for the tool icons (requires restart)"), false);

        _misc_small_toolbar.init( "/toolbox/small", sizeLabels, sizeValues, G_N_ELEMENTS(sizeLabels), 0 );
        _page_ui.add_line( false, _("Control bar icon size:"), _misc_small_toolbar, "",
                           _("Set the size for the icons in tools' control bars to use (requires restart)"), false);

        _misc_small_secondary.init( "/toolbox/secondary", sizeLabels, sizeValues, G_N_ELEMENTS(sizeLabels), 1 );
        _page_ui.add_line( false, _("Secondary toolbar icon size:"), _misc_small_secondary, "",
                           _("Set the size for the icons in secondary toolbars to use (requires restart)"), false);
    }

    _ui_colorsliders_top.init( _("Work-around color sliders not drawing"), "/options/workarounds/colorsontop", false);
    _page_ui.add_line( false, "", _ui_colorsliders_top, "",
                       _("When on, will attempt to work around bugs in certain GTK themes drawing color sliders"), true);


    _misc_recent.init("/options/maxrecentdocuments/value", 0.0, 1000.0, 1.0, 1.0, 1.0, true, false);

    Gtk::Button* reset_recent = Gtk::manage(new Gtk::Button(_("Clear list")));
    reset_recent->signal_clicked().connect(sigc::mem_fun(*this, &InkscapePreferences::on_reset_open_recent_clicked));

    _page_ui.add_line( false, _("Maximum documents in Open _Recent:"), _misc_recent, "",
                              _("Set the maximum length of the Open Recent list in the File menu, or clear the list"), false, reset_recent);

    _ui_zoom_correction.init(300, 30, 1.00, 200.0, 1.0, 10.0, 1.0);
    _page_ui.add_line( false, _("_Zoom correction factor (in %):"), _ui_zoom_correction, "",
                              _("Adjust the slider until the length of the ruler on your screen matches its real length. This information is used when zooming to 1:1, 1:2, etc., to display objects in their true sizes"), true);


    _ui_partialdynamic.init( _("Enable dynamic relayout for incomplete sections"), "/options/workarounds/dynamicnotdone", false);
    _page_ui.add_line( false, "", _ui_partialdynamic, "",
                       _("When on, will allow dynamic layout of components that are not completely finished being refactored"), true);

    /* show infobox */
    _show_filters_info_box.init( _("Show filter primitives infobox (requires restart)"), "/options/showfiltersinfobox/value", true);
    _page_ui.add_line(false, "", _show_filters_info_box, "",
                        _("Show icons and descriptions for the filter primitives available at the filter effects dialog"));

    {
        Glib::ustring dockbarstyleLabels[] = {_("Icons only"), _("Text only"), _("Icons and text")};
        int dockbarstyleValues[] = {0, 1, 2};

        /* dockbar style */
        _dockbar_style.init( "/options/dock/dockbarstyle", dockbarstyleLabels, dockbarstyleValues, G_N_ELEMENTS(dockbarstyleLabels), 0);
        _page_ui.add_line(false, _("Dockbar style (requires restart):"),  _dockbar_style, "",
                        _("Selects whether the vertical bars on the dockbar will show text labels, icons, or both"), false);
	
        Glib::ustring switcherstyleLabels[] = {_("Text only"), _("Icons only"), _("Icons and text")}; /* see bug #1098437   */
        int switcherstyleValues[] = {0, 1, 2};
	
        /* switcher style */
        _switcher_style.init( "/options/dock/switcherstyle", switcherstyleLabels, switcherstyleValues, G_N_ELEMENTS(switcherstyleLabels), 0);
        _page_ui.add_line(false, _("Switcher style (requires restart):"),  _switcher_style, "",
                        _("Selects whether the dockbar switcher will show text labels, icons, or both"), false);
    }
    
    // Windows
    _win_save_geom.init ( _("Save and restore window geometry for each document"), "/options/savewindowgeometry/value", 1, true, 0);
    _win_save_geom_prefs.init ( _("Remember and use last window's geometry"), "/options/savewindowgeometry/value", 2, false, &_win_save_geom);
    _win_save_geom_off.init ( _("Don't save window geometry"), "/options/savewindowgeometry/value", 0, false, &_win_save_geom);

    _win_save_dialog_pos_on.init ( _("Save and restore dialogs status"), "/options/savedialogposition/value", 1, true, 0);
    _win_save_dialog_pos_off.init ( _("Don't save dialogs status"), "/options/savedialogposition/value", 0, false, &_win_save_dialog_pos_on);

    _win_dockable.init ( _("Dockable"), "/options/dialogtype/value", 1, true, 0);
    _win_floating.init ( _("Floating"), "/options/dialogtype/value", 0, false, &_win_dockable);


    _win_native.init ( _("Native open/save dialogs"), "/options/desktopintegration/value", 1, true, 0);
    _win_gtk.init ( _("GTK open/save dialogs"), "/options/desktopintegration/value", 0, false, &_win_native);
    
    _win_hide_task.init ( _("Dialogs are hidden in taskbar"), "/options/dialogsskiptaskbar/value", true);
    _win_save_viewport.init ( _("Save and restore documents viewport"), "/options/savedocviewport/value", true);
    _win_zoom_resize.init ( _("Zoom when window is resized"), "/options/stickyzoom/value", false);
    _win_show_close.init ( _("Show close button on dialogs"), "/dialogs/showclose", false);
    _win_ontop_none.init ( C_("Dialog on top", "None"), "/options/transientpolicy/value", 0, false, 0);
    _win_ontop_normal.init ( _("Normal"), "/options/transientpolicy/value", 1, true, &_win_ontop_none);
    _win_ontop_agressive.init ( _("Aggressive"), "/options/transientpolicy/value", 2, false, &_win_ontop_none);

    {
        Glib::ustring defaultSizeLabels[] = {_("Small"), _("Large"), _("Maximized")};
        int defaultSizeValues[] = {0, 1, 2};

        _win_default_size.init( "/options/defaultwindowsize/value", defaultSizeLabels, defaultSizeValues, G_N_ELEMENTS(defaultSizeLabels), 1 );
        _page_windows.add_line( false, _("Default window size:"),  _win_default_size, "",
                           _("Set the default window size"), false);
    }

    _page_windows.add_group_header( _("Saving window geometry (size and position)"));
    _page_windows.add_line( true, "", _win_save_geom_off, "",
                            _("Let the window manager determine placement of all windows"));
    _page_windows.add_line( true, "", _win_save_geom_prefs, "",
                            _("Remember and use the last window's geometry (saves geometry to user preferences)"));
    _page_windows.add_line( true, "", _win_save_geom, "",
                            _("Save and restore window geometry for each document (saves geometry in the document)"));

    _page_windows.add_group_header( _("Saving dialogs status"));
    _page_windows.add_line( true, "", _win_save_dialog_pos_off, "",
                            _("Don't save dialogs status"));
    _page_windows.add_line( true, "", _win_save_dialog_pos_on, "",
                            _("Save and restore dialogs status (the last open windows dialogs are saved when it closes)"));



    _page_windows.add_group_header( _("Dialog behavior (requires restart)"));
    _page_windows.add_line( true, "", _win_dockable, "",
                            _("Dockable"));
    _page_windows.add_line( true, "", _win_floating, "",
                            _("Floating"));
#ifdef WIN32
    _page_windows.add_group_header( _("Desktop integration"));
    _page_windows.add_line( true, "", _win_native, "",
                            _("Use Windows like open and save dialogs"));
    _page_windows.add_line( true, "", _win_gtk, "",
                            _("Use GTK open and save dialogs "));
#endif

#ifndef WIN32 // non-Win32 special code to enable transient dialogs
    _page_windows.add_group_header( _("Dialogs on top:"));

    _page_windows.add_line( true, "", _win_ontop_none, "",
                            _("Dialogs are treated as regular windows"));
    _page_windows.add_line( true, "", _win_ontop_normal, "",
                            _("Dialogs stay on top of document windows"));
    _page_windows.add_line( true, "", _win_ontop_agressive, "",
                            _("Same as Normal but may work better with some window managers"));
#endif

    _page_windows.add_group_header( _("Dialog Transparency"));
    _win_trans_focus.init("/dialogs/transparency/on-focus", 0.5, 1.0, 0.01, 0.1, 1.0, false, false);
    _page_windows.add_line( true, _("_Opacity when focused:"), _win_trans_focus, "", "", false);
    _win_trans_blur.init("/dialogs/transparency/on-blur", 0.0, 1.0, 0.01, 0.1, 0.5, false, false);
    _page_windows.add_line( true, _("Opacity when _unfocused:"), _win_trans_blur, "", "", false);
    _win_trans_time.init("/dialogs/transparency/animate-time", 0, 1000, 10, 100, 100, true, false);
    _page_windows.add_line( true, _("_Time of opacity change animation:"), _win_trans_time, "ms", "", false);


    _page_windows.add_group_header( _("Miscellaneous"));
#ifndef WIN32 // FIXME: Temporary Win32 special code to enable transient dialogs
    _page_windows.add_line( true, "", _win_hide_task, "",
                            _("Whether dialog windows are to be hidden in the window manager taskbar"));
#endif
    _page_windows.add_line( true, "", _win_zoom_resize, "",
                            _("Zoom drawing when document window is resized, to keep the same area visible (this is the default which can be changed in any window using the button above the right scrollbar)"));
    _page_windows.add_line( true, "", _win_save_viewport, "",
                            _("Save documents viewport (zoom and panning position). Useful to turn off when sharing version controlled files."));
    _page_windows.add_line( true, "", _win_show_close, "",
                            _("Whether dialog windows have a close button (requires restart)"));
    this->AddPage(_page_windows, _("Windows"), iter_ui, PREFS_PAGE_UI_WINDOWS);

    // Grids
    _page_grids.add_group_header( _("Line color when zooming out"));

    _grids_no_emphasize_on_zoom.init( _("Minor grid line color"), "/options/grids/no_emphasize_when_zoomedout", 1, true, 0);
    _page_grids.add_line( true, "", _grids_no_emphasize_on_zoom, "", _("The gridlines will be shown in minor grid line color"), false);
    _grids_emphasize_on_zoom.init( _("Major grid line color"), "/options/grids/no_emphasize_when_zoomedout", 0, false, &_grids_no_emphasize_on_zoom);
    _page_grids.add_line( true, "", _grids_emphasize_on_zoom, "", _("The gridlines will be shown in major grid line color"), false);

    _page_grids.add_group_header( _("Default grid settings"));

    _page_grids.add_line( true, "", _grids_notebook, "", "", false);
    _grids_notebook.append_page(_grids_xy,     CanvasGrid::getName( GRID_RECTANGULAR ));
    _grids_notebook.append_page(_grids_axonom, CanvasGrid::getName( GRID_AXONOMETRIC ));
        _grids_xy_units.init("/options/grids/xy/units");
        _grids_xy.add_line( false, _("Grid units:"), _grids_xy_units, "", "", false);
        _grids_xy_origin_x.init("/options/grids/xy/origin_x", -10000.0, 10000.0, 0.1, 1.0, 0.0, false, false);
        _grids_xy_origin_y.init("/options/grids/xy/origin_y", -10000.0, 10000.0, 0.1, 1.0, 0.0, false, false);
        _grids_xy_origin_x.set_digits(5);
        _grids_xy_origin_y.set_digits(5);
        _grids_xy.add_line( false, _("Origin X:"), _grids_xy_origin_x, "", _("X coordinate of grid origin"), false);
        _grids_xy.add_line( false, _("Origin Y:"), _grids_xy_origin_y, "", _("Y coordinate of grid origin"), false);
        _grids_xy_spacing_x.init("/options/grids/xy/spacing_x", -10000.0, 10000.0, 0.1, 1.0, 1.0, false, false);
        _grids_xy_spacing_y.init("/options/grids/xy/spacing_y", -10000.0, 10000.0, 0.1, 1.0, 1.0, false, false);
        _grids_xy_spacing_x.set_digits(5);
        _grids_xy_spacing_y.set_digits(5);
        _grids_xy.add_line( false, _("Spacing X:"), _grids_xy_spacing_x, "", _("Distance between vertical grid lines"), false);
        _grids_xy.add_line( false, _("Spacing Y:"), _grids_xy_spacing_y, "", _("Distance between horizontal grid lines"), false);

        _grids_xy_color.init(_("Minor grid line color:"), "/options/grids/xy/color", 0x0000ff20);
        _grids_xy.add_line( false, _("Minor grid line color:"), _grids_xy_color, "", _("Color used for normal grid lines"), false);
        _grids_xy_empcolor.init(_("Major grid line color:"), "/options/grids/xy/empcolor", 0x0000ff40);
        _grids_xy.add_line( false, _("Major grid line color:"), _grids_xy_empcolor, "", _("Color used for major (highlighted) grid lines"), false);
        _grids_xy_empspacing.init("/options/grids/xy/empspacing", 1.0, 1000.0, 1.0, 5.0, 5.0, true, false);
        _grids_xy.add_line( false, _("Major grid line every:"), _grids_xy_empspacing, "", "", false);
        _grids_xy_dotted.init( _("Show dots instead of lines"), "/options/grids/xy/dotted", false);
        _grids_xy.add_line( false, "", _grids_xy_dotted, "", _("If set, display dots at gridpoints instead of gridlines"), false);

    // CanvasAxonomGrid properties:
        _grids_axonom_units.init("/options/grids/axonom/units");
        _grids_axonom.add_line( false, _("Grid units:"), _grids_axonom_units, "", "", false);
        _grids_axonom_origin_x.init("/options/grids/axonom/origin_x", -10000.0, 10000.0, 0.1, 1.0, 0.0, false, false);
        _grids_axonom_origin_y.init("/options/grids/axonom/origin_y", -10000.0, 10000.0, 0.1, 1.0, 0.0, false, false);
        _grids_axonom_origin_x.set_digits(5);
        _grids_axonom_origin_y.set_digits(5);
        _grids_axonom.add_line( false, _("Origin X:"), _grids_axonom_origin_x, "", _("X coordinate of grid origin"), false);
        _grids_axonom.add_line( false, _("Origin Y:"), _grids_axonom_origin_y, "", _("Y coordinate of grid origin"), false);
        _grids_axonom_spacing_y.init("/options/grids/axonom/spacing_y", -10000.0, 10000.0, 0.1, 1.0, 1.0, false, false);
        _grids_axonom_spacing_y.set_digits(5);
        _grids_axonom.add_line( false, _("Spacing Y:"), _grids_axonom_spacing_y, "", _("Base length of z-axis"), false);
        _grids_axonom_angle_x.init("/options/grids/axonom/angle_x", -360.0, 360.0, 1.0, 10.0, 30.0, false, false);
        _grids_axonom_angle_z.init("/options/grids/axonom/angle_z", -360.0, 360.0, 1.0, 10.0, 30.0, false, false);
        _grids_axonom.add_line( false, _("Angle X:"), _grids_axonom_angle_x, "", _("Angle of x-axis"), false);
        _grids_axonom.add_line( false, _("Angle Z:"), _grids_axonom_angle_z, "", _("Angle of z-axis"), false);
        _grids_axonom_color.init(_("Minor grid line color:"), "/options/grids/axonom/color", 0x0000ff20);
        _grids_axonom.add_line( false, _("Minor grid line color:"), _grids_axonom_color, "", _("Color used for normal grid lines"), false);
        _grids_axonom_empcolor.init(_("Major grid line color:"), "/options/grids/axonom/empcolor", 0x0000ff40);
        _grids_axonom.add_line( false, _("Major grid line color:"), _grids_axonom_empcolor, "", _("Color used for major (highlighted) grid lines"), false);
        _grids_axonom_empspacing.init("/options/grids/axonom/empspacing", 1.0, 1000.0, 1.0, 5.0, 5.0, true, false);
        _grids_axonom.add_line( false, _("Major grid line every:"), _grids_axonom_empspacing, "", "", false);

    this->AddPage(_page_grids, _("Grids"), iter_ui, PREFS_PAGE_UI_GRIDS);

    initKeyboardShortcuts(iter_ui);
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
static void profileComboChanged( Gtk::ComboBoxText* combo )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int rowNum = combo->get_active_row_number();
    if ( rowNum < 1 ) {
        prefs->setString("/options/displayprofile/uri", "");
    } else {
        Glib::ustring active = combo->get_active_text();

        Glib::ustring path = CMSSystem::getPathForProfile(active);
        if ( !path.empty() ) {
            prefs->setString("/options/displayprofile/uri", path);
        }
    }
}

static void proofComboChanged( Gtk::ComboBoxText* combo )
{
    Glib::ustring active = combo->get_active_text();
    Glib::ustring path = CMSSystem::getPathForProfile(active);

    if ( !path.empty() ) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString("/options/softproof/uri", path);
    }
}

static void gamutColorChanged( Gtk::ColorButton* btn ) {
#if WITH_GTKMM_3_0
    Gdk::RGBA rgba = btn->get_rgba();
    gushort r = rgba.get_red_u();
    gushort g = rgba.get_green_u();
    gushort b = rgba.get_blue_u();
#else
    Gdk::Color color = btn->get_color();
    gushort r = color.get_red();
    gushort g = color.get_green();
    gushort b = color.get_blue();
#endif

    gchar* tmp = g_strdup_printf("#%02x%02x%02x", (r >> 8), (g >> 8), (b >> 8) );

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString("/options/softproof/gamutcolor", tmp);
    g_free(tmp);
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

void InkscapePreferences::initPageIO()
{
    Gtk::TreeModel::iterator iter_io = this->AddPage(_page_io, _("Input/Output"), PREFS_PAGE_IO);
    _path_io = _page_list.get_model()->get_path(iter_io);

    _save_use_current_dir.init( _("Use current directory for \"Save As ...\""), "/dialogs/save_as/use_current_dir", true);
    _page_io.add_line( false, "", _save_use_current_dir, "",
                         _("When this option is on, the \"Save as...\" and \"Save a Copy...\" dialogs will always open in the directory where the currently open document is; when it's off, each will open in the directory where you last saved a file using it"), true);

    _misc_comment.init( _("Add label comments to printing output"), "/printing/debug/show-label-comments", false);
    _page_io.add_line( false, "", _misc_comment, "",
                           _("When on, a comment will be added to the raw print output, marking the rendered output for an object with its label"), true);

    _misc_default_metadata.init( _("Add default metadata to new documents"), "/metadata/addToNewFile", false);
    _page_io.add_line( false, "", _misc_default_metadata, "",
                           _("Add default metadata to new documents. Default metadata can be set from Document Properties->Metadata."), true);

    // Input devices options
    _mouse_sens.init ( "/options/cursortolerance/value", 0.0, 30.0, 1.0, 1.0, 8.0, true, false);
    _page_mouse.add_line( false, _("_Grab sensitivity:"), _mouse_sens, _("pixels (requires restart)"),
                           _("How close on the screen you need to be to an object to be able to grab it with mouse (in screen pixels)"), false);
    _mouse_thres.init ( "/options/dragtolerance/value", 0.0, 20.0, 1.0, 1.0, 4.0, true, false);
    _page_mouse.add_line( false, _("_Click/drag threshold:"), _mouse_thres, _("pixels"),
                           _("Maximum mouse drag (in screen pixels) which is considered a click, not a drag"), false);

    _mouse_grabsize.init("/options/grabsize/value", 1, 7, 1, 2, 3, 0);
    _page_mouse.add_line(false, _("_Handle size:"), _mouse_grabsize, "",
                         _("Set the relative size of node handles"), true);

    _mouse_use_ext_input.init( _("Use pressure-sensitive tablet (requires restart)"), "/options/useextinput/value", true);
    _page_mouse.add_line(false, "",_mouse_use_ext_input, "",
                        _("Use the capabilities of a tablet or other pressure-sensitive device. Disable this only if you have problems with the tablet (you can still use it as a mouse)"));

    _mouse_switch_on_ext_input.init( _("Switch tool based on tablet device (requires restart)"), "/options/switchonextinput/value", false);
    _page_mouse.add_line(false, "",_mouse_switch_on_ext_input, "",
                        _("Change tool as different devices are used on the tablet (pen, eraser, mouse)"));
    this->AddPage(_page_mouse, _("Input devices"), iter_io, PREFS_PAGE_IO_MOUSE);

    // SVG output options
    _svgoutput_usenamedcolors.init( _("Use named colors"), "/options/svgoutput/usenamedcolors", false);
    _page_svgoutput.add_line( false, "", _svgoutput_usenamedcolors, "", _("If set, write the CSS name of the color when available (e.g. 'red' or 'magenta') instead of the numeric value"), false);

    _page_svgoutput.add_group_header( _("XML formatting"));

    _svgoutput_inlineattrs.init( _("Inline attributes"), "/options/svgoutput/inlineattrs", false);
    _page_svgoutput.add_line( true, "", _svgoutput_inlineattrs, "", _("Put attributes on the same line as the element tag"), false);

    _svgoutput_indent.init("/options/svgoutput/indent", 0.0, 1000.0, 1.0, 2.0, 2.0, true, false);
    _page_svgoutput.add_line( true, _("_Indent, spaces:"), _svgoutput_indent, "", _("The number of spaces to use for indenting nested elements; set to 0 for no indentation"), false);

    _page_svgoutput.add_group_header( _("Path data"));

    int const numPathstringFormat = 3;
    Glib::ustring pathstringFormatLabels[numPathstringFormat] = {_("Absolute"), _("Relative"), _("Optimized")};
    int pathstringFormatValues[numPathstringFormat] = {0, 1, 2};

    _svgoutput_pathformat.init("/options/svgoutput/pathstring_format", pathstringFormatLabels, pathstringFormatValues, numPathstringFormat, 2);
    _page_svgoutput.add_line( true, _("Path string format:"), _svgoutput_pathformat, "", _("Path data should be written: only with absolute coordinates, only with relative coordinates, or optimized for string length (mixed absolute and relative coordinates)"), false);

    _svgoutput_forcerepeatcommands.init( _("Force repeat commands"), "/options/svgoutput/forcerepeatcommands", false);
    _page_svgoutput.add_line( true, "", _svgoutput_forcerepeatcommands, "", _("Force repeating of the same path command (for example, 'L 1,2 L 3,4' instead of 'L 1,2 3,4')"), false);

    _page_svgoutput.add_group_header( _("Numbers"));

    _svgoutput_numericprecision.init("/options/svgoutput/numericprecision", 1.0, 16.0, 1.0, 2.0, 8.0, true, false);
    _page_svgoutput.add_line( true, _("_Numeric precision:"), _svgoutput_numericprecision, "", _("Significant figures of the values written to the SVG file"), false);

    _svgoutput_minimumexponent.init("/options/svgoutput/minimumexponent", -32.0, -1, 1.0, 2.0, -8.0, true, false);
    _page_svgoutput.add_line( true, _("Minimum _exponent:"), _svgoutput_minimumexponent, "", _("The smallest number written to SVG is 10 to the power of this exponent; anything smaller is written as zero"), false);

    /* Code to add controls for attribute checking options */

    /* Add incorrect style properties options  */
    _page_svgoutput.add_group_header( _("Improper Attributes Actions"));

    _svgoutput_attrwarn.init( _("Print warnings"), "/options/svgoutput/incorrect_attributes_warn", true);
    _page_svgoutput.add_line( true, "", _svgoutput_attrwarn, "", _("Print warning if invalid or non-useful attributes found. Database files located in inkscape_data_dir/attributes."), false);
    _svgoutput_attrremove.init( _("Remove attributes"), "/options/svgoutput/incorrect_attributes_remove", false);
    _page_svgoutput.add_line( true, "", _svgoutput_attrremove, "", _("Delete invalid or non-useful attributes from element tag"), false);

    /* Add incorrect style properties options  */
    _page_svgoutput.add_group_header( _("Inappropriate Style Properties Actions"));

    _svgoutput_stylepropwarn.init( _("Print warnings"), "/options/svgoutput/incorrect_style_properties_warn", true);
    _page_svgoutput.add_line( true, "", _svgoutput_stylepropwarn, "", _("Print warning if inappropriate style properties found (i.e. 'font-family' set on a <rect>). Database files located in inkscape_data_dir/attributes."), false);
    _svgoutput_stylepropremove.init( _("Remove style properties"), "/options/svgoutput/incorrect_style_properties_remove", false);
    _page_svgoutput.add_line( true, "", _svgoutput_stylepropremove, "", _("Delete inappropriate style properties"), false);

    /* Add default or inherited style properties options  */
    _page_svgoutput.add_group_header( _("Non-useful Style Properties Actions"));

    _svgoutput_styledefaultswarn.init( _("Print warnings"), "/options/svgoutput/style_defaults_warn", true);
    _page_svgoutput.add_line( true, "", _svgoutput_styledefaultswarn, "", _("Print warning if redundant style properties found (i.e. if a property has the default value and a different value is not inherited or if value is the same as would be inherited). Database files located in inkscape_data_dir/attributes."), false);
    _svgoutput_styledefaultsremove.init( _("Remove style properties"), "/options/svgoutput/style_defaults_remove", false);
    _page_svgoutput.add_line( true, "", _svgoutput_styledefaultsremove, "", _("Delete redundant style properties"), false);

    _page_svgoutput.add_group_header( _("Check Attributes and Style Properties on"));

    _svgoutput_check_reading.init( _("Reading"), "/options/svgoutput/check_on_reading", false);
    _page_svgoutput.add_line( true, "", _svgoutput_check_reading, "", _("Check attributes and style properties on reading in SVG files (including those internal to Inkscape which will slow down startup)"), false);
    _svgoutput_check_editing.init( _("Editing"), "/options/svgoutput/check_on_editing", false);
    _page_svgoutput.add_line( true, "", _svgoutput_check_editing, "", _("Check attributes and style properties while editing SVG files (may slow down Inkscape, mostly useful for debugging)"), false);
    _svgoutput_check_writing.init( _("Writing"), "/options/svgoutput/check_on_writing", true);
    _page_svgoutput.add_line( true, "", _svgoutput_check_writing, "", _("Check attributes and style properties on writing out SVG files"), false);

    this->AddPage(_page_svgoutput, _("SVG output"), iter_io, PREFS_PAGE_IO_SVGOUTPUT);

    // CMS options
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int const numIntents = 4;
    /* TRANSLATORS: see http://www.newsandtech.com/issues/2004/03-04/pt/03-04_rendering.htm */
    Glib::ustring intentLabels[numIntents] = {_("Perceptual"), _("Relative Colorimetric"), _("Saturation"), _("Absolute Colorimetric")};
    int intentValues[numIntents] = {0, 1, 2, 3};

#if !defined(HAVE_LIBLCMS1) && !defined(HAVE_LIBLCMS2)
    Gtk::Label* lbl = new Gtk::Label(_("(Note: Color management has been disabled in this build)"));
    _page_cms.add_line( false, "", *lbl, "", "", true);
#endif // !defined(HAVE_LIBLCMS1) && !defined(HAVE_LIBLCMS2)

    _page_cms.add_group_header( _("Display adjustment"));

    Glib::ustring tmpStr;
    std::vector<Glib::ustring> sources = ColorProfile::getBaseProfileDirs();
    for ( std::vector<Glib::ustring>::const_iterator it = sources.begin(); it != sources.end(); ++it ) {
        gchar* part = g_strdup_printf( "\n%s", it->c_str() );
        tmpStr += part;
        g_free(part);
    }

    gchar* profileTip = g_strdup_printf(_("The ICC profile to use to calibrate display output.\nSearched directories:%s"), tmpStr.c_str());
    _page_cms.add_line( true, _("Display profile:"), _cms_display_profile, "",
                        profileTip, false);
    g_free(profileTip);
    profileTip = 0;

    _cms_from_display.init( _("Retrieve profile from display"), "/options/displayprofile/from_display", false);
    _page_cms.add_line( true, "", _cms_from_display, "",
#ifdef GDK_WINDOWING_X11
                        _("Retrieve profiles from those attached to displays via XICC"), false);
#else
                        _("Retrieve profiles from those attached to displays"), false);
#endif // GDK_WINDOWING_X11


    _cms_intent.init("/options/displayprofile/intent", intentLabels, intentValues, numIntents, 0);
    _page_cms.add_line( true, _("Display rendering intent:"), _cms_intent, "",
                        _("The rendering intent to use to calibrate display output"), false);

    _page_cms.add_group_header( _("Proofing"));

    _cms_softproof.init( _("Simulate output on screen"), "/options/softproof/enable", false);
    _page_cms.add_line( true, "", _cms_softproof, "",
                        _("Simulates output of target device"), false);

    _cms_gamutwarn.init( _("Mark out of gamut colors"), "/options/softproof/gamutwarn", false);
    _page_cms.add_line( true, "", _cms_gamutwarn, "",
                        _("Highlights colors that are out of gamut for the target device"), false);

    Glib::ustring colorStr = prefs->getString("/options/softproof/gamutcolor");

#if WITH_GTKMM_3_0
    Gdk::RGBA tmpColor( colorStr.empty() ? "#00ff00" : colorStr);
    _cms_gamutcolor.set_rgba( tmpColor );
#else
    Gdk::Color tmpColor( colorStr.empty() ? "#00ff00" : colorStr);
    _cms_gamutcolor.set_color( tmpColor );
#endif

    _page_cms.add_line( true, _("Out of gamut warning color:"), _cms_gamutcolor, "",
                        _("Selects the color used for out of gamut warning"), false);

    _page_cms.add_line( true, _("Device profile:"), _cms_proof_profile, "",
                        _("The ICC profile to use to simulate device output"), false);

    _cms_proof_intent.init("/options/softproof/intent", intentLabels, intentValues, numIntents, 0);
    _page_cms.add_line( true, _("Device rendering intent:"), _cms_proof_intent, "",
                        _("The rendering intent to use to calibrate device output"), false);

    _cms_proof_blackpoint.init( _("Black point compensation"), "/options/softproof/bpc", false);
    _page_cms.add_line( true, "", _cms_proof_blackpoint, "",
                        _("Enables black point compensation"), false);

    _cms_proof_preserveblack.init( _("Preserve black"), "/options/softproof/preserveblack", false);

#if !defined(HAVE_LIBLCMS2)
    _page_cms.add_line( true, "", _cms_proof_preserveblack,
#if defined(cmsFLAGS_PRESERVEBLACK)
                        "",
#else
                        _("(LittleCMS 1.15 or later required)"),
#endif // defined(cmsFLAGS_PRESERVEBLACK)
                        _("Preserve K channel in CMYK -> CMYK transforms"), false);
#endif // !defined(HAVE_LIBLCMS2)

#if !defined(cmsFLAGS_PRESERVEBLACK)
    _cms_proof_preserveblack.set_sensitive( false );
#endif // !defined(cmsFLAGS_PRESERVEBLACK)


#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    {
        std::vector<Glib::ustring> names = ::Inkscape::CMSSystem::getDisplayNames();
        Glib::ustring current = prefs->getString( "/options/displayprofile/uri" );

        gint index = 0;
        _cms_display_profile.append(_("<none>"));
        index++;
        for ( std::vector<Glib::ustring>::iterator it = names.begin(); it != names.end(); ++it ) {
            _cms_display_profile.append( *it );
            Glib::ustring path = CMSSystem::getPathForProfile(*it);
            if ( !path.empty() && path == current ) {
                _cms_display_profile.set_active(index);
            }
            index++;
        }
        if ( current.empty() ) {
            _cms_display_profile.set_active(0);
        }

        names = ::Inkscape::CMSSystem::getSoftproofNames();
        current = prefs->getString("/options/softproof/uri");
        index = 0;
        for ( std::vector<Glib::ustring>::iterator it = names.begin(); it != names.end(); ++it ) {
            _cms_proof_profile.append( *it );
            Glib::ustring path = CMSSystem::getPathForProfile(*it);
            if ( !path.empty() && path == current ) {
                _cms_proof_profile.set_active(index);
            }
            index++;
        }
    }

    _cms_gamutcolor.signal_color_set().connect( sigc::bind( sigc::ptr_fun(gamutColorChanged), &_cms_gamutcolor) );

    _cms_display_profile.signal_changed().connect( sigc::bind( sigc::ptr_fun(profileComboChanged), &_cms_display_profile) );
    _cms_proof_profile.signal_changed().connect( sigc::bind( sigc::ptr_fun(proofComboChanged), &_cms_proof_profile) );
#else
    // disable it, but leave it visible
    _cms_intent.set_sensitive( false );
    _cms_display_profile.set_sensitive( false );
    _cms_from_display.set_sensitive( false );
    _cms_softproof.set_sensitive( false );
    _cms_gamutwarn.set_sensitive( false );
    _cms_gamutcolor.set_sensitive( false );
    _cms_proof_intent.set_sensitive( false );
    _cms_proof_profile.set_sensitive( false );
    _cms_proof_blackpoint.set_sensitive( false );
    _cms_proof_preserveblack.set_sensitive( false );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    this->AddPage(_page_cms, _("Color management"), iter_io, PREFS_PAGE_IO_CMS);

    // Autosave options
    _save_autosave_enable.init( _("Enable autosave (requires restart)"), "/options/autosave/enable", false);
    _page_autosave.add_line(false, "", _save_autosave_enable, "", _("Automatically save the current document(s) at a given interval, thus minimizing loss in case of a crash"), false);
    _save_autosave_path.init("/options/autosave/path", true);
    if (prefs->getString("/options/autosave/path").empty()) {
        // Show the default fallback "tmp dir" if autosave path is not set.
        _save_autosave_path.set_text(Glib::get_tmp_dir());
    }
    _page_autosave.add_line(false, C_("Filesystem", "Autosave _directory:"), _save_autosave_path, "", _("The directory where autosaves will be written. This should be an absolute path (starts with / on UNIX or a drive letter such as C: on Windows). "), false);
    _save_autosave_interval.init("/options/autosave/interval", 1.0, 10800.0, 1.0, 10.0, 10.0, true, false);
    _page_autosave.add_line(false, _("_Interval (in minutes):"), _save_autosave_interval, "", _("Interval (in minutes) at which document will be autosaved"), false);
    _save_autosave_max.init("/options/autosave/max", 1.0, 100.0, 1.0, 10.0, 10.0, true, false);
    _page_autosave.add_line(false, _("_Maximum number of autosaves:"), _save_autosave_max, "", _("Maximum number of autosaved files; use this to limit the storage space used"), false);

    /* When changing the interval or enabling/disabling the autosave function,
     * update our running configuration
     *
     * FIXME!
     * the inkscape_autosave_init should be called AFTER the values have been changed
     * (which cannot be guaranteed from here) - use a PrefObserver somewhere
     */
    /*
    _autosave_autosave_enable.signal_toggled().connect( sigc::ptr_fun(inkscape_autosave_init), TRUE );
    _autosave_autosave_interval.signal_changed().connect( sigc::ptr_fun(inkscape_autosave_init), TRUE );
    */

    // -----------
    this->AddPage(_page_autosave, _("Autosave"), iter_io, PREFS_PAGE_IO_AUTOSAVE);

    // Open Clip Art options
    _importexport_ocal_url.init("/options/ocalurl/str", true, g_strdup_printf("openclipart.org"));
    _page_openclipart.add_line( false, _("Open Clip Art Library _Server Name:"), _importexport_ocal_url, "",
        _("The server name of the Open Clip Art Library webdav server; it's used by the Import and Export to OCAL function"), true);
    _importexport_ocal_username.init("/options/ocalusername/str", true);
    _page_openclipart.add_line( false, _("Open Clip Art Library _Username:"), _importexport_ocal_username, "",
            _("The username used to log into Open Clip Art Library"), true);
    _importexport_ocal_password.init("/options/ocalpassword/str", false);
    _page_openclipart.add_line( false, _("Open Clip Art Library _Password:"), _importexport_ocal_password, "",
            _("The password used to log into Open Clip Art Library"), true);
    this->AddPage(_page_openclipart, _("Open Clip Art"), iter_io, PREFS_PAGE_IO_OPENCLIPART);
}

void InkscapePreferences::initPageBehavior()
{
    Gtk::TreeModel::iterator iter_behavior = this->AddPage(_page_behavior, _("Behavior"), PREFS_PAGE_BEHAVIOR);
    _path_behavior = _page_list.get_model()->get_path(iter_behavior);

    _misc_simpl.init("/options/simplifythreshold/value", 0.0001, 1.0, 0.0001, 0.0010, 0.0010, false, false);
    _page_behavior.add_line( false, _("_Simplification threshold:"), _misc_simpl, "",
                           _("How strong is the Node tool's Simplify command by default. If you invoke this command several times in quick succession, it will act more and more aggressively; invoking it again after a pause restores the default threshold."), false);

    _markers_color_stock.init ( _("Color stock markers the same color as object"), "/options/markers/colorStockMarkers", true);
    _markers_color_custom.init ( _("Color custom markers the same color as object"), "/options/markers/colorCustomMarkers", false);
    _markers_color_update.init ( _("Update marker color when object color changes"), "/options/markers/colorUpdateMarkers", true);

    // Selecting options
    _sel_all.init ( _("Select in all layers"), "/options/kbselection/inlayer", PREFS_SELECTION_ALL, false, 0);
    _sel_current.init ( _("Select only within current layer"), "/options/kbselection/inlayer", PREFS_SELECTION_LAYER, true, &_sel_all);
    _sel_recursive.init ( _("Select in current layer and sublayers"), "/options/kbselection/inlayer", PREFS_SELECTION_LAYER_RECURSIVE, false, &_sel_all);
    _sel_hidden.init ( _("Ignore hidden objects and layers"), "/options/kbselection/onlyvisible", true);
    _sel_locked.init ( _("Ignore locked objects and layers"), "/options/kbselection/onlysensitive", true);
    _sel_layer_deselects.init ( _("Deselect upon layer change"), "/options/selection/layerdeselect", true);

    _page_select.add_line( false, "", _sel_layer_deselects, "",
                           _("Uncheck this to be able to keep the current objects selected when the current layer changes"));

    _page_select.add_group_header( _("Ctrl+A, Tab, Shift+Tab"));
    _page_select.add_line( true, "", _sel_all, "",
                           _("Make keyboard selection commands work on objects in all layers"));
    _page_select.add_line( true, "", _sel_current, "",
                           _("Make keyboard selection commands work on objects in current layer only"));
    _page_select.add_line( true, "", _sel_recursive, "",
                           _("Make keyboard selection commands work on objects in current layer and all its sublayers"));
    _page_select.add_line( true, "", _sel_hidden, "",
                           _("Uncheck this to be able to select objects that are hidden (either by themselves or by being in a hidden layer)"));
    _page_select.add_line( true, "", _sel_locked, "",
                           _("Uncheck this to be able to select objects that are locked (either by themselves or by being in a locked layer)"));

    _sel_cycle.init ( _("Wrap when cycling objects in z-order"), "/options/selection/cycleWrap", true);

    _page_select.add_group_header( _("Alt+Scroll Wheel"));
    _page_select.add_line( true, "", _sel_cycle, "",
                           _("Wrap around at start and end when cycling objects in z-order"));

    this->AddPage(_page_select, _("Selecting"), iter_behavior, PREFS_PAGE_BEHAVIOR_SELECTING);

    // Transforms options
    _trans_scale_stroke.init ( _("Scale stroke width"), "/options/transform/stroke", true);
    _trans_scale_corner.init ( _("Scale rounded corners in rectangles"), "/options/transform/rectcorners", false);
    _trans_gradient.init ( _("Transform gradients"), "/options/transform/gradient", true);
    _trans_pattern.init ( _("Transform patterns"), "/options/transform/pattern", false);
    _trans_optimized.init ( _("Optimized"), "/options/preservetransform/value", 0, true, 0);
    _trans_preserved.init ( _("Preserved"), "/options/preservetransform/value", 1, false, &_trans_optimized);

    _page_transforms.add_line( false, "", _trans_scale_stroke, "",
                               _("When scaling objects, scale the stroke width by the same proportion"));
    _page_transforms.add_line( false, "", _trans_scale_corner, "",
                               _("When scaling rectangles, scale the radii of rounded corners"));
    _page_transforms.add_line( false, "", _trans_gradient, "",
                               _("Move gradients (in fill or stroke) along with the objects"));
    _page_transforms.add_line( false, "", _trans_pattern, "",
                               _("Move patterns (in fill or stroke) along with the objects"));
    _page_transforms.add_group_header( _("Store transformation"));
    _page_transforms.add_line( true, "", _trans_optimized, "",
                               _("If possible, apply transformation to objects without adding a transform= attribute"));
    _page_transforms.add_line( true, "", _trans_preserved, "",
                               _("Always store transformation as a transform= attribute on objects"));

    this->AddPage(_page_transforms, _("Transforms"), iter_behavior, PREFS_PAGE_BEHAVIOR_TRANSFORMS);

    // Scrolling options
    _scroll_wheel.init ( "/options/wheelscroll/value", 0.0, 1000.0, 1.0, 1.0, 40.0, true, false);
    _page_scrolling.add_line( false, _("Mouse _wheel scrolls by:"), _scroll_wheel, _("pixels"),
                           _("One mouse wheel notch scrolls by this distance in screen pixels (horizontally with Shift)"), false);
    _page_scrolling.add_group_header( _("Ctrl+arrows"));
    _scroll_arrow_px.init ( "/options/keyscroll/value", 0.0, 1000.0, 1.0, 1.0, 10.0, true, false);
    _page_scrolling.add_line( true, _("Sc_roll by:"), _scroll_arrow_px, _("pixels"),
                           _("Pressing Ctrl+arrow key scrolls by this distance (in screen pixels)"), false);
    _scroll_arrow_acc.init ( "/options/scrollingacceleration/value", 0.0, 5.0, 0.01, 1.0, 0.35, false, false);
    _page_scrolling.add_line( true, _("_Acceleration:"), _scroll_arrow_acc, "",
                           _("Pressing and holding Ctrl+arrow will gradually speed up scrolling (0 for no acceleration)"), false);
    _page_scrolling.add_group_header( _("Autoscrolling"));
    _scroll_auto_speed.init ( "/options/autoscrollspeed/value", 0.0, 5.0, 0.01, 1.0, 0.7, false, false);
    _page_scrolling.add_line( true, _("_Speed:"), _scroll_auto_speed, "",
                           _("How fast the canvas autoscrolls when you drag beyond canvas edge (0 to turn autoscroll off)"), false);
    _scroll_auto_thres.init ( "/options/autoscrolldistance/value", -600.0, 600.0, 1.0, 1.0, -10.0, true, false);
    _page_scrolling.add_line( true, _("_Threshold:"), _scroll_auto_thres, _("pixels"),
                           _("How far (in screen pixels) you need to be from the canvas edge to trigger autoscroll; positive is outside the canvas, negative is within the canvas"), false);
/*
    _scroll_space.init ( _("Left mouse button pans when Space is pressed"), "/options/spacepans/value", false);
    _page_scrolling.add_line( false, "", _scroll_space, "",
                            _("When on, pressing and holding Space and dragging with left mouse button pans canvas (as in Adobe Illustrator); when off, Space temporarily switches to Selector tool (default)"));
*/
    _wheel_zoom.init ( _("Mouse wheel zooms by default"), "/options/wheelzooms/value", false);
    _page_scrolling.add_line( false, "", _wheel_zoom, "",
                            _("When on, mouse wheel zooms without Ctrl and scrolls canvas with Ctrl; when off, it zooms with Ctrl and scrolls without Ctrl"));
    this->AddPage(_page_scrolling, _("Scrolling"), iter_behavior, PREFS_PAGE_BEHAVIOR_SCROLLING);

    // Snapping options
    _snap_indicator.init( _("Enable snap indicator"), "/options/snapindicator/value", true);
    _page_snapping.add_line( false, "", _snap_indicator, "",
                             _("After snapping, a symbol is drawn at the point that has snapped"));

    _snap_delay.init("/options/snapdelay/value", 0, 1000, 50, 100, 300, 0);
    _page_snapping.add_line( false, _("_Delay (in ms):"), _snap_delay, "",
                             _("Postpone snapping as long as the mouse is moving, and then wait an additional fraction of a second. This additional delay is specified here. When set to zero or to a very small number, snapping will be immediate."), true);

    _snap_closest_only.init( _("Only snap the node closest to the pointer"), "/options/snapclosestonly/value", false);
    _page_snapping.add_line( false, "", _snap_closest_only, "",
                             _("Only try to snap the node that is initially closest to the mouse pointer"));

    _snap_weight.init("/options/snapweight/value", 0, 1, 0.1, 0.2, 0.5, 1);
    _page_snapping.add_line( false, _("_Weight factor:"), _snap_weight, "",
                             _("When multiple snap solutions are found, then Inkscape can either prefer the closest transformation (when set to 0), or prefer the node that was initially the closest to the pointer (when set to 1)"), true);

    _snap_mouse_pointer.init( _("Snap the mouse pointer when dragging a constrained knot"), "/options/snapmousepointer/value", false);
    _page_snapping.add_line( false, "", _snap_mouse_pointer, "",
                             _("When dragging a knot along a constraint line, then snap the position of the mouse pointer instead of snapping the projection of the knot onto the constraint line"));

    this->AddPage(_page_snapping, _("Snapping"), iter_behavior, PREFS_PAGE_BEHAVIOR_SNAPPING);

    // Steps options
    _steps_arrow.init ( "/options/nudgedistance/value", 0.0, 1000.0, 0.01, 2.0, UNIT_TYPE_LINEAR, "px");
    //nudgedistance is limited to 1000 in select-context.cpp: use the same limit here
    _page_steps.add_line( false, _("_Arrow keys move by:"), _steps_arrow, "",
                          _("Pressing an arrow key moves selected object(s) or node(s) by this distance"), false);
    _steps_scale.init ( "/options/defaultscale/value", 0.0, 1000.0, 0.01, 2.0, UNIT_TYPE_LINEAR, "px");
    //defaultscale is limited to 1000 in select-context.cpp: use the same limit here
    _page_steps.add_line( false, _("> and < _scale by:"), _steps_scale, "",
                          _("Pressing > or < scales selection up or down by this increment"), false);
    _steps_inset.init ( "/options/defaultoffsetwidth/value", 0.0, 3000.0, 0.01, 2.0, UNIT_TYPE_LINEAR, "px");
    _page_steps.add_line( false, _("_Inset/Outset by:"), _steps_inset, "",
                          _("Inset and Outset commands displace the path by this distance"), false);
    _steps_compass.init ( _("Compass-like display of angles"), "/options/compassangledisplay/value", true);
    _page_steps.add_line( false, "", _steps_compass, "",
                            _("When on, angles are displayed with 0 at north, 0 to 360 range, positive clockwise; otherwise with 0 at east, -180 to 180 range, positive counterclockwise"));
    int const num_items = 17;
    Glib::ustring labels[num_items] = {"90", "60", "45", "36", "30", "22.5", "18", "15", "12", "10", "7.5", "6", "3", "2", "1", "0.5", C_("Rotation angle", "None")};
    int values[num_items] = {2, 3, 4, 5, 6, 8, 10, 12, 15, 18, 24, 30, 60, 90, 180, 360, 0};
    _steps_rot_snap.set_size_request(_sb_width);
    _steps_rot_snap.init("/options/rotationsnapsperpi/value", labels, values, num_items, 12);
    _page_steps.add_line( false, _("_Rotation snaps every:"), _steps_rot_snap, _("degrees"),
                           _("Rotating with Ctrl pressed snaps every that much degrees; also, pressing [ or ] rotates by this amount"), false);
    _steps_rot_relative.init ( _("Relative snapping of guideline angles"), "/options/relativeguiderotationsnap/value", false);
    _page_steps.add_line( false, "", _steps_rot_relative, "",
                            _("When on, the snap angles when rotating a guideline will be relative to the original angle"));
    _steps_zoom.init ( "/options/zoomincrement/value", 101.0, 500.0, 1.0, 1.0, 1.414213562, true, true);
    _page_steps.add_line( false, _("_Zoom in/out by:"), _steps_zoom, _("%"),
                          _("Zoom tool click, +/- keys, and middle click zoom in and out by this multiplier"), false);
    this->AddPage(_page_steps, _("Steps"), iter_behavior, PREFS_PAGE_BEHAVIOR_STEPS);

    // Clones options
    _clone_option_parallel.init ( _("Move in parallel"), "/options/clonecompensation/value",
                                  SP_CLONE_COMPENSATION_PARALLEL, true, 0);
    _clone_option_stay.init ( _("Stay unmoved"), "/options/clonecompensation/value",
                                  SP_CLONE_COMPENSATION_UNMOVED, false, &_clone_option_parallel);
    _clone_option_transform.init ( _("Move according to transform"), "/options/clonecompensation/value",
                                  SP_CLONE_COMPENSATION_NONE, false, &_clone_option_parallel);
    _clone_option_unlink.init ( _("Are unlinked"), "/options/cloneorphans/value",
                                  SP_CLONE_ORPHANS_UNLINK, true, 0);
    _clone_option_delete.init ( _("Are deleted"), "/options/cloneorphans/value",
                                  SP_CLONE_ORPHANS_DELETE, false, &_clone_option_unlink);

    _page_clones.add_group_header( _("Moving original: clones and linked offsets"));
    _page_clones.add_line(true, "", _clone_option_parallel, "",
                           _("Clones are translated by the same vector as their original"));
    _page_clones.add_line(true, "", _clone_option_stay, "",
                           _("Clones preserve their positions when their original is moved"));
    _page_clones.add_line(true, "", _clone_option_transform, "",
                           _("Each clone moves according to the value of its transform= attribute; for example, a rotated clone will move in a different direction than its original"));
    _page_clones.add_group_header( _("Deleting original: clones"));
    _page_clones.add_line(true, "", _clone_option_unlink, "",
                           _("Orphaned clones are converted to regular objects"));
    _page_clones.add_line(true, "", _clone_option_delete, "",
                           _("Orphaned clones are deleted along with their original"));

    _page_clones.add_group_header( _("Duplicating original+clones/linked offset"));

    _clone_relink_on_duplicate.init ( _("Relink duplicated clones"), "/options/relinkclonesonduplicate/value", false);
    _page_clones.add_line(true, "", _clone_relink_on_duplicate, "",
                        _("When duplicating a selection containing both a clone and its original (possibly in groups), relink the duplicated clone to the duplicated original instead of the old original"));

    //TRANSLATORS: Heading for the Inkscape Preferences "Clones" Page
    this->AddPage(_page_clones, _("Clones"), iter_behavior, PREFS_PAGE_BEHAVIOR_CLONES);

    // Clip paths and masks options
    _mask_mask_on_top.init ( _("When applying, use the topmost selected object as clippath/mask"), "/options/maskobject/topmost", true);
    _page_mask.add_line(false, "", _mask_mask_on_top, "",
                        _("Uncheck this to use the bottom selected object as the clipping path or mask"));
    _mask_mask_remove.init ( _("Remove clippath/mask object after applying"), "/options/maskobject/remove", true);
    _page_mask.add_line(false, "", _mask_mask_remove, "",
                        _("After applying, remove the object used as the clipping path or mask from the drawing"));
    
    _page_mask.add_group_header( _("Before applying"));
    
    _mask_grouping_none.init( _("Do not group clipped/masked objects"), "/options/maskobject/grouping", PREFS_MASKOBJECT_GROUPING_NONE, true, 0);
    _mask_grouping_separate.init( _("Put every clipped/masked object in its own group"), "/options/maskobject/grouping", PREFS_MASKOBJECT_GROUPING_SEPARATE, false, &_mask_grouping_none);
    _mask_grouping_all.init( _("Put all clipped/masked objects into one group"), "/options/maskobject/grouping", PREFS_MASKOBJECT_GROUPING_ALL, false, &_mask_grouping_none);
    
    _page_mask.add_line(true, "", _mask_grouping_none, "",
                        _("Apply clippath/mask to every object"));
    
    _page_mask.add_line(true, "", _mask_grouping_separate, "",
                        _("Apply clippath/mask to groups containing single object"));
    
    _page_mask.add_line(true, "", _mask_grouping_all, "",
                        _("Apply clippath/mask to group containing all objects"));
                        
    _page_mask.add_group_header( _("After releasing"));
    
    _mask_ungrouping.init ( _("Ungroup automatically created groups"), "/options/maskobject/ungrouping", true);
    _page_mask.add_line(true, "", _mask_ungrouping, "",
                        _("Ungroup groups created when setting clip/mask"));
    
    this->AddPage(_page_mask, _("Clippaths and masks"), iter_behavior, PREFS_PAGE_BEHAVIOR_MASKS);


    _page_markers.add_group_header( _("Stroke Style Markers"));
    _page_markers.add_line( true, "", _markers_color_stock, "",
                           _("Stroke color same as object, fill color either object fill color or marker fill color"));
    _page_markers.add_line( true, "", _markers_color_custom, "",
                           _("Stroke color same as object, fill color either object fill color or marker fill color"));
    _page_markers.add_line( true, "", _markers_color_update, "",
                           _("Update marker color when object color changes"));

    this->AddPage(_page_markers, _("Markers"), iter_behavior, PREFS_PAGE_BEHAVIOR_MARKERS);
    
    
    _page_cleanup.add_group_header( _("Document cleanup"));
    _cleanup_swatches.init ( _("Remove unused swatches when doing a document cleanup"), "/options/cleanupswatches/value", false); // text label
    _page_cleanup.add_line( true, "", _cleanup_swatches, "",
                           _("Remove unused swatches when doing a document cleanup")); // tooltip
    this->AddPage(_page_cleanup, _("Cleanup"), iter_behavior, PREFS_PAGE_BEHAVIOR_CLEANUP);
}

void InkscapePreferences::initPageRendering()
{

    /* threaded blur */ //related comments/widgets/functions should be renamed and option should be moved elsewhere when inkscape is fully multi-threaded
    _filter_multi_threaded.init("/options/threading/numthreads", 1.0, 8.0, 1.0, 2.0, 4.0, true, false);
    _page_rendering.add_line( false, _("Number of _Threads:"), _filter_multi_threaded, _("(requires restart)"),
                           _("Configure number of processors/threads to use when rendering filters"), false);

    // rendering cache
    _rendering_cache_size.init("/options/renderingcache/size", 0.0, 4096.0, 1.0, 32.0, 64.0, true, false);
    _page_rendering.add_line( false, _("Rendering _cache size:"), _rendering_cache_size, C_("mebibyte (2^20 bytes) abbreviation","MiB"), _("Set the amount of memory per document which can be used to store rendered parts of the drawing for later reuse; set to zero to disable caching"), false);

    /* blur quality */
    _blur_quality_best.init ( _("Best quality (slowest)"), "/options/blurquality/value",
                                  BLUR_QUALITY_BEST, false, 0);
    _blur_quality_better.init ( _("Better quality (slower)"), "/options/blurquality/value",
                                  BLUR_QUALITY_BETTER, false, &_blur_quality_best);
    _blur_quality_normal.init ( _("Average quality"), "/options/blurquality/value",
                                  BLUR_QUALITY_NORMAL, true, &_blur_quality_best);
    _blur_quality_worse.init ( _("Lower quality (faster)"), "/options/blurquality/value",
                                  BLUR_QUALITY_WORSE, false, &_blur_quality_best);
    _blur_quality_worst.init ( _("Lowest quality (fastest)"), "/options/blurquality/value",
                                  BLUR_QUALITY_WORST, false, &_blur_quality_best);

    _page_rendering.add_group_header( _("Gaussian blur quality for display"));
    _page_rendering.add_line( true, "", _blur_quality_best, "",
                           _("Best quality, but display may be very slow at high zooms (bitmap export always uses best quality)"));
    _page_rendering.add_line( true, "", _blur_quality_better, "",
                           _("Better quality, but slower display"));
    _page_rendering.add_line( true, "", _blur_quality_normal, "",
                           _("Average quality, acceptable display speed"));
    _page_rendering.add_line( true, "", _blur_quality_worse, "",
                           _("Lower quality (some artifacts), but display is faster"));
    _page_rendering.add_line( true, "", _blur_quality_worst, "",
                           _("Lowest quality (considerable artifacts), but display is fastest"));

    /* filter quality */
    _filter_quality_best.init ( _("Best quality (slowest)"), "/options/filterquality/value",
                                  Inkscape::Filters::FILTER_QUALITY_BEST, false, 0);
    _filter_quality_better.init ( _("Better quality (slower)"), "/options/filterquality/value",
                                  Inkscape::Filters::FILTER_QUALITY_BETTER, false, &_filter_quality_best);
    _filter_quality_normal.init ( _("Average quality"), "/options/filterquality/value",
                                  Inkscape::Filters::FILTER_QUALITY_NORMAL, true, &_filter_quality_best);
    _filter_quality_worse.init ( _("Lower quality (faster)"), "/options/filterquality/value",
                                  Inkscape::Filters::FILTER_QUALITY_WORSE, false, &_filter_quality_best);
    _filter_quality_worst.init ( _("Lowest quality (fastest)"), "/options/filterquality/value",
                                  Inkscape::Filters::FILTER_QUALITY_WORST, false, &_filter_quality_best);

    _page_rendering.add_group_header( _("Filter effects quality for display"));
    _page_rendering.add_line( true, "", _filter_quality_best, "",
                           _("Best quality, but display may be very slow at high zooms (bitmap export always uses best quality)"));
    _page_rendering.add_line( true, "", _filter_quality_better, "",
                           _("Better quality, but slower display"));
    _page_rendering.add_line( true, "", _filter_quality_normal, "",
                           _("Average quality, acceptable display speed"));
    _page_rendering.add_line( true, "", _filter_quality_worse, "",
                           _("Lower quality (some artifacts), but display is faster"));
    _page_rendering.add_line( true, "", _filter_quality_worst, "",
                           _("Lowest quality (considerable artifacts), but display is fastest"));

    this->AddPage(_page_rendering, _("Rendering"), PREFS_PAGE_RENDERING);
}

void InkscapePreferences::initPageBitmaps()
{
    /* Note: /options/bitmapoversample removed with Cairo renderer */
    _page_bitmaps.add_group_header( _("Edit"));
    _misc_bitmap_autoreload.init(_("Automatically reload bitmaps"), "/options/bitmapautoreload/value", true);
    _page_bitmaps.add_line( false, "", _misc_bitmap_autoreload, "",
                           _("Automatically reload linked images when file is changed on disk"));
    _misc_bitmap_editor.init("/options/bitmapeditor/value", true);
    _page_bitmaps.add_line( false, _("_Bitmap editor:"), _misc_bitmap_editor, "", "", true);

    _page_bitmaps.add_group_header( _("Export"));
    _importexport_export_res.init("/dialogs/export/defaultxdpi/value", 0.0, 6000.0, 1.0, 1.0, Inkscape::Util::Quantity::convert(1, "in", "px"), true, false);
    _page_bitmaps.add_line( false, _("Default export _resolution:"), _importexport_export_res, _("dpi"),
                            _("Default bitmap resolution (in dots per inch) in the Export dialog"), false);
    _page_bitmaps.add_group_header( _("Create"));
    _bitmap_copy_res.init("/options/createbitmap/resolution", 1.0, 6000.0, 1.0, 1.0, Inkscape::Util::Quantity::convert(1, "in", "px"), true, false);
    _page_bitmaps.add_line( false, _("Resolution for Create Bitmap _Copy:"), _bitmap_copy_res, _("dpi"),
                            _("Resolution used by the Create Bitmap Copy command"), false);

    _page_bitmaps.add_group_header( _("Import"));
    _bitmap_ask.init(_("Ask about linking and scaling when importing"), "/dialogs/import/ask", true);
    _page_bitmaps.add_line( true, "", _bitmap_ask, "",
                           _("Pop-up linking and scaling dialog when importing bitmap image."));

    {
        Glib::ustring labels[] = {_("Embed"), _("Link")};
        Glib::ustring values[] = {"embed", "link"};
        _bitmap_link.init("/dialogs/import/link", labels, values, G_N_ELEMENTS(values), "link");
        _page_bitmaps.add_line( false, _("Bitmap link:"), _bitmap_link, "", "", false);
    }

    {
        Glib::ustring labels[] = {_("None (auto)"), _("Smooth (optimizeQuality)"), _("Blocky (optimizeSpeed)") };
        Glib::ustring values[] = {"auto", "optimizeQuality", "optimizeSpeed"};
        _bitmap_scale.init("/dialogs/import/scale", labels, values, G_N_ELEMENTS(values), "scale");
        _page_bitmaps.add_line( false, _("Bitmap scale (image-rendering):"), _bitmap_scale, "", "", false);
    }

    /* Note: /dialogs/import/quality removed use of in r12542 */
    _importexport_import_res.init("/dialogs/import/defaultxdpi/value", 0.0, 6000.0, 1.0, 1.0, Inkscape::Util::Quantity::convert(1, "in", "px"), true, false);
    _page_bitmaps.add_line( false, _("Default _import resolution:"), _importexport_import_res, _("dpi"),
                            _("Default bitmap resolution (in dots per inch) for bitmap import"), false);
    _importexport_import_res_override.init(_("Override file resolution"), "/dialogs/import/forcexdpi", false);
    _page_bitmaps.add_line( false, "", _importexport_import_res_override, "",
                            _("Use default bitmap resolution in favor of information from file"));

    _page_bitmaps.add_group_header( _("Render"));
    // rendering outlines for pixmap image tags
    _rendering_image_outline.init( _("Images in Outline Mode"), "/options/rendering/imageinoutlinemode", false);
    _page_bitmaps.add_line(false, "", _rendering_image_outline, "", _("When active will render images while in outline mode instead of a red box with an x. This is useful for manual tracing."));

    this->AddPage(_page_bitmaps, _("Bitmaps"), PREFS_PAGE_BITMAPS);
}

void InkscapePreferences::initKeyboardShortcuts(Gtk::TreeModel::iterator iter_ui)
{
    std::vector<Glib::ustring> fileNames;
    std::vector<Glib::ustring> fileLabels;

    sp_shortcut_get_file_names(&fileLabels, &fileNames);

    _kb_filelist.init( "/options/kbshortcuts/shortcutfile", &fileLabels[0], &fileNames[0], fileLabels.size(), fileNames[0]);

    Glib::ustring tooltip(_("Select a file of predefined shortcuts to use. Any customized shortcuts you create will be added seperately to "));
    tooltip += Glib::ustring(IO::Resource::get_path(IO::Resource::USER, IO::Resource::KEYS, "default.xml"));

    _page_keyshortcuts.add_line( false, _("Shortcut file:"), _kb_filelist, "", tooltip.c_str(), false);

    _kb_search.init("/options/kbshortcuts/value", true);
    _page_keyshortcuts.add_line( false, _("Search:"), _kb_search, "", "", true);

    _kb_store = Gtk::TreeStore::create( _kb_columns );
    _kb_store->set_sort_column (_kb_columns.id, Gtk::SORT_ASCENDING );

    _kb_filter = Gtk::TreeModelFilter::create(_kb_store);
    _kb_filter->set_visible_func (sigc::mem_fun(*this, &InkscapePreferences::onKBSearchFilter));

    _kb_shortcut_renderer.property_editable() = true;

    _kb_tree.set_model(_kb_filter);
    _kb_tree.append_column(_("Name"), _kb_columns.name);
    _kb_tree.append_column(_("Shortcut"), _kb_shortcut_renderer);
    _kb_tree.append_column(_("Description"), _kb_columns.description);
    _kb_tree.append_column(_("ID"), _kb_columns.id);

    _kb_tree.set_expander_column(*_kb_tree.get_column(0));

    _kb_tree.get_column(0)->set_resizable(true);
    _kb_tree.get_column(0)->set_clickable(true);
    _kb_tree.get_column(0)->set_fixed_width (200);

    _kb_tree.get_column(1)->set_resizable(true);
    _kb_tree.get_column(1)->set_clickable(true);
    _kb_tree.get_column(1)->set_fixed_width (150);
    //_kb_tree.get_column(1)->add_attribute(_kb_shortcut_renderer.property_text(), _kb_columns.shortcut);
    _kb_tree.get_column(1)->set_cell_data_func(_kb_shortcut_renderer, sigc::ptr_fun(InkscapePreferences::onKBShortcutRenderer));

    _kb_tree.get_column(2)->set_resizable(true);
    _kb_tree.get_column(2)->set_clickable(true);

    _kb_tree.get_column(3)->set_resizable(true);
    _kb_tree.get_column(3)->set_clickable(true);

    _kb_shortcut_renderer.signal_accel_edited().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBTreeEdited) );
    _kb_shortcut_renderer.signal_accel_cleared().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBTreeCleared) );

    Gtk::ScrolledWindow* scroller = new Gtk::ScrolledWindow();
    scroller->add(_kb_tree);

    int row = 3;

#if WITH_GTKMM_3_0
    scroller->set_hexpand();
    scroller->set_vexpand();
    _page_keyshortcuts.attach(*scroller, 0, row, 2, 1);
#else
    _page_keyshortcuts.attach(*scroller, 0, 2, row, row+1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL);
#endif

    row++;

#if WITH_GTKMM_3_0
    Gtk::ButtonBox *box_buttons = Gtk::manage(new Gtk::ButtonBox);
#else
    Gtk::HButtonBox *box_buttons = Gtk::manage (new Gtk::HButtonBox);
#endif

    box_buttons->set_layout(Gtk::BUTTONBOX_END);
    box_buttons->set_spacing(4);

#if WITH_GTKMM_3_0
    box_buttons->set_hexpand();
    _page_keyshortcuts.attach(*box_buttons, 0, row, 3, 1);
#else
    _page_keyshortcuts.attach(*box_buttons, 0, 3, row, row+1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
#endif

    UI::Widget::Button *kb_reset = Gtk::manage(new UI::Widget::Button(_("Reset"), _("Remove all your customized keyboard shortcuts, and revert to the shortcuts in the shortcut file listed above")));
    box_buttons->pack_start(*kb_reset, true, true, 6);
    box_buttons->set_child_secondary(*kb_reset);

    UI::Widget::Button *kb_import = Gtk::manage(new UI::Widget::Button(_("Import ..."), _("Import custom keyboard shortcuts from a file")));
    box_buttons->pack_end(*kb_import, true, true, 6);

    UI::Widget::Button *kb_export = Gtk::manage(new UI::Widget::Button(_("Export ..."), _("Export custom keyboard shortcuts to a file")));
    box_buttons->pack_end(*kb_export, true, true, 6);

    kb_reset->signal_clicked().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBReset) );
    kb_import->signal_clicked().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBImport) );
    kb_export->signal_clicked().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBExport) );
    _kb_search.signal_key_release_event().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBSearchKeyEvent) );
    _kb_filelist.signal_changed().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBList) );
    _page_keyshortcuts.signal_realize().connect( sigc::mem_fun(*this, &InkscapePreferences::onKBRealize) );

    this->AddPage(_page_keyshortcuts, _("Keyboard Shortcuts"), iter_ui, PREFS_PAGE_UI_KEYBOARD_SHORTCUTS);

    _kb_shortcuts_loaded = false;
    Gtk::TreeStore::iterator iter_group = _kb_store->append();
    (*iter_group)[_kb_columns.name] = "Loading ...";
    (*iter_group)[_kb_columns.shortcut] = "";
    (*iter_group)[_kb_columns.id] = "";
    (*iter_group)[_kb_columns.description] = "";
    (*iter_group)[_kb_columns.shortcutid] = 0;
    (*iter_group)[_kb_columns.user_set] = 0;

}

void InkscapePreferences::onKBList()
{
    sp_shortcut_init();
    onKBListKeyboardShortcuts();
}

void InkscapePreferences::onKBReset()
{
    sp_shortcuts_delete_all_from_file();
    sp_shortcut_init();
    onKBListKeyboardShortcuts();
}

void InkscapePreferences::onKBImport()
{
    if (sp_shortcut_file_import()) {
        onKBListKeyboardShortcuts();
    }
}

void InkscapePreferences::onKBExport()
{
    sp_shortcut_file_export();
}

bool InkscapePreferences::onKBSearchKeyEvent(GdkEventKey * /*event*/)
{
    _kb_filter->refilter();
    return FALSE;
}

void InkscapePreferences::onKBTreeCleared(const Glib::ustring& path)
{
    Gtk::TreeModel::iterator iter = _kb_filter->get_iter(path);
    Glib::ustring id = (*iter)[_kb_columns.id];
    unsigned int const current_shortcut_id = (*iter)[_kb_columns.shortcutid];

    // Remove current shortcut from file
    sp_shortcut_delete_from_file(id.c_str(), current_shortcut_id);

    sp_shortcut_init();
    onKBListKeyboardShortcuts();

}

void InkscapePreferences::onKBTreeEdited (const Glib::ustring& path, guint accel_key, Gdk::ModifierType accel_mods, guint hardware_keycode)
{
    Gtk::TreeModel::iterator iter = _kb_filter->get_iter(path);

    Glib::ustring id = (*iter)[_kb_columns.id];
    Glib::ustring current_shortcut = (*iter)[_kb_columns.shortcut];
    unsigned int const current_shortcut_id = (*iter)[_kb_columns.shortcutid];

    Inkscape::Verb *const verb = Inkscape::Verb::getbyid(id.c_str());
    if (!verb) {
        return;
    }

    unsigned int const new_shortcut_id =  sp_gdkmodifier_to_shortcut(accel_key, accel_mods, hardware_keycode);
    if (new_shortcut_id) {

        // Delete current shortcut if it existed
        sp_shortcut_delete_from_file(id.c_str(), current_shortcut_id);
        // Delete any references to the new shortcut
        sp_shortcut_delete_from_file(id.c_str(), new_shortcut_id);
        // Add the new shortcut
        sp_shortcut_add_to_file(id.c_str(), new_shortcut_id);

        sp_shortcut_init();
        onKBListKeyboardShortcuts();
    }
}

bool InkscapePreferences::onKBSearchFilter(const Gtk::TreeModel::const_iterator& iter)
{
    Glib::ustring search = _kb_search.get_text().lowercase();
    if (search.empty()) {
        return TRUE;
    }

    Glib::ustring name = (*iter)[_kb_columns.name];
    Glib::ustring desc = (*iter)[_kb_columns.description];
    Glib::ustring shortcut = (*iter)[_kb_columns.shortcut];
    Glib::ustring id = (*iter)[_kb_columns.id];

    if (id.empty()) {
        return TRUE;    // Keep all group nodes visible
    }

    return (name.lowercase().find(search) != name.npos
            || shortcut.lowercase().find(search) != name.npos
            || desc.lowercase().find(search) != name.npos
            || id.lowercase().find(search) != name.npos);
}

void InkscapePreferences::onKBRealize()
{
    if (!_kb_shortcuts_loaded /*&& _current_page == &_page_keyshortcuts*/) {
        _kb_shortcuts_loaded = true;
        onKBListKeyboardShortcuts();
    }
}

InkscapePreferences::ModelColumns &InkscapePreferences::onKBGetCols()
{
    static InkscapePreferences::ModelColumns cols;
    return cols;
}

void InkscapePreferences::onKBShortcutRenderer(Gtk::CellRenderer *renderer, Gtk::TreeIter const &iter) {

    Glib::ustring shortcut = (*iter)[onKBGetCols().shortcut];
    unsigned int user_set = (*iter)[onKBGetCols().user_set];
    Gtk::CellRendererAccel *accel = dynamic_cast<Gtk::CellRendererAccel *>(renderer);
    if (user_set) {
        accel->property_markup() = Glib::ustring("<span foreground=\"blue\"> " + shortcut + " </span>").c_str();
    } else {
        accel->property_markup() = Glib::ustring("<span> " + shortcut + " </span>").c_str();
    }
}

void InkscapePreferences::onKBListKeyboardShortcuts()
{
    // Save the current selection
    Gtk::TreeStore::iterator iter = _kb_tree.get_selection()->get_selected();
    Glib::ustring selected_id = "";
    if (iter) {
        selected_id = (*iter)[_kb_columns.id];
    }

    _kb_store->clear();

    std::vector<Verb *>verbs = Inkscape::Verb::getList();

    for (unsigned int i = 0; i < verbs.size(); i++) {

        Inkscape::Verb* verb = verbs[i];
        if (!verb) {
            continue;
        }
        if (!verb->get_name()){
            continue;
        }

        Gtk::TreeStore::Path path;
        if (_kb_store->iter_is_valid(_kb_store->get_iter("0"))) {
            path = _kb_store->get_path(_kb_store->get_iter("0"));
        }

        // Find this group in the tree
        Glib::ustring group = verb->get_group() ? _(verb->get_group()) : _("Misc");
        Gtk::TreeStore::iterator iter_group;
        bool found = false;
        while (path) {
            iter_group = _kb_store->get_iter(path);
            if (!_kb_store->iter_is_valid(iter_group)) {
                break;
            }
            Glib::ustring name = (*iter_group)[_kb_columns.name];
            if ((*iter_group)[_kb_columns.name] == group) {
                found = true;
                break;
            }
            path.next();
        }

        if (!found) {
            // Add the group if not there
            iter_group = _kb_store->append();
            (*iter_group)[_kb_columns.name] = group;
            (*iter_group)[_kb_columns.shortcut] = "";
            (*iter_group)[_kb_columns.id] = "";
            (*iter_group)[_kb_columns.description] = "";
            (*iter_group)[_kb_columns.shortcutid] = 0;
            (*iter_group)[_kb_columns.user_set] = 0;
        }

        // Remove the key accelerators from the verb name
        Glib::ustring name = _(verb->get_name());
        std::string::size_type k = 0;
        while((k=name.find('_',k))!=name.npos) {
            name.erase(k, 1);
        }

        // Get the shortcut label
        unsigned int shortcut_id = sp_shortcut_get_primary(verb);
        Glib::ustring shortcut_label = "";
        if (shortcut_id != GDK_KEY_VoidSymbol) {
            gchar* str = sp_shortcut_get_label(shortcut_id);
            if (str) {
                shortcut_label = str;
                g_free(str);
                str = 0;
            }
        }
        // Add the verb to the group
        Gtk::TreeStore::iterator row = _kb_store->append(iter_group->children());
        (*row)[_kb_columns.name] =  name;
        (*row)[_kb_columns.shortcut] = shortcut_label;
        (*row)[_kb_columns.description] = verb->get_short_tip() ? _(verb->get_short_tip()) : "";
        (*row)[_kb_columns.shortcutid] = shortcut_id;
        (*row)[_kb_columns.id] = verb->get_id();
        (*row)[_kb_columns.user_set] = sp_shortcut_is_user_set(verb);

        if (selected_id == verb->get_id()) {
            Gtk::TreeStore::Path sel_path = _kb_filter->convert_child_path_to_path(_kb_store->get_path(row));
            _kb_tree.expand_to_path(sel_path);
            _kb_tree.get_selection()->select(sel_path);
        }
    }

    if (selected_id.empty()) {
        _kb_tree.expand_to_path(_kb_store->get_path(_kb_store->get_iter("0:1")));
    }

}

void InkscapePreferences::initPageSpellcheck()
{
#ifdef HAVE_ASPELL

    std::vector<Glib::ustring> languages;
    std::vector<Glib::ustring> langValues;

  AspellConfig *config = new_aspell_config();

#ifdef WIN32
    // on windows, dictionaries are in a lib/aspell-0.60 subdir off inkscape's executable dir;
    // this is some black magick to find out the executable path to give it to aspell
    char exeName[MAX_PATH+1];
    GetModuleFileName(NULL, exeName, MAX_PATH);
    char *slashPos = strrchr(exeName, '\\');
    if (slashPos)
    {
        *slashPos = '\0';
    }
    // g_print ("%s\n", exeName);
    aspell_config_replace(config, "prefix", exeName);
#endif

    /* the returned pointer should _not_ need to be deleted */
    AspellDictInfoList *dlist = get_aspell_dict_info_list(config);
    
    /* config is no longer needed */
    delete_aspell_config(config);
    
    AspellDictInfoEnumeration *dels = aspell_dict_info_list_elements(dlist);
    
    languages.push_back(Glib::ustring(C_("Spellchecker language", "None")));
    langValues.push_back(Glib::ustring(""));
    
    const AspellDictInfo *entry;
    int en_index = 0;
    int i = 0;
    while ( (entry = aspell_dict_info_enumeration_next(dels)) != 0)
    {
        languages.push_back(Glib::ustring(entry->name));
        langValues.push_back(Glib::ustring(entry->name));
        if (!strcmp (entry->name, "en"))
        {
            en_index = i;
        }
        i++;
    }

    delete_aspell_dict_info_enumeration(dels);

    _spell_language.init( "/dialogs/spellcheck/lang", &languages[0], &langValues[0], languages.size(), languages[en_index]);
    _page_spellcheck.add_line( false, _("Language:"), _spell_language, "",
                              _("Set the main spell check language"), false);

    _spell_language2.init( "/dialogs/spellcheck/lang2", &languages[0], &langValues[0], languages.size(), languages[0]);
    _page_spellcheck.add_line( false, _("Second language:"), _spell_language2, "",
                              _("Set the second spell check language; checking will only stop on words unknown in ALL chosen languages"), false);

    _spell_language3.init( "/dialogs/spellcheck/lang3", &languages[0], &langValues[0], languages.size(), languages[0]);
    _page_spellcheck.add_line( false, _("Third language:"), _spell_language3, "",
                              _("Set the third spell check language; checking will only stop on words unknown in ALL chosen languages"), false);

    _spell_ignorenumbers.init( _("Ignore words with digits"), "/dialogs/spellcheck/ignorenumbers", true);
    _page_spellcheck.add_line( false, "", _spell_ignorenumbers, "",
                           _("Ignore words containing digits, such as \"R2D2\""), true);

    _spell_ignoreallcaps.init( _("Ignore words in ALL CAPITALS"), "/dialogs/spellcheck/ignoreallcaps", false);
    _page_spellcheck.add_line( false, "", _spell_ignoreallcaps, "",
                           _("Ignore words in all capitals, such as \"IUPAC\""), true);

    this->AddPage(_page_spellcheck, _("Spellcheck"), PREFS_PAGE_SPELLCHECK);
#endif
}

static void appendList( Glib::ustring& tmp, const gchar* const*listing )
{
    bool first = true;
    for (const gchar* const* ptr = listing; *ptr; ptr++) {
        if (!first) {
            tmp += "  ";
        }
        first = false;
        tmp += *ptr;
        tmp += "\n";
    }
}

void InkscapePreferences::initPageSystem()
{
    _misc_latency_skew.init("/debug/latency/skew", 0.5, 2.0, 0.01, 0.10, 1.0, false, false);
    _page_system.add_line( false, _("Latency _skew:"), _misc_latency_skew, _("(requires restart)"),
                           _("Factor by which the event clock is skewed from the actual time (0.9766 on some systems)"), false);

    _misc_namedicon_delay.init( _("Pre-render named icons"), "/options/iconrender/named_nodelay", false);
    _page_system.add_line( false, "", _misc_namedicon_delay, "",
                           _("When on, named icons will be rendered before displaying the ui. This is for working around bugs in GTK+ named icon notification"), true);


    {
        // TRANSLATORS: following strings are paths in Inkscape preferences - Misc - System info

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        _page_system.add_group_header( _("System info"));

        _sys_user_config.set_text((char const *)Inkscape::Application::profile_path(""));
        _sys_user_config.set_editable(false);
        _page_system.add_line(true, _("User config: "), _sys_user_config, "", _("Location of users configuration"), true);

        _sys_user_prefs.set_text(prefs->getPrefsFilename());
        _sys_user_prefs.set_editable(false);
        _page_system.add_line(true, _("User preferences: "), _sys_user_prefs, "", _("Location of the users preferences file"), true);

        _sys_user_extension_dir.set_text((char const *)IO::Resource::get_path(IO::Resource::USER, IO::Resource::EXTENSIONS, ""));
        _sys_user_extension_dir.set_editable(false);
        _page_system.add_line(true, _("User extensions: "), _sys_user_extension_dir, "", _("Location of the users extensions"), true);

        _sys_user_cache.set_text(g_get_user_cache_dir());
        _sys_user_cache.set_editable(false);
        _page_system.add_line(true, _("User cache: "), _sys_user_cache, "", _("Location of users cache"), true);

        Glib::ustring tmp_dir = prefs->getString("/options/autosave/path");
        if (tmp_dir.empty()) {
            tmp_dir = Glib::get_tmp_dir();
        }
        _sys_tmp_files.set_text(tmp_dir);
        _sys_tmp_files.set_editable(false);
        _page_system.add_line(true, _("Temporary files: "), _sys_tmp_files, "", _("Location of the temporary files used for autosave"), true);

        _sys_data.set_text( INKSCAPE_DATADIR );
        _sys_data.set_editable(false);
        _page_system.add_line(true, _("Inkscape data: "), _sys_data, "", _("Location of Inkscape data"), true);

        _sys_extension_dir.set_text(INKSCAPE_EXTENSIONDIR);
        _sys_extension_dir.set_editable(false);
        _page_system.add_line(true, _("Inkscape extensions: "), _sys_extension_dir, "", _("Location of the Inkscape extensions"), true);

        Glib::ustring tmp;
        appendList( tmp, g_get_system_data_dirs() );
        _sys_systemdata.get_buffer()->insert(_sys_systemdata.get_buffer()->end(), tmp);
        _sys_systemdata.set_editable(false);
        _sys_systemdata_scroll.add(_sys_systemdata);
        _sys_systemdata_scroll.set_size_request(0, 80);
        _sys_systemdata_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
         _page_system.add_line(true,  _("System data: "), _sys_systemdata_scroll, "", _("Locations of system data"), true);

        {
            tmp = "";
            gchar** paths = 0;
            gint count = 0;
            gtk_icon_theme_get_search_path(gtk_icon_theme_get_default(), &paths, &count);
            if (count > 0) {
                tmp += paths[0];
                tmp += "\n";
                for (int i = 1; i < count; i++) {
                    tmp += "  ";
                    tmp += paths[i];
                    tmp += "\n";
                }
            }
        }

        _sys_icon.get_buffer()->insert(_sys_icon.get_buffer()->end(), tmp);
    }
    _sys_icon.set_editable(false);
    _sys_icon_scroll.add(_sys_icon);
    _sys_icon_scroll.set_size_request(0, 80);
    _sys_icon_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _page_system.add_line(true,  _("Icon theme: "), _sys_icon_scroll, "", _("Locations of icon themes"), true);

    this->AddPage(_page_system, _("System"), PREFS_PAGE_SYSTEM);
}

bool InkscapePreferences::SetMaxDialogSize(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    DialogPage* page = row[_page_list_columns._col_page];
    _page_frame.add(*page);
    this->show_all_children();
    Gtk::Requisition sreq;
#if WITH_GTKMM_3_0
    Gtk::Requisition sreq_natural;
    this->get_preferred_size(sreq_natural, sreq);
#else
    sreq = this->size_request();
#endif
    _max_dialog_width=std::max(_max_dialog_width, sreq.width);
    _max_dialog_height=std::max(_max_dialog_height, sreq.height);
    _page_frame.remove();
    return false;
}

bool InkscapePreferences::PresentPage(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int desired_page = prefs->getInt("/dialogs/preferences/page", 0);
    _init = false;
    if (desired_page == row[_page_list_columns._col_id])
    {
        if (desired_page >= PREFS_PAGE_TOOLS && desired_page <= PREFS_PAGE_TOOLS_CONNECTOR)
            _page_list.expand_row(_path_tools, false);
        if (desired_page >= PREFS_PAGE_TOOLS_SHAPES && desired_page <= PREFS_PAGE_TOOLS_SHAPES_SPIRAL)
            _page_list.expand_row(_path_shapes, false);
        if (desired_page >= PREFS_PAGE_UI && desired_page <= PREFS_PAGE_UI_KEYBOARD_SHORTCUTS)
            _page_list.expand_row(_path_ui, false);
        if (desired_page >= PREFS_PAGE_BEHAVIOR && desired_page <= PREFS_PAGE_BEHAVIOR_MASKS)
            _page_list.expand_row(_path_behavior, false);
        if (desired_page >= PREFS_PAGE_IO && desired_page <= PREFS_PAGE_IO_OPENCLIPART)
            _page_list.expand_row(_path_io, false);
        _page_list.get_selection()->select(iter);
        return true;
    }
    return false;
}

void InkscapePreferences::on_reset_open_recent_clicked()
{
    GtkRecentManager* manager = gtk_recent_manager_get_default();
    GList* recent_list = gtk_recent_manager_get_items(manager);
    GList* element;
    GError* error;

    //Remove only elements that were added by Inkscape
    for (element = g_list_first(recent_list); element; element = g_list_next(element)){
        error = NULL;
        GtkRecentInfo* info = (GtkRecentInfo*) element->data;
        if (gtk_recent_info_has_application(info, g_get_prgname())){
            gtk_recent_manager_remove_item(manager, gtk_recent_info_get_uri(info), &error);
        }
        gtk_recent_info_unref (info);
    }
    g_list_free(recent_list);
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
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (!_init) {
            prefs->setInt("/dialogs/preferences/page", row[_page_list_columns._col_id]);
        }
        Glib::ustring col_name_escaped = Glib::Markup::escape_text( row[_page_list_columns._col_name] );
        _page_title.set_markup("<span size='large'><b>" + col_name_escaped + "</b></span>");
        _page_frame.add(*_current_page);
        _current_page->show();
        while (Gtk::Main::events_pending())
        {
            Gtk::Main::iteration();
        }
        this->show_all_children();
    }
}

void InkscapePreferences::_presentPages()
{
    _page_list_model->foreach_iter(sigc::mem_fun(*this, &InkscapePreferences::PresentPage));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
