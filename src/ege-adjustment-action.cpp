

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is EGE Adjustment Action.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/* Note: this file should be kept compliable as both .cpp and .c */

#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtktoolitem.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmisc.h>
#include <gtk/gtktoolbar.h>

#include "ege-adjustment-action.h"


static void ege_adjustment_action_class_init( EgeAdjustmentActionClass* klass );
static void ege_adjustment_action_init( EgeAdjustmentAction* action );
static void ege_adjustment_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec );
static void ege_adjustment_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec );

static GtkWidget* create_menu_item( GtkAction* action );
static GtkWidget* create_tool_item( GtkAction* action );
static void connect_proxy( GtkAction *action, GtkWidget *proxy );
static void disconnect_proxy( GtkAction *action, GtkWidget *proxy );

static gboolean focus_in_cb( GtkWidget *widget, GdkEventKey *event, gpointer data );
static gboolean focus_out_cb( GtkWidget *widget, GdkEventKey *event, gpointer data );
static gboolean keypress_cb( GtkWidget *widget, GdkEventKey *event, gpointer data );

static void ege_adjustment_action_defocus( EgeAdjustmentAction* action );


static GtkActionClass* gParentClass = 0;


struct _EgeAdjustmentActionPrivate
{
    GtkAdjustment* adj;
    GtkWidget* focusWidget;
    gdouble climbRate;
    guint digits;
    gchar* selfId;
    EgeWidgetFixup toolPost;
    gdouble lastVal;
    gdouble step;
    gdouble page;
    gboolean transferFocus;
};

#define EGE_ADJUSTMENT_ACTION_GET_PRIVATE( o ) ( G_TYPE_INSTANCE_GET_PRIVATE( (o), EGE_ADJUSTMENT_ACTION_TYPE, EgeAdjustmentActionPrivate ) )

enum {
    PROP_ADJUSTMENT = 1,
    PROP_FOCUS_WIDGET,
    PROP_CLIMB_RATE,
    PROP_DIGITS,
    PROP_SELFID,
    PROP_TOOL_POST
};

GType ege_adjustment_action_get_type( void )
{
    static GType myType = 0;
    if ( !myType ) {
        static const GTypeInfo myInfo = {
            sizeof( EgeAdjustmentActionClass ),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)ege_adjustment_action_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof( EgeAdjustmentAction ),
            0, /* n_preallocs */
            (GInstanceInitFunc)ege_adjustment_action_init,
            NULL
        };

        myType = g_type_register_static( GTK_TYPE_ACTION, "EgeAdjustmentAction", &myInfo, (GTypeFlags)0 );
    }

    return myType;
}


static void ege_adjustment_action_class_init( EgeAdjustmentActionClass* klass )
{
    if ( klass ) {
        gParentClass = GTK_ACTION_CLASS( g_type_class_peek_parent( klass ) );
        GObjectClass * objClass = G_OBJECT_CLASS( klass );

        objClass->get_property = ege_adjustment_action_get_property;
        objClass->set_property = ege_adjustment_action_set_property;

        klass->parent_class.create_menu_item = create_menu_item;
        klass->parent_class.create_tool_item = create_tool_item;
        klass->parent_class.connect_proxy = connect_proxy;
        klass->parent_class.disconnect_proxy = disconnect_proxy;

        g_object_class_install_property( objClass,
                                         PROP_ADJUSTMENT,
                                         g_param_spec_object( "adjustment",
                                                              "Adjustment",
                                                              "The adjustment to change",
                                                              GTK_TYPE_ADJUSTMENT,
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_FOCUS_WIDGET,
                                         g_param_spec_pointer( "focus-widget",
                                                               "Focus Widget",
                                                               "The widget to return focus to",
                                                               (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_CLIMB_RATE,
                                         g_param_spec_double( "climb-rate",
                                                              "Climb Rate",
                                                              "The acelleraton rate",
                                                              0.0, G_MAXDOUBLE, 0.0,
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_DIGITS,
                                         g_param_spec_uint( "digits",
                                                            "Digits",
                                                            "The number of digits to show",
                                                            0, 20, 0,
                                                            (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_SELFID,
                                         g_param_spec_string( "self-id",
                                                              "Self ID",
                                                              "Marker for self pointer",
                                                              0,
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_TOOL_POST,
                                         g_param_spec_pointer( "tool-post",
                                                               "Tool Widget post process",
                                                               "Function for final adjustments",
                                                               (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_type_class_add_private( klass, sizeof(EgeAdjustmentActionClass) );
    }
}

static void ege_adjustment_action_init( EgeAdjustmentAction* action )
{
    action->private_data = EGE_ADJUSTMENT_ACTION_GET_PRIVATE( action );
    action->private_data->adj = 0;
    action->private_data->focusWidget = 0;
    action->private_data->climbRate = 0.0;
    action->private_data->digits = 2;
    action->private_data->selfId = 0;
    action->private_data->toolPost = 0;
    action->private_data->lastVal = 0.0;
    action->private_data->step = 0.0;
    action->private_data->page = 0.0;
    action->private_data->transferFocus = FALSE;
}

EgeAdjustmentAction* ege_adjustment_action_new( GtkAdjustment* adjustment,
                                                const gchar *name,
                                                const gchar *label,
                                                const gchar *tooltip,
                                                const gchar *stock_id,
                                                gdouble climb_rate,
                                                guint digits )
{
    GObject* obj = (GObject*)g_object_new( EGE_ADJUSTMENT_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "stock_id", stock_id,
                                           "adjustment", adjustment,
                                           "climb-rate", climb_rate,
                                           "digits", digits,
                                           NULL );

    EgeAdjustmentAction* action = EGE_ADJUSTMENT_ACTION( obj );

    return action;
}

static void ege_adjustment_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec )
{
    EgeAdjustmentAction* action = EGE_ADJUSTMENT_ACTION( obj );
    switch ( propId ) {
        case PROP_ADJUSTMENT:
            g_value_set_object( value, action->private_data->adj );
            break;

        case PROP_FOCUS_WIDGET:
            g_value_set_pointer( value, action->private_data->focusWidget );
            break;

        case PROP_CLIMB_RATE:
            g_value_set_double( value, action->private_data->climbRate );
            break;

        case PROP_DIGITS:
            g_value_set_uint( value, action->private_data->digits );
            break;

        case PROP_SELFID:
            g_value_set_string( value, action->private_data->selfId );
            break;

        case PROP_TOOL_POST:
            g_value_set_pointer( value, (void*)action->private_data->toolPost );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

void ege_adjustment_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec )
{
    EgeAdjustmentAction* action = EGE_ADJUSTMENT_ACTION( obj );
    switch ( propId ) {
        case PROP_ADJUSTMENT:
        {
            action->private_data->adj = GTK_ADJUSTMENT( g_value_get_object( value ) );
            g_object_get( G_OBJECT(action->private_data->adj),
                          "step-increment", &action->private_data->step,
                          "page-increment", &action->private_data->page,
                          NULL );
        }
        break;

        case PROP_FOCUS_WIDGET:
        {
            /* TODO unhook prior */
            action->private_data->focusWidget = (GtkWidget*)g_value_get_pointer( value );
        }
        break;

        case PROP_CLIMB_RATE:
        {
            /* TODO pass on */
            action->private_data->climbRate = g_value_get_double( value );
        }
        break;

        case PROP_DIGITS:
        {
            /* TODO pass on */
            action->private_data->digits = g_value_get_uint( value );
        }
        break;

        case PROP_SELFID:
        {
            /* TODO pass on */
            gchar* prior = action->private_data->selfId;
            action->private_data->selfId = g_value_dup_string( value );
            g_free( prior );
        }
        break;

        case PROP_TOOL_POST:
        {
            action->private_data->toolPost = (EgeWidgetFixup)g_value_get_pointer( value );
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

GtkAdjustment* ege_adjustment_action_get_adjustment( EgeAdjustmentAction* action )
{
    g_return_val_if_fail( IS_EGE_ADJUSTMENT_ACTION(action), NULL );

    return action->private_data->adj;
}

void ege_adjustment_action_set_focuswidget( EgeAdjustmentAction* action, GtkWidget* widget )
{
    g_return_if_fail( IS_EGE_ADJUSTMENT_ACTION(action) );

    /* TODO unhook prior */

    action->private_data->focusWidget = widget;
}

GtkWidget* ege_adjustment_action_get_focuswidget( EgeAdjustmentAction* action )
{
    g_return_val_if_fail( IS_EGE_ADJUSTMENT_ACTION(action), NULL );

    return action->private_data->focusWidget;
}

static GtkWidget* create_menu_item( GtkAction* action )
{
    GtkWidget* item = 0;

    item = gParentClass->create_menu_item( action );

    return item;
}

void value_changed_cb( GtkSpinButton* spin, EgeAdjustmentAction* act )
{
    if ( GTK_WIDGET_HAS_FOCUS( GTK_WIDGET(spin) ) ) {
        ege_adjustment_action_defocus( act );
    }
}

static GtkWidget* create_tool_item( GtkAction* action )
{
    GtkWidget* item = 0;

    if ( IS_EGE_ADJUSTMENT_ACTION(action) ) {
        EgeAdjustmentAction* act = EGE_ADJUSTMENT_ACTION( action );
        GtkWidget* spinbutton = gtk_spin_button_new( act->private_data->adj, act->private_data->climbRate, act->private_data->digits );
        GtkWidget* hb = gtk_hbox_new( FALSE, 5 );
        GValue value;

        item = GTK_WIDGET( gtk_tool_item_new() );

        memset( &value, 0, sizeof(value) );
        g_value_init( &value, G_TYPE_STRING );
        g_object_get_property( G_OBJECT(action), "label", &value );
        const gchar* sss = g_value_get_string( &value );
        GtkWidget* lbl = gtk_label_new( sss ? sss : "wwww" );

        gtk_misc_set_alignment( GTK_MISC(lbl), 1.0, 0.5 );

        gtk_box_pack_start( GTK_BOX(hb), lbl, FALSE, FALSE, 0 );
        gtk_box_pack_end( GTK_BOX(hb), spinbutton, FALSE, FALSE, 0 );
        gtk_container_add( GTK_CONTAINER(item), hb );

        if ( act->private_data->selfId ) {
            gtk_object_set_data( GTK_OBJECT(spinbutton), act->private_data->selfId, spinbutton );
        }

        g_signal_connect( G_OBJECT(spinbutton), "focus-in-event", G_CALLBACK(focus_in_cb), action );
        g_signal_connect( G_OBJECT(spinbutton), "focus-out-event", G_CALLBACK(focus_out_cb), action );
        g_signal_connect( G_OBJECT(spinbutton), "key-press-event", G_CALLBACK(keypress_cb), action );

        g_signal_connect( G_OBJECT(spinbutton), "value-changed", G_CALLBACK(value_changed_cb), action );
/*      g_signal_connect( G_OBJECT(EGE_ADJUSTMENT_ACTION(action)->private_data->adj), "value-changed", G_CALLBACK(flippy), action ); */


        gtk_widget_show_all( item );

        /* Shrink or whatnot after shown */
        if ( act->private_data->toolPost ) {
            act->private_data->toolPost( item );
        }
    } else {
        item = gParentClass->create_tool_item( action );
    }

    return item;
}

static void connect_proxy( GtkAction *action, GtkWidget *proxy )
{
    gParentClass->connect_proxy( action, proxy );
}

static void disconnect_proxy( GtkAction *action, GtkWidget *proxy )
{
    gParentClass->disconnect_proxy( action, proxy );
}

void ege_adjustment_action_defocus( EgeAdjustmentAction* action )
{
    if ( action->private_data->transferFocus ) {
        if ( action->private_data->focusWidget ) {
            gtk_widget_grab_focus( action->private_data->focusWidget );
        }
    }
}

gboolean focus_in_cb( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
    (void)event;
    if ( IS_EGE_ADJUSTMENT_ACTION(data) ) {
        EgeAdjustmentAction* action = EGE_ADJUSTMENT_ACTION( data );
        action->private_data->lastVal = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
        action->private_data->transferFocus = TRUE;
    }

    return FALSE; /* report event not consumed */
}

static gboolean focus_out_cb( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
    (void)widget;
    (void)event;
    if ( IS_EGE_ADJUSTMENT_ACTION(data) ) {
        EgeAdjustmentAction* action = EGE_ADJUSTMENT_ACTION( data );
        action->private_data->transferFocus = FALSE;
    }

    return FALSE; /* report event not consumed */
}


static gboolean process_tab( GtkWidget* widget, int direction )
{
    gboolean handled = FALSE;
    GtkWidget* parent = gtk_widget_get_parent(widget);
    GtkWidget* gp = parent ? gtk_widget_get_parent(parent) : 0;
    GtkWidget* ggp = gp ? gtk_widget_get_parent(gp) : 0;

    if ( ggp && GTK_IS_TOOLBAR(ggp) ) {
        GList* kids = gtk_container_get_children( GTK_CONTAINER(ggp) );
        if ( kids ) {
            GtkWidget* curr = widget;
            while ( curr && (gtk_widget_get_parent(curr) != ggp) ) {
                curr = gtk_widget_get_parent( curr );
            }
            if ( curr ) {
                GList* mid = g_list_find( kids, curr );
                while ( mid ) {
                    mid = ( direction < 0 ) ? g_list_previous(mid) : g_list_next(mid);
                    if ( mid && GTK_IS_TOOL_ITEM(mid->data) ) {
                        /* potential target */
                        GtkWidget* child = gtk_bin_get_child( GTK_BIN(mid->data) );
                        if ( child && GTK_IS_HBOX(child) ) { /* could be ours */
                            GList* subChildren = gtk_container_get_children( GTK_CONTAINER(child) );
                            if ( subChildren ) {
                                GList* last = g_list_last(subChildren);
                                if ( last && GTK_IS_SPIN_BUTTON(last->data) && GTK_WIDGET_IS_SENSITIVE( GTK_WIDGET(last->data) ) ) {
                                    gtk_widget_grab_focus( GTK_WIDGET(last->data) );
                                    handled = TRUE;
                                    mid = 0; /* to stop loop */
                                }

                                g_list_free(subChildren);
                            }
                        }
                    }
                }
            }
            g_list_free( kids );
        }
    }

    return handled;
}

gboolean keypress_cb( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
    gboolean wasConsumed = FALSE; /* default to report event not consumed */
    EgeAdjustmentAction* action = EGE_ADJUSTMENT_ACTION(data);
    guint key = 0;
    gdk_keymap_translate_keyboard_state( gdk_keymap_get_for_display( gdk_display_get_default() ),
                                         event->hardware_keycode, (GdkModifierType)event->state,
                                         0, &key, 0, 0, 0 );

    switch ( key ) {
        case GDK_Escape:
        {
            action->private_data->transferFocus = TRUE;
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), action->private_data->lastVal );
            ege_adjustment_action_defocus( action );
            wasConsumed = TRUE;
        }
        break;

        case GDK_Return:
        case GDK_KP_Enter:
        {
            action->private_data->transferFocus = TRUE;
            ege_adjustment_action_defocus( action );
            wasConsumed = TRUE;
        }
        break;

        case GDK_Tab:
        {
            action->private_data->transferFocus = FALSE;
            wasConsumed = process_tab( widget, 1 );
        }
        break;

        case GDK_ISO_Left_Tab:
        {
            action->private_data->transferFocus = FALSE;
            wasConsumed = process_tab( widget, -1 );
        }
        break;

        case GDK_Up:
        case GDK_KP_Up:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val + action->private_data->step );
            wasConsumed = TRUE;
        }
        break;

        case GDK_Down:
        case GDK_KP_Down:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val - action->private_data->step );
            wasConsumed = TRUE;
        }
        break;

        case GDK_Page_Up:
        case GDK_KP_Page_Up:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val + action->private_data->page );
            wasConsumed = TRUE;
        }
        break;

        case GDK_Page_Down:
        case GDK_KP_Page_Down:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val - action->private_data->page );
            wasConsumed = TRUE;
        }
        break;

        case GDK_z:
        case GDK_Z:
        {
            action->private_data->transferFocus = FALSE;
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), action->private_data->lastVal );
            wasConsumed = TRUE;
        }
        break;

    }

    return wasConsumed;
}
