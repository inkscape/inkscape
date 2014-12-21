/*
 * Author:
 *   buliabyak@gmail.com
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2005 author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "selected-style.h"
#include <gtkmm/separatormenuitem.h>

#include "widgets/spw-utilities.h"
#include "ui/widget/color-preview.h"

#include "selection.h"
#include "desktop-handles.h"
#include "style.h"
#include "desktop-style.h"
#include "sp-namedview.h"
#include "sp-linear-gradient.h"
#include "sp-mesh-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-pattern.h"
#include "ui/dialog/dialog-manager.h"
#include "ui/dialog/fill-and-stroke.h"
#include "ui/dialog/panel-dialog.h"
#include "xml/repr.h"
#include "document.h"
#include "document-undo.h"
#include "widgets/widget-sizes.h"
#include "widgets/spinbutton-events.h"
#include "widgets/gradient-image.h"
#include "sp-gradient.h"
#include "svg/svg-color.h"
#include "svg/css-ostringstream.h"
#include "ui/tools/tool-base.h"
#include "message-context.h"
#include "verbs.h"
#include "color.h"
#include <display/sp-canvas.h>
#include "pixmaps/cursor-adj-h.xpm"
#include "pixmaps/cursor-adj-s.xpm"
#include "pixmaps/cursor-adj-l.xpm"
#include "pixmaps/cursor-adj-a.xpm"
#include "sp-cursor.h"
#include "gradient-chemistry.h"
#include "util/units.h"

using Inkscape::Util::unit_table;

static gdouble const _sw_presets[]     = { 32 ,  16 ,  10 ,  8 ,  6 ,  4 ,  3 ,  2 ,  1.5 ,  1 ,  0.75 ,  0.5 ,  0.25 ,  0.1 };
static gchar const *const _sw_presets_str[] = {"32", "16", "10", "8", "6", "4", "3", "2", "1.5", "1", "0.75", "0.5", "0.25", "0.1"};

static void
ss_selection_changed (Inkscape::Selection *, gpointer data)
{
    Inkscape::UI::Widget::SelectedStyle *ss = (Inkscape::UI::Widget::SelectedStyle *) data;
    ss->update();
}

static void
ss_selection_modified( Inkscape::Selection *selection, guint /*flags*/, gpointer data )
{
    ss_selection_changed (selection, data);
}

static void
ss_subselection_changed( gpointer /*dragger*/, gpointer data )
{
    ss_selection_changed (NULL, data);
}

namespace {

void clearTooltip( Gtk::Widget &widget )
{
    widget.set_tooltip_text("");
    widget.set_has_tooltip(false);
}

} // namespace

namespace Inkscape {
namespace UI {
namespace Widget {


typedef struct {
    SelectedStyle* parent;
    int item;
} DropTracker;

/* Drag and Drop */
typedef enum {
    APP_X_COLOR
} ui_drop_target_info;

//TODO: warning: deprecated conversion from string constant to ‘gchar*’
//
//Turn out to be warnings that we should probably leave in place. The
// pointers/types used need to be read-only. So until we correct the using
// code, those warnings are actually desired. They say "Hey! Fix this". We
// definitely don't want to hide/ignore them. --JonCruz
static const GtkTargetEntry ui_drop_target_entries [] = {
    {"application/x-color", 0, APP_X_COLOR}
};

#define ENTRIES_SIZE(n) sizeof(n)/sizeof(n[0])
static guint nui_drop_target_entries = ENTRIES_SIZE(ui_drop_target_entries);

/* convenience function */
static Dialog::FillAndStroke *get_fill_and_stroke_panel(SPDesktop *desktop);

SelectedStyle::SelectedStyle(bool /*layout*/)
    :
      current_stroke_width(0),

      _desktop (NULL),
#if WITH_GTKMM_3_0
      _table(),
#else
      _table(2, 6),
#endif
      _fill_label (_("Fill:")),
      _stroke_label (_("Stroke:")),
      _opacity_label (_("O:")),

      _fill_place(this, SS_FILL),
      _stroke_place(this, SS_STROKE),

      _fill_flag_place (),
      _stroke_flag_place (),

      _opacity_place (),
#if WITH_GTKMM_3_0
      _opacity_adjustment(Gtk::Adjustment::create(100, 0.0, 100, 1.0, 10.0)),
#else
      _opacity_adjustment (100, 0.0, 100, 1.0, 10.0),
#endif
      _opacity_sb (0.02, 0),

      _stroke (),
      _stroke_width_place(this),
      _stroke_width (""),

      _opacity_blocked (false),

      _unit_mis(NULL),
      _sw_unit(NULL)
{
    _drop[0] = _drop[1] = 0;
    _dropEnabled[0] = _dropEnabled[1] = false;

    _fill_label.set_alignment(0.0, 0.5);
    _fill_label.set_padding(0, 0);
    _stroke_label.set_alignment(0.0, 0.5);
    _stroke_label.set_padding(0, 0);
    _opacity_label.set_alignment(0.0, 0.5);
    _opacity_label.set_padding(0, 0);

#if WITH_GTKMM_3_0
    _table.set_column_spacing(2);
    _table.set_row_spacing(0);
#else
    _table.set_col_spacings (2);
    _table.set_row_spacings (0);
#endif

    for (int i = SS_FILL; i <= SS_STROKE; i++) {

        _na[i].set_markup (_("N/A"));
        sp_set_font_size_smaller (GTK_WIDGET(_na[i].gobj()));
        _na[i].show_all();
        __na[i] = (_("Nothing selected"));

        if (i == SS_FILL) {
            _none[i].set_markup (C_("Fill", "<i>None</i>"));
        } else {
            _none[i].set_markup (C_("Stroke", "<i>None</i>"));
        }
        sp_set_font_size_smaller (GTK_WIDGET(_none[i].gobj()));
        _none[i].show_all();
        __none[i] = (i == SS_FILL)? (C_("Fill and stroke", "No fill")) : (C_("Fill and stroke", "No stroke"));

        _pattern[i].set_markup (_("Pattern"));
        sp_set_font_size_smaller (GTK_WIDGET(_pattern[i].gobj()));
        _pattern[i].show_all();
        __pattern[i] = (i == SS_FILL)? (_("Pattern fill")) : (_("Pattern stroke"));

        _lgradient[i].set_markup (_("<b>L</b>"));
        sp_set_font_size_smaller (GTK_WIDGET(_lgradient[i].gobj()));
        _lgradient[i].show_all();
        __lgradient[i] = (i == SS_FILL)? (_("Linear gradient fill")) : (_("Linear gradient stroke"));

        _gradient_preview_l[i] =  GTK_WIDGET(sp_gradient_image_new (NULL));
        _gradient_box_l[i].pack_start(_lgradient[i]);
        _gradient_box_l[i].pack_start(*(Glib::wrap(_gradient_preview_l[i])));
        _gradient_box_l[i].show_all();

        _rgradient[i].set_markup (_("<b>R</b>"));
        sp_set_font_size_smaller (GTK_WIDGET(_rgradient[i].gobj()));
        _rgradient[i].show_all();
        __rgradient[i] = (i == SS_FILL)? (_("Radial gradient fill")) : (_("Radial gradient stroke"));

        _gradient_preview_r[i] = GTK_WIDGET(sp_gradient_image_new (NULL));
        _gradient_box_r[i].pack_start(_rgradient[i]);
        _gradient_box_r[i].pack_start(*(Glib::wrap(_gradient_preview_r[i])));
        _gradient_box_r[i].show_all();

#ifdef WITH_MESH
        _mgradient[i].set_markup (_("<b>M</b>"));
        sp_set_font_size_smaller (GTK_WIDGET(_mgradient[i].gobj()));
        _mgradient[i].show_all();
        __mgradient[i] = (i == SS_FILL)? (_("Mesh gradient fill")) : (_("Mesh gradient stroke"));

        _gradient_preview_m[i] = GTK_WIDGET(sp_gradient_image_new (NULL));
        _gradient_box_m[i].pack_start(_mgradient[i]);
        _gradient_box_m[i].pack_start(*(Glib::wrap(_gradient_preview_m[i])));
        _gradient_box_m[i].show_all();
#endif

        _many[i].set_markup (_("Different"));
        sp_set_font_size_smaller (GTK_WIDGET(_many[i].gobj()));
        _many[i].show_all();
        __many[i] = (i == SS_FILL)? (_("Different fills")) : (_("Different strokes"));

        _unset[i].set_markup (_("<b>Unset</b>"));
        sp_set_font_size_smaller (GTK_WIDGET(_unset[i].gobj()));
        _unset[i].show_all();
        __unset[i] = (i == SS_FILL)? (_("Unset fill")) : (_("Unset stroke"));

        _color_preview[i] = new Inkscape::UI::Widget::ColorPreview (0);
        __color[i] = (i == SS_FILL)? (_("Flat color fill")) : (_("Flat color stroke"));

        // TRANSLATOR COMMENT: A means "Averaged"
        _averaged[i].set_markup (_("<b>a</b>"));
        sp_set_font_size_smaller (GTK_WIDGET(_averaged[i].gobj()));
        _averaged[i].show_all();
        __averaged[i] = (i == SS_FILL)? (_("Fill is averaged over selected objects")) : (_("Stroke is averaged over selected objects"));

        // TRANSLATOR COMMENT: M means "Multiple"
        _multiple[i].set_markup (_("<b>m</b>"));
        sp_set_font_size_smaller (GTK_WIDGET(_multiple[i].gobj()));
        _multiple[i].show_all();
        __multiple[i] = (i == SS_FILL)? (_("Multiple selected objects have the same fill")) : (_("Multiple selected objects have the same stroke"));

        _popup_edit[i].add(*(new Gtk::Label((i == SS_FILL)? _("Edit fill...") : _("Edit stroke..."), 0.0, 0.5)));
        _popup_edit[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_edit : &SelectedStyle::on_stroke_edit ));

        _popup_lastused[i].add(*(new Gtk::Label(_("Last set color"), 0.0, 0.5)));
        _popup_lastused[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_lastused : &SelectedStyle::on_stroke_lastused ));

        _popup_lastselected[i].add(*(new Gtk::Label(_("Last selected color"), 0.0, 0.5)));
        _popup_lastselected[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_lastselected : &SelectedStyle::on_stroke_lastselected ));

        _popup_invert[i].add(*(new Gtk::Label(_("Invert"), 0.0, 0.5)));
        _popup_invert[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_invert : &SelectedStyle::on_stroke_invert ));

        _popup_white[i].add(*(new Gtk::Label(_("White"), 0.0, 0.5)));
        _popup_white[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_white : &SelectedStyle::on_stroke_white ));

        _popup_black[i].add(*(new Gtk::Label(_("Black"), 0.0, 0.5)));
        _popup_black[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_black : &SelectedStyle::on_stroke_black ));

        _popup_copy[i].add(*(new Gtk::Label(_("Copy color"), 0.0, 0.5)));
        _popup_copy[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_copy : &SelectedStyle::on_stroke_copy ));

        _popup_paste[i].add(*(new Gtk::Label(_("Paste color"), 0.0, 0.5)));
        _popup_paste[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_paste : &SelectedStyle::on_stroke_paste ));

        _popup_swap[i].add(*(new Gtk::Label(_("Swap fill and stroke"), 0.0, 0.5)));
        _popup_swap[i].signal_activate().connect(sigc::mem_fun(*this,
                               &SelectedStyle::on_fillstroke_swap));

        _popup_opaque[i].add(*(new Gtk::Label((i == SS_FILL)? _("Make fill opaque") : _("Make stroke opaque"), 0.0, 0.5)));
        _popup_opaque[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_opaque : &SelectedStyle::on_stroke_opaque ));

        //TRANSLATORS COMMENT: unset is a verb here
        _popup_unset[i].add(*(new Gtk::Label((i == SS_FILL)? _("Unset fill") : _("Unset stroke"), 0.0, 0.5)));
        _popup_unset[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_unset : &SelectedStyle::on_stroke_unset ));

        _popup_remove[i].add(*(new Gtk::Label((i == SS_FILL)? _("Remove fill") : _("Remove stroke"), 0.0, 0.5)));
        _popup_remove[i].signal_activate().connect(sigc::mem_fun(*this,
                               (i == SS_FILL)? &SelectedStyle::on_fill_remove : &SelectedStyle::on_stroke_remove ));

        _popup[i].attach(_popup_edit[i], 0,1, 0,1);
          _popup[i].attach(*(new Gtk::SeparatorMenuItem()), 0,1, 1,2);
        _popup[i].attach(_popup_lastused[i], 0,1, 2,3);
        _popup[i].attach(_popup_lastselected[i], 0,1, 3,4);
          _popup[i].attach(*(new Gtk::SeparatorMenuItem()), 0,1, 4,5);
        _popup[i].attach(_popup_invert[i], 0,1, 5,6);
          _popup[i].attach(*(new Gtk::SeparatorMenuItem()), 0,1, 6,7);
        _popup[i].attach(_popup_white[i], 0,1, 7,8);
        _popup[i].attach(_popup_black[i], 0,1, 8,9);
          _popup[i].attach(*(new Gtk::SeparatorMenuItem()), 0,1, 9,10);
        _popup[i].attach(_popup_copy[i], 0,1, 10,11);
        _popup_copy[i].set_sensitive(false);
        _popup[i].attach(_popup_paste[i], 0,1, 11,12);
        _popup[i].attach(_popup_swap[i], 0,1, 12,13);
          _popup[i].attach(*(new Gtk::SeparatorMenuItem()), 0,1, 13,14);
        _popup[i].attach(_popup_opaque[i], 0,1, 14,15);
        _popup[i].attach(_popup_unset[i], 0,1, 15,16);
        _popup[i].attach(_popup_remove[i], 0,1, 16,17);
        _popup[i].show_all();

        _mode[i] = SS_NA;
    }

    {
        int row = 0;

        Inkscape::Util::UnitTable::UnitMap m = unit_table.units(Inkscape::Util::UNIT_TYPE_LINEAR);
        Inkscape::Util::UnitTable::UnitMap::iterator iter = m.begin();
        while(iter != m.end()) {
            Gtk::RadioMenuItem *mi = Gtk::manage(new Gtk::RadioMenuItem(_sw_group));
            mi->add(*(new Gtk::Label(iter->first, 0.0, 0.5)));
            _unit_mis = g_slist_append(_unit_mis, mi);
            Inkscape::Util::Unit const *u = unit_table.getUnit(iter->first);
            mi->signal_activate().connect(sigc::bind<Inkscape::Util::Unit const *>(sigc::mem_fun(*this, &SelectedStyle::on_popup_units), u));
            _popup_sw.attach(*mi, 0,1, row, row+1);
            row++;
            ++iter;
        }

        _popup_sw.attach(*(new Gtk::SeparatorMenuItem()), 0,1, row, row+1);
        row++;

        for (guint i = 0; i < G_N_ELEMENTS(_sw_presets_str); ++i) {
            Gtk::MenuItem *mi = Gtk::manage(new Gtk::MenuItem());
            mi->add(*(new Gtk::Label(_sw_presets_str[i], 0.0, 0.5)));
            mi->signal_activate().connect(sigc::bind<int>(sigc::mem_fun(*this, &SelectedStyle::on_popup_preset), i));
            _popup_sw.attach(*mi, 0,1, row, row+1);
            row++;
        }

        _popup_sw.attach(*(new Gtk::SeparatorMenuItem()), 0,1, row, row+1);
        row++;

        _popup_sw_remove.add(*(new Gtk::Label(_("Remove"), 0.0, 0.5)));
        _popup_sw_remove.signal_activate().connect(sigc::mem_fun(*this, &SelectedStyle::on_stroke_remove));
        _popup_sw.attach(_popup_sw_remove, 0,1, row, row+1);
        row++;

        sp_set_font_size_smaller (GTK_WIDGET(_popup_sw.gobj()));

        _popup_sw.show_all();
    }

    _fill_place.signal_button_release_event().connect(sigc::mem_fun(*this, &SelectedStyle::on_fill_click));
    _stroke_place.signal_button_release_event().connect(sigc::mem_fun(*this, &SelectedStyle::on_stroke_click));
    _opacity_place.signal_button_press_event().connect(sigc::mem_fun(*this, &SelectedStyle::on_opacity_click));
    _stroke_width_place.signal_button_press_event().connect(sigc::mem_fun(*this, &SelectedStyle::on_sw_click));
    _stroke_width_place.signal_button_release_event().connect(sigc::mem_fun(*this, &SelectedStyle::on_sw_click));


    _opacity_sb.signal_populate_popup().connect(sigc::mem_fun(*this, &SelectedStyle::on_opacity_menu));
    _opacity_sb.signal_value_changed().connect(sigc::mem_fun(*this, &SelectedStyle::on_opacity_changed));
    // Connect to key-press to ensure focus is consistent with other spin buttons when using the keys vs mouse-click
    g_signal_connect (G_OBJECT (_opacity_sb.gobj()), "key-press-event", G_CALLBACK (spinbutton_keypress), _opacity_sb.gobj());
    g_signal_connect (G_OBJECT (_opacity_sb.gobj()), "focus-in-event", G_CALLBACK (spinbutton_focus_in), _opacity_sb.gobj());

    _fill_place.add(_na[SS_FILL]);
    _fill_place.set_tooltip_text(__na[SS_FILL]);

    _stroke_place.add(_na[SS_STROKE]);
    _stroke_place.set_tooltip_text(__na[SS_STROKE]);

    _stroke.pack_start(_stroke_place);
    _stroke_width_place.add(_stroke_width);
    _stroke.pack_start(_stroke_width_place, Gtk::PACK_SHRINK);

    _opacity_sb.set_adjustment(_opacity_adjustment);
    sp_set_font_size_smaller (GTK_WIDGET(_opacity_sb.gobj()));
    _opacity_sb.set_size_request (SELECTED_STYLE_SB_WIDTH, -1);
    _opacity_sb.set_sensitive (false);

#if WITH_GTKMM_3_0
    _table.attach(_fill_label, 0, 0, 1, 1);
    _table.attach(_stroke_label, 0, 1, 1, 1);

    _table.attach(_fill_flag_place, 1, 0, 1, 1);
    _table.attach(_stroke_flag_place, 1, 1, 1, 1);

    _table.attach(_fill_place, 2, 0, 1, 1);
    _table.attach(_stroke, 2, 1, 1, 1);
#else
    _table.attach(_fill_label, 0,1, 0,1, Gtk::FILL, Gtk::SHRINK);
    _table.attach(_stroke_label, 0,1, 1,2, Gtk::FILL, Gtk::SHRINK);

    _table.attach(_fill_flag_place, 1,2, 0,1, Gtk::SHRINK, Gtk::SHRINK);
    _table.attach(_stroke_flag_place, 1,2, 1,2, Gtk::SHRINK, Gtk::SHRINK);

    _table.attach(_fill_place, 2,3, 0,1);
    _table.attach(_stroke, 2,3, 1,2);
#endif

    _opacity_place.add(_opacity_label);

#if WITH_GTKMM_3_0
    _table.attach(_opacity_place, 4, 0, 1, 2);
    _table.attach(_opacity_sb, 5, 0, 1, 2);
#else
    _table.attach(_opacity_place, 4,5, 0,2, Gtk::SHRINK, Gtk::SHRINK);
    _table.attach(_opacity_sb, 5,6, 0,2, Gtk::SHRINK, Gtk::SHRINK);
#endif

    pack_start(_table, true, true, 2);

    set_size_request (SELECTED_STYLE_WIDTH, -1);

    sp_set_font_size_smaller (GTK_WIDGET(_opacity_label.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_opacity_sb.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_fill_place.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_fill_flag_place.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_stroke_place.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_stroke_flag_place.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_stroke_width.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_fill_label.gobj()));
    sp_set_font_size_smaller (GTK_WIDGET(_stroke_label.gobj()));

    _drop[SS_FILL] = new DropTracker();
    ((DropTracker*)_drop[SS_FILL])->parent = this;
    ((DropTracker*)_drop[SS_FILL])->item = SS_FILL;

    _drop[SS_STROKE] = new DropTracker();
    ((DropTracker*)_drop[SS_STROKE])->parent = this;
    ((DropTracker*)_drop[SS_STROKE])->item = SS_STROKE;

    g_signal_connect(_stroke_place.gobj(),
                     "drag_data_received",
                     G_CALLBACK(dragDataReceived),
                     _drop[SS_STROKE]);

    g_signal_connect(_fill_place.gobj(),
                     "drag_data_received",
                     G_CALLBACK(dragDataReceived),
                     _drop[SS_FILL]);
}

SelectedStyle::~SelectedStyle()
{
    selection_changed_connection->disconnect();
    delete selection_changed_connection;
    selection_modified_connection->disconnect();
    delete selection_modified_connection;
    subselection_changed_connection->disconnect();
    delete subselection_changed_connection;

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        delete _color_preview[i];
        // FIXME: do we need this? the destroy methods are not exported
        //sp_gradient_image_destroy(GTK_OBJECT(_gradient_preview_l[i]));
        //sp_gradient_image_destroy(GTK_OBJECT(_gradient_preview_r[i]));
    }

    delete (DropTracker*)_drop[SS_FILL];
    delete (DropTracker*)_drop[SS_STROKE];
}

void
SelectedStyle::setDesktop(SPDesktop *desktop)
{
    _desktop = desktop;
    g_object_set_data (G_OBJECT(_opacity_sb.gobj()), "dtw", _desktop->canvas);

    Inkscape::Selection *selection = desktop->getSelection();

    selection_changed_connection = new sigc::connection (selection->connectChanged(
        sigc::bind (
            sigc::ptr_fun(&ss_selection_changed),
            this )
    ));
    selection_modified_connection = new sigc::connection (selection->connectModified(
        sigc::bind (
            sigc::ptr_fun(&ss_selection_modified),
            this )
    ));
    subselection_changed_connection = new sigc::connection (desktop->connectToolSubselectionChanged(
        sigc::bind (
            sigc::ptr_fun(&ss_subselection_changed),
            this )
    ));

    _sw_unit = sp_desktop_namedview(desktop)->display_units;

    // Set the doc default unit active in the units list
    gint length = g_slist_length(_unit_mis);
    for (int i = 0; i < length; i++) {
        Gtk::RadioMenuItem *mi = (Gtk::RadioMenuItem *) g_slist_nth_data(_unit_mis, i);
        if (mi && mi->get_label() == _sw_unit->abbr) {
            mi->set_active();
            break;
        }
    }
}

void SelectedStyle::dragDataReceived( GtkWidget */*widget*/,
                                      GdkDragContext */*drag_context*/,
                                      gint /*x*/, gint /*y*/,
                                      GtkSelectionData *data,
                                      guint /*info*/,
                                      guint /*event_time*/,
                                      gpointer user_data )
{
    DropTracker* tracker = (DropTracker*)user_data;

    switch ( (int)tracker->item ) {
        case SS_FILL:
        case SS_STROKE:
        {
            if (gtk_selection_data_get_length(data) == 8 ) {
                gchar c[64];
                // Careful about endian issues.
                guint16* dataVals = (guint16*)gtk_selection_data_get_data(data);
                sp_svg_write_color( c, sizeof(c),
                                    SP_RGBA32_U_COMPOSE(
                                        0x0ff & (dataVals[0] >> 8),
                                        0x0ff & (dataVals[1] >> 8),
                                        0x0ff & (dataVals[2] >> 8),
                                        0xff // can't have transparency in the color itself
                                        //0x0ff & (data->data[3] >> 8),
                                        ));
                SPCSSAttr *css = sp_repr_css_attr_new();
                sp_repr_css_set_property( css, (tracker->item == SS_FILL) ? "fill":"stroke", c );
                sp_desktop_set_style( tracker->parent->_desktop, css );
                sp_repr_css_attr_unref( css );
                DocumentUndo::done( sp_desktop_document(tracker->parent->_desktop) , SP_VERB_NONE,
                                    _("Drop color"));
            }
        }
        break;
    }
}

void SelectedStyle::on_fill_remove() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "fill", "none");
    sp_desktop_set_style (_desktop, css, true, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                        _("Remove fill"));
}

void SelectedStyle::on_stroke_remove() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "stroke", "none");
    sp_desktop_set_style (_desktop, css, true, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Remove stroke"));
}

void SelectedStyle::on_fill_unset() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_unset_property (css, "fill");
    sp_desktop_set_style (_desktop, css, true, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Unset fill"));
}

void SelectedStyle::on_stroke_unset() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_unset_property (css, "stroke");
    sp_repr_css_unset_property (css, "stroke-opacity");
    sp_repr_css_unset_property (css, "stroke-width");
    sp_repr_css_unset_property (css, "stroke-miterlimit");
    sp_repr_css_unset_property (css, "stroke-linejoin");
    sp_repr_css_unset_property (css, "stroke-linecap");
    sp_repr_css_unset_property (css, "stroke-dashoffset");
    sp_repr_css_unset_property (css, "stroke-dasharray");
    sp_desktop_set_style (_desktop, css, true, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Unset stroke"));
}

void SelectedStyle::on_fill_opaque() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "fill-opacity", "1");
    sp_desktop_set_style (_desktop, css, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Make fill opaque"));
}

void SelectedStyle::on_stroke_opaque() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "stroke-opacity", "1");
    sp_desktop_set_style (_desktop, css, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Make fill opaque"));
}

void SelectedStyle::on_fill_lastused() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    guint32 color = sp_desktop_get_color(_desktop, true);
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), color);
    sp_repr_css_set_property (css, "fill", c);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Apply last set color to fill"));
}

void SelectedStyle::on_stroke_lastused() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    guint32 color = sp_desktop_get_color(_desktop, false);
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), color);
    sp_repr_css_set_property (css, "stroke", c);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Apply last set color to stroke"));
}

void SelectedStyle::on_fill_lastselected() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), _lastselected[SS_FILL]);
    sp_repr_css_set_property (css, "fill", c);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Apply last selected color to fill"));
}

void SelectedStyle::on_stroke_lastselected() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), _lastselected[SS_STROKE]);
    sp_repr_css_set_property (css, "stroke", c);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Apply last selected color to stroke"));
}

void SelectedStyle::on_fill_invert() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    guint32 color = _thisselected[SS_FILL];
    gchar c[64];
    if (_mode[SS_FILL] == SS_LGRADIENT || _mode[SS_FILL] == SS_RGRADIENT) {
        sp_gradient_invert_selected_gradients(_desktop, Inkscape::FOR_FILL);
        return;

    }

    if (_mode[SS_FILL] != SS_COLOR) return;
    sp_svg_write_color (c, sizeof(c),
        SP_RGBA32_U_COMPOSE(
                (255 - SP_RGBA32_R_U(color)),
                (255 - SP_RGBA32_G_U(color)),
                (255 - SP_RGBA32_B_U(color)),
                SP_RGBA32_A_U(color)
        )
    );
    sp_repr_css_set_property (css, "fill", c);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Invert fill"));
}

void SelectedStyle::on_stroke_invert() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    guint32 color = _thisselected[SS_STROKE];
    gchar c[64];
    if (_mode[SS_STROKE] == SS_LGRADIENT || _mode[SS_STROKE] == SS_RGRADIENT) {
        sp_gradient_invert_selected_gradients(_desktop, Inkscape::FOR_STROKE);
        return;
    }
    if (_mode[SS_STROKE] != SS_COLOR) return;
    sp_svg_write_color (c, sizeof(c),
        SP_RGBA32_U_COMPOSE(
                (255 - SP_RGBA32_R_U(color)),
                (255 - SP_RGBA32_G_U(color)),
                (255 - SP_RGBA32_B_U(color)),
                SP_RGBA32_A_U(color)
        )
    );
    sp_repr_css_set_property (css, "stroke", c);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Invert stroke"));
}

void SelectedStyle::on_fill_white() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), 0xffffffff);
    sp_repr_css_set_property (css, "fill", c);
    sp_repr_css_set_property (css, "fill-opacity", "1");
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("White fill"));
}

void SelectedStyle::on_stroke_white() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), 0xffffffff);
    sp_repr_css_set_property (css, "stroke", c);
    sp_repr_css_set_property (css, "stroke-opacity", "1");
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("White stroke"));
}

void SelectedStyle::on_fill_black() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), 0x000000ff);
    sp_repr_css_set_property (css, "fill", c);
    sp_repr_css_set_property (css, "fill-opacity", "1.0");
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Black fill"));
}

void SelectedStyle::on_stroke_black() {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), 0x000000ff);
    sp_repr_css_set_property (css, "stroke", c);
    sp_repr_css_set_property (css, "stroke-opacity", "1.0");
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Black stroke"));
}

void SelectedStyle::on_fill_copy() {
    if (_mode[SS_FILL] == SS_COLOR) {
        gchar c[64];
        sp_svg_write_color (c, sizeof(c), _thisselected[SS_FILL]);
        Glib::ustring text;
        text += c;
        if (!text.empty()) {
            Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
            refClipboard->set_text(text);
        }
    }
}

void SelectedStyle::on_stroke_copy() {
    if (_mode[SS_STROKE] == SS_COLOR) {
        gchar c[64];
        sp_svg_write_color (c, sizeof(c), _thisselected[SS_STROKE]);
        Glib::ustring text;
        text += c;
        if (!text.empty()) {
            Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
            refClipboard->set_text(text);
        }
    }
}

void SelectedStyle::on_fill_paste() {
    Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
    Glib::ustring const text = refClipboard->wait_for_text();

    if (!text.empty()) {
        guint32 color = sp_svg_read_color(text.c_str(), 0x000000ff); // impossible value, as SVG color cannot have opacity
        if (color == 0x000000ff) // failed to parse color string
            return;

        SPCSSAttr *css = sp_repr_css_attr_new ();
        sp_repr_css_set_property (css, "fill", text.c_str());
        sp_desktop_set_style (_desktop, css);
        sp_repr_css_attr_unref (css);
        DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                           _("Paste fill"));
    }
}

void SelectedStyle::on_stroke_paste() {
    Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
    Glib::ustring const text = refClipboard->wait_for_text();

    if (!text.empty()) {
        guint32 color = sp_svg_read_color(text.c_str(), 0x000000ff); // impossible value, as SVG color cannot have opacity
        if (color == 0x000000ff) // failed to parse color string
            return;

        SPCSSAttr *css = sp_repr_css_attr_new ();
        sp_repr_css_set_property (css, "stroke", text.c_str());
        sp_desktop_set_style (_desktop, css);
        sp_repr_css_attr_unref (css);
        DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                           _("Paste stroke"));
    }
}

void SelectedStyle::on_fillstroke_swap() {
    SPCSSAttr *css = sp_repr_css_attr_new ();

    switch (_mode[SS_FILL]) {
    case SS_NA:
    case SS_MANY:
        break;
    case SS_NONE:
        sp_repr_css_set_property (css, "stroke", "none");
        break;
    case SS_UNSET:
        sp_repr_css_unset_property (css, "stroke");
        break;
    case SS_COLOR:
        gchar c[64];
        sp_svg_write_color (c, sizeof(c), _thisselected[SS_FILL]);
        sp_repr_css_set_property (css, "stroke", c);
        break;
    case SS_LGRADIENT:
    case SS_RGRADIENT:
    case SS_PATTERN:
        sp_repr_css_set_property (css, "stroke", _paintserver_id[SS_FILL].c_str());
        break;
    }

    switch (_mode[SS_STROKE]) {
    case SS_NA:
    case SS_MANY:
        break;
    case SS_NONE:
        sp_repr_css_set_property (css, "fill", "none");
        break;
    case SS_UNSET:
        sp_repr_css_unset_property (css, "fill");
        break;
    case SS_COLOR:
        gchar c[64];
        sp_svg_write_color (c, sizeof(c), _thisselected[SS_STROKE]);
        sp_repr_css_set_property (css, "fill", c);
        break;
    case SS_LGRADIENT:
    case SS_RGRADIENT:
    case SS_PATTERN:
        sp_repr_css_set_property (css, "fill", _paintserver_id[SS_STROKE].c_str());
        break;
    }

    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                       _("Swap fill and stroke"));
}

void SelectedStyle::on_fill_edit() {
    if (Dialog::FillAndStroke *fs = get_fill_and_stroke_panel(_desktop))
        fs->showPageFill();
}

void SelectedStyle::on_stroke_edit() {
    if (Dialog::FillAndStroke *fs = get_fill_and_stroke_panel(_desktop))
        fs->showPageStrokePaint();
}

bool
SelectedStyle::on_fill_click(GdkEventButton *event)
{
    if (event->button == 1) { // click, open fill&stroke

        if (Dialog::FillAndStroke *fs = get_fill_and_stroke_panel(_desktop))
            fs->showPageFill();

    } else if (event->button == 3) { // right-click, popup menu
        _popup[SS_FILL].popup(event->button, event->time);
    } else if (event->button == 2) { // middle click, toggle none/lastcolor
        if (_mode[SS_FILL] == SS_NONE) {
            on_fill_lastused();
        } else {
            on_fill_remove();
        }
    }
    return true;
}

bool
SelectedStyle::on_stroke_click(GdkEventButton *event)
{
    if (event->button == 1) { // click, open fill&stroke
        if (Dialog::FillAndStroke *fs = get_fill_and_stroke_panel(_desktop))
            fs->showPageStrokePaint();
    } else if (event->button == 3) { // right-click, popup menu
        _popup[SS_STROKE].popup(event->button, event->time);
    } else if (event->button == 2) { // middle click, toggle none/lastcolor
        if (_mode[SS_STROKE] == SS_NONE) {
            on_stroke_lastused();
        } else {
            on_stroke_remove();
        }
    }
    return true;
}

bool
SelectedStyle::on_sw_click(GdkEventButton *event)
{
    if (event->button == 1) { // click, open fill&stroke
        if (Dialog::FillAndStroke *fs = get_fill_and_stroke_panel(_desktop))
            fs->showPageStrokeStyle();
    } else if (event->button == 3) { // right-click, popup menu
        _popup_sw.popup(event->button, event->time);
    } else if (event->button == 2) { // middle click, toggle none/lastwidth?
        //
    }
    return true;
}

bool
SelectedStyle::on_opacity_click(GdkEventButton *event)
{
    if (event->button == 2) { // middle click
        const char* opacity = _opacity_sb.get_value() < 50? "0.5" : (_opacity_sb.get_value() == 100? "0" : "1");
        SPCSSAttr *css = sp_repr_css_attr_new ();
        sp_repr_css_set_property (css, "opacity", opacity);
        sp_desktop_set_style (_desktop, css);
        sp_repr_css_attr_unref (css);
        DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_FILL_STROKE,
                           _("Change opacity"));
        return true;
    }

    return false;
}

void SelectedStyle::on_popup_units(Inkscape::Util::Unit const *unit) {
    _sw_unit = unit;
    update();
}

void SelectedStyle::on_popup_preset(int i) {
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gdouble w;
    if (_sw_unit) {
        w = Inkscape::Util::Quantity::convert(_sw_presets[i], _sw_unit, "px");
    } else {
        w = _sw_presets[i];
    }
    Inkscape::CSSOStringStream os;
    os << w;
    sp_repr_css_set_property (css, "stroke-width", os.str().c_str());
    // FIXME: update dash patterns!
    sp_desktop_set_style (_desktop, css, true);
    sp_repr_css_attr_unref (css);
    DocumentUndo::done(sp_desktop_document(_desktop), SP_VERB_DIALOG_SWATCHES,
                       _("Change stroke width"));
}

void
SelectedStyle::update()
{
    if (_desktop == NULL)
        return;

    // create temporary style
    SPStyle *query = sp_style_new (sp_desktop_document(_desktop));

    for (int i = SS_FILL; i <= SS_STROKE; i++) {
        Gtk::EventBox *place = (i == SS_FILL)? &_fill_place : &_stroke_place;
        Gtk::EventBox *flag_place = (i == SS_FILL)? &_fill_flag_place : &_stroke_flag_place;

        place->remove();
        flag_place->remove();

        clearTooltip(*place);
        clearTooltip(*flag_place);

        _mode[i] = SS_NA;
        _paintserver_id[i].clear();

        _popup_copy[i].set_sensitive(false);

        // query style from desktop. This returns a result flag and fills query with the style of subselection, if any, or selection
        int result = sp_desktop_query_style (_desktop, query,
                                             (i == SS_FILL)? QUERY_STYLE_PROPERTY_FILL : QUERY_STYLE_PROPERTY_STROKE);
        switch (result) {
        case QUERY_STYLE_NOTHING:
            place->add(_na[i]);
            place->set_tooltip_text(__na[i]);
            _mode[i] = SS_NA;
            if ( _dropEnabled[i] ) {
                gtk_drag_dest_unset( GTK_WIDGET((i==SS_FILL) ? _fill_place.gobj():_stroke_place.gobj()) );
                _dropEnabled[i] = false;
            }
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED:
        case QUERY_STYLE_MULTIPLE_SAME:
            if ( !_dropEnabled[i] ) {
                gtk_drag_dest_set( GTK_WIDGET( (i==SS_FILL) ? _fill_place.gobj():_stroke_place.gobj()),
                                   GTK_DEST_DEFAULT_ALL,
                                   ui_drop_target_entries,
                                   nui_drop_target_entries,
                                   GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE) );
                _dropEnabled[i] = true;
            }
            SPIPaint *paint;
            if (i == SS_FILL) {
                paint = &(query->fill);
            } else {
                paint = &(query->stroke);
            }
            if (paint->set && paint->isPaintserver()) {
                SPPaintServer *server = (i == SS_FILL)? SP_STYLE_FILL_SERVER (query) : SP_STYLE_STROKE_SERVER (query);
                if ( server ) {
                    Inkscape::XML::Node *srepr = server->getRepr();
                    _paintserver_id[i] += "url(#";
                    _paintserver_id[i] += srepr->attribute("id");
                    _paintserver_id[i] += ")";

                    if (SP_IS_LINEARGRADIENT (server)) {
                        SPGradient *vector = SP_GRADIENT(server)->getVector();
                        sp_gradient_image_set_gradient(SP_GRADIENT_IMAGE(_gradient_preview_l[i]), vector);
                        place->add(_gradient_box_l[i]);
                        place->set_tooltip_text(__lgradient[i]);
                        _mode[i] = SS_LGRADIENT;
                    } else if (SP_IS_RADIALGRADIENT (server)) {
                        SPGradient *vector = SP_GRADIENT(server)->getVector();
                        sp_gradient_image_set_gradient(SP_GRADIENT_IMAGE(_gradient_preview_r[i]), vector);
                        place->add(_gradient_box_r[i]);
                        place->set_tooltip_text(__rgradient[i]);
                        _mode[i] = SS_RGRADIENT;
#ifdef WITH_MESH
                    } else if (SP_IS_MESHGRADIENT(server)) {
                        SPGradient *vector = SP_GRADIENT(server)->getVector();
                        sp_gradient_image_set_gradient(SP_GRADIENT_IMAGE(_gradient_preview_m[i]), vector);
                        place->add(_gradient_box_m[i]);
                        place->set_tooltip_text(__mgradient[i]);
                        _mode[i] = SS_MGRADIENT;
#endif
                    } else if (SP_IS_PATTERN (server)) {
                        place->add(_pattern[i]);
                        place->set_tooltip_text(__pattern[i]);
                        _mode[i] = SS_PATTERN;
                    }
                } else {
                    g_warning ("file %s: line %d: Unknown paint server", __FILE__, __LINE__);
                }
            } else if (paint->set && paint->isColor()) {
                guint32 color = paint->value.color.toRGBA32(
                                     SP_SCALE24_TO_FLOAT ((i == SS_FILL)? query->fill_opacity.value : query->stroke_opacity.value));
                _lastselected[i] = _thisselected[i];
                _thisselected[i] = color; // include opacity
                ((Inkscape::UI::Widget::ColorPreview*)_color_preview[i])->setRgba32 (color);
                _color_preview[i]->show_all();
                place->add(*_color_preview[i]);
                gchar c_string[64];
                g_snprintf (c_string, 64, "%06x/%.3g", color >> 8, SP_RGBA32_A_F(color));
                place->set_tooltip_text(__color[i] + ": " + c_string + _(", drag to adjust"));
                _mode[i] = SS_COLOR;
                _popup_copy[i].set_sensitive(true);

            } else if (paint->set && paint->isNone()) {
                place->add(_none[i]);
                place->set_tooltip_text(__none[i]);
                _mode[i] = SS_NONE;
            } else if (!paint->set) {
                place->add(_unset[i]);
                place->set_tooltip_text(__unset[i]);
                _mode[i] = SS_UNSET;
            }
            if (result == QUERY_STYLE_MULTIPLE_AVERAGED) {
                flag_place->add(_averaged[i]);
                flag_place->set_tooltip_text(__averaged[i]);
            } else if (result == QUERY_STYLE_MULTIPLE_SAME) {
                flag_place->add(_multiple[i]);
                flag_place->set_tooltip_text(__multiple[i]);
            }
            break;
        case QUERY_STYLE_MULTIPLE_DIFFERENT:
            place->add(_many[i]);
            place->set_tooltip_text(__many[i]);
            _mode[i] = SS_MANY;
            break;
        default:
            break;
        }
    }

// Now query opacity
    clearTooltip(_opacity_place);
    clearTooltip(_opacity_sb);

    int result = sp_desktop_query_style (_desktop, query, QUERY_STYLE_PROPERTY_MASTEROPACITY);

    switch (result) {
    case QUERY_STYLE_NOTHING:
        _opacity_place.set_tooltip_text(_("Nothing selected"));
        _opacity_sb.set_tooltip_text(_("Nothing selected"));
        _opacity_sb.set_sensitive(false);
        break;
    case QUERY_STYLE_SINGLE:
    case QUERY_STYLE_MULTIPLE_AVERAGED:
    case QUERY_STYLE_MULTIPLE_SAME:
        _opacity_place.set_tooltip_text(_("Opacity (%)"));
        _opacity_sb.set_tooltip_text(_("Opacity (%)"));
        if (_opacity_blocked) break;
        _opacity_blocked = true;
        _opacity_sb.set_sensitive(true);
#if WITH_GTKMM_3_0
        _opacity_adjustment->set_value(SP_SCALE24_TO_FLOAT(query->opacity.value) * 100);
#else
        _opacity_adjustment.set_value(SP_SCALE24_TO_FLOAT(query->opacity.value) * 100);
#endif
        _opacity_blocked = false;
        break;
    }

// Now query stroke_width
    int result_sw = sp_desktop_query_style (_desktop, query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
    switch (result_sw) {
    case QUERY_STYLE_NOTHING:
        _stroke_width.set_markup("");
        current_stroke_width = 0;
        break;
    case QUERY_STYLE_SINGLE:
    case QUERY_STYLE_MULTIPLE_AVERAGED:
    case QUERY_STYLE_MULTIPLE_SAME:
    {
        double w;
        if (_sw_unit) {
            w = Inkscape::Util::Quantity::convert(query->stroke_width.computed, "px", _sw_unit);
        } else {
            w = query->stroke_width.computed;
        }
        current_stroke_width = w;

        {
            gchar *str = g_strdup_printf(" %.3g", w);
            _stroke_width.set_markup(str);
            g_free (str);
        }
        {
            gchar *str = g_strdup_printf(_("Stroke width: %.5g%s%s"),
                                         w,
                                         _sw_unit? _sw_unit->abbr.c_str() : "px",
                                         (result_sw == QUERY_STYLE_MULTIPLE_AVERAGED)?
                                         _(" (averaged)") : "");
            _stroke_width_place.set_tooltip_text(str);
            g_free (str);
        }
        break;
    }
    default:
        break;
    }

    sp_style_unref(query);
}

void SelectedStyle::opacity_0(void) {_opacity_sb.set_value(0);}
void SelectedStyle::opacity_025(void) {_opacity_sb.set_value(25);}
void SelectedStyle::opacity_05(void) {_opacity_sb.set_value(50);}
void SelectedStyle::opacity_075(void) {_opacity_sb.set_value(75);}
void SelectedStyle::opacity_1(void) {_opacity_sb.set_value(100);}

void SelectedStyle::on_opacity_menu (Gtk::Menu *menu) {

    Glib::ListHandle<Gtk::Widget *> children = menu->get_children();
    for (Glib::ListHandle<Gtk::Widget *>::iterator iter = children.begin(); iter != children.end(); ++iter) {
        menu->remove(*(*iter));
    }

    {
        Gtk::MenuItem *item = new Gtk::MenuItem;
        item->add(*(new Gtk::Label(_("0 (transparent)"), 0, 0)));
        item->signal_activate().connect(sigc::mem_fun(*this, &SelectedStyle::opacity_0 ));
        menu->add(*item);
    }
    {
        Gtk::MenuItem *item = new Gtk::MenuItem;
        item->add(*(new Gtk::Label("25%", 0, 0)));
        item->signal_activate().connect(sigc::mem_fun(*this, &SelectedStyle::opacity_025 ));
        menu->add(*item);
    }
    {
        Gtk::MenuItem *item = new Gtk::MenuItem;
        item->add(*(new Gtk::Label("50%", 0, 0)));
        item->signal_activate().connect(sigc::mem_fun(*this, &SelectedStyle::opacity_05 ));
        menu->add(*item);
    }
    {
        Gtk::MenuItem *item = new Gtk::MenuItem;
        item->add(*(new Gtk::Label("75%", 0, 0)));
        item->signal_activate().connect(sigc::mem_fun(*this, &SelectedStyle::opacity_075 ));
        menu->add(*item);
    }
    {
        Gtk::MenuItem *item = new Gtk::MenuItem;
        item->add(*(new Gtk::Label(_("100% (opaque)"), 0, 0)));
        item->signal_activate().connect(sigc::mem_fun(*this, &SelectedStyle::opacity_1 ));
        menu->add(*item);
    }

    menu->show_all();
}

void SelectedStyle::on_opacity_changed ()
{
    g_return_if_fail(_desktop); // TODO this shouldn't happen!
    if (_opacity_blocked)
        return;
    _opacity_blocked = true;
    SPCSSAttr *css = sp_repr_css_attr_new ();
    Inkscape::CSSOStringStream os;
#if WITH_GTKMM_3_0
    os << CLAMP ((_opacity_adjustment->get_value() / 100), 0.0, 1.0);
#else
    os << CLAMP ((_opacity_adjustment.get_value() / 100), 0.0, 1.0);
#endif
    sp_repr_css_set_property (css, "opacity", os.str().c_str());
    // FIXME: workaround for GTK breakage: display interruptibility sometimes results in GTK
    // sending multiple value-changed events. As if when Inkscape interrupts redraw for main loop
    // iterations, GTK discovers that this callback hasn't finished yet, and for some weird reason
    // decides to add yet another value-changed event to the queue. Totally braindead if you ask
    // me. As a result, scrolling the spinbutton once results in runaway change until it hits 1.0
    // or 0.0. (And no, this is not a race with ::update, I checked that.)
    // Sigh. So we disable interruptibility while we're setting the new value.
    sp_desktop_canvas(_desktop)->forceFullRedrawAfterInterruptions(0);
    sp_desktop_set_style (_desktop, css);
    sp_repr_css_attr_unref (css);
    DocumentUndo::maybeDone(sp_desktop_document(_desktop), "fillstroke:opacity", SP_VERB_DIALOG_FILL_STROKE,
                            _("Change opacity"));
    // resume interruptibility
    sp_desktop_canvas(_desktop)->endForcedFullRedraws();
    spinbutton_defocus(GTK_WIDGET(_opacity_sb.gobj()));
    _opacity_blocked = false;
}

/* =============================================  RotateableSwatch  */

RotateableSwatch::RotateableSwatch(SelectedStyle *parent, guint mode) :
    fillstroke(mode),
    parent(parent),
    startcolor(0),
    startcolor_set(false),
    undokey("ssrot1"),
    cr(0),
    cr_set(false)

{
}

RotateableSwatch::~RotateableSwatch() {
}

double
RotateableSwatch::color_adjust(float *hsla, double by, guint32 cc, guint modifier)
{
    sp_color_rgb_to_hsl_floatv (hsla, SP_RGBA32_R_F(cc), SP_RGBA32_G_F(cc), SP_RGBA32_B_F(cc));
    hsla[3] = SP_RGBA32_A_F(cc);
    double diff = 0;
    if (modifier == 2) { // saturation
        double old = hsla[1];
        if (by > 0) {
            hsla[1] += by * (1 - hsla[1]);
        } else {
            hsla[1] += by * (hsla[1]);
        }
        diff = hsla[1] - old;
    } else if (modifier == 1) { // lightness
        double old = hsla[2];
        if (by > 0) {
            hsla[2] += by * (1 - hsla[2]);
        } else {
            hsla[2] += by * (hsla[2]);
        }
        diff = hsla[2] - old;
    } else if (modifier == 3) { // alpha
        double old = hsla[3];
        hsla[3] += by/2;
        if (hsla[3] < 0) {
            hsla[3] = 0;
        } else if (hsla[3] > 1) {
            hsla[3] = 1;
        }
        diff = hsla[3] - old;
    } else { // hue
        double old = hsla[0];
        hsla[0] += by/2;
        while (hsla[0] < 0)
            hsla[0] += 1;
        while (hsla[0] > 1)
            hsla[0] -= 1;
        diff = hsla[0] - old;
    }

    float rgb[3];
    sp_color_hsl_to_rgb_floatv (rgb, hsla[0], hsla[1], hsla[2]);

    gchar c[64];
    sp_svg_write_color (c, sizeof(c),
        SP_RGBA32_U_COMPOSE(
                (SP_COLOR_F_TO_U(rgb[0])),
                (SP_COLOR_F_TO_U(rgb[1])),
                (SP_COLOR_F_TO_U(rgb[2])),
                0xff
        )
    );

    SPCSSAttr *css = sp_repr_css_attr_new ();

    if (modifier == 3) { // alpha
        Inkscape::CSSOStringStream osalpha;
        osalpha << hsla[3];
        sp_repr_css_set_property(css, (fillstroke == SS_FILL) ? "fill-opacity" : "stroke-opacity", osalpha.str().c_str());
    } else {
        sp_repr_css_set_property (css, (fillstroke == SS_FILL) ? "fill" : "stroke", c);
    }
    sp_desktop_set_style (parent->getDesktop(), css);
    sp_repr_css_attr_unref (css);
    return diff;
}

void
RotateableSwatch::do_motion(double by, guint modifier) {
    if (parent->_mode[fillstroke] != SS_COLOR)
        return;

    if (!scrolling && !cr_set) {
        GtkWidget *w = GTK_WIDGET(gobj());
        GdkPixbuf *pixbuf = NULL;

        if (modifier == 2) { // saturation
            pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)cursor_adj_s_xpm);
        } else if (modifier == 1) { // lightness
            pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)cursor_adj_l_xpm);
        } else if (modifier == 3) { // alpha
            pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)cursor_adj_a_xpm);
        } else { // hue
            pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)cursor_adj_h_xpm);
        }

	if (pixbuf != NULL) {
	    cr = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), pixbuf, 16, 16);

            g_object_unref(pixbuf);	    
            gdk_window_set_cursor(gtk_widget_get_window(w), cr);
#if GTK_CHECK_VERSION(3,0,0)
	    g_object_unref(cr);
#else
            gdk_cursor_unref(cr);
#endif
	    cr = NULL;
            cr_set = true;
        }
    }

    guint32 cc;
    if (!startcolor_set) {
        cc = startcolor = parent->_thisselected[fillstroke];
        startcolor_set = true;
    } else {
        cc = startcolor;
    }

    float hsla[4];
    double diff = 0;

    diff = color_adjust(hsla, by, cc, modifier);

    if (modifier == 3) { // alpha
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, (_("Adjust alpha")));
        double ch = hsla[3];
        parent->getDesktop()->event_context->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("Adjusting <b>alpha</b>: was %.3g, now <b>%.3g</b> (diff %.3g); with <b>Ctrl</b> to adjust lightness, with <b>Shift</b> to adjust saturation, without modifiers to adjust hue"), ch - diff, ch, diff);

    } else if (modifier == 2) { // saturation
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, (_("Adjust saturation")));
        double ch = hsla[1];
        parent->getDesktop()->event_context->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("Adjusting <b>saturation</b>: was %.3g, now <b>%.3g</b> (diff %.3g); with <b>Ctrl</b> to adjust lightness, with <b>Alt</b> to adjust alpha, without modifiers to adjust hue"), ch - diff, ch, diff);

    } else if (modifier == 1) { // lightness
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, (_("Adjust lightness")));
        double ch = hsla[2];
        parent->getDesktop()->event_context->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("Adjusting <b>lightness</b>: was %.3g, now <b>%.3g</b> (diff %.3g); with <b>Shift</b> to adjust saturation, with <b>Alt</b> to adjust alpha, without modifiers to adjust hue"), ch - diff, ch, diff);

    } else { // hue
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, (_("Adjust hue")));
        double ch = hsla[0];
        parent->getDesktop()->event_context->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("Adjusting <b>hue</b>: was %.3g, now <b>%.3g</b> (diff %.3g); with <b>Shift</b> to adjust saturation, with <b>Alt</b> to adjust alpha, with <b>Ctrl</b> to adjust lightness"), ch - diff, ch, diff);
    }
}


void
RotateableSwatch::do_scroll(double by, guint modifier) {
    do_motion(by/30.0, modifier);
    do_release(by/30.0, modifier);
}

void
RotateableSwatch::do_release(double by, guint modifier) {
    if (parent->_mode[fillstroke] != SS_COLOR)
        return;

    float hsla[4];
    color_adjust(hsla, by, startcolor, modifier);

    if (cr_set) {
        GtkWidget *w = GTK_WIDGET(gobj());
        gdk_window_set_cursor(gtk_widget_get_window(w), NULL);
        if (cr) {
#if GTK_CHECK_VERSION(3,0,0)
           g_object_unref(cr);
#else
           gdk_cursor_unref (cr);
#endif
           cr = NULL;
        }
        cr_set = false;
    }

    if (modifier == 3) { // alpha
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, ("Adjust alpha"));
    } else if (modifier == 2) { // saturation
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, ("Adjust saturation"));

    } else if (modifier == 1) { // lightness
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, ("Adjust lightness"));

    } else { // hue
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, ("Adjust hue"));
    }

    if (!strcmp(undokey, "ssrot1")) {
        undokey = "ssrot2";
    } else {
        undokey = "ssrot1";
    }

    parent->getDesktop()->event_context->message_context->clear();
    startcolor_set = false;
}

/* =============================================  RotateableStrokeWidth  */

RotateableStrokeWidth::RotateableStrokeWidth(SelectedStyle *parent) :
    parent(parent),
    startvalue(0),
    startvalue_set(false),
    undokey("swrot1")
{
}

RotateableStrokeWidth::~RotateableStrokeWidth() {
}

double
RotateableStrokeWidth::value_adjust(double current, double by, guint /*modifier*/, bool final)
{
    double newval;
    // by is -1..1
    if (by < 0) {
        // map negative 0..-1 to current..0
        newval = current * (1  +  by);
    } else {
        // map positive 0..1 to current..4*current
        newval = current * (1  +  by) * (1  +  by);
    }

    SPCSSAttr *css = sp_repr_css_attr_new ();
    if (final && newval < 1e-6) {
        // if dragged into zero and this is the final adjust on mouse release, delete stroke;
        // if it's not final, leave it a chance to increase again (which is not possible with "none")
        sp_repr_css_set_property (css, "stroke", "none");
    } else {
        Inkscape::CSSOStringStream os;
        os << newval;
        sp_repr_css_set_property (css, "stroke-width", os.str().c_str());
    }

    sp_desktop_set_style (parent->getDesktop(), css);
    sp_repr_css_attr_unref (css);
    return newval - current;
}

void
RotateableStrokeWidth::do_motion(double by, guint modifier) {

    // if this is the first motion after a mouse grab, remember the current width
    if (!startvalue_set) {
        startvalue = parent->current_stroke_width;
        // if it's 0, adjusting (which uses multiplication) will not be able to change it, so we
        // cheat and provide a non-zero value
        if (startvalue == 0)
            startvalue = 1;
        startvalue_set = true;
    }

    if (modifier == 3) { // Alt, do nothing
    } else {
        double diff = value_adjust(startvalue, by, modifier, false);
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, (_("Adjust stroke width")));
        parent->getDesktop()->event_context->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("Adjusting <b>stroke width</b>: was %.3g, now <b>%.3g</b> (diff %.3g)"), startvalue, startvalue + diff, diff);
    }
}

void
RotateableStrokeWidth::do_release(double by, guint modifier) {

    if (modifier == 3) { // do nothing

    } else {
        value_adjust(startvalue, by, modifier, true);
        startvalue_set = false;
        DocumentUndo::maybeDone(sp_desktop_document(parent->getDesktop()), undokey,
                                SP_VERB_DIALOG_FILL_STROKE, (_("Adjust stroke width")));
    }

    if (!strcmp(undokey, "swrot1")) {
        undokey = "swrot2";
    } else {
        undokey = "swrot1";
    }
    parent->getDesktop()->event_context->message_context->clear();
}

void
RotateableStrokeWidth::do_scroll(double by, guint modifier) {
    do_motion(by/10.0, modifier);
    startvalue_set = false;
}

Dialog::FillAndStroke *get_fill_and_stroke_panel(SPDesktop *desktop)
{
    if (Dialog::PanelDialogBase *panel_dialog =
        dynamic_cast<Dialog::PanelDialogBase *>(desktop->_dlg_mgr->getDialog("FillAndStroke"))) {
        try {
            Dialog::FillAndStroke &fill_and_stroke =
                dynamic_cast<Dialog::FillAndStroke &>(panel_dialog->getPanel());
            return &fill_and_stroke;
        } catch (std::exception &e) { }
    }

    return 0;
}

} // namespace Widget
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
