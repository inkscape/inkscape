/**
 * @file
 * Calligraphy aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ui/dialog/calligraphic-profile-rename.h"
#include <glibmm/i18n.h>
#include "calligraphy-toolbar.h"

#include "desktop.h"
#include "document-undo.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ege-select-one-action.h"
#include "widgets/ink-action.h"
#include "preferences.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/uxmanager.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;


//########################
//##     Calligraphy    ##
//########################

std::vector<Glib::ustring> get_presets_list() {

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    std::vector<Glib::ustring> presets = prefs->getAllDirs("/tools/calligraphic/preset");

    return presets;
}

void update_presets_list(GObject *tbl)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (g_object_get_data(tbl, "presets_blocked")) {
        return;
    }

    EgeSelectOneAction *sel = static_cast<EgeSelectOneAction *>(g_object_get_data(tbl, "profile_selector"));
    if (!sel) {
        // WTF!? This will cause a segfault if ever reached
        //ege_select_one_action_set_active(sel, 0);
        return;
    }

    std::vector<Glib::ustring> presets = get_presets_list();

    int ege_index = 1;
    for (std::vector<Glib::ustring>::iterator i = presets.begin(); i != presets.end(); ++i, ++ege_index) {
        bool match = true;

        std::vector<Inkscape::Preferences::Entry> preset = prefs->getAllEntries(*i);
        for (std::vector<Inkscape::Preferences::Entry>::iterator j = preset.begin(); j != preset.end(); ++j) {
            Glib::ustring entry_name = j->getEntryName();
            if (entry_name == "id" || entry_name == "name") {
                continue;
            }

            void *widget = g_object_get_data(tbl, entry_name.data());
            if (widget) {
                if (GTK_IS_ADJUSTMENT(widget)) {
                    double v = j->getDouble();
                    GtkAdjustment* adj = static_cast<GtkAdjustment *>(widget);
                    //std::cout << "compared adj " << attr_name << gtk_adjustment_get_value(adj) << " to " << v << "\n";
                    if (fabs(gtk_adjustment_get_value(adj) - v) > 1e-6) {
                        match = false;
                        break;
                    }
                } else if (GTK_IS_TOGGLE_ACTION(widget)) {
                    bool v = j->getBool();
                    GtkToggleAction* toggle = static_cast<GtkToggleAction *>(widget);
                    //std::cout << "compared toggle " << attr_name << gtk_toggle_action_get_active(toggle) << " to " << v << "\n";
                    if ( static_cast<bool>(gtk_toggle_action_get_active(toggle)) != v ) {
                        match = false;
                        break;
                    }
                }
            }
        }

        if (match) {
            // newly added item is at the same index as the
            // save command, so we need to change twice for it to take effect
            ege_select_one_action_set_active(sel, 0);
            ege_select_one_action_set_active(sel, ege_index); // one-based index
            return;
        }
    }

    // no match found
    ege_select_one_action_set_active(sel, 0);
}

static void sp_ddc_mass_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/mass", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_wiggle_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/wiggle", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_angle_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/angle", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_width_value_changed( GtkAdjustment *adj, GObject *tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/width", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_velthin_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/tools/calligraphic/thinning", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_flatness_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/flatness", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_tremor_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/tremor", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_cap_rounding_value_changed( GtkAdjustment *adj, GObject* tbl )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/cap_rounding", gtk_adjustment_get_value(adj) );
    update_presets_list(tbl);
}

static void sp_ddc_tilt_state_changed( GtkToggleAction *act, GObject*  tbl )
{
    // TODO merge into PrefPusher
    GtkAction * calligraphy_angle = static_cast<GtkAction *> (g_object_get_data(tbl,"angle_action"));
    if (calligraphy_angle ) {
        gtk_action_set_sensitive( calligraphy_angle, !gtk_toggle_action_get_active( act ) );
    }
}


static gchar const *const widget_names[] = {
    "width",
    "mass",
    "wiggle",
    "angle",
    "thinning",
    "tremor",
    "flatness",
    "cap_rounding",
    "usepressure",
    "tracebackground",
    "usetilt"
};


static void sp_dcc_build_presets_list(GObject *tbl)
{
    g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(TRUE));

    EgeSelectOneAction* selector = static_cast<EgeSelectOneAction *>(g_object_get_data(tbl, "profile_selector"));
    GtkListStore* model = GTK_LIST_STORE(ege_select_one_action_get_model(selector));
    gtk_list_store_clear (model);

    {
        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter, 0, _("No preset"), 1, 0, -1 );
    }

    // iterate over all presets to populate the list
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    std::vector<Glib::ustring> presets = get_presets_list();
    int ii=1;

    for (std::vector<Glib::ustring>::iterator i = presets.begin(); i != presets.end(); ++i) {
        GtkTreeIter iter;
        Glib::ustring preset_name = prefs->getString(*i + "/name");
        if (!preset_name.empty()) {
            gtk_list_store_append( model, &iter );
            gtk_list_store_set( model, &iter, 0, _(preset_name.data()), 1, ii++, -1 );
        }
    }

/*    {
        GtkTreeIter iter;
        gtk_list_store_append( model, &iter );
        gtk_list_store_set( model, &iter, 0, _("Save..."), 1, ii, -1 );
        g_object_set_data(tbl, "save_presets_index", GINT_TO_POINTER(ii));
    }*/

    g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(FALSE));

    update_presets_list (tbl);
}

static void sp_dcc_save_profile(GtkWidget * /*widget*/, GObject *tbl)
{
    using Inkscape::UI::Dialog::CalligraphicProfileRename;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data(tbl, "desktop" ));
    if (! desktop) {
        return;
    }

    if (g_object_get_data(tbl, "presets_blocked")) {
        return;
    }

    EgeSelectOneAction *sel = static_cast<EgeSelectOneAction *>(g_object_get_data(tbl, "profile_selector"));
    //gint preset_index = ege_select_one_action_get_active( sel );
    Glib::ustring current_profile_name = _("No preset");
    if (ege_select_one_action_get_active_text( sel )) {
        current_profile_name = ege_select_one_action_get_active_text( sel );
    }

    if (current_profile_name == _("No preset")) {
        current_profile_name = "";
    }
    CalligraphicProfileRename::show(desktop, current_profile_name);
    if ( !CalligraphicProfileRename::applied()) {
        // dialog cancelled
        update_presets_list (tbl);
        return;
    }
    Glib::ustring new_profile_name = CalligraphicProfileRename::getProfileName();

    if (new_profile_name.empty()) {
        // empty name entered
        update_presets_list (tbl);
        return;
    }

    g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(TRUE));

    // If there's a preset with the given name, find it and set save_path appropriately
    std::vector<Glib::ustring> presets = get_presets_list();
    int total_presets = presets.size();
    int new_index = -1;
    Glib::ustring save_path; // profile pref path without a trailing slash

    int temp_index = 0;
    for (std::vector<Glib::ustring>::iterator i = presets.begin(); i != presets.end(); ++i, ++temp_index) {
        Glib::ustring name = prefs->getString(*i + "/name");
        if (!name.empty() && (new_profile_name == name || current_profile_name == name)) {
            new_index = temp_index;
            save_path = *i;
            break;
        }
    }


    if ( CalligraphicProfileRename::deleted() && new_index != -1) {
        prefs->remove(save_path);
        g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(FALSE));
        sp_dcc_build_presets_list (tbl);
        return;
    }

    if (new_index == -1) {
        // no preset with this name, create
        new_index = total_presets + 1;
        gchar *profile_id = g_strdup_printf("/dcc%d", new_index);
        save_path = Glib::ustring("/tools/calligraphic/preset") + profile_id;
        g_free(profile_id);
    }

    for (unsigned i = 0; i < G_N_ELEMENTS(widget_names); ++i) {
        gchar const *const widget_name = widget_names[i];
        void *widget = g_object_get_data(tbl, widget_name);
        if (widget) {
            if (GTK_IS_ADJUSTMENT(widget)) {
                GtkAdjustment* adj = static_cast<GtkAdjustment *>(widget);
                prefs->setDouble(save_path + "/" + widget_name, gtk_adjustment_get_value(adj));
                //std::cout << "wrote adj " << widget_name << ": " << v << "\n";
            } else if (GTK_IS_TOGGLE_ACTION(widget)) {
                GtkToggleAction* toggle = static_cast<GtkToggleAction *>(widget);
                prefs->setBool(save_path + "/" + widget_name, gtk_toggle_action_get_active(toggle));
                //std::cout << "wrote tog " << widget_name << ": " << v << "\n";
            } else {
                g_warning("Unknown widget type for preset: %s\n", widget_name);
            }
        } else {
            g_warning("Bad key when writing preset: %s\n", widget_name);
        }
    }
    prefs->setString(save_path + "/name", new_profile_name);

    g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(FALSE));
    sp_dcc_build_presets_list (tbl);
}


static void sp_ddc_change_profile(EgeSelectOneAction* act, GObject* tbl)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    guint preset_index = ege_select_one_action_get_active( act );
    // This is necessary because EgeSelectOneAction spams us with GObject "changed" signal calls
    // even when the preset is not changed. It would be good to replace it with something more
    // modern. Index 0 means "No preset", so we don't do anything.
    if (preset_index == 0) {
        return;
    }

/*
    gint save_presets_index = GPOINTER_TO_INT(g_object_get_data(tbl, "save_presets_index"));

    if (preset_index == save_presets_index) {
        // this is the Save command
        sp_dcc_save_profile(NULL, tbl);
        return;
    }
*/

    if (g_object_get_data(tbl, "presets_blocked")) {
        return;
    }

    // preset_index is one-based so we subtract 1
    std::vector<Glib::ustring> presets = get_presets_list();

    Glib::ustring preset_path = "";
    if (preset_index - 1 < presets.size()) {
        preset_path = presets.at(preset_index - 1);
    }

    if (!preset_path.empty()) {
        g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(TRUE)); //temporarily block the selector so no one will updadte it while we're reading it

        std::vector<Inkscape::Preferences::Entry> preset = prefs->getAllEntries(preset_path);

        // Shouldn't this be std::map?
        for (std::vector<Inkscape::Preferences::Entry>::iterator i = preset.begin(); i != preset.end(); ++i) {
            Glib::ustring entry_name = i->getEntryName();
            if (entry_name == "id" || entry_name == "name") {
                continue;
            }
            void *widget = g_object_get_data(tbl, entry_name.data());
            if (widget) {
                if (GTK_IS_ADJUSTMENT(widget)) {
                    GtkAdjustment* adj = static_cast<GtkAdjustment *>(widget);
                    gtk_adjustment_set_value(adj, i->getDouble());
                    //std::cout << "set adj " << attr_name << " to " << v << "\n";
                } else if (GTK_IS_TOGGLE_ACTION(widget)) {
                    GtkToggleAction* toggle = static_cast<GtkToggleAction *>(widget);
                    gtk_toggle_action_set_active(toggle, i->getBool());
                    //std::cout << "set toggle " << attr_name << " to " << v << "\n";
                } else {
                    g_warning("Unknown widget type for preset: %s\n", entry_name.data());
                }
            } else {
                g_warning("Bad key found in a preset record: %s\n", entry_name.data());
            }
        }
        g_object_set_data(tbl, "presets_blocked", GINT_TO_POINTER(FALSE));
    } else {
        ege_select_one_action_set_active(act, 0);
    }
}

static void sp_ddc_edit_profile(GtkAction * /*act*/, GObject* tbl)
{
    sp_dcc_save_profile(NULL, tbl);
}

void sp_calligraphy_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    {
        g_object_set_data(holder, "presets_blocked", GINT_TO_POINTER(TRUE));

        EgeAdjustmentAction* calligraphy_angle = 0;

        {
        /* Width */
        gchar const* labels[] = {_("(hairline)"), 0, 0, 0, _("(default)"), 0, 0, 0, 0, _("(broad stroke)")};
        gdouble values[] = {1, 3, 5, 10, 15, 20, 30, 50, 75, 100};
        EgeAdjustmentAction *eact = create_adjustment_action( "CalligraphyWidthAction",
                                                              _("Pen Width"), _("Width:"),
                                                              _("The width of the calligraphic pen (relative to the visible canvas area)"),
                                                              "/tools/calligraphic/width", 15,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-calligraphy",
                                                              1, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_width_value_changed, NULL /*unit tracker*/, 1, 0 );
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Thinning */
            gchar const* labels[] = {_("(speed blows up stroke)"), 0, 0, _("(slight widening)"), _("(constant width)"), _("(slight thinning, default)"), 0, 0, _("(speed deflates stroke)")};
            gdouble values[] = {-100, -40, -20, -10, 0, 10, 20, 40, 100};
        EgeAdjustmentAction* eact = create_adjustment_action( "ThinningAction",
                                                              _("Stroke Thinning"), _("Thinning:"),
                                                              _("How much velocity thins the stroke (> 0 makes fast strokes thinner, < 0 makes them broader, 0 makes width independent of velocity)"),
                                                              "/tools/calligraphic/thinning", 10,
                                                              GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                                              -100, 100, 1, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_velthin_value_changed, NULL /*unit tracker*/, 1, 0);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Angle */
        gchar const* labels[] = {_("(left edge up)"), 0, 0, _("(horizontal)"), _("(default)"), 0, _("(right edge up)")};
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        EgeAdjustmentAction* eact = create_adjustment_action( "AngleAction",
                                                              _("Pen Angle"), _("Angle:"),
                                                              _("The angle of the pen's nib (in degrees; 0 = horizontal; has no effect if fixation = 0)"),
                                                              "/tools/calligraphic/angle", 30,
                                                              GTK_WIDGET(desktop->canvas), holder, TRUE, "calligraphy-angle",
                                                              -90.0, 90.0, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_angle_value_changed, NULL /*unit tracker*/, 1, 0 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "angle_action", eact );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        calligraphy_angle = eact;
        }

        {
        /* Fixation */
            gchar const* labels[] = {_("(perpendicular to stroke, \"brush\")"), 0, 0, 0, _("(almost fixed, default)"), _("(fixed by Angle, \"pen\")")};
        gdouble values[] = {0, 20, 40, 60, 90, 100};
        EgeAdjustmentAction* eact = create_adjustment_action( "FixationAction",
                                                              _("Fixation"), _("Fixation:"),
                                                              _("Angle behavior (0 = nib always perpendicular to stroke direction, 100 = fixed angle)"),
                                                              "/tools/calligraphic/flatness", 90,
                                                              GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                                              0.0, 100, 1.0, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_flatness_value_changed, NULL /*unit tracker*/, 1, 0);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Cap Rounding */
            gchar const* labels[] = {_("(blunt caps, default)"), _("(slightly bulging)"), 0, 0, _("(approximately round)"), _("(long protruding caps)")};
        gdouble values[] = {0, 0.3, 0.5, 1.0, 1.4, 5.0};
        // TRANSLATORS: "cap" means "end" (both start and finish) here
        EgeAdjustmentAction* eact = create_adjustment_action( "CapRoundingAction",
                                                              _("Cap rounding"), _("Caps:"),
                                                              _("Increase to make caps at the ends of strokes protrude more (0 = no caps, 1 = round caps)"),
                                                              "/tools/calligraphic/cap_rounding", 0.0,
                                                              GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                                              0.0, 5.0, 0.01, 0.1,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_cap_rounding_value_changed, NULL /*unit tracker*/, 0.01, 2 );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Tremor */
            gchar const* labels[] = {_("(smooth line)"), _("(slight tremor)"), _("(noticeable tremor)"), 0, 0, _("(maximum tremor)")};
        gdouble values[] = {0, 10, 20, 40, 60, 100};
        EgeAdjustmentAction* eact = create_adjustment_action( "TremorAction",
                                                              _("Stroke Tremor"), _("Tremor:"),
                                                              _("Increase to make strokes rugged and trembling"),
                                                              "/tools/calligraphic/tremor", 0.0,
                                                              GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                                              0.0, 100, 1, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_tremor_value_changed, NULL /*unit tracker*/, 1, 0);

        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Wiggle */
        gchar const* labels[] = {_("(no wiggle)"), _("(slight deviation)"), 0, 0, _("(wild waves and curls)")};
        gdouble values[] = {0, 20, 40, 60, 100};
        EgeAdjustmentAction* eact = create_adjustment_action( "WiggleAction",
                                                              _("Pen Wiggle"), _("Wiggle:"),
                                                              _("Increase to make the pen waver and wiggle"),
                                                              "/tools/calligraphic/wiggle", 0.0,
                                                              GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                                              0.0, 100, 1, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_wiggle_value_changed, NULL /*unit tracker*/, 1, 0);
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }

        {
        /* Mass */
            gchar const* labels[] = {_("(no inertia)"), _("(slight smoothing, default)"), _("(noticeable lagging)"), 0, 0, _("(maximum inertia)")};
        gdouble values[] = {0.0, 2, 10, 20, 50, 100};
        EgeAdjustmentAction* eact = create_adjustment_action( "MassAction",
                                                              _("Pen Mass"), _("Mass:"),
                                                              _("Increase to make the pen drag behind, as if slowed by inertia"),
                                                              "/tools/calligraphic/mass", 2.0,
                                                              GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                                              0.0, 100, 1, 10.0,
                                                              labels, values, G_N_ELEMENTS(labels),
                                                              sp_ddc_mass_value_changed, NULL /*unit tracker*/, 1, 0);
        ege_adjustment_action_set_appearance( eact, TOOLBAR_SLIDER_HINT );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
        }


        /* Trace Background button */
        {
            InkToggleAction* act = ink_toggle_action_new( "TraceAction",
                                                          _("Trace Background"),
                                                          _("Trace the lightness of the background by the width of the pen (white - minimum width, black - maximum width)"),
                                                          INKSCAPE_ICON("draw-trace-background"),
                                                          Inkscape::ICON_SIZE_DECORATION );
            gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
            PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/calligraphic/tracebackground", update_presets_list, holder);
            g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
            g_object_set_data( holder, "tracebackground", act );
        }

        /* Use Pressure button */
        {
            InkToggleAction* act = ink_toggle_action_new( "PressureAction",
                                                          _("Pressure"),
                                                          _("Use the pressure of the input device to alter the width of the pen"),
                                                          INKSCAPE_ICON("draw-use-pressure"),
                                                          Inkscape::ICON_SIZE_DECORATION );
            gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
            PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/calligraphic/usepressure", update_presets_list, holder);
            g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
            g_object_set_data( holder, "usepressure", act );
        }

        /* Use Tilt button */
        {
            InkToggleAction* act = ink_toggle_action_new( "TiltAction",
                                                          _("Tilt"),
                                                          _("Use the tilt of the input device to alter the angle of the pen's nib"),
                                                          INKSCAPE_ICON("draw-use-tilt"),
                                                          Inkscape::ICON_SIZE_DECORATION );
            gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
            PrefPusher *pusher = new PrefPusher(GTK_TOGGLE_ACTION(act), "/tools/calligraphic/usetilt", update_presets_list, holder);
            g_signal_connect( holder, "destroy", G_CALLBACK(delete_prefspusher), pusher);
            g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(sp_ddc_tilt_state_changed), holder );
            gtk_action_set_sensitive( GTK_ACTION(calligraphy_angle), !prefs->getBool("/tools/calligraphic/usetilt", true) );
            gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/calligraphic/usetilt", true) );
            g_object_set_data( holder, "usetilt", act );
        }

        /*calligraphic profile */
        {
            GtkListStore* model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
            EgeSelectOneAction* act1 = ege_select_one_action_new ("SetProfileAction", "" , (_("Choose a preset")), NULL, GTK_TREE_MODEL(model));
            ege_select_one_action_set_appearance (act1, "compact");
            g_object_set_data (holder, "profile_selector", act1 );

            g_object_set_data(holder, "presets_blocked", GINT_TO_POINTER(FALSE));

            sp_dcc_build_presets_list (holder);

            g_signal_connect(G_OBJECT(act1), "changed", G_CALLBACK(sp_ddc_change_profile), holder);
            gtk_action_group_add_action(mainActions, GTK_ACTION(act1));
        }

        /*calligraphic profile editor */
        {
            InkAction* inky = ink_action_new( "ProfileEditAction",
                                              _("Add/Edit Profile"),
                                              _("Add or edit calligraphic profile"),
                                              INKSCAPE_ICON("document-properties"),
                                              Inkscape::ICON_SIZE_DECORATION );
            g_object_set( inky, "short_label", _("Edit"), NULL );
            g_signal_connect_after( G_OBJECT(inky), "activate", G_CALLBACK(sp_ddc_edit_profile), (GObject*)holder );
            gtk_action_group_add_action( mainActions, GTK_ACTION(inky) );
        }
    }
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
