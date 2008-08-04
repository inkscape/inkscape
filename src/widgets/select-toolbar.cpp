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
#include "helper/unit-tracker.h"
#include "ege-adjustment-action.h"
#include "ege-output-action.h"
#include "ink-action.h"
#include <2geom/rect.h>

using Inkscape::UnitTracker;

static void
sp_selection_layout_widget_update(SPWidget *spw, Inkscape::Selection *sel)
{
    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    using Geom::X;
    using Geom::Y;
    if ( sel && !sel->isEmpty() ) {
        int prefs_bbox = prefs_get_int_attribute("tools", "bounding_box", 0);
        SPItem::BBoxType bbox_type = (prefs_bbox ==0)? 
            SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        NR::Maybe<NR::Rect> const bbox(sel->bounds(bbox_type));
        if ( bbox && !bbox->isEmpty() ) {
            UnitTracker *tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(G_OBJECT(spw), "tracker"));
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
                    GtkAdjustment *a = (GtkAdjustment *) g_object_get_data(G_OBJECT(spw), keyval[i].key);
                    gtk_adjustment_set_value(a, val);
                    tracker->setFullVal( a, keyval[i].val );
                }
            } else {
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    GtkAdjustment *a = (GtkAdjustment *) g_object_get_data(G_OBJECT(spw), keyval[i].key);
                    gtk_adjustment_set_value(a, sp_pixels_get_units(keyval[i].val, unit));
                }
            }
        }
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
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
    if (sp_desktop_selection(desktop) == selection) { // only respond to changes in our desktop
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
sp_object_layout_any_value_changed(GtkAdjustment *adj, SPWidget *spw)
{
    if (g_object_get_data(G_OBJECT(spw), "update")) {
        return;
    }

    UnitTracker *tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(G_OBJECT(spw), "tracker"));
    if ( !tracker || tracker->isUpdating() ) {
        /*
         * When only units are being changed, don't treat changes
         * to adjuster values as object changes.
         */
        return;
    }
    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    SPDocument *document = sp_desktop_document(desktop);

    sp_document_ensure_up_to_date (document);
    int prefs_bbox = prefs_get_int_attribute("tools", "bounding_box", 0);
    SPItem::BBoxType bbox_type = (prefs_bbox ==0)? 
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
    NR::Maybe<NR::Rect> bbox = selection->bounds(bbox_type);

    if ( !bbox || bbox->isEmpty() ) {
        g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
        return;
    }

    gdouble x0 = 0;
    gdouble y0 = 0;
    gdouble x1 = 0;
    gdouble y1 = 0;
    gdouble xrel = 0;
    gdouble yrel = 0;
    SPUnit const &unit = *tracker->getActiveUnit();

    GtkAdjustment* a_x = GTK_ADJUSTMENT( g_object_get_data( G_OBJECT(spw), "X" ) );
    GtkAdjustment* a_y = GTK_ADJUSTMENT( g_object_get_data( G_OBJECT(spw), "Y" ) );
    GtkAdjustment* a_w = GTK_ADJUSTMENT( g_object_get_data( G_OBJECT(spw), "width" ) );
    GtkAdjustment* a_h = GTK_ADJUSTMENT( g_object_get_data( G_OBJECT(spw), "height" ) );

    if (unit.base == SP_UNIT_ABSOLUTE || unit.base == SP_UNIT_DEVICE) {
        x0 = sp_units_get_pixels (a_x->value, unit);
        y0 = sp_units_get_pixels (a_y->value, unit);
        x1 = x0 + sp_units_get_pixels (a_w->value, unit);
        xrel = sp_units_get_pixels (a_w->value, unit) / bbox->extent(Geom::X);
        y1 = y0 + sp_units_get_pixels (a_h->value, unit);
        yrel = sp_units_get_pixels (a_h->value, unit) / bbox->extent(Geom::Y);
    } else {
        double const x0_propn = a_x->value * unit.unittobase;
        x0 = bbox->min()[Geom::X] * x0_propn;
        double const y0_propn = a_y->value * unit.unittobase;
        y0 = y0_propn * bbox->min()[Geom::Y];
        xrel = a_w->value * unit.unittobase;
        x1 = x0 + xrel * bbox->extent(Geom::X);
        yrel = a_h->value * unit.unittobase;
        y1 = y0 + yrel * bbox->extent(Geom::Y);
    }

    // Keep proportions if lock is on
    GtkToggleAction *lock = GTK_TOGGLE_ACTION( g_object_get_data(G_OBJECT(spw), "lock") );
    if ( gtk_toggle_action_get_active(lock) ) {
        if (adj == a_h) {
            x1 = x0 + yrel * bbox->extent(Geom::X);
        } else if (adj == a_w) {
            y1 = y0 + xrel * bbox->extent(Geom::Y);
        }
    }

    // scales and moves, in px
    double mh = fabs(x0 - bbox->min()[Geom::X]);
    double sh = fabs(x1 - bbox->max()[Geom::X]);
    double mv = fabs(y0 - bbox->min()[Geom::Y]);
    double sv = fabs(y1 - bbox->max()[Geom::Y]);

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

        // resume interruptibility
        sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));
    }

    g_object_set_data(G_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}

static EgeAdjustmentAction * create_adjustment_action( gchar const *name,
                                                       gchar const *label,
                                                       gchar const *shortLabel,
                                                       gchar const *data,
                                                       gdouble lower,
                                                       GtkWidget* focusTarget,
                                                       UnitTracker* tracker,
                                                       GtkWidget* spw,
                                                       gchar const *tooltip,
                                                       gboolean altx )
{
    GtkAdjustment* adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, lower, 1e6, SPIN_STEP, SPIN_PAGE_STEP, SPIN_PAGE_STEP ) );
    if (tracker) {
        tracker->addAdjustment(adj);
    }
    if ( spw ) {
        g_object_set_data( G_OBJECT(spw), data, adj );
    }

    EgeAdjustmentAction* act = ege_adjustment_action_new( adj, name, Q_(label), tooltip, 0, SPIN_STEP, 3 );
    if ( shortLabel ) {
        g_object_set( act, "short_label", Q_(shortLabel), NULL );
    }

    gtk_signal_connect( GTK_OBJECT(adj), "value_changed", GTK_SIGNAL_FUNC(sp_object_layout_any_value_changed), spw );
    if ( focusTarget ) {
        ege_adjustment_action_set_focuswidget( act, focusTarget );
    }

    if ( altx ) { // this spinbutton will be activated by alt-x
        g_object_set( G_OBJECT(act), "self-id", "altx", NULL );
    }

    // Using a cast just to make sure we pass in the right kind of function pointer
    g_object_set( G_OBJECT(act), "tool-post", static_cast<EgeWidgetFixup>(sp_set_font_size_smaller), NULL );

    return act;
}

// toggle button callbacks and updaters

static void toggle_stroke( GtkToggleAction* act, gpointer data ) {
    gboolean active = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute( "options.transform", "stroke", active ? 1 : 0 );
    SPDesktop *desktop = (SPDesktop *)data;
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>scaled</b> when objects are scaled."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>stroke width</b> is <b>not scaled</b> when objects are scaled."));
    }
}

static void toggle_corners( GtkToggleAction* act, gpointer data) {
    gboolean active = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute( "options.transform", "rectcorners", active ? 1 : 0 );
    SPDesktop *desktop = (SPDesktop *)data;
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>scaled</b> when rectangles are scaled."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>rounded rectangle corners</b> are <b>not scaled</b> when rectangles are scaled."));
    }
}

static void toggle_gradient( GtkToggleAction *act, gpointer data ) {
    gboolean active = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute( "options.transform", "gradient", active ? 1 : 0 );
    SPDesktop *desktop = (SPDesktop *)data;
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>gradients</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

static void toggle_pattern( GtkToggleAction* act, gpointer data ) {
    gboolean active = gtk_toggle_action_get_active( act );
    prefs_set_int_attribute( "options.transform", "pattern", active ? 1 : 0 );
    SPDesktop *desktop = (SPDesktop *)data;
    if ( active ) {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> are <b>transformed</b> along with their objects when those are transformed (moved, scaled, rotated, or skewed)."));
    } else {
        desktop->messageStack()->flash(Inkscape::INFORMATION_MESSAGE, _("Now <b>patterns</b> remain <b>fixed</b> when objects are transformed (moved, scaled, rotated, or skewed)."));
    }
}

static void toggle_lock( GtkToggleAction *act, gpointer /*data*/ ) {
    gboolean active = gtk_toggle_action_get_active( act );
    if ( active ) {
        g_object_set( G_OBJECT(act), "iconId", "width_height_lock", NULL );
    } else {
        g_object_set( G_OBJECT(act), "iconId", "lock_unlocked", NULL );
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

    SPAction* targetAction = verb->get_action(view);
    InkAction* inky = ink_action_new( verb->get_id(), verb->get_name(), verb->get_tip(), verb->get_image(), size  );
    act = GTK_ACTION(inky);

    g_signal_connect( G_OBJECT(inky), "activate", GTK_SIGNAL_FUNC(trigger_sp_action), targetAction );

    return act;
}

void sp_select_toolbox_prep(SPDesktop *desktop, GtkActionGroup* mainActions, GObject* holder)
{
    Inkscape::UI::View::View *view = desktop;
    Inkscape::IconSize secondarySize = prefToSize("toolbox", "secondary", 1);

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
    GtkWidget *spw = sp_widget_new_global(INKSCAPE);

    // Remember the desktop's canvas widget, to be used for defocusing.
    g_object_set_data(G_OBJECT(spw), "dtw", sp_desktop_canvas(desktop));

    // The vb frame holds all other widgets and is used to set sensitivity depending on selection state.
    GtkWidget *vb = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(vb);
    gtk_container_add(GTK_CONTAINER(spw), vb);

    // Create the units menu.
    UnitTracker* tracker = new UnitTracker( SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE );
    tracker->addUnit( SP_UNIT_PERCENT, 0 );
    tracker->setActiveUnit( sp_desktop_namedview(desktop)->doc_units );

    g_object_set_data( G_OBJECT(spw), "tracker", tracker );
    g_signal_connect( G_OBJECT(spw), "destroy", G_CALLBACK(destroy_tracker), spw );

    EgeAdjustmentAction* eact = 0;

    // four spinbuttons

    //TRANSLATORS: only translate "string" in "context|string".
    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
    eact = create_adjustment_action( "XAction", _("select_toolbar|X position"), _("select_toolbar|X"), "X",
                                     -1e6, GTK_WIDGET(desktop->canvas), tracker, spw,
                                     _("Horizontal coordinate of selection"), TRUE );
    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    //TRANSLATORS: only translate "string" in "context|string".
    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
    eact = create_adjustment_action( "YAction", _("select_toolbar|Y position"), _("select_toolbar|Y"), "Y",
                                     -1e6, GTK_WIDGET(desktop->canvas), tracker, spw,
                                     _("Vertical coordinate of selection"), FALSE );
    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    //TRANSLATORS: only translate "string" in "context|string".
    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
    eact = create_adjustment_action( "WidthAction", _("select_toolbar|Width"), _("select_toolbar|W"), "width",
                                     1e-3, GTK_WIDGET(desktop->canvas), tracker, spw,
                                     _("Width of selection"), FALSE );
    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    // lock toggle
    {
    InkToggleAction* itact = ink_toggle_action_new( "LockAction",
                                                    _("Lock width and height"),
                                                    _("When locked, change both width and height by the same proportion"),
                                                    "lock_unlocked",
                                                    Inkscape::ICON_SIZE_DECORATION );
    g_object_set( itact, "short_label", "Lock", NULL );
    g_object_set_data( G_OBJECT(spw), "lock", itact );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_lock), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    //TRANSLATORS: only translate "string" in "context|string".
    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
    eact = create_adjustment_action( "HeightAction", _("select_toolbar|Height"), _("select_toolbar|H"), "height",
                                     1e-3, GTK_WIDGET(desktop->canvas), tracker, spw,
                                     _("Height of selection"), FALSE );
    gtk_action_group_add_action( selectionActions, GTK_ACTION(eact) );
    contextActions->push_back( GTK_ACTION(eact) );

    // Add the units menu.
    act = tracker->createAction( "UnitsAction", _("Units"), ("") );
    gtk_action_group_add_action( selectionActions, act );

    g_object_set_data( G_OBJECT(spw), "selectionActions", selectionActions );
    g_object_set_data( G_OBJECT(spw), "contextActions", contextActions );

    // Force update when selection changes.
    gtk_signal_connect(GTK_OBJECT(spw), "modify_selection", GTK_SIGNAL_FUNC(sp_selection_layout_widget_modify_selection), desktop);
    gtk_signal_connect(GTK_OBJECT(spw), "change_selection", GTK_SIGNAL_FUNC(sp_selection_layout_widget_change_selection), desktop);

    // Update now.
    sp_selection_layout_widget_update(SP_WIDGET(spw), SP_ACTIVE_DESKTOP ? sp_desktop_selection(SP_ACTIVE_DESKTOP) : NULL);

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
        gtk_toolbar_append_widget( GTK_TOOLBAR(holder), spw, "Text", "priv" );
    } else {
        g_warning("Unexpected holder type");
    }

    // "Transform with object" buttons

    {
        EgeOutputAction* act = ege_output_action_new( "transform_affect_label", _("Affect:"), "", 0 );
        ege_output_action_set_use_markup( act, TRUE );
        g_object_set( act, "visible-overflown", FALSE, NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_stroke",
                                                    _("Scale stroke width"),
                                                    _("When scaling objects, scale the stroke width by the same proportion"),
                                                    "transform_stroke",
                                                    Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs_get_int_attribute("options.transform", "stroke", 1) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_stroke), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_corners",
                                                    _("Scale rounded corners"),
                                                    _("When scaling rectangles, scale the radii of rounded corners"),
                                                    "transform_corners",
                                                  Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs_get_int_attribute("options.transform", "rectcorners", 1) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_corners), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_gradient",
                                                    _("Move gradients"),
                                                    _("Move gradients (in fill or stroke) along with the objects"),
                                                    "transform_gradient",
                                                  Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs_get_int_attribute("options.transform", "gradient", 1) );
    g_signal_connect_after( G_OBJECT(itact), "toggled", G_CALLBACK(toggle_gradient), desktop) ;
    gtk_action_group_add_action( mainActions, GTK_ACTION(itact) );
    }

    {
    InkToggleAction* itact = ink_toggle_action_new( "transform_pattern",
                                                    _("Move patterns"),
                                                    _("Move patterns (in fill or stroke) along with the objects"),
                                                    "transform_pattern",
                                                  Inkscape::ICON_SIZE_DECORATION );
    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(itact), prefs_get_int_attribute("options.transform", "pattern", 1) );
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
