/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <brycehar@bryceharrington.org>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Josh Andler <scislac@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 * Copyright (C) 2008 Maximilian Albert (gtkmm-ification)
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_SS_VERBOSE

#include "widgets/dash-selector.h"
#include <gtkmm/radiobutton.h>
#include <gtkmm/table.h>
#include <glibmm/i18n.h>

#include "desktop-handles.h"
#include "desktop-style.h"
#include "dialogs/dialog-events.h"
#include "display/canvas-bpath.h" // for SP_STROKE_LINEJOIN_*
#include "display/drawing.h"
#include "document-private.h"
#include "document-undo.h"
#include "gradient-chemistry.h"
#include "helper/stock-items.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "inkscape.h"
#include "io/sys.h"
#include "marker.h"
#include "path-prefix.h"
#include "selection.h"
#include "sp-linear-gradient.h"
#include "sp-namedview.h"
#include "sp-pattern.h"
#include "sp-radial-gradient.h"
#include "sp-rect.h"
#include "sp-text.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "ui/cache/svg_preview_cache.h"
#include "ui/icon-names.h"
#include "widgets/icon.h"
#include "widgets/paint-selector.h"
#include "widgets/sp-widget.h"
#include "widgets/spw-utilities.h"
#include "ui/widget/spinbutton.h"
#include "xml/repr.h"

#include "stroke-style.h"
#include "stroke-marker-selector.h"
#include "fill-style.h" // to get sp_fill_style_widget_set_desktop
#include "fill-n-stroke-factory.h"

#include "verbs.h"

using Inkscape::DocumentUndo;


static MarkerComboBox *start_marker_combobox = NULL;
static MarkerComboBox *mid_marker_combobox = NULL;
static MarkerComboBox *end_marker_combobox = NULL;

sigc::connection start_marker_connection;
sigc::connection mid_marker_connection;
sigc::connection end_marker_connection;

static SPObject *ink_extract_marker_name(gchar const *n, SPDocument *doc);
static void      ink_markers_combo_update(Gtk::Container* spw, SPMarkerLoc const which);

Gtk::Widget *sp_stroke_style_paint_widget_new(void)
{
    return Inkscape::Widgets::createStyleWidget( STROKE );
}

void sp_stroke_style_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop)
{
    sp_fill_style_widget_set_desktop(widget, desktop);
}

/* Line */

static void sp_stroke_style_line_selection_modified(SPWidget *spw, Inkscape::Selection *selection, guint flags, gpointer data);
static void sp_stroke_style_line_selection_changed(SPWidget *spw, Inkscape::Selection *selection, gpointer data);

static void sp_stroke_style_line_update(Gtk::Container *spw, Inkscape::Selection *sel);

static void sp_stroke_style_set_join_buttons(Gtk::Container *spw, Gtk::ToggleButton *active);

static void sp_stroke_style_set_cap_buttons(Gtk::Container *spw, Gtk::ToggleButton *active);

static void sp_stroke_style_width_changed(Gtk::Container *spw);
static void sp_stroke_style_miterlimit_changed(Gtk::Container *spw);
static void sp_stroke_style_any_toggled(Gtk::ToggleButton *tb, Gtk::Container *spw);
static void sp_stroke_style_line_dash_changed(Gtk::Container *spw);

static void sp_stroke_style_update_marker_combo(Gtk::Container *spw, GSList const *objects);


/**
 * Helper function for creating radio buttons.  This should probably be re-thought out
 * when reimplementing this with Gtkmm.
 */
static Gtk::RadioButton *
sp_stroke_radio_button(Gtk::RadioButton *tb, char const *icon,
                       Gtk::HBox *hb, Gtk::Container *spw,
                       gchar const *key, gchar const *data)
{
    g_assert(icon != NULL);
    g_assert(hb  != NULL);
    g_assert(spw != NULL);

    if (tb == NULL) {
        tb = new Gtk::RadioButton();
    } else {
        Gtk::RadioButtonGroup grp = tb->get_group();
        tb = new Gtk::RadioButton(grp);
    }

    tb->show();
    tb->set_mode(false);
    hb->pack_start(*tb, false, false, 0);
    spw->set_data(icon, tb);
    tb->set_data(key, (gpointer*)data);
    tb->signal_toggled().connect(sigc::bind<Gtk::RadioButton *, Gtk::Container *>(
                                     sigc::ptr_fun(&sp_stroke_style_any_toggled), tb, spw));
    Gtk::Widget *px = manage(Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_LARGE_TOOLBAR, icon)));
    g_assert(px != NULL);
    px->show();
    tb->add(*px);

    return tb;

}

/**
 * Handles when user selects one of the markers from the marker combobox.
 * Gets the marker uri string and applies it to all selected
 * items in the current desktop.
 */
static void
sp_marker_select(MarkerComboBox *marker_combo, Gtk::Container *spw, SPMarkerLoc const which)
{
    if (spw->get_data("update")) {
        return;
    }

    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *document = sp_desktop_document(desktop);
    if (!document) {
        return;
    }

    /* Get Marker */
    gchar const *marker = marker_combo->get_active_marker_uri();


    SPCSSAttr *css = sp_repr_css_attr_new();
    gchar const *combo_id = marker_combo->get_id();
    sp_repr_css_set_property(css, combo_id, marker);

    // Also update the marker combobox, so the document's markers
    // show up at the top of the combobox
//    sp_stroke_style_line_update( SP_WIDGET(spw), desktop ? sp_desktop_selection(desktop) : NULL);
    ink_markers_combo_update(spw, which);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        SPItem *item = reinterpret_cast<SPItem *>(items->data);
        if (!SP_IS_SHAPE(item) || SP_IS_RECT(item)) { // can't set marker to rect, until it's converted to using <path>
            continue;
        }
        Inkscape::XML::Node *selrepr = item->getRepr();
        if (selrepr) {
            sp_repr_css_change_recursive(selrepr, css, "style");
        }
        item->requestModified(SP_OBJECT_MODIFIED_FLAG);
        item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    }

    sp_repr_css_attr_unref(css);
    css = 0;

    DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                       _("Set markers"));

};

static void
ink_markers_combo_update(Gtk::Container* /*spw*/, SPMarkerLoc const which) {

    switch (which) {
        case SP_MARKER_LOC_START:
            start_marker_connection.block();
            start_marker_combobox->set_active_history();
            start_marker_connection.unblock();
            break;

        case SP_MARKER_LOC_MID:
            mid_marker_connection.block();
            mid_marker_combobox->set_active_history();
            mid_marker_connection.unblock();
            break;

        case SP_MARKER_LOC_END:
            end_marker_connection.block();
            end_marker_combobox->set_active_history();
            end_marker_connection.unblock();
            break;
        default:
            g_assert_not_reached();
    }
}

/**
 * Sets the stroke width units for all selected items.
 * Also handles absolute and dimensionless units.
 */
static gboolean stroke_width_set_unit(SPUnitSelector *,
                                      SPUnit const *old,
                                      SPUnit const *new_units,
                                      Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return FALSE;
    }

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (!desktop) {
        return FALSE;
    }

    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    if (selection->isEmpty())
        return FALSE;

    GSList const *objects = selection->itemList();

    if ((old->base == SP_UNIT_ABSOLUTE || old->base == SP_UNIT_DEVICE) &&
       (new_units->base == SP_UNIT_DIMENSIONLESS)) {

        /* Absolute to percentage */
        spw->set_data ("update", GUINT_TO_POINTER (TRUE));

#if WITH_GTKMM_3_0
        Glib::RefPtr<Gtk::Adjustment> *a = static_cast<Glib::RefPtr<Gtk::Adjustment> *>(spw->get_data("width"));
        float w = sp_units_get_pixels( (*a)->get_value(), *old);
#else
        Gtk::Adjustment *a = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
        float w = sp_units_get_pixels(a->get_value(), *old);
#endif

        gdouble average = stroke_average_width (objects);

        if (average == Geom::infinity() || average == 0)
            return FALSE;

#if WITH_GTKMM_3_0
        (*a)->set_value(100.0 * w / average);
#else
        a->set_value(100.0 * w / average);
#endif

        spw->set_data ("update", GUINT_TO_POINTER (FALSE));
        return TRUE;

    } else if ((old->base == SP_UNIT_DIMENSIONLESS) &&
              (new_units->base == SP_UNIT_ABSOLUTE || new_units->base == SP_UNIT_DEVICE)) {

        /* Percentage to absolute */
        spw->set_data ("update", GUINT_TO_POINTER (TRUE));

#if WITH_GTKMM_3_0
        Glib::RefPtr<Gtk::Adjustment> *a = static_cast<Glib::RefPtr<Gtk::Adjustment> *>(spw->get_data("width"));
#else
        Gtk::Adjustment *a = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
#endif

        gdouble average = stroke_average_width (objects);

#if WITH_GTKMM_3_0
        (*a)->set_value (sp_pixels_get_units (0.01 * (*a)->get_value() * average, *new_units));
#else
        a->set_value (sp_pixels_get_units (0.01 * a->get_value() * average, *new_units));
#endif

        spw->set_data ("update", GUINT_TO_POINTER (FALSE));
        return TRUE;
    }

    return FALSE;
}


/**
 * Creates a new widget for the line stroke style.
 */
Gtk::Container *sp_stroke_style_line_widget_new(void)
{
    Inkscape::UI::Widget::SpinButton *sb;
    Gtk::RadioButton *tb;
    Gtk::HBox *hb;

    GtkWidget *spw_old = sp_widget_new_global(INKSCAPE);
    Gtk::Container *spw = dynamic_cast<Gtk::Container *>(manage(Glib::wrap(spw_old)));

    Gtk::HBox *f = new Gtk::HBox(false, 0);
    f->show();
    spw->add(*f);

    Gtk::Table *t = new Gtk::Table(3, 6, false);
    t->show();
    t->set_border_width(4);
    t->set_row_spacings(4);
    f->add(*t);
    spw->set_data("stroke", t);

    gint i = 0;

    //spw_label(t, C_("Stroke width", "_Width:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

// TODO: when this is gtkmmified, use an Inkscape::UI::Widget::ScalarUnit instead of the separate
// spinbutton and unit selector for stroke width. In sp_stroke_style_line_update, use
// setHundredPercent to remember the averaged width corresponding to 100%. Then the
// stroke_width_set_unit will be removed (because ScalarUnit takes care of conversions itself), and
// with it, the two remaining calls of stroke_average_width, allowing us to get rid of that
// function in desktop-style.

#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> *a = new Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(1.0, 0.0, 1000.0, 0.1, 10.0, 0.0));
    spw->set_data("width", a);
    sb = new Inkscape::UI::Widget::SpinButton(*a, 0.1, 3);
#else
    Gtk::Adjustment *a = new Gtk::Adjustment(1.0, 0.0, 1000.0, 0.1, 10.0, 0.0);
    spw->set_data("width", a);
    sb = new Inkscape::UI::Widget::SpinButton(*a, 0.1, 3);
#endif
    sb->set_tooltip_text(_("Stroke width"));
    sb->show();
    spw_label(t, C_("Stroke width", "_Width:"), 0, i, sb);

    sp_dialog_defocus_on_enter_cpp(sb);

    hb->pack_start(*sb, false, false, 0);
    GtkWidget *us_old = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
    Gtk::Widget *us = manage(Glib::wrap(us_old));
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop)
        sp_unit_selector_set_unit (SP_UNIT_SELECTOR(us_old), sp_desktop_namedview(desktop)->doc_units);
    sp_unit_selector_add_unit(SP_UNIT_SELECTOR(us_old), &sp_unit_get_by_id(SP_UNIT_PERCENT), 0);
    g_signal_connect ( G_OBJECT (us_old), "set_unit", G_CALLBACK (stroke_width_set_unit), spw );
    us->show();

#if WITH_GTKMM_3_0
    sp_unit_selector_add_adjustment( SP_UNIT_SELECTOR(us_old), GTK_ADJUSTMENT((*a)->gobj()) );
#else
    sp_unit_selector_add_adjustment( SP_UNIT_SELECTOR(us_old), GTK_ADJUSTMENT(a->gobj()) );
#endif

    hb->pack_start(*us, FALSE, FALSE, 0);
    spw->set_data("units", us_old);

#if WITH_GTKMM_3_0
    (*a)->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_width_changed), spw));
#else
    a->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_width_changed), spw));
#endif
    i++;

    /* Join type */
    // TRANSLATORS: The line join style specifies the shape to be used at the
    //  corners of paths. It can be "miter", "round" or "bevel".
    spw_label(t, _("Join:"), 0, i, NULL);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    tb = sp_stroke_radio_button(tb, INKSCAPE_ICON("stroke-join-miter"),
                                hb, spw, "join", "miter");

    // TRANSLATORS: Miter join: joining lines with a sharp (pointed) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    tb->set_tooltip_text(_("Miter join"));
    spw->set_data("miter join", tb);

    tb = sp_stroke_radio_button(tb, INKSCAPE_ICON("stroke-join-round"),
                                hb, spw, "join", "round");


    // TRANSLATORS: Round join: joining lines with a rounded corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    tb->set_tooltip_text(_("Round join"));
    spw->set_data("round join", tb);

    tb = sp_stroke_radio_button(tb, INKSCAPE_ICON("stroke-join-bevel"),
                                hb, spw, "join", "bevel");


    // TRANSLATORS: Bevel join: joining lines with a blunted (flattened) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    tb->set_tooltip_text(_("Bevel join"));
    spw->set_data("bevel join", tb);

    i++;

    /* Miterlimit  */
    // TRANSLATORS: Miter limit: only for "miter join", this limits the length
    //  of the sharp "spike" when the lines connect at too sharp an angle.
    // When two line segments meet at a sharp angle, a miter join results in a
    //  spike that extends well beyond the connection point. The purpose of the
    //  miter limit is to cut off such spikes (i.e. convert them into bevels)
    //  when they become too long.
    //spw_label(t, _("Miter _limit:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

#if WITH_GTKMM_3_0
    a = new Glib::RefPtr<Gtk::Adjustment>(Gtk::Adjustment::create(4.0, 0.0, 100.0, 0.1, 10.0, 0.0));
    spw->set_data("miterlimit", a);
    sb = new Inkscape::UI::Widget::SpinButton(*a, 0.1, 2);
#else
    a = new Gtk::Adjustment(4.0, 0.0, 100.0, 0.1, 10.0, 0.0);
    spw->set_data("miterlimit", a);
    sb = new Inkscape::UI::Widget::SpinButton(*a, 0.1, 2);
#endif

    sb->set_tooltip_text(_("Maximum length of the miter (in units of stroke width)"));
    sb->show();
    spw_label(t, _("Miter _limit:"), 0, i, sb);
    spw->set_data("miterlimit_sb", sb);
    sp_dialog_defocus_on_enter_cpp(sb);

    hb->pack_start(*sb, false, false, 0);

#if WITH_GTKMM_3_0
    (*a)->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_miterlimit_changed), spw));
#else
    a->signal_value_changed().connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_miterlimit_changed), spw));
#endif
    i++;

    /* Cap type */
    // TRANSLATORS: cap type specifies the shape for the ends of lines
    //spw_label(t, _("_Cap:"), 0, i);
    spw_label(t, _("Cap:"), 0, i, NULL);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    tb = sp_stroke_radio_button(tb, INKSCAPE_ICON("stroke-cap-butt"),
                                hb, spw, "cap", "butt");
    spw->set_data("cap butt", tb);

    // TRANSLATORS: Butt cap: the line shape does not extend beyond the end point
    //  of the line; the ends of the line are square
    tb->set_tooltip_text(_("Butt cap"));

    tb = sp_stroke_radio_button(tb, INKSCAPE_ICON("stroke-cap-round"),
                                hb, spw, "cap", "round");
    spw->set_data("cap round", tb);

    // TRANSLATORS: Round cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are rounded
    tb->set_tooltip_text(_("Round cap"));

    tb = sp_stroke_radio_button(tb, INKSCAPE_ICON("stroke-cap-square"),
                                hb, spw, "cap", "square");
    spw->set_data("cap square", tb);

    // TRANSLATORS: Square cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are square
    tb->set_tooltip_text(_("Square cap"));

    i++;


    /* Dash */
    spw_label(t, _("Dashes:"), 0, i, NULL); //no mnemonic for now
                                            //decide what to do:
                                            //   implement a set_mnemonic_source function in the
                                            //   SPDashSelector class, so that we do not have to
                                            //   expose any of the underlying widgets?
    SPDashSelector *ds = manage(new SPDashSelector);

    ds->show();
    t->attach(*ds, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("dash", ds);
    ds->changed_signal.connect(sigc::bind(sigc::ptr_fun(&sp_stroke_style_line_dash_changed), spw));
    i++;

    /* Drop down marker selectors*/
    // TRANSLATORS: Path markers are an SVG feature that allows you to attach arbitrary shapes
    // (arrowheads, bullets, faces, whatever) to the start, end, or middle nodes of a path.

    start_marker_combobox = manage(new MarkerComboBox("marker-start"));
    spw_label(t, _("_Start Markers:"), 0, i, start_marker_combobox);
    start_marker_combobox->set_tooltip_text(_("Start Markers are drawn on the first node of a path or shape"));
    start_marker_connection = start_marker_combobox->signal_changed().connect(
            sigc::bind<MarkerComboBox *, Gtk::Container *, SPMarkerLoc>(
                sigc::ptr_fun(&sp_marker_select), start_marker_combobox, spw, SP_MARKER_LOC_START));
    start_marker_combobox->show();
    t->attach(*start_marker_combobox, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("start_marker_combobox", start_marker_combobox);
    i++;

    mid_marker_combobox =  manage(new MarkerComboBox("marker-mid"));
    spw_label(t, _("_Mid Markers:"), 0, i, mid_marker_combobox);
    mid_marker_combobox->set_tooltip_text(_("Mid Markers are drawn on every node of a path or shape except the first and last nodes"));
    mid_marker_connection = mid_marker_combobox->signal_changed().connect(
        sigc::bind<MarkerComboBox *, Gtk::Container *, SPMarkerLoc>(
            sigc::ptr_fun(&sp_marker_select), mid_marker_combobox, spw, SP_MARKER_LOC_MID));
    mid_marker_combobox->show();
    t->attach(*mid_marker_combobox, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("mid_marker_combobox", mid_marker_combobox);
    i++;

    end_marker_combobox = manage(new MarkerComboBox("marker-end"));
    spw_label(t, _("_End Markers:"), 0, i, end_marker_combobox);
    end_marker_combobox->set_tooltip_text(_("End Markers are drawn on the last node of a path or shape"));
    end_marker_connection = end_marker_combobox->signal_changed().connect(
        sigc::bind<MarkerComboBox *, Gtk::Container *, SPMarkerLoc>(
            sigc::ptr_fun(&sp_marker_select), end_marker_combobox, spw, SP_MARKER_LOC_END));
    end_marker_combobox->show();
    t->attach(*end_marker_combobox, 1, 4, i, i+1, (Gtk::EXPAND | Gtk::FILL), static_cast<Gtk::AttachOptions>(0), 0, 0);
    spw->set_data("end_marker_combobox", end_marker_combobox);
    i++;

    // FIXME: we cheat and still use gtk+ signals

    g_signal_connect(G_OBJECT(spw_old), "modify_selection",
                     G_CALLBACK(sp_stroke_style_line_selection_modified),
                     spw);
    g_signal_connect(G_OBJECT(spw_old), "change_selection",
                     G_CALLBACK(sp_stroke_style_line_selection_changed),
                     spw);

    sp_stroke_style_line_update(spw, desktop ? sp_desktop_selection(desktop) : NULL);

    return spw;
}

/**
 * Callback for when stroke style widget is modified.
 * Triggers update action.
 */
static void
sp_stroke_style_line_selection_modified(SPWidget *,
                                        Inkscape::Selection *selection,
                                        guint flags,
                                        gpointer data)
{
    Gtk::Container *spw = static_cast<Gtk::Container *>(data);
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        sp_stroke_style_line_update(spw, selection);
    }

}

/**
 * Callback for when stroke style widget is changed.
 * Triggers update action.
 */
static void
sp_stroke_style_line_selection_changed(SPWidget *,
                                       Inkscape::Selection *selection,
                                       gpointer data)
{
    Gtk::Container *spw = static_cast<Gtk::Container *>(data);
    sp_stroke_style_line_update(spw, selection);
}

/**
 * Sets selector widgets' dash style from an SPStyle object.
 */
static void
sp_dash_selector_set_from_style(SPDashSelector *dsel, SPStyle *style)
{
    if (style->stroke_dash.n_dash > 0) {
        double d[64];
        int len = MIN(style->stroke_dash.n_dash, 64);
        for (int i = 0; i < len; i++) {
            if (style->stroke_width.computed != 0)
                d[i] = style->stroke_dash.dash[i] / style->stroke_width.computed;
            else
                d[i] = style->stroke_dash.dash[i]; // is there a better thing to do for stroke_width==0?
        }
        dsel->set_dash(len, d, style->stroke_width.computed != 0 ?
                       style->stroke_dash.offset / style->stroke_width.computed  :
                       style->stroke_dash.offset);
    } else {
        dsel->set_dash(0, NULL, 0.0);
    }
}

/**
 * Sets the join type for a line, and updates the stroke style widget's buttons
 */
static void
sp_jointype_set (Gtk::Container *spw, unsigned const jointype)
{
    Gtk::RadioButton *tb = NULL;
    switch (jointype) {
        case SP_STROKE_LINEJOIN_MITER:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-join-miter")));
            break;
        case SP_STROKE_LINEJOIN_ROUND:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-join-round")));
            break;
        case SP_STROKE_LINEJOIN_BEVEL:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-join-bevel")));
            break;
        default:
            break;
    }
    sp_stroke_style_set_join_buttons(spw, tb);
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
static void
sp_captype_set (Gtk::Container *spw, unsigned const captype)
{
    Gtk::RadioButton *tb = NULL;
    switch (captype) {
        case SP_STROKE_LINECAP_BUTT:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-cap-butt")));
            break;
        case SP_STROKE_LINECAP_ROUND:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-cap-round")));
            break;
        case SP_STROKE_LINECAP_SQUARE:
            tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-cap-square")));
            break;
        default:
            break;
    }
    sp_stroke_style_set_cap_buttons(spw, tb);
}

/**
 * Callback for when stroke style widget is updated, including markers, cap type,
 * join type, etc.
 */
static void
sp_stroke_style_line_update(Gtk::Container *spw, Inkscape::Selection *sel)
{
    if (spw->get_data("update")) {
        return;
    }

    spw->set_data("update", GINT_TO_POINTER(TRUE));

    FillOrStroke kind = GPOINTER_TO_INT(spw->get_data("kind")) ? FILL : STROKE;

    Gtk::Table *sset = static_cast<Gtk::Table *>(spw->get_data("stroke"));

#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> *width = static_cast<Glib::RefPtr<Gtk::Adjustment> *>(spw->get_data("width"));
    Glib::RefPtr<Gtk::Adjustment> *ml = static_cast<Glib::RefPtr<Gtk::Adjustment> *>(spw->get_data("miterlimit"));
#else
    Gtk::Adjustment *width = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
    Gtk::Adjustment *ml = static_cast<Gtk::Adjustment *>(spw->get_data("miterlimit"));
#endif

    SPUnitSelector *us = SP_UNIT_SELECTOR(spw->get_data("units"));
    SPDashSelector *dsel = static_cast<SPDashSelector *>(spw->get_data("dash"));

    // create temporary style
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    // query into it
    int result_sw = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
    int result_ml = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKEMITERLIMIT);
    int result_cap = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKECAP);
    int result_join = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_STROKEJOIN);
    SPIPaint &targPaint = (kind == FILL) ? query->fill : query->stroke;

    if (!sel || sel->isEmpty()) {
        // Nothing selected, grey-out all controls in the stroke-style dialog
        sset->set_sensitive(false);

        spw->set_data("update", GINT_TO_POINTER(FALSE));

        return;
    } else {
        sset->set_sensitive(true);

        SPUnit const *unit = sp_unit_selector_get_unit(us);

        if (result_sw == QUERY_STYLE_MULTIPLE_AVERAGED) {
            sp_unit_selector_set_unit(us, &sp_unit_get_by_id(SP_UNIT_PERCENT));
        } else {
            // same width, or only one object; no sense to keep percent, switch to absolute
            if (unit->base != SP_UNIT_ABSOLUTE && unit->base != SP_UNIT_DEVICE) {
                sp_unit_selector_set_unit(us, sp_desktop_namedview(SP_ACTIVE_DESKTOP)->doc_units);
            }
        }

        unit = sp_unit_selector_get_unit(us);

        if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
            double avgwidth = sp_pixels_get_units (query->stroke_width.computed, *unit);
#if WITH_GTKMM_3_0
            (*width)->set_value(avgwidth);
#else
            width->set_value(avgwidth);
#endif
        } else {
#if WITH_GTKMM_3_0
            (*width)->set_value(100);
#else
            width->set_value(100);
#endif
        }

        // if none of the selected objects has a stroke, than quite some controls should be disabled
        // The markers might still be shown though, so these will not be disabled
        bool enabled = (result_sw != QUERY_STYLE_NOTHING) && !targPaint.isNoneSet();
        /* No objects stroked, set insensitive */
        Gtk::RadioButton *tb = NULL;
        tb = static_cast<Gtk::RadioButton *>(spw->get_data("miter join"));
        tb->set_sensitive(enabled);
        tb = static_cast<Gtk::RadioButton *>(spw->get_data("round join"));
        tb->set_sensitive(enabled);
        tb = static_cast<Gtk::RadioButton *>(spw->get_data("bevel join"));
        tb->set_sensitive(enabled);

        Inkscape::UI::Widget::SpinButton* sb = NULL;
        sb = static_cast<Inkscape::UI::Widget::SpinButton *>(spw->get_data("miterlimit_sb"));
        sb->set_sensitive(enabled);

        tb = static_cast<Gtk::RadioButton *>(spw->get_data("cap butt"));
        tb->set_sensitive(enabled);
        tb = static_cast<Gtk::RadioButton *>(spw->get_data("cap round"));
        tb->set_sensitive(enabled);
        tb = static_cast<Gtk::RadioButton *>(spw->get_data("cap square"));
        tb->set_sensitive(enabled);

        dsel->set_sensitive(enabled);
    }

    if (result_ml != QUERY_STYLE_NOTHING)
#if WITH_GTKMM_3_0
        (*ml)->set_value(query->stroke_miterlimit.value); // TODO: reflect averagedness?
#else
        ml->set_value(query->stroke_miterlimit.value); // TODO: reflect averagedness?
#endif

    if (result_join != QUERY_STYLE_MULTIPLE_DIFFERENT) {
        sp_jointype_set(spw, query->stroke_linejoin.value);
    } else {
        sp_stroke_style_set_join_buttons(spw, NULL);
    }

    if (result_cap != QUERY_STYLE_MULTIPLE_DIFFERENT) {
        sp_captype_set (spw, query->stroke_linecap.value);
    } else {
        sp_stroke_style_set_cap_buttons(spw, NULL);
    }

    sp_style_unref(query);

    if (!sel || sel->isEmpty())
        return;

    GSList const *objects = sel->itemList();
    SPObject * const object = SP_OBJECT(objects->data);
    SPStyle * const style = object->style;

    /* Markers */
    sp_stroke_style_update_marker_combo(spw, objects); // FIXME: make this desktop query too

    /* Dash */
    sp_dash_selector_set_from_style(dsel, style); // FIXME: make this desktop query too

    sset->set_sensitive(true);

    spw->set_data("update", GINT_TO_POINTER(FALSE));
}

/**
 * Sets a line's dash properties in a CSS style object.
 */
static void
sp_stroke_style_set_scaled_dash(SPCSSAttr *css,
                                int ndash, double *dash, double offset,
                                double scale)
{
    if (ndash > 0) {
        Inkscape::CSSOStringStream osarray;
        for (int i = 0; i < ndash; i++) {
            osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
                osarray << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", osarray.str().c_str());

        Inkscape::CSSOStringStream osoffset;
        osoffset << offset * scale;
        sp_repr_css_set_property(css, "stroke-dashoffset", osoffset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", NULL);
    }
}

/**
 * Sets line properties like width, dashes, markers, etc. on all currently selected items.
 */
static void
sp_stroke_style_scale_line(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    spw->set_data("update", GINT_TO_POINTER(TRUE));

#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> *wadj = static_cast<Glib::RefPtr<Gtk::Adjustment> *>(spw->get_data("width"));
    Glib::RefPtr<Gtk::Adjustment> *ml = static_cast<Glib::RefPtr<Gtk::Adjustment> *>(spw->get_data("miterlimit"));
#else
    Gtk::Adjustment *wadj = static_cast<Gtk::Adjustment *>(spw->get_data("width"));
    Gtk::Adjustment *ml = static_cast<Gtk::Adjustment *>(spw->get_data("miterlimit"));
#endif
    
    SPUnitSelector *us = SP_UNIT_SELECTOR(spw->get_data("units"));
    SPDashSelector *dsel = static_cast<SPDashSelector *>(spw->get_data("dash"));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPDocument *document = sp_desktop_document (desktop);
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    GSList const *items = selection->itemList();

    /* TODO: Create some standardized method */
    SPCSSAttr *css = sp_repr_css_attr_new();

    if (items) {
#if WITH_GTKMM_3_0
        double width_typed = (*wadj)->get_value();
        double const miterlimit = (*ml)->get_value();
#else
        double width_typed = wadj->get_value();
        double const miterlimit = ml->get_value();
#endif

        SPUnit const *const unit = sp_unit_selector_get_unit(SP_UNIT_SELECTOR(us));

        double *dash, offset;
        int ndash;
        dsel->get_dash(&ndash, &dash, &offset);

        for (GSList const *i = items; i != NULL; i = i->next) {
            /* Set stroke width */
            double width;
            if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
                width = sp_units_get_pixels (width_typed, *unit);
            } else { // percentage
                gdouble old_w = SP_OBJECT(i->data)->style->stroke_width.computed;
                width = old_w * width_typed / 100;
            }

            {
                Inkscape::CSSOStringStream os_width;
                os_width << width;
                sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());
            }

            {
                Inkscape::CSSOStringStream os_ml;
                os_ml << miterlimit;
                sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());
            }

            /* Set dash */
            sp_stroke_style_set_scaled_dash(css, ndash, dash, offset, width);

            sp_desktop_apply_css_recursive (SP_OBJECT(i->data), css, true);
        }

        g_free(dash);

        if (unit->base != SP_UNIT_ABSOLUTE && unit->base != SP_UNIT_DEVICE) {
            // reset to 100 percent
#if WITH_GTKMM_3_0
            (*wadj)->set_value(100.0);
#else
            wadj->set_value(100.0);
#endif
        }

    }

    // we have already changed the items, so set style without changing selection
    // FIXME: move the above stroke-setting stuff, including percentages, to desktop-style
    sp_desktop_set_style (desktop, css, false);

    sp_repr_css_attr_unref(css);
    css = 0;

    DocumentUndo::done(document, SP_VERB_DIALOG_FILL_STROKE,
                       _("Set stroke style"));

    spw->set_data("update", GINT_TO_POINTER(FALSE));
}

/**
 * Callback for when the stroke style's width changes.
 * Causes all line styles to be applied to all selected items.
 */
static void
sp_stroke_style_width_changed(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

/**
 * Callback for when the stroke style's miterlimit changes.
 * Causes all line styles to be applied to all selected items.
 */
static void
sp_stroke_style_miterlimit_changed(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

/**
 * Callback for when the stroke style's dash changes.
 * Causes all line styles to be applied to all selected items.
 */

static void
sp_stroke_style_line_dash_changed(Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

/**
 * This routine handles toggle events for buttons in the stroke style dialog.
 *
 * When activated, this routine gets the data for the various widgets, and then
 * calls the respective routines to update css properties, etc.
 *
 */
static void sp_stroke_style_any_toggled(Gtk::ToggleButton *tb, Gtk::Container *spw)
{
    if (spw->get_data("update")) {
        return;
    }


    if (tb->get_active()) {

        gchar const *join
            = static_cast<gchar const *>(tb->get_data("join"));
        gchar const *cap
            = static_cast<gchar const *>(tb->get_data("cap"));

        if (join) {
            Gtk::SpinButton *ml = static_cast<Gtk::SpinButton *>(spw->get_data("miterlimit_sb"));
            ml->set_sensitive(!strcmp(join, "miter"));
        }

        SPDesktop *desktop = SP_ACTIVE_DESKTOP;

        /* TODO: Create some standardized method */
        SPCSSAttr *css = sp_repr_css_attr_new();

        if (join) {
            sp_repr_css_set_property(css, "stroke-linejoin", join);

            sp_desktop_set_style (desktop, css);

            sp_stroke_style_set_join_buttons(spw, tb);
        } else if (cap) {
            sp_repr_css_set_property(css, "stroke-linecap", cap);

            sp_desktop_set_style (desktop, css);

            sp_stroke_style_set_cap_buttons(spw, tb);
        }

        sp_repr_css_attr_unref(css);
        css = 0;

        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_DIALOG_FILL_STROKE,
                           _("Set stroke style"));
    }
}

/**
 * Updates the join style toggle buttons
 */
static void
sp_stroke_style_set_join_buttons(Gtk::Container *spw, Gtk::ToggleButton *active)
{
    Gtk::RadioButton *tb;

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-join-miter")));
    tb->set_active(active == tb);

    Gtk::SpinButton *ml = static_cast<Gtk::SpinButton *>(spw->get_data("miterlimit_sb"));
    ml->set_sensitive(active == tb);

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-join-round")));
    tb->set_active(active == tb);

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-join-bevel")));
    tb->set_active(active == tb);
}

/**
 * Updates the cap style toggle buttons
 */
static void
sp_stroke_style_set_cap_buttons(Gtk::Container *spw, Gtk::ToggleButton *active)
{
    Gtk::RadioButton *tb;

    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-cap-butt")));
    tb->set_active(active == tb);
    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-cap-round")));
    tb->set_active(active == tb);
    tb = static_cast<Gtk::RadioButton *>(spw->get_data(INKSCAPE_ICON("stroke-cap-square")));
    tb->set_active(active == tb);
}


/**
 * Updates the marker combobox to highlight the appropriate marker and scroll to
 * that marker.
 */
static void
sp_stroke_style_update_marker_combo(Gtk::Container *spw, GSList const *objects)
{
    struct { char const *key; int loc; } const keyloc[] = {
            { "start_marker_combobox", SP_MARKER_LOC_START },
            { "mid_marker_combobox", SP_MARKER_LOC_MID },
            { "end_marker_combobox", SP_MARKER_LOC_END }
    };

    bool all_texts = true;
    for (GSList *i = (GSList *) objects; i != NULL; i = i->next) {
        if (!SP_IS_TEXT (i->data)) {
            all_texts = false;
        }
    }

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        MarkerComboBox *combo = static_cast<MarkerComboBox *>(spw->get_data(keyloc[i].key));
        // Per SVG spec, text objects cannot have markers; disable combobox if only texts are selected
        combo->set_sensitive(!all_texts);
    }

    // We show markers of the first object in the list only
    // FIXME: use the first in the list that has the marker of each type, if any
    SPObject *object = SP_OBJECT(objects->data);

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        // For all three marker types,

        // find the corresponding combobox item
        MarkerComboBox *combo = static_cast<MarkerComboBox *>(spw->get_data(keyloc[i].key));

        // Quit if we're in update state
        if (combo->update()) {
            return;
        }

        if (object->style->marker[keyloc[i].loc].value != NULL && !all_texts) {
            // If the object has this type of markers,

            // Extract the name of the marker that the object uses
            SPObject *marker = ink_extract_marker_name(object->style->marker[keyloc[i].loc].value, object->document);
            // Scroll the combobox to that marker
            combo->set_current(marker);

        } else {
            combo->set_current(NULL);
        }
    }

}


/**
 * Extract the actual name of the link
 * e.g. get mTriangle from url(#mTriangle).
 * \return Buffer containing the actual name, allocated from GLib;
 * the caller should free the buffer when they no longer need it.
 */
static SPObject*
ink_extract_marker_name(gchar const *n, SPDocument *doc)
{
    gchar const *p = n;
    while (*p != '\0' && *p != '#') {
        p++;
    }

    if (*p == '\0' || p[1] == '\0') {
        return NULL;
    }

    p++;
    int c = 0;
    while (p[c] != '\0' && p[c] != ')') {
        c++;
    }

    if (p[c] == '\0') {
        return NULL;
    }

    gchar* b = g_strdup(p);
    b[c] = '\0';

    // FIXME: get the document from the object and let the caller pass it in
    SPObject *marker = doc->getObjectById(b);
    return marker;
}

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
