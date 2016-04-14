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

/* Note: this file should be kept compilable as both .cpp and .c */

#include <cmath>
#include <string.h>

#include "widgets/icon.h"
#include <gdk/gdkkeysyms.h>

#include "icon-size.h"
#include "widgets/ege-adjustment-action.h"
#include "ui/widget/gimpspinscale.h"
#include "ui/icon-names.h"


static void ege_adjustment_action_finalize( GObject* object );
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

static void egeAct_free_description( gpointer data, gpointer user_data );
static void egeAct_free_all_descriptions( EgeAdjustmentAction* action );

static EgeCreateAdjWidgetCB gFactoryCb = 0;
static GQuark gDataName = 0;

enum {
    APPEARANCE_UNKNOWN = -1,
    APPEARANCE_NONE = 0,
    APPEARANCE_FULL,    /* label, then all choices represented by separate buttons */
    APPEARANCE_COMPACT, /* label, then choices in a drop-down menu */
    APPEARANCE_MINIMAL, /* no label, just choices in a drop-down menu */
};

/* TODO need to have appropriate icons setup for these: */
static const gchar *floogles[] = {
    INKSCAPE_ICON("list-remove"),
    INKSCAPE_ICON("list-add"),
    INKSCAPE_ICON("go-down"),
    INKSCAPE_ICON("help-about"),
    INKSCAPE_ICON("go-up"),
    0};

typedef struct _EgeAdjustmentDescr EgeAdjustmentDescr;

struct _EgeAdjustmentDescr
{
    gchar* descr;
    gdouble value;
};

struct _EgeAdjustmentActionPrivate
{
    GtkAdjustment* adj;
    GtkWidget* focusWidget;
    gdouble climbRate;
    guint digits;
    gdouble epsilon;
    gchar* format;
    gchar* selfId;
    EgeWidgetFixup toolPost;
    gdouble lastVal;
    gdouble step;
    gdouble page;
    gint appearanceMode;
    gboolean transferFocus;
    GList* descriptions;
    gchar* appearance;
    gchar* iconId;
    Inkscape::IconSize iconSize;
    Inkscape::UI::Widget::UnitTracker *unitTracker;
};

#define EGE_ADJUSTMENT_ACTION_GET_PRIVATE( o ) ( G_TYPE_INSTANCE_GET_PRIVATE( (o), EGE_ADJUSTMENT_ACTION_TYPE, EgeAdjustmentActionPrivate ) )

enum {
    PROP_ADJUSTMENT = 1,
    PROP_FOCUS_WIDGET,
    PROP_CLIMB_RATE,
    PROP_DIGITS,
    PROP_SELFID,
    PROP_TOOL_POST,
    PROP_APPEARANCE,
    PROP_ICON_ID,
    PROP_ICON_SIZE,
    PROP_UNIT_TRACKER
};

enum {
    BUMP_TOP = 0,
    BUMP_PAGE_UP,
    BUMP_UP,
    BUMP_NONE,
    BUMP_DOWN,
    BUMP_PAGE_DOWN,
    BUMP_BOTTOM,
    BUMP_CUSTOM = 100
};

G_DEFINE_TYPE(EgeAdjustmentAction, ege_adjustment_action, GTK_TYPE_ACTION);

static void ege_adjustment_action_class_init( EgeAdjustmentActionClass* klass )
{
    if ( klass ) {
        GObjectClass * objClass = G_OBJECT_CLASS( klass );

        gDataName = g_quark_from_string("ege-adj-action");

  
        objClass->finalize = ege_adjustment_action_finalize;

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

        g_object_class_install_property( objClass,
                                         PROP_APPEARANCE,
                                         g_param_spec_string( "appearance",
                                                              "Appearance hint",
                                                              "A hint for how to display",
                                                              "",
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_ICON_ID,
                                         g_param_spec_string( "iconId",
                                                              "Icon ID",
                                                              "The id for the icon",
                                                              "",
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_ICON_SIZE,
                                         g_param_spec_int( "iconSize",
                                                           "Icon Size",
                                                           "The size the icon",
                                                           (int)Inkscape::ICON_SIZE_MENU,
                                                           (int)Inkscape::ICON_SIZE_DECORATION,
                                                           (int)Inkscape::ICON_SIZE_SMALL_TOOLBAR,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_UNIT_TRACKER,
                                         g_param_spec_pointer( "unit_tracker",
                                                               "Unit Tracker",
                                                               "The widget that keeps track of the unit",
                                                               (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_type_class_add_private( klass, sizeof(EgeAdjustmentActionClass) );
    }
}

void ege_adjustment_action_set_compact_tool_factory( EgeCreateAdjWidgetCB factoryCb )
{
    gFactoryCb = factoryCb;
}

static void ege_adjustment_action_init( EgeAdjustmentAction* action )
{
    action->private_data = EGE_ADJUSTMENT_ACTION_GET_PRIVATE( action );
    action->private_data->adj = 0;
    action->private_data->focusWidget = 0;
    action->private_data->climbRate = 0.0;
    action->private_data->digits = 2;
    action->private_data->epsilon = 0.009;
    action->private_data->format = g_strdup_printf("%%0.%df%%s%%s", action->private_data->digits);
    action->private_data->selfId = 0;
    action->private_data->toolPost = 0;
    action->private_data->lastVal = 0.0;
    action->private_data->step = 0.0;
    action->private_data->page = 0.0;
    action->private_data->appearanceMode = APPEARANCE_NONE;
    action->private_data->transferFocus = FALSE;
    action->private_data->descriptions = 0;
    action->private_data->appearance = 0;
    action->private_data->iconId = 0;
    action->private_data->iconSize = Inkscape::ICON_SIZE_SMALL_TOOLBAR;
    action->private_data->unitTracker = NULL;
}

static void ege_adjustment_action_finalize( GObject* object )
{
    EgeAdjustmentAction* action = 0;
    g_return_if_fail( object != NULL );
    g_return_if_fail( IS_EGE_ADJUSTMENT_ACTION(object) );

    action = EGE_ADJUSTMENT_ACTION( object );

    // g_free(NULL) does nothing
    g_free( action->private_data->format );
    g_free( action->private_data->selfId );
    g_free( action->private_data->appearance );
    g_free( action->private_data->iconId );

    egeAct_free_all_descriptions( action );

    if ( G_OBJECT_CLASS(ege_adjustment_action_parent_class)->finalize ) {
        (*G_OBJECT_CLASS(ege_adjustment_action_parent_class)->finalize)(object);
    }
}

EgeAdjustmentAction* ege_adjustment_action_new( GtkAdjustment* adjustment,
                                                const gchar *name,
                                                const gchar *label,
                                                const gchar *tooltip,
                                                const gchar *stock_id,
                                                gdouble climb_rate,
                                                guint digits,
                                                Inkscape::UI::Widget::UnitTracker *unit_tracker )
{
    GObject* obj = (GObject*)g_object_new( EGE_ADJUSTMENT_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "stock_id", stock_id,
                                           "adjustment", adjustment,
                                           "climb-rate", climb_rate,
                                           "digits", digits,
                                           "unit_tracker", unit_tracker,
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

        case PROP_APPEARANCE:
            g_value_set_string( value, action->private_data->appearance );
            break;

        case PROP_ICON_ID:
            g_value_set_string( value, action->private_data->iconId );
            break;

        case PROP_ICON_SIZE:
            g_value_set_int( value, action->private_data->iconSize );
            break;

        case PROP_UNIT_TRACKER:
            g_value_set_pointer( value, action->private_data->unitTracker );
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
            switch ( action->private_data->digits ) {
                case 0: action->private_data->epsilon = 0.9; break;
                case 1: action->private_data->epsilon = 0.09; break;
                case 2: action->private_data->epsilon = 0.009; break;
                case 3: action->private_data->epsilon = 0.0009; break;
                case 4: action->private_data->epsilon = 0.00009; break;
            }
            if ( action->private_data->format ) {
                g_free( action->private_data->format );
            }
            action->private_data->format = g_strdup_printf("%%0.%df%%s%%s", action->private_data->digits);
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

        case PROP_APPEARANCE:
        {
            gchar* tmp = action->private_data->appearance;
            gchar* newVal = g_value_dup_string( value );
            action->private_data->appearance = newVal;
            g_free( tmp );

            if ( !action->private_data->appearance || (strcmp("", newVal) == 0) ) {
                action->private_data->appearanceMode = APPEARANCE_NONE;
            } else if ( strcmp("full", newVal) == 0 ) {
                action->private_data->appearanceMode = APPEARANCE_FULL;
            } else if ( strcmp("compact", newVal) == 0 ) {
                action->private_data->appearanceMode = APPEARANCE_COMPACT;
            } else if ( strcmp("minimal", newVal) == 0 ) {
                action->private_data->appearanceMode = APPEARANCE_MINIMAL;
            } else {
                action->private_data->appearanceMode = APPEARANCE_UNKNOWN;
            }
        }
        break;

        case PROP_ICON_ID:
        {
            gchar* tmp = action->private_data->iconId;
            action->private_data->iconId = g_value_dup_string( value );
            g_free( tmp );
        }
        break;

        case PROP_ICON_SIZE:
        {
            action->private_data->iconSize = (Inkscape::IconSize)g_value_get_int( value );
        }
        break;

        case PROP_UNIT_TRACKER:
        {
            action->private_data->unitTracker = (Inkscape::UI::Widget::UnitTracker*)g_value_get_pointer( value );
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

static void egeAct_free_description( gpointer data, gpointer user_data ) {
    (void)user_data;
    if ( data ) {
        EgeAdjustmentDescr* descr = (EgeAdjustmentDescr*)data;
        if ( descr->descr ) {
            g_free( descr->descr );
            descr->descr = 0;
        }
        g_free( descr );
    }
}

static void egeAct_free_all_descriptions( EgeAdjustmentAction* action )
{
    if ( action->private_data->descriptions ) {
        g_list_foreach( action->private_data->descriptions, egeAct_free_description, 0 );
        g_list_free( action->private_data->descriptions );
        action->private_data->descriptions = 0;
    }
}

static gint egeAct_compare_descriptions( gconstpointer a, gconstpointer b )
{
    gint val = 0;

    EgeAdjustmentDescr const * aa = (EgeAdjustmentDescr const *)a;
    EgeAdjustmentDescr const * bb = (EgeAdjustmentDescr const *)b;

    if ( aa && bb ) {
        if ( aa->value < bb->value ) {
            val = -1;
        } else if ( aa->value > bb->value ) {
            val = 1;
        }
    }

    return val;
}

void ege_adjustment_action_set_descriptions( EgeAdjustmentAction* action, gchar const** descriptions, gdouble const* values, guint count )
{
    g_return_if_fail( IS_EGE_ADJUSTMENT_ACTION(action) );

    egeAct_free_all_descriptions( action );

    if ( count && descriptions && values ) {
        guint i = 0;
        for ( i = 0; i < count; i++ ) {
            EgeAdjustmentDescr* descr = g_new0( EgeAdjustmentDescr, 1 );
            descr->descr = descriptions[i] ? g_strdup( descriptions[i] ) : 0;
            descr->value = values[i];
            action->private_data->descriptions = g_list_insert_sorted( action->private_data->descriptions, (gpointer)descr, egeAct_compare_descriptions );
        }
    }
}

void ege_adjustment_action_set_appearance( EgeAdjustmentAction* action, gchar const* val )
{
    g_object_set( G_OBJECT(action), "appearance", val, NULL );
}

static void process_menu_action( GtkWidget* obj, gpointer data )
{
    GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM(obj);
    if ( gtk_check_menu_item_get_active (item)) {
        EgeAdjustmentAction* act = (EgeAdjustmentAction*)g_object_get_qdata( G_OBJECT(obj), gDataName );
        gint what = GPOINTER_TO_INT(data);


        gdouble base = gtk_adjustment_get_value( act->private_data->adj );
        gdouble lower = 0.0;
        gdouble upper = 0.0;
        gdouble step = 0.0;
        gdouble page = 0.0;
        g_object_get( G_OBJECT(act->private_data->adj),
                      "lower", &lower,
                      "upper", &upper,
                      "step-increment", &step,
                      "page-increment", &page,
                      NULL );

        switch ( what ) {
            case BUMP_TOP:
                gtk_adjustment_set_value( act->private_data->adj, upper );
                break;

            case BUMP_PAGE_UP:
                gtk_adjustment_set_value( act->private_data->adj, base + page );
                break;

            case BUMP_UP:
                gtk_adjustment_set_value( act->private_data->adj, base + step );
                break;

            case BUMP_DOWN:
                gtk_adjustment_set_value( act->private_data->adj, base - step );
                break;

            case BUMP_PAGE_DOWN:
                gtk_adjustment_set_value( act->private_data->adj, base - page );
                break;

            case BUMP_BOTTOM:
                gtk_adjustment_set_value( act->private_data->adj, lower );
                break;

            default:
                if ( what >= BUMP_CUSTOM ) {
                    guint index = what - BUMP_CUSTOM;
                    if ( index < g_list_length( act->private_data->descriptions ) ) {
                        EgeAdjustmentDescr* descr = (EgeAdjustmentDescr*)g_list_nth_data( act->private_data->descriptions, index );
                        if ( descr ) {
                            gtk_adjustment_set_value( act->private_data->adj, descr->value );
                        }
                    }
                }
        }
    }
}

static void create_single_menu_item( GCallback toggleCb, int val, GtkWidget* menu, EgeAdjustmentAction* act, GtkWidget** dst, GSList** group, gdouble num, gboolean active )
{
    char* str = 0;
    EgeAdjustmentDescr* marker = 0;
    GList* cur = act->private_data->descriptions;

    while ( cur ) {
        EgeAdjustmentDescr* descr = (EgeAdjustmentDescr*)cur->data;
        gdouble delta = num - descr->value;
        if ( delta < 0.0 ) {
            delta = -delta;
        }
        if ( delta < act->private_data->epsilon ) {
            marker = descr;
            break;
        }
        cur = g_list_next( cur );
    }

    str = g_strdup_printf( act->private_data->format, num,
                           ((marker && marker->descr) ? ": " : ""),
                           ((marker && marker->descr) ? marker->descr : ""));

    *dst = gtk_radio_menu_item_new_with_label( *group, str );
    if ( !*group) {
        *group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(*dst) );
    }
    if ( active ) {
        gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(*dst), TRUE );
    }
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), *dst );
    g_object_set_qdata( G_OBJECT(*dst), gDataName, act );

    g_signal_connect( G_OBJECT(*dst), "toggled", toggleCb, GINT_TO_POINTER(val) );

    g_free(str);
}

static GList* flush_explicit_items( GList* descriptions,
                                    GCallback toggleCb,
                                    int val,
                                    GtkWidget* menu,
                                    EgeAdjustmentAction* act,
                                    GtkWidget** dst,
                                    GSList** group,
                                    gdouble num )
{
    GList* cur = descriptions;

    if ( cur ) {
        gdouble valUpper = num + act->private_data->epsilon;
        gdouble valLower = num - act->private_data->epsilon;

        EgeAdjustmentDescr* descr = (EgeAdjustmentDescr*)cur->data;

        while ( cur && descr && (descr->value >= valLower) ) {
            if ( descr->value > valUpper ) {
                create_single_menu_item( toggleCb, val + g_list_position(act->private_data->descriptions, cur), menu, act, dst, group, descr->value, FALSE );
            }

            cur = g_list_previous( cur );
            descr = cur ? (EgeAdjustmentDescr*)cur->data : 0;
        }
    }

    return cur;
}

static GtkWidget* create_popup_number_menu( EgeAdjustmentAction* act )
{
    GtkWidget* menu = gtk_menu_new();

    GSList* group = 0;
    GtkWidget* single = 0;

    GList* addOns = g_list_last( act->private_data->descriptions );

    gdouble base = gtk_adjustment_get_value( act->private_data->adj );
    gdouble lower = 0.0;
    gdouble upper = 0.0;
    gdouble step = 0.0;
    gdouble page = 0.0;
    g_object_get( G_OBJECT(act->private_data->adj),
                  "lower", &lower,
                  "upper", &upper,
                  "step-increment", &step,
                  "page-increment", &page,
                  NULL );


    if ( base < upper ) {
        addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, upper );
        create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_TOP, menu, act, &single, &group, upper, FALSE );
        if ( (base + page) < upper ) {
            addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, base + page );
            create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_PAGE_UP, menu, act, &single, &group, base + page, FALSE );
        }
        if ( (base + step) < upper ) {
            addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, base + step );
            create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_UP, menu, act, &single, &group, base + step, FALSE );
        }
    }

    addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, base );
    create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_NONE, menu, act, &single, &group, base, TRUE );

    if ( base > lower ) {
        if ( (base - step) > lower ) {
            addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, base - step );
            create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_DOWN, menu, act, &single, &group, base - step, FALSE );
        }
        if ( (base - page) > lower ) {
            addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, base - page );
            create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_PAGE_DOWN, menu, act, &single, &group, base - page, FALSE );
        }
        addOns = flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, lower );
        create_single_menu_item( G_CALLBACK(process_menu_action), BUMP_BOTTOM, menu, act, &single, &group, lower, FALSE );
    }

    if ( act->private_data->descriptions ) {
        gdouble value = ((EgeAdjustmentDescr*)act->private_data->descriptions->data)->value;
        flush_explicit_items( addOns, G_CALLBACK(process_menu_action), BUMP_CUSTOM, menu, act, &single, &group, value );
    }

    return menu;
}

static GtkWidget* create_menu_item( GtkAction* action )
{
    GtkWidget* item = 0;

    if ( IS_EGE_ADJUSTMENT_ACTION(action) ) {
        EgeAdjustmentAction* act = EGE_ADJUSTMENT_ACTION( action );
        GValue value;
        GtkWidget*  subby = 0;

        memset( &value, 0, sizeof(value) );
        g_value_init( &value, G_TYPE_STRING );
        g_object_get_property( G_OBJECT(action), "label", &value );

        item = gtk_menu_item_new_with_label( g_value_get_string( &value ) );

        subby = create_popup_number_menu( act );
        gtk_menu_item_set_submenu( GTK_MENU_ITEM(item), subby );
        gtk_widget_show_all( subby );
        g_value_unset( &value );
    } else {
        item = GTK_ACTION_CLASS(ege_adjustment_action_parent_class)->create_menu_item( action );
    }

    return item;
}

static void value_changed_cb( GtkSpinButton* spin, EgeAdjustmentAction* act )
{
    if ( gtk_widget_has_focus( GTK_WIDGET(spin) ) ) {
        gint start = 0, end = 0;
        if (GTK_IS_EDITABLE(spin) && gtk_editable_get_selection_bounds (GTK_EDITABLE(spin), &start, &end)
                && start != end) {
            // #167846, #363000 If the spin button has a selection, its probably
            // because we got here from a Tab key from another spin, if so dont defocus
            return;
        }
        ege_adjustment_action_defocus( act );
    }
}

static gboolean event_cb( EgeAdjustmentAction* act, GdkEvent* evt )
{
    gboolean handled = FALSE;
    if ( evt->type == GDK_BUTTON_PRESS ) {
        if ( evt->button.button == 3 ) {
            if ( IS_EGE_ADJUSTMENT_ACTION(act) ) {
                GdkEventButton* btnevt = (GdkEventButton*)evt;
                GtkWidget* menu = create_popup_number_menu(act);
                gtk_widget_show_all( menu );
                gtk_menu_popup( GTK_MENU(menu), NULL, NULL, NULL, NULL, btnevt->button, btnevt->time );
            }
            handled = TRUE;
        }
    }

    return handled;
}

static GtkWidget* create_tool_item( GtkAction* action )
{
    GtkWidget* item = 0;

    if ( IS_EGE_ADJUSTMENT_ACTION(action) ) {
        EgeAdjustmentAction* act = EGE_ADJUSTMENT_ACTION( action );
        GtkWidget* spinbutton = 0;
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget* hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
        GtkWidget* hb = gtk_hbox_new( FALSE, 5 );
#endif
        GValue value;
        memset( &value, 0, sizeof(value) );
        g_value_init( &value, G_TYPE_STRING );
        g_object_get_property( G_OBJECT(action), "short_label", &value );

        if ( act->private_data->appearanceMode == APPEARANCE_FULL ) {
            /* Slider */
            spinbutton  = gimp_spin_scale_new (act->private_data->adj, g_value_get_string( &value ), 0);
            gtk_widget_set_size_request(spinbutton, 100, -1);

        } else if ( act->private_data->appearanceMode == APPEARANCE_MINIMAL ) {
            spinbutton = gtk_scale_button_new( GTK_ICON_SIZE_MENU, 0, 100, 2, 0 );
            gtk_scale_button_set_adjustment( GTK_SCALE_BUTTON(spinbutton), act->private_data->adj );
            gtk_scale_button_set_icons( GTK_SCALE_BUTTON(spinbutton), floogles );
        } else {
            if ( gFactoryCb ) {
                spinbutton = gFactoryCb( act->private_data->adj, act->private_data->climbRate, act->private_data->digits, act->private_data->unitTracker );
            } else {
                spinbutton = gtk_spin_button_new( act->private_data->adj, act->private_data->climbRate, act->private_data->digits );
            }
        }

        item = GTK_WIDGET( gtk_tool_item_new() );

        {
            GValue tooltip;
            memset( &tooltip, 0, sizeof(tooltip) );
            g_value_init( &tooltip, G_TYPE_STRING );
            g_object_get_property( G_OBJECT(action), "tooltip", &tooltip );
            const gchar* tipstr = g_value_get_string( &tooltip );
            if ( tipstr && *tipstr ) {
                gtk_widget_set_tooltip_text( spinbutton, tipstr );
            }
            g_value_unset( &tooltip );
        }

        if ( act->private_data->appearanceMode != APPEARANCE_FULL ) {
            GtkWidget* filler1 = gtk_label_new(" ");
            gtk_box_pack_start( GTK_BOX(hb), filler1, FALSE, FALSE, 0 );

            /* Use an icon if available or use short-label */
            if ( act->private_data->iconId && strcmp( act->private_data->iconId, "" ) != 0 ) {
                GtkWidget* icon = sp_icon_new( act->private_data->iconSize, act->private_data->iconId );
                gtk_box_pack_start( GTK_BOX(hb), icon, FALSE, FALSE, 0 );
            } else {
                GtkWidget* lbl = gtk_label_new( g_value_get_string( &value ) ? g_value_get_string( &value ) : "wwww" );

#if GTK_CHECK_VERSION(3,0,0)
                gtk_widget_set_halign(lbl, GTK_ALIGN_END);
#else
                gtk_misc_set_alignment( GTK_MISC(lbl), 1.0, 0.5 );
#endif

                gtk_box_pack_start( GTK_BOX(hb), lbl, FALSE, FALSE, 0 );
            }
        }

        if ( act->private_data->appearanceMode == APPEARANCE_FULL ) {
            gtk_box_pack_start( GTK_BOX(hb), spinbutton, TRUE, TRUE, 0 );
        }  else {
            gtk_box_pack_start( GTK_BOX(hb), spinbutton, FALSE, FALSE, 0 );
        }

        gtk_container_add( GTK_CONTAINER(item), hb );

        if ( act->private_data->selfId ) {
            g_object_set_data( G_OBJECT(spinbutton), act->private_data->selfId, spinbutton );
        }

        g_signal_connect( G_OBJECT(spinbutton), "focus-in-event", G_CALLBACK(focus_in_cb), action );
        g_signal_connect( G_OBJECT(spinbutton), "focus-out-event", G_CALLBACK(focus_out_cb), action );
        g_signal_connect( G_OBJECT(spinbutton), "key-press-event", G_CALLBACK(keypress_cb), action );

        g_signal_connect( G_OBJECT(spinbutton), "value-changed", G_CALLBACK(value_changed_cb), action );

        g_signal_connect_swapped( G_OBJECT(spinbutton), "event", G_CALLBACK(event_cb), action );
        if ( act->private_data->appearanceMode == APPEARANCE_FULL ) {
            /* */
        } else if ( act->private_data->appearanceMode == APPEARANCE_MINIMAL ) {
            /* */
        } else {
            gtk_entry_set_width_chars( GTK_ENTRY(spinbutton), act->private_data->digits + 3 );
        }

        gtk_widget_show_all( item );

        /* Shrink or whatnot after shown */
        if ( act->private_data->toolPost ) {
            act->private_data->toolPost( item );
        }

        g_value_unset( &value );
    } else {
        item = GTK_ACTION_CLASS(ege_adjustment_action_parent_class)->create_tool_item( action );
    }

    return item;
}

static void connect_proxy( GtkAction *action, GtkWidget *proxy )
{
    GTK_ACTION_CLASS(ege_adjustment_action_parent_class)->connect_proxy( action, proxy );
}

static void disconnect_proxy( GtkAction *action, GtkWidget *proxy )
{
    GTK_ACTION_CLASS(ege_adjustment_action_parent_class)->disconnect_proxy( action, proxy );
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
        if ( GTK_IS_SPIN_BUTTON(widget) ) {
            action->private_data->lastVal = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
        } else if ( GTK_IS_SCALE_BUTTON(widget) ) {
            action->private_data->lastVal = gtk_scale_button_get_value( GTK_SCALE_BUTTON(widget) );
        } else if (GTK_IS_RANGE(widget) ) {
            action->private_data->lastVal = gtk_range_get_value( GTK_RANGE(widget) );
        }
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
                        if ( child && GTK_IS_BOX(child) ) { /* could be ours */
                            GList* subChildren = gtk_container_get_children( GTK_CONTAINER(child) );
                            if ( subChildren ) {
                                GList* last = g_list_last(subChildren);
                                if ( last && GTK_IS_SPIN_BUTTON(last->data) && gtk_widget_is_sensitive( GTK_WIDGET(last->data) ) ) {
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
        case GDK_KEY_Escape:
        {
            action->private_data->transferFocus = TRUE;
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), action->private_data->lastVal );
            ege_adjustment_action_defocus( action );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        {
            action->private_data->transferFocus = TRUE;
            ege_adjustment_action_defocus( action );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_Tab:
        {
            action->private_data->transferFocus = FALSE;
            wasConsumed = process_tab( widget, 1 );
        }
        break;

        case GDK_KEY_ISO_Left_Tab:
        {
            action->private_data->transferFocus = FALSE;
            wasConsumed = process_tab( widget, -1 );
        }
        break;

        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val + action->private_data->step );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val - action->private_data->step );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_Page_Up:
        case GDK_KEY_KP_Page_Up:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val + action->private_data->page );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_Page_Down:
        case GDK_KEY_KP_Page_Down:
        {
            action->private_data->transferFocus = FALSE;
            gdouble val = gtk_spin_button_get_value( GTK_SPIN_BUTTON(widget) );
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), val - action->private_data->page );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_z:
        case GDK_KEY_Z:
        {
            action->private_data->transferFocus = FALSE;
            gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), action->private_data->lastVal );
            wasConsumed = TRUE;
        }
        break;

    }

    return wasConsumed;
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
