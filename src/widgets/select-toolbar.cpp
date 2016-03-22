/*
 * Selector aux toolbar
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2003-2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <2geom/rect.h>

#include "ui/widget/spinbutton.h"
#include <glibmm/i18n.h>
#include "select-toolbar.h"

#include "desktop.h"
#include "display/sp-canvas.h"
#include "document-undo.h"
#include "document.h"
#include "widgets/ege-adjustment-action.h"
#include "helper/action-context.h"
#include "helper/action.h"
#include "widgets/ink-action.h"
#include "inkscape.h"
#include "message-stack.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "sp-item-transform.h"
#include "sp-namedview.h"
#include "toolbox.h"
#include "ui/icon-names.h"
#include "ui/widget/unit-tracker.h"
#include "util/units.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "widgets/sp-widget.h"
#include "widgets/spw-utilities.h"
#include "widgets/widget-sizes.h"

using Inkscape::UI::Widget::UnitTracker;
using Inkscape::Util::Unit;
using Inkscape::Util::Quantity;
using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

static void
sp_selection_layout_widget_update(SPWidget *spw, Inkscape::Selection *sel)
{
    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    using Geom::X;
    using Geom::Y;
    if ( sel && !sel->isEmpty() ) {
        int prefs_bbox = prefs->getInt("/tools/bounding_box", 0);
        SPItem::BBoxType bbox_type = (prefs_bbox ==0)?
            SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
        Geom::OptRect const bbox(sel->bounds(bbox_type));
        if ( bbox ) {
            UnitTracker *tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(G_OBJECT(spw), "tracker"));
            Unit const *unit = tracker->getActiveUnit();
            g_return_if_fail(unit != NULL);

            struct { char const *key; double val; } const keyval[] = {
                { "X", bbox->min()[X] },
                { "Y", bbox->min()[Y] },
                { "width", bbox->dimensions()[X] },
                { "height", bbox->dimensions()[Y] }
            };

            if (unit->type == Inkscape::Util::UNIT_TYPE_DIMENSIONLESS) {
                double const val = unit->factor * 100;
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    GtkAdjustment *a = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(spw), keyval[i].key));
                    gtk_adjustment_set_value(a, val);
                    tracker->setFullVal( a, keyval[i].val );
                }
            } else {
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    GtkAdjustment *a = GTK_ADJUSTMENT(g_object_get_data(G_OBJECT(spw), keyval[i].key));
                    gtk_adjustment_set_value(a, Quantity::convert(keyval[i].val, "px", unit));
                }
            }
        }
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}


static void
sp_selection_layout_widget_modify_selection(SPWidget *spw, Inkscape::Selection *selection, guint flags, gpointer data)
{
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ((desktop->getSelection() == selection) // only respond to changes in our desktop
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
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if (desktop->getSelection() == selection) { // only respond to changes in our desktop
        gboolean setActive = (selection && !selection->isEmpty());
        std::vector<GtkAction*> *contextActions = reinterpret_cast<std::vector<GtkAction*> *>(g_object_get_data(G_OBJECT(spw), "contextActions"));
        if ( contextActions ) {
            for ( std::vector<GtkAction*>::iterator iter = contextActions->begin();
                  iter != contextActions->end(); ++iter) {
                if ( setActive != gtk_action_is_sensitive(*iter) ) {
                    gtk_action_set_sensitive( *iter, setActive );
                }
            }
        }

        sp_selection_layout_widget_update(spw, selection);
    }
}

static void
sp_object_layout_any_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    if (g_object_get_data(tbl, "update")) {
        return;
    }

    UnitTracker *tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(tbl, "tracker"));
    if ( !tracker || tracker->isUpdating() ) {
        /*
         * When only units are being changed, don't treat changes
         * to adjuster values as object changes.
         */
        return;
    }
    g_object_set_data(tbl, "update", GINT_TO_POINTER(TRUE));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = desktop->getSelection();
    SPDocument *document = desktop->getDocument();

    document->ensureUpToDate ();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    Geom::OptRect bbox_vis = selection->visualBounds();
    Geom::OptRect bbox_geom = selection->geometricBounds();

    int prefs_bbox = prefs->getInt("/tools/bounding_box");
    SPItem::BBoxType bbox_type = (prefs_bbox == 0)?
        SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
    Geom::OptRect bbox_user = selection->bounds(bbox_type);

    if ( !bbox_user ) {
        g_object_set_data(tbl, "update", GINT_TO_POINTER(FALSE));
        return;
    }

    gdouble x0 = 0;
    gdouble y0 = 0;
    gdouble x1 = 0;
    gdouble y1 = 0;
    gdouble xrel = 0;
    gdouble yrel = 0;
    Unit const *unit = tracker->getActiveUnit();
    g_return_if_fail(unit != NULL);

    GtkAdjustment* a_x = GTK_ADJUSTMENT( g_object_get_data( tbl, "X" ) );
    GtkAdjustment* a_y = GTK_ADJUSTMENT( g_object_get_data( tbl, "Y" ) );
    GtkAdjustment* a_w = GTK_ADJUSTMENT( g_object_get_data( tbl, "width" ) );
    GtkAdjustment* a_h = GTK_ADJUSTMENT( g_object_get_data( tbl, "height" ) );

    if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
        x0 = Quantity::convert(gtk_adjustment_get_value(a_x), unit, "px");
        y0 = Quantity::convert(gtk_adjustment_get_value(a_y), unit, "px");
        x1 = x0 + Quantity::convert(gtk_adjustment_get_value(a_w), unit, "px");
        xrel = Quantity::convert(gtk_adjustment_get_value(a_w), unit, "px") / bbox_user->dimensions()[Geom::X];
        y1 = y0 + Quantity::convert(gtk_adjustment_get_value(a_h), unit, "px");;
        yrel = Quantity::convert(gtk_adjustment_get_value(a_h), unit, "px") / bbox_user->dimensions()[Geom::Y];
    } else {
        double const x0_propn = gtk_adjustment_get_value (a_x) / 100 / unit->factor;
        x0 = bbox_user->min()[Geom::X] * x0_propn;
        double const y0_propn = gtk_adjustment_get_value (a_y) / 100 / unit->factor;
        y0 = y0_propn * bbox_user->min()[Geom::Y];
        xrel = gtk_adjustment_get_value (a_w) / (100 / unit->factor);
        x1 = x0 + xrel * bbox_user->dimensions()[Geom::X];
        yrel = gtk_adjustment_get_value (a_h) / (100 / unit->factor);
        y1 = y0 + yrel * bbox_user->dimensions()[Geom::Y];
    }

    // Keep proportions if lock is on
    GtkToggleAction *lock = GTK_TOGGLE_ACTION( g_object_get_data(tbl, "lock") );
    if ( gtk_toggle_action_get_active(lock) ) {
        if (adj == a_h) {
            x1 = x0 + yrel * bbox_user->dimensions()[Geom::X];
        } else if (adj == a_w) {
            y1 = y0 + xrel * bbox_user->dimensions()[Geom::Y];
        }
    }

    // scales and moves, in px
    double mh = fabs(x0 - bbox_user->min()[Geom::X]);
    double sh = fabs(x1 - bbox_user->max()[Geom::X]);
    double mv = fabs(y0 - bbox_user->min()[Geom::Y]);
    double sv = fabs(y1 - bbox_user->max()[Geom::Y]);

    // unless the unit is %, convert the scales and moves to the unit
    if (unit->type == Inkscape::Util::UNIT_TYPE_LINEAR) {
        mh = Quantity::convert(mh, "px", unit);
        sh = Quantity::convert(sh, "px", unit);
        mv = Quantity::convert(mv, "px", unit);
        sv = Quantity::convert(sv, "px", unit);
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
        desktop->getCanvas()->forceFullRedrawAfterInterruptions(0);

        bool transform_stroke = prefs->getBool("/options/transform/stroke", true);
        bool preserve = prefs->getBool("/options/preservetransform/value", false);

        Geom::Affine scaler;
        if (bbox_type == SPItem::VISUAL_BBOX) {
            scaler = get_scale_transform_for_variable_stroke (*bbox_vis, *bbox_geom, transform_stroke, preserve, x0, y0, x1, y1);
        } else {
            // 1) We could have use the newer get_scale_transform_for_variable_stroke() here, but to avoid regressions
            // we'll just use the old get_scale_transform_for_uniform_stroke() for now.
            // 2) get_scale_transform_for_uniform_stroke() is intended for visual bounding boxes, not geometrical ones!
            // we'll trick it into using a geometric bounding box though, by setting the stroke width to zero
            scaler = get_scale_transform_for_uniform_stroke (*bbox_geom, 0, 0, false, false, x0, y0, x1, y1);
        }

        sp_selection_apply_affine(selection, scaler);
        DocumentUndo::maybeDone(document, actionkey, SP_VERB_CONTEXT_SELECT,
                                _("Transform by toolbar"));

        // resume interruptibility
        desktop->getCanvas()->endForcedFullRedraws();
    }

    g_object_set_data(tbl, "update", GINT_TO_POINTER(FALSE));
}

// toggle button callbacks and updaters

static void toggle_stroke( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/options/transform/stroke", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>scaled</b> when objects are scaled."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>not scaled</b> when objects are scaled."));
    }
}

static void toggle_corners( GtkToggleAction* act, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/options/transform/rectcorners", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>scaled</b> when rectangles are scaled."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>not scaled</b> when rectangles are scaled."));
    }
}

static void toggle_gradient( GtkToggleAction *act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setBool("/options/transform/gradient", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

static void toggle_pattern( GtkToggleAction* act, gpointer data )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean active = gtk_toggle_action_get_active(act);
    prefs->setInt("/options/transform/pattern", active);
    SPDesktop *desktop = static_cast<SPDesktop *>(data);
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

static void toggle_lock( GtkToggleAction *act, gpointer /*data*/ ) {
    gboolean active = gtk_toggle_action_get_active( act );
    if ( active ) {
        g_object_set( G_OBJECT(act), "iconId", INKSCAPE_ICON("object-locked"), NULL );
    } else {
        g_object_set( G_OBJECT(act), "iconId", INKSCAPE_ICON("object-unlocked"), NULL );
    }
}

static void destroy_tracker( GObject* obj, gpointer /*user_data*/ )
{
    UnitTracker *tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(obj, "tracker"));
    if ( tracker ) {
        delete tracker;
        g_object_set_data( obj, "tracker", 0 );
    }
}

static void trigger_sp_action( GtkAction* /*act*/, gpointer user_data )
{
    SPAction* targetAction = SP_ACTION(user_data);
    if ( targetAction ) {
        sp_action_perform( targetAction, NULL );
    }
}

static GtkAction* create_action_for_verb( Inkscape::Verb* verb, Inkscape::UI::View::View* view, Inkscape::IconSize size )
{
    GtkAction* act = 0;

    SPAction* targetAction = verb->get_action(Inkscape::ActionContext(view));
    InkAction* inky = ink_action_new( verb->get_id(), verb->get_name(), verb->get_tip(), verb->get_image(), size  );
    act = GTK_ACTION(inky);

    g_signal_connect( G_OBJECT(inky), "activate", G_CALLBACK(trigger_sp_action), targetAction );

    Inkscape::queueIconPrerender( verb->get_image(), size );

    return act;
}

void sp_select_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::UI::View::View *view = desktop;
    Inkscape::IconSize secondarySize = Inkscape::UI::ToolboxFactory::prefToSize("/toolbox/secondary", 1);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    GtkAction* act = 0;

    GtkActionGroup* selectionActions = mainActions; // temporary
    std::vector<GtkAction*>* contextActions = new std::vector<GtkAction*>();

    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_EDIT_SELECT_ALL), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_EDIT_DESELECT), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );

    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_OBJECT_ROTATE_90_CCW), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_OBJECT_ROTATE_90_CW), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_OBJECT_FLIP_HORIZONTAL), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_OBJECT_FLIP_VERTICAL), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );

    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_SELECTION_TO_BACK), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_SELECTION_LOWER), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_SELECTION_RAISE), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );
    act = create_action_for_verb( Inkscape::Verb::get(SP_VERB_SELECTION_TO_FRONT), view, secondarySize );
    gtk_action_group_add_action( selectionActions, act );
    contextActions->push_back( act );

    // Create the parent widget for x y w h tracker.
    GtkWidget *spw = sp_widget_new_global();

    // Remember the desktop's canvas widget, to be used for defocusing.
    g_object_set_data(G_OBJECT(spw), "dtw", desktop->getCanvas());

    // The vb frame holds all other widgets and is used to set sensitivity depending on selection state.
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *vb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(vb), FALSE);
#else
    GtkWidget *vb = gtk_hbox_new(FALSE, 0);
#endif
    gtk_widget_show(vb);
    gtk_container_add(GTK_CONTAINER(spw), vb);

    // Create the units menu.
    UnitTracker* tracker = new UnitTracker(Inkscape::Util::UNIT_TYPE_LINEAR);
    tracker->addUnit(unit_table.getUnit("%"));
    tracker->setActiveUnit( desktop->getNamedView()->display_units );

    g_object_set_data( G_OBJECT(spw), "tracker", tracker );
    g_signal_connect( G_OBJECT(spw), "destroy", G_CALLBACK(destroy_tracker), spw );

    EgeAdjustmentAction* eact = 0;

    // four spinbuttons

    eact = create_adjustment_action(
            "XAction",                            /* name */ 
            C_("Select toolbar", "X position"),   /* label */ 
            C_("Select toolbar", "X:"),           /* shortLabel */ 
            C_("Select toolbar", "Horizontal coordinate of selection"), /* tooltip */ 
            "/tools/select/X",                    /* path */ 
            0.0,                                  /* def(default) */ 
            GTK_WIDGET(desktop->canvas),          /* focusTarget */ 
            G_OBJECT(spw),                        /* dataKludge */ 
            TRUE, "altx",                         /* altx, altx_mark */ 
            -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP, /* lower, uppper, step, page */ 
            0, 0, 0,                              /* descrLabels, descrValues, descrCount */ 
            sp_object_layout_any_value_changed,   /* callback */ 
            tracker,                              /* unit_tracker */ 
            SPIN_STEP, 3, 1);                     /* climb, digits, factor */

    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    eact = create_adjustment_action(
            "YAction",                            /* name */
            C_("Select toolbar", "Y position"),   /* label */
            C_("Select toolbar", "Y:"),           /* shortLabel */
            C_("Select toolbar", "Vertical coordinate of selection"), /* tooltip */
            "/tools/select/Y",                    /* path */
            0.0,                                  /* def(default) */
            GTK_WIDGET(desktop->canvas),          /* focusTarget */
            G_OBJECT(spw),                        /* dataKludge */
            TRUE, "altx",                         /* altx, altx_mark */
            -1e6, 1e6, SPIN_STEP, SPIN_PAGE_STEP, /* lower, uppper, step, page */
            0, 0, 0,                              /* descrLabels, descrValues, descrCount */
            sp_object_layout_any_value_changed,   /* callback */
            tracker,                              /* unit_tracker */
            SPIN_STEP, 3, 1);                     /* climb, digits, factor */              

    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    eact = create_adjustment_action(
            "WidthAction",                        /* name */
            C_("Select toolbar", "Width"),        /* label */
            C_("Select toolbar", "W:"),           /* shortLabel */
            C_("Select toolbar", "Width of selection"), /* tooltip */
            "/tools/select/width",                /* path */                      
            0.0,                                  /* def(default) */
            GTK_WIDGET(desktop->canvas),          /* focusTarget */
            G_OBJECT(spw),                        /* dataKludge */
            TRUE, "altx",                         /* altx, altx_mark */
            0.0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,  /* lower, uppper, step, page */
            0, 0, 0,                              /* descrLabels, descrValues, descrCount */
            sp_object_layout_any_value_changed,   /* callback */
            tracker,                              /* unit_tracker */
            SPIN_STEP, 3, 1);                     /* climb, digits, factor */

    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    // lock toggle
    {
    InkToggleAction* itact = ink_toggle_action_new( "LockAction",
                                                    _("Lock width and height"),
                                                    _("When locked, change both width and height by the same proportion"),
                                                    INKSCAPE_ICON("object-unlocked"),
                                                    Inkscape::ICON_SIZE_DECORATION );
    g_object_set( itact, "short_label", "Lock", NULL );
    g_object_set_data( G_OBJECT(spw), "lock", itact );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_lock), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    eact = create_adjustment_action(
            "HeightAction",                       /* name */
            C_("Select toolbar", "Height"),       /* label */
            C_("Select toolbar", "H:"),           /* shortLabel */
            C_("Select toolbar", "Height of selection"), /* tooltip */
            "/tools/select/height",               /* path */                      
            0.0,                                  /* def(default) */
            GTK_WIDGET(desktop->canvas),          /* focusTarget */
            G_OBJECT(spw),                        /* dataKludge */
            TRUE, "altx",                         /* altx, altx_mark */
            0.0, 1e6, SPIN_STEP, SPIN_PAGE_STEP,  /* lower, uppper, step, page */
            0, 0, 0,                              /* descrLabels, descrValues, descrCount */
            sp_object_layout_any_value_changed,   /* callback */
            tracker,                              /* unit_tracker */
            SPIN_STEP, 3, 1);                     /* climb, digits, factor */


    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    // Add the units menu.
    act = tracker->createAction( "UnitsAction", _("Units"), ("") );
    gtk_action_group_add_action( selectionActions, act );

    g_object_set_data( G_OBJECT(spw), "selectionActions", selectionActions );
    g_object_set_data( G_OBJECT(spw), "contextActions", contextActions );

    // Force update when selection changes.
    g_signal_connect(G_OBJECT(spw), "modify_selection", G_CALLBACK(sp_selection_layout_widget_modify_selection), desktop);
    g_signal_connect(G_OBJECT(spw), "change_selection", G_CALLBACK(sp_selection_layout_widget_change_selection), desktop);

    // Update now.
    sp_selection_layout_widget_update(SP_WIDGET(spw), SP_ACTIVE_DESKTOP ? SP_ACTIVE_DESKTOP->getSelection() : NULL);

    for ( std::vector<GtkAction*>::iterator iter = contextActions->begin();
          iter != contextActions->end(); ++iter) {
        if ( gtk_action_is_sensitive(*iter) ) {
            gtk_action_set_sensitive( *iter, FALSE );
        }
    }

    // Insert spw into the toolbar.
    if ( GTK_IS_BOX(holder) ) {
        gtk_box_pack_start(GTK_BOX(holder), spw, FALSE, FALSE, 0);
    } else if ( GTK_IS_TOOLBAR(holder) ) {
	GtkToolItem *spw_toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(spw_toolitem), spw);
	gtk_toolbar_insert(GTK_TOOLBAR(holder), spw_toolitem, -1);
    } else {
        g_warning("Unexpected holder type");
    }

    // "Transform with object" buttons
    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_stroke",
                                                    _("Scale stroke width"),
                                                    _("When scaling objects, scale the stroke width by the same proportion"),
                                                    INKSCAPE_ICON("transform-affect-stroke"),
                                                    Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs->getBool("/options/transform/stroke", true) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_stroke), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_corners",
                                                    _("Scale rounded corners"),
                                                    _("When scaling rectangles, scale the radii of rounded corners"),
                                                    INKSCAPE_ICON("transform-affect-rounded-corners"),
                                                  Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs->getBool("/options/transform/rectcorners", true) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_corners), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_gradient",
                                                    _("Move gradients"),
                                                    _("Move gradients (in fill or stroke) along with the objects"),
                                                    INKSCAPE_ICON("transform-affect-gradient"),
                                                  Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs->getBool("/options/transform/gradient", true) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_gradient), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_pattern",
                                                    _("Move patterns"),
                                                    _("Move patterns (in fill or stroke) along with the objects"),
                                                    INKSCAPE_ICON("transform-affect-pattern"),
                                                  Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs->getBool("/options/transform/pattern", true) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_pattern), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
