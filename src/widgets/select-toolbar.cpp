/*
 * Selector aux toolbar
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2003-2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <gtk/gtkaction.h>

#include "widgets/button.h"
#include "widgets/spw-utilities.h"
#include "widgets/widget-sizes.h"
#include "widgets/spinbutton-events.h"
#include "widgets/icon.h"
#include "widgets/sp-widget.h"

#include "prefs-utils.h"
#include "selection-chemistry.h"
#include "document.h"
#include "inkscape.h"
#include "desktop-style.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "sp-namedview.h"
#include "toolbox.h"
#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "inkscape.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "sp-item-transform.h"
#include "message-stack.h"
#include "display/sp-canvas.h"
#include "ege-select-one-action.h"
#include "helper/unit-tracker.h"

using Inkscape::UnitTracker;

static void
sp_selection_layout_widget_update(SPWidget *spw, Inkscape::Selection *sel)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    GtkWidget *f = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(spw), "frame");

    using NR::X;
    using NR::Y;
    if ( sel && !sel->isEmpty() ) {
        NR::Maybe<NR::Rect> const bbox(sel->bounds());
        if ( bbox && !bbox->isEmpty() ) {
            UnitTracker *tracker = reinterpret_cast<UnitTracker*>(gtk_object_get_data(GTK_OBJECT(spw), "tracker"));
            SPUnit const &unit = *tracker->getActiveUnit();

            struct { char const *key; double val; } const keyval[] = {
                { "X", bbox->min()[X] },
                { "Y", bbox->min()[Y] },
                { "width", bbox->extent(X) },
                { "height", bbox->extent(Y) }
            };

            if (unit.base == SP_UNIT_DIMENSIONLESS) {
                double const val = 1. / unit.unittobase;
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    GtkAdjustment *a = (GtkAdjustment *) gtk_object_get_data(GTK_OBJECT(spw), keyval[i].key);
                    gtk_adjustment_set_value(a, val);
                    tracker->setFullVal( a, keyval[i].val );
                }
            } else {
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    GtkAdjustment *a = (GtkAdjustment *) gtk_object_get_data(GTK_OBJECT(spw), keyval[i].key);
                    gtk_adjustment_set_value(a, sp_pixels_get_units(keyval[i].val, unit));
                }
            }

            gtk_widget_set_sensitive(f, TRUE);
        } else {
            gtk_widget_set_sensitive(f, FALSE);
        }
    } else {
        gtk_widget_set_sensitive(f, FALSE);
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}


static void
sp_selection_layout_widget_modify_selection(SPWidget *spw, Inkscape::Selection *selection, guint flags, gpointer data)
{
    SPDesktop *desktop = (SPDesktop *) data;
    if ((sp_desktop_selection(desktop) == selection) // only respond to changes in our desktop
        && (flags & (SP_OBJECT_MODIFIED_FLAG        |
                     SP_OBJECT_PARENT_MODIFIED_FLAG |
                     SP_OBJECT_CHILD_MODIFIED_FLAG   )))
    {
        sp_selection_layout_widget_update(spw, selection);
    }
}

static void
sp_selection_layout_widget_change_selection(SPWidget *spw, Inkscape::Selection *selection, gpointer data)
{
    SPDesktop *desktop = (SPDesktop *) data;
    if (sp_desktop_selection(desktop) == selection) // only respond to changes in our desktop
        sp_selection_layout_widget_update(spw, selection);
}

static void
sp_object_layout_any_value_changed(GtkAdjustment *adj, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    UnitTracker *tracker = reinterpret_cast<UnitTracker*>(gtk_object_get_data(GTK_OBJECT(spw), "tracker"));
    if ( !tracker || tracker->isUpdating() ) {
        /*
         * When only units are being changed, don't treat changes
         * to adjuster values as object changes.
         */
        return;
    }
    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    SPDocument *document = sp_desktop_document(desktop);

    sp_document_ensure_up_to_date (document);
    NR::Maybe<NR::Rect> bbox = selection->bounds();

    if ( !bbox || bbox->isEmpty() ) {
        return;
    }

    gdouble x0 = 0;
    gdouble y0 = 0;
    gdouble x1 = 0;
    gdouble y1 = 0;
    gdouble xrel = 0;
    gdouble yrel = 0;
    SPUnit const &unit = *tracker->getActiveUnit();

    GtkAdjustment* a_x = (GtkAdjustment *)gtk_object_get_data( GTK_OBJECT(spw), "X" );
    GtkAdjustment* a_y = (GtkAdjustment *)gtk_object_get_data( GTK_OBJECT(spw), "Y" );
    GtkAdjustment* a_w = (GtkAdjustment *)gtk_object_get_data( GTK_OBJECT(spw), "width" );
    GtkAdjustment* a_h = (GtkAdjustment *)gtk_object_get_data( GTK_OBJECT(spw), "height" );

    if (unit.base == SP_UNIT_ABSOLUTE || unit.base == SP_UNIT_DEVICE) {
        x0 = sp_units_get_pixels (a_x->value, unit);
        y0 = sp_units_get_pixels (a_y->value, unit);
        x1 = x0 + sp_units_get_pixels (a_w->value, unit);
        xrel = sp_units_get_pixels (a_w->value, unit) / bbox->extent(NR::X);
        y1 = y0 + sp_units_get_pixels (a_h->value, unit);
        yrel = sp_units_get_pixels (a_h->value, unit) / bbox->extent(NR::Y);
    } else {
        double const x0_propn = a_x->value * unit.unittobase;
        x0 = bbox->min()[NR::X] * x0_propn;
        double const y0_propn = a_y->value * unit.unittobase;
        y0 = y0_propn * bbox->min()[NR::Y];
        xrel = a_w->value * unit.unittobase;
        x1 = x0 + xrel * bbox->extent(NR::X);
        yrel = a_h->value * unit.unittobase;
        y1 = y0 + yrel * bbox->extent(NR::Y);
    }

    // Keep proportions if lock is on
    GtkWidget *lock = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "lock"));
    if (SP_BUTTON_IS_DOWN(lock)) {
        if (adj == a_h) {
            x1 = x0 + yrel * bbox->extent(NR::X);
        } else if (adj == a_w) {
            y1 = y0 + xrel * bbox->extent(NR::Y);
        }
    }

    // scales and moves, in px
    double mh = fabs(x0 - bbox->min()[NR::X]);
    double sh = fabs(x1 - bbox->max()[NR::X]);
    double mv = fabs(y0 - bbox->min()[NR::Y]);
    double sv = fabs(y1 - bbox->max()[NR::Y]);

    // unless the unit is %, convert the scales and moves to the unit
    if (unit.base == SP_UNIT_ABSOLUTE || unit.base == SP_UNIT_DEVICE) {
        mh = sp_pixels_get_units (mh, unit);
        sh = sp_pixels_get_units (sh, unit);
        mv = sp_pixels_get_units (mv, unit);
        sv = sp_pixels_get_units (sv, unit);
    }

    // do the action only if one of the scales/moves is greater than half the last significant
    // digit in the spinbox (currently spinboxes have 3 fractional digits, so that makes 0.0005). If
    // the value was changed by the user, the difference will be at least that much; otherwise it's
    // just rounding difference between the spinbox value and actual value, so no action is
    // performed
    char const * const actionkey = ( mh > 5e-4 ? "selector:toolbar:move:horizontal" :
                                     sh > 5e-4 ? "selector:toolbar:scale:horizontal" :
                                     mv > 5e-4 ? "selector:toolbar:move:vertical" :
                                     sv > 5e-4 ? "selector:toolbar:scale:vertical" : NULL );

    if (actionkey != NULL) {

        // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed
        sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);

        gdouble strokewidth = stroke_average_width (selection->itemList());
        int transform_stroke = prefs_get_int_attribute ("options.transform", "stroke", 1);

        NR::Matrix scaler = get_scale_transform_with_stroke (*bbox, strokewidth, transform_stroke, x0, y0, x1, y1);

        sp_selection_apply_affine(selection, scaler);
        sp_document_maybe_done (document, actionkey, SP_VERB_CONTEXT_SELECT,
                                _("Transform by toolbar"));

        // defocus spinbuttons by moving focus to the canvas, unless "stay" is on
        spinbutton_defocus(GTK_OBJECT(spw));

        // resume interruptibility
        sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}

GtkWidget *
sp_select_toolbox_spinbutton(gchar *label, gchar *data, float lower_limit, UnitTracker* tracker, GtkWidget *spw, gchar *tooltip, gboolean altx)
{
    GtkTooltips *tt = gtk_tooltips_new();

    GtkWidget *hb = gtk_hbox_new(FALSE, 1);
    GtkWidget *l = gtk_label_new(Q_(label));
    gtk_tooltips_set_tip(tt, l, tooltip, NULL);
    gtk_widget_show(l);
    gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
    gtk_container_add(GTK_CONTAINER(hb), l);

    GtkObject *a = gtk_adjustment_new(0.0, lower_limit, 1e6, SPIN_STEP, SPIN_PAGE_STEP, SPIN_PAGE_STEP);
    if ( tracker ) {
        tracker->addAdjustment( GTK_ADJUSTMENT(a) );
    }
    gtk_object_set_data(GTK_OBJECT(spw), data, a);

    GtkWidget *sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), SPIN_STEP, 3);
    gtk_tooltips_set_tip(tt, sb, tooltip, NULL);
    gtk_widget_set_size_request(sb, AUX_SPINBUTTON_WIDTH, AUX_SPINBUTTON_HEIGHT);
    gtk_widget_show(sb);
    gtk_signal_connect(GTK_OBJECT(sb), "focus-in-event", GTK_SIGNAL_FUNC(spinbutton_focus_in), spw);
    gtk_signal_connect(GTK_OBJECT(sb), "key-press-event", GTK_SIGNAL_FUNC(spinbutton_keypress), spw);

    gtk_container_add(GTK_CONTAINER(hb), sb);
    gtk_signal_connect(GTK_OBJECT(a), "value_changed", GTK_SIGNAL_FUNC(sp_object_layout_any_value_changed), spw);

    if (altx) { // this spinbutton will be activated by alt-x
        gtk_object_set_data(GTK_OBJECT(sb), "altx", sb);
    }

    return hb;
}

// toggle button callbacks and updaters

static void toggle_stroke (GtkWidget *button, gpointer data) {
    prefs_set_int_attribute ("options.transform", "stroke", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 1 : 0);
    SPDesktop *desktop = (SPDesktop *)data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>scaled</b> when objects are scaled."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>not scaled</b> when objects are scaled."));
    }
}

static void toggle_corners (GtkWidget *button, gpointer data) {
    prefs_set_int_attribute ("options.transform", "rectcorners", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 1 : 0);
    SPDesktop *desktop = (SPDesktop *)data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>scaled</b> when rectangles are scaled."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>not scaled</b> when rectangles are scaled."));
    }
}

static void toggle_gradient (GtkWidget *button, gpointer data) {
    prefs_set_int_attribute ("options.transform", "gradient", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 1 : 0);
    SPDesktop *desktop = (SPDesktop *)data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

static void toggle_pattern (GtkWidget *button, gpointer data) {
    prefs_set_int_attribute ("options.transform", "pattern", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)) ? 1 : 0);
    SPDesktop *desktop = (SPDesktop *)data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

static void toggle_lock (GtkWidget *button, gpointer data) {

    GtkWidget *old_child = gtk_bin_get_child(GTK_BIN(button));
    gtk_container_remove (GTK_CONTAINER(button), old_child);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
        GtkWidget *child = sp_icon_new (Inkscape::ICON_SIZE_DECORATION, "width_height_lock");
        gtk_widget_show (child);
        gtk_container_add (GTK_CONTAINER (button), child);
    } else {
        GtkWidget *child = sp_icon_new (Inkscape::ICON_SIZE_DECORATION, "lock_unlocked");
        gtk_widget_show (child);
        gtk_container_add (GTK_CONTAINER (button), child);
    }
}

static void destroy_tracker( GtkObject* obj, gpointer /*user_data*/ )
{
    UnitTracker *tracker = reinterpret_cast<UnitTracker*>(gtk_object_get_data(obj, "tracker"));
    if ( tracker ) {
        delete tracker;
        gtk_object_set_data( obj, "tracker", 0 );
    }
}

GtkWidget *
sp_select_toolbox_new(SPDesktop *desktop)
{
    Inkscape::UI::View::View *view = desktop;

    GtkTooltips *tt = gtk_tooltips_new();
    GtkWidget *tb = gtk_hbox_new(FALSE, 0);

    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_OBJECT_ROTATE_90_CCW), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_OBJECT_ROTATE_90_CW), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_OBJECT_FLIP_HORIZONTAL), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_OBJECT_FLIP_VERTICAL), view, tt);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_SELECTION_TO_BACK), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_SELECTION_LOWER), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_SELECTION_RAISE), view, tt);
    sp_toolbox_button_normal_new_from_verb(tb, Inkscape::ICON_SIZE_SMALL_TOOLBAR, Inkscape::Verb::get(SP_VERB_SELECTION_TO_FRONT), view, tt);

    // Create the parent widget for x y w h tracker.
    GtkWidget *spw = sp_widget_new_global(INKSCAPE);

    // Remember the desktop's canvas widget, to be used for defocusing.
    gtk_object_set_data(GTK_OBJECT(spw), "dtw", sp_desktop_canvas(desktop));

    // The vb frame holds all other widgets and is used to set sensitivity depending on selection state.
    GtkWidget *vb = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(vb);
    gtk_container_add(GTK_CONTAINER(spw), vb);
    gtk_object_set_data(GTK_OBJECT(spw), "frame", vb);

    // Create the units menu.
    UnitTracker* tracker = new UnitTracker( SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE );
    tracker->addUnit( SP_UNIT_PERCENT, 0 );
    tracker->setActiveUnit( sp_desktop_namedview(desktop)->doc_units );

    gtk_object_set_data( GTK_OBJECT(spw), "tracker", tracker );
    g_signal_connect( G_OBJECT(spw), "destroy", G_CALLBACK(destroy_tracker), spw );


    // four spinbuttons

    gtk_container_add(GTK_CONTAINER(vb),
                      //TRANSLATORS: only translate "string" in "context|string".
                      // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                      sp_select_toolbox_spinbutton(_("select_toolbar|X"), "X", -1e6, tracker, spw, _("Horizontal coordinate of selection"), TRUE));
    aux_toolbox_space(vb, AUX_BETWEEN_SPINBUTTONS);
    gtk_container_add(GTK_CONTAINER(vb),
                      //TRANSLATORS: only translate "string" in "context|string".
                      // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                      sp_select_toolbox_spinbutton(_("select_toolbar|Y"), "Y", -1e6, tracker, spw, _("Vertical coordinate of selection"), FALSE));
    aux_toolbox_space(vb, AUX_BETWEEN_BUTTON_GROUPS);

    gtk_container_add(GTK_CONTAINER(vb),
                      //TRANSLATORS: only translate "string" in "context|string".
                      // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                      sp_select_toolbox_spinbutton(_("select_toolbar|W"), "width", 1e-3, tracker, spw, _("Width of selection"), FALSE));

    // lock toggle
    GtkWidget *lockbox = gtk_vbox_new(TRUE, 0);
    GtkWidget *lock = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "lock_unlocked",
                                              _("When locked, change both width and height by the same proportion"),
                                              tt);
    gtk_box_pack_start(GTK_BOX(lockbox), lock, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vb), lockbox, FALSE, FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(spw), "lock", lock);
    g_signal_connect_after (G_OBJECT (lock), "clicked", G_CALLBACK (toggle_lock), desktop);

    gtk_container_add(GTK_CONTAINER(vb),
                      //TRANSLATORS: only translate "string" in "context|string".
                      // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                      sp_select_toolbox_spinbutton(_("select_toolbar|H"), "height", 1e-3, tracker, spw, _("Height of selection"), FALSE));

    aux_toolbox_space(vb, 2);

    // Add the units menu.
    {
        GtkAction* act = tracker->createAction( "UnitAction", _("Units"), _("") );

        GtkWidget* normal = gtk_action_create_tool_item( act );
        gtk_widget_show( normal );
        gtk_container_add( GTK_CONTAINER(vb), normal );
    }

    // Set font size.
    sp_set_font_size_smaller (vb);

    // Force update when selection changes.
    gtk_signal_connect(GTK_OBJECT(spw), "modify_selection", GTK_SIGNAL_FUNC(sp_selection_layout_widget_modify_selection), desktop);
    gtk_signal_connect(GTK_OBJECT(spw), "change_selection", GTK_SIGNAL_FUNC(sp_selection_layout_widget_change_selection), desktop);

    // Update now.
    sp_selection_layout_widget_update(SP_WIDGET(spw), SP_ACTIVE_DESKTOP ? sp_desktop_selection(SP_ACTIVE_DESKTOP) : NULL);

    // Insert spw into the toolbar.
    gtk_box_pack_start(GTK_BOX(tb), spw, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    aux_toolbox_space(tb, AUX_BETWEEN_BUTTON_GROUPS);

    // "Transform with object" buttons

    GtkWidget *cvbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *cbox = gtk_hbox_new (FALSE, 0);

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "transform_stroke",
                                              _("When scaling objects, scale the stroke width by the same proportion"),
                                              tt);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("options.transform", "stroke", 1));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (toggle_stroke), desktop);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "transform_corners",
                                              _("When scaling rectangles, scale the radii of rounded corners"),
                                              tt);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("options.transform", "rectcorners", 1));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (toggle_corners), desktop);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "transform_gradient",
                                              _("Transform gradients (in fill or stroke) along with the objects"),
                                              tt);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("options.transform", "gradient", 1));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (toggle_gradient), desktop);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    {
    GtkWidget *button = sp_button_new_from_data( Inkscape::ICON_SIZE_DECORATION,
                                              SP_BUTTON_TYPE_TOGGLE,
                                              NULL,
                                              "transform_pattern",
                                              _("Transform patterns (in fill or stroke) along with the objects"),
                                              tt);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), prefs_get_int_attribute ("options.transform", "pattern", 1));
    g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (toggle_pattern), desktop);
    gtk_box_pack_start(GTK_BOX(cbox), button, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(cvbox), cbox, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tb), cvbox, FALSE, FALSE, 0);

    gtk_widget_show_all(tb);

    return tb;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
