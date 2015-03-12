/**
 * @file
 * 3d box aux toolbar
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

#include <glibmm/i18n.h>

#include "box3d-toolbar.h"
#include "box3d.h"

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "widgets/ege-adjustment-action.h"
#include "widgets/ink-action.h"
#include "inkscape.h"
#include "persp3d.h"
#include "selection.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/tools/box3d-tool.h"
#include "ui/uxmanager.h"
#include "verbs.h"
#include "xml/node-event-vector.h"

using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//########################
//##       3D Box       ##
//########################

// normalize angle so that it lies in the interval [0,360]
static double box3d_normalize_angle (double a) {
    double angle = a + ((int) (a/360.0))*360;
    if (angle < 0) {
        angle += 360.0;
    }
    return angle;
}

static void box3d_set_button_and_adjustment(Persp3D *persp,
                                            Proj::Axis axis,
                                            GtkAdjustment *adj,
                                            GtkAction *act,
                                            GtkToggleAction *tact)
{
    // TODO: Take all selected perspectives into account but don't touch the state button if not all of them
    //       have the same state (otherwise a call to box3d_vp_z_state_changed() is triggered and the states
    //       are reset).
    bool is_infinite = !persp3d_VP_is_finite(persp->perspective_impl, axis);

    if (is_infinite) {
        gtk_toggle_action_set_active(tact, TRUE);
        gtk_action_set_sensitive(act, TRUE);

        double angle = persp3d_get_infinite_angle(persp, axis);
        if (angle != Geom::infinity()) { // FIXME: We should catch this error earlier (don't show the spinbutton at all)
            gtk_adjustment_set_value(adj, box3d_normalize_angle(angle));
        }
    } else {
        gtk_toggle_action_set_active(tact, FALSE);
        gtk_action_set_sensitive(act, FALSE);
    }
}

static void box3d_resync_toolbar(Inkscape::XML::Node *persp_repr, GObject *data)
{
    if (!persp_repr) {
        g_print ("No perspective given to box3d_resync_toolbar().\n");
        return;
    }

    GtkWidget *tbl = GTK_WIDGET(data);
    GtkAdjustment *adj = 0;
    GtkAction *act = 0;
    GtkToggleAction *tact = 0;
    Persp3D *persp = persp3d_get_from_repr(persp_repr);
    if (!persp) {
        // Hmm, is it an error if this happens?
        return;
    }
    {
        adj = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(tbl), "box3d_angle_x"));
        act = GTK_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_angle_x_action"));
        tact = &INK_TOGGLE_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_vp_x_state_action"))->action;

        box3d_set_button_and_adjustment(persp, Proj::X, adj, act, tact);
    }
    {
        adj = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(tbl), "box3d_angle_y"));
        act = GTK_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_angle_y_action"));
        tact = &INK_TOGGLE_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_vp_y_state_action"))->action;

        box3d_set_button_and_adjustment(persp, Proj::Y, adj, act, tact);
    }
    {
        adj = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(tbl), "box3d_angle_z"));
        act = GTK_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_angle_z_action"));
        tact = &INK_TOGGLE_ACTION(g_object_get_data(G_OBJECT(tbl), "box3d_vp_z_state_action"))->action;

        box3d_set_button_and_adjustment(persp, Proj::Z, adj, act, tact);
    }
}

static void box3d_persp_tb_event_attr_changed(Inkscape::XML::Node *repr,
                                              gchar const * /*name*/,
                                              gchar const * /*old_value*/,
                                              gchar const * /*new_value*/,
                                              bool /*is_interactive*/,
                                              gpointer data)
{
    GtkWidget *tbl = GTK_WIDGET(data);

    // quit if run by the attr_changed or selection changed listener
    if (g_object_get_data(G_OBJECT(tbl), "freeze")) {
        return;
    }

    // set freeze so that it can be caught in box3d_angle_z_value_changed() (to avoid calling
    // SPDocumentUndo::maybeDone() when the document is undo insensitive)
    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(TRUE));

    // TODO: Only update the appropriate part of the toolbar
//    if (!strcmp(name, "inkscape:vp_z")) {
        box3d_resync_toolbar(repr, G_OBJECT(tbl));
//    }

    Persp3D *persp = persp3d_get_from_repr(repr);
    persp3d_update_box_reprs(persp);

    g_object_set_data(G_OBJECT(tbl), "freeze", GINT_TO_POINTER(FALSE));
}

static Inkscape::XML::NodeEventVector box3d_persp_tb_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    box3d_persp_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
 *  \param selection Should not be NULL.
 */
// FIXME: This should rather be put into persp3d-reference.cpp or something similar so that it reacts upon each
//        Change of the perspective, and not of the current selection (but how to refer to the toolbar then?)
static void box3d_toolbox_selection_changed(Inkscape::Selection *selection, GObject *tbl)
{
    // Here the following should be done: If all selected boxes have finite VPs in a certain direction,
    // disable the angle entry fields for this direction (otherwise entering a value in them should only
    // update the perspectives with infinite VPs and leave the other ones untouched).

    Inkscape::XML::Node *persp_repr = NULL;
    purge_repr_listener(tbl, tbl);

    SPItem *item = selection->singleItem();
    SPBox3D *box = dynamic_cast<SPBox3D *>(item);
    if (box) {
        // FIXME: Also deal with multiple selected boxes
        Persp3D *persp = box3d_get_perspective(box);
        persp_repr = persp->getRepr();
        if (persp_repr) {
            g_object_set_data(tbl, "repr", persp_repr);
            Inkscape::GC::anchor(persp_repr);
            sp_repr_add_listener(persp_repr, &box3d_persp_tb_repr_events, tbl);
            sp_repr_synthesize_events(persp_repr, &box3d_persp_tb_repr_events, tbl);

            SP_ACTIVE_DOCUMENT->setCurrentPersp3D(persp3d_get_from_repr(persp_repr));
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setString("/tools/shapes/3dbox/persp", persp_repr->attribute("id"));

            g_object_set_data(tbl, "freeze", GINT_TO_POINTER(TRUE));
            box3d_resync_toolbar(persp_repr, tbl);
            g_object_set_data(tbl, "freeze", GINT_TO_POINTER(FALSE));
        }
    }
}

static void box3d_angle_value_changed(GtkAdjustment *adj, GObject *dataKludge, Proj::Axis axis)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(g_object_get_data( dataKludge, "desktop" ));
    SPDocument *document = desktop->getDocument();

    // quit if run by the attr_changed or selection changed listener
    if (g_object_get_data( dataKludge, "freeze" )) {
        return;
    }

    // in turn, prevent listener from responding
    g_object_set_data(dataKludge, "freeze", GINT_TO_POINTER(TRUE));

    std::list<Persp3D *> sel_persps = desktop->getSelection()->perspList();
    if (sel_persps.empty()) {
        // this can happen when the document is created; we silently ignore it
        return;
    }
    Persp3D *persp = sel_persps.front();

    persp->perspective_impl->tmat.set_infinite_direction (axis,
            gtk_adjustment_get_value(adj));
    persp->updateRepr();

    // TODO: use the correct axis here, too
    DocumentUndo::maybeDone(document, "perspangle", SP_VERB_CONTEXT_3DBOX, _("3D Box: Change perspective (angle of infinite axis)"));

    g_object_set_data( dataKludge, "freeze", GINT_TO_POINTER(FALSE) );
}


static void box3d_angle_x_value_changed(GtkAdjustment *adj, GObject *dataKludge)
{
    box3d_angle_value_changed(adj, dataKludge, Proj::X);
}

static void box3d_angle_y_value_changed(GtkAdjustment *adj, GObject *dataKludge)
{
    box3d_angle_value_changed(adj, dataKludge, Proj::Y);
}

static void box3d_angle_z_value_changed(GtkAdjustment *adj, GObject *dataKludge)
{
    box3d_angle_value_changed(adj, dataKludge, Proj::Z);
}


static void box3d_vp_state_changed( GtkToggleAction *act, GtkAction * /*box3d_angle*/, Proj::Axis axis )
{
    // TODO: Take all selected perspectives into account
    std::list<Persp3D *> sel_persps = SP_ACTIVE_DESKTOP->getSelection()->perspList();
    if (sel_persps.empty()) {
        // this can happen when the document is created; we silently ignore it
        return;
    }
    Persp3D *persp = sel_persps.front();

    bool set_infinite = gtk_toggle_action_get_active(act);
    persp3d_set_VP_state (persp, axis, set_infinite ? Proj::VP_INFINITE : Proj::VP_FINITE);
}

static void box3d_vp_x_state_changed( GtkToggleAction *act, GtkAction *box3d_angle )
{
    box3d_vp_state_changed(act, box3d_angle, Proj::X);
}

static void box3d_vp_y_state_changed( GtkToggleAction *act, GtkAction *box3d_angle )
{
    box3d_vp_state_changed(act, box3d_angle, Proj::Y);
}

static void box3d_vp_z_state_changed( GtkToggleAction *act, GtkAction *box3d_angle )
{
    box3d_vp_state_changed(act, box3d_angle, Proj::Z);
}

static void box3d_toolbox_check_ec(SPDesktop* dt, Inkscape::UI::Tools::ToolBase* ec, GObject* holder);

void box3d_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    EgeAdjustmentAction* eact = 0;
    SPDocument *document = desktop->getDocument();
    Persp3DImpl *persp_impl = document->getCurrentPersp3DImpl();

    EgeAdjustmentAction* box3d_angle_x = 0;
    EgeAdjustmentAction* box3d_angle_y = 0;
    EgeAdjustmentAction* box3d_angle_z = 0;

    /* Angle X */
    {
        gchar const* labels[] = { 0, 0, 0, 0, 0, 0, 0 };
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        eact = create_adjustment_action( "3DBoxAngleXAction",
                                         _("Angle in X direction"), _("Angle X:"),
                                         // Translators: PL is short for 'perspective line'
                                         _("Angle of PLs in X direction"),
                                         "/tools/shapes/3dbox/box3d_angle_x", 30,
                                         GTK_WIDGET(desktop->canvas), holder, TRUE, "altx-box3d",
                                         -360.0, 360.0, 1.0, 10.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         box3d_angle_x_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "box3d_angle_x_action", eact );
        box3d_angle_x = eact;
    }

    if (!persp_impl || !persp3d_VP_is_finite(persp_impl, Proj::X)) {
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    } else {
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
    }


    /* VP X state */
    {
        InkToggleAction* act = ink_toggle_action_new( "3DBoxVPXStateAction",
                                                      // Translators: VP is short for 'vanishing point'
                                                      _("State of VP in X direction"),
                                                      _("Toggle VP in X direction between 'finite' and 'infinite' (=parallel)"),
                                                      INKSCAPE_ICON("perspective-parallel"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "box3d_vp_x_state_action", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(box3d_vp_x_state_changed), box3d_angle_x );
        gtk_action_set_sensitive( GTK_ACTION(box3d_angle_x), !prefs->getBool("/tools/shapes/3dbox/vp_x_state", true) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/shapes/3dbox/vp_x_state", true) );
    }

    /* Angle Y */
    {
        gchar const* labels[] = { 0, 0, 0, 0, 0, 0, 0 };
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        eact = create_adjustment_action( "3DBoxAngleYAction",
                                         _("Angle in Y direction"), _("Angle Y:"),
                                         // Translators: PL is short for 'perspective line'
                                         _("Angle of PLs in Y direction"),
                                         "/tools/shapes/3dbox/box3d_angle_y", 30,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         box3d_angle_y_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "box3d_angle_y_action", eact );
        box3d_angle_y = eact;
    }

    if (!persp_impl || !persp3d_VP_is_finite(persp_impl, Proj::Y)) {
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    } else {
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
    }

    /* VP Y state */
    {
        InkToggleAction* act = ink_toggle_action_new( "3DBoxVPYStateAction",
                                                      // Translators: VP is short for 'vanishing point'
                                                      _("State of VP in Y direction"),
                                                      _("Toggle VP in Y direction between 'finite' and 'infinite' (=parallel)"),
                                                      INKSCAPE_ICON("perspective-parallel"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "box3d_vp_y_state_action", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(box3d_vp_y_state_changed), box3d_angle_y );
        gtk_action_set_sensitive( GTK_ACTION(box3d_angle_y), !prefs->getBool("/tools/shapes/3dbox/vp_y_state", true) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/shapes/3dbox/vp_y_state", true) );
    }

    /* Angle Z */
    {
        gchar const* labels[] = { 0, 0, 0, 0, 0, 0, 0 };
        gdouble values[] = {-90, -60, -30, 0, 30, 60, 90};
        eact = create_adjustment_action( "3DBoxAngleZAction",
                                         _("Angle in Z direction"), _("Angle Z:"),
                                         // Translators: PL is short for 'perspective line'
                                         _("Angle of PLs in Z direction"),
                                         "/tools/shapes/3dbox/box3d_angle_z", 30,
                                         GTK_WIDGET(desktop->canvas), holder, FALSE, NULL,
                                         -360.0, 360.0, 1.0, 10.0,
                                         labels, values, G_N_ELEMENTS(labels),
                                         box3d_angle_z_value_changed );
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
        g_object_set_data( holder, "box3d_angle_z_action", eact );
        box3d_angle_z = eact;
    }

    if (!persp_impl || !persp3d_VP_is_finite(persp_impl, Proj::Z)) {
        gtk_action_set_sensitive( GTK_ACTION(eact), TRUE );
    } else {
        gtk_action_set_sensitive( GTK_ACTION(eact), FALSE );
    }

    /* VP Z state */
    {
        InkToggleAction* act = ink_toggle_action_new( "3DBoxVPZStateAction",
                                                      // Translators: VP is short for 'vanishing point'
                                                      _("State of VP in Z direction"),
                                                      _("Toggle VP in Z direction between 'finite' and 'infinite' (=parallel)"),
                                                      INKSCAPE_ICON("perspective-parallel"),
                                                      Inkscape::ICON_SIZE_DECORATION );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
        g_object_set_data( holder, "box3d_vp_z_state_action", act );
        g_signal_connect_after( G_OBJECT(act), "toggled", G_CALLBACK(box3d_vp_z_state_changed), box3d_angle_z );
        gtk_action_set_sensitive( GTK_ACTION(box3d_angle_z), !prefs->getBool("/tools/shapes/3dbox/vp_z_state", true) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(act), prefs->getBool("/tools/shapes/3dbox/vp_z_state", true) );
    }

    desktop->connectEventContextChanged(sigc::bind(sigc::ptr_fun(box3d_toolbox_check_ec), holder));
    g_signal_connect(holder, "destroy", G_CALLBACK(purge_repr_listener), holder);
}

static void box3d_toolbox_check_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* ec, GObject* holder)
{
    static sigc::connection changed;
    if (SP_IS_BOX3D_CONTEXT(ec)) {
        changed = desktop->getSelection()->connectChanged(sigc::bind(sigc::ptr_fun(box3d_toolbox_selection_changed), holder));
        box3d_toolbox_selection_changed(desktop->getSelection(), holder);
    } else {
        if (changed)
            changed.disconnect();
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
