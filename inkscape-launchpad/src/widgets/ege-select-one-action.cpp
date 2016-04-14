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
 * The Original Code is EGE Select One Action.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#include <string.h>

#include <gtk/gtk.h>

#include "widgets/ege-select-one-action.h"

enum {
    CHANGED = 0,
    LAST_SIGNAL};

static void ege_select_one_action_finalize( GObject* action );
static void ege_select_one_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec );
static void ege_select_one_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec );

static gint find_text_index(EgeSelectOneAction *act, gchar const* text);
static void commit_pending_change(EgeSelectOneAction *act);
static void resync_active( EgeSelectOneAction* act, gint active, gboolean override );
static void resync_sensitive( EgeSelectOneAction* act );
static void combo_entry_changed_cb( GtkEntry* widget, gpointer user_data );
static gboolean combo_entry_focus_lost_cb( GtkWidget *widget, GdkEventFocus *event, gpointer data );
static void combo_changed_cb( GtkComboBox* widget, gpointer user_data );
static void menu_toggled_cb( GtkWidget* obj, gpointer data );
static void proxy_action_chagned_cb( GtkRadioAction* action, GtkRadioAction* current, gpointer user_data );

static GtkWidget* create_menu_item( GtkAction* action );
static GtkWidget* create_tool_item( GtkAction* action );
static void connect_proxy( GtkAction *action, GtkWidget *proxy );
static void disconnect_proxy( GtkAction *action, GtkWidget *proxy );

static int scan_max_width( GtkTreeModel *model, gint labelColumn );

static guint signals[LAST_SIGNAL] = {0};
static GQuark gDataName = 0;


enum {
    APPEARANCE_UNKNOWN = -1,
    APPEARANCE_NONE = 0,
    APPEARANCE_FULL,    /* label, then all choices represented by separate buttons */
    APPEARANCE_COMPACT, /* label, then choices in a drop-down menu */
    APPEARANCE_MINIMAL, /* no label, just choices in a drop-down menu */
};

enum {
    SELECTION_UNKNOWN = -1,
    SELECTION_CLOSED = 0,
    SELECTION_OPEN,
};

struct _EgeSelectOneActionPrivate
{
    gint active;
    gint labelColumn;
    gint iconColumn;
    gint tooltipColumn;
    gint sensitiveColumn;
    gint appearanceMode;
    gint selectionMode;
    gint iconSize;
    GType radioActionType;
    GtkTreeModel* model;
    gchar *iconProperty;
    gchar *appearance;
    gchar *selection;
    gchar *activeText;
    gchar *pendingText;
};

#define EGE_SELECT_ONE_ACTION_GET_PRIVATE( o ) ( G_TYPE_INSTANCE_GET_PRIVATE( (o), EGE_SELECT_ONE_ACTION_TYPE, EgeSelectOneActionPrivate ) )

enum {
    PROP_MODEL = 1,
    PROP_ACTIVE,
    PROP_LABEL_COLUMN,
    PROP_ICON_COLUMN,
    PROP_TOOLTIP_COLUMN,
    PROP_SENSITIVE_COLUMN,
    PROP_ICON_PROP,
    PROP_ICON_SIZE,
    PROP_APPEARANCE,
    PROP_SELECTION
};

G_DEFINE_TYPE(EgeSelectOneAction, ege_select_one_action, GTK_TYPE_ACTION);

GtkTreeModel *ege_select_one_action_get_model(EgeSelectOneAction* action ){
    return GTK_TREE_MODEL(action->private_data->model);
}
void ege_select_one_action_class_init( EgeSelectOneActionClass* klass )
{
    if ( klass ) {
        GObjectClass* objClass = G_OBJECT_CLASS( klass );

        gDataName = g_quark_from_string("ege-select1-action");

        objClass->finalize = ege_select_one_action_finalize;
        objClass->get_property = ege_select_one_action_get_property;
        objClass->set_property = ege_select_one_action_set_property;

        klass->parent_class.create_menu_item = create_menu_item;
        klass->parent_class.create_tool_item = create_tool_item;
        klass->parent_class.connect_proxy = connect_proxy;
        klass->parent_class.disconnect_proxy = disconnect_proxy;

        g_object_class_install_property( objClass,
                                         PROP_MODEL,
                                         g_param_spec_object( "model",
                                                              "Tree Model",
                                                              "Tree model of possible items",
                                                              GTK_TYPE_TREE_MODEL,
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_ACTIVE,
                                         g_param_spec_int( "active",
                                                           "Active Selection",
                                                           "The index of the selected item",
                                                           -1, G_MAXINT, 0,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_LABEL_COLUMN,
                                         g_param_spec_int( "label-column",
                                                           "Display Column",
                                                           "The column of the model that holds display strings",
                                                           0, G_MAXINT, 0,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_ICON_COLUMN,
                                         g_param_spec_int( "icon-column",
                                                           "Icon Column",
                                                           "The column of the model that holds display icon name",
                                                           -1, G_MAXINT, -1,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_TOOLTIP_COLUMN,
                                         g_param_spec_int( "tooltip-column",
                                                           "Tooltip Column",
                                                          "The column of the model that holds tooltip strings",
                                                           -1, G_MAXINT, -1,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_SENSITIVE_COLUMN,
                                         g_param_spec_int( "sensitive-column",
                                                           "Sensitive Column",
                                                          "The column of the model that holds sensitive state",
                                                           -1, G_MAXINT, -1,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_ICON_PROP,
                                         g_param_spec_string( "icon-property",
                                                              "Icon Property",
                                                              "Target icon property",
                                                              "",
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_ICON_SIZE,
                                         g_param_spec_int( "icon-size",
                                                           "Icon Size",
                                                          "Target icon size",
                                                           -1, G_MAXINT, -1,
                                                           (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_APPEARANCE,
                                         g_param_spec_string( "appearance",
                                                              "Appearance hint",
                                                              "A hint for how to display",
                                                              "",
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_object_class_install_property( objClass,
                                         PROP_SELECTION,
                                         g_param_spec_string( "selection",
                                                              "Selection set open or closed",
                                                              "'open' to allow edits/additions, 'closed' to disallow.",
                                                              "",
                                                              (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        signals[CHANGED] = g_signal_new( "changed",
                                         G_TYPE_FROM_CLASS(klass),
                                         G_SIGNAL_RUN_FIRST,
                                         G_STRUCT_OFFSET(EgeSelectOneActionClass, changed),
                                         NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0);

        g_type_class_add_private( klass, sizeof(EgeSelectOneActionClass) );
    }
}


void ege_select_one_action_init( EgeSelectOneAction* action )
{
    action->private_data = EGE_SELECT_ONE_ACTION_GET_PRIVATE( action );
    action->private_data->active = 0;
    action->private_data->labelColumn = 0;
    action->private_data->iconColumn = -1;
    action->private_data->tooltipColumn = -1;
    action->private_data->sensitiveColumn = -1;
    action->private_data->appearanceMode = APPEARANCE_NONE;
    action->private_data->selectionMode = SELECTION_CLOSED;
    action->private_data->radioActionType = 0;
    action->private_data->model = 0;
    action->private_data->iconProperty = g_strdup("stock-id");
    action->private_data->iconSize = -1;
    action->private_data->appearance = 0;
    action->private_data->selection = 0;
    action->private_data->activeText = 0;
    action->private_data->pendingText = 0;

/*     g_signal_connect( action, "notify", G_CALLBACK( fixup_labels ), NULL ); */
}

void ege_select_one_action_finalize( GObject* object )
{
    EgeSelectOneAction *action = EGE_SELECT_ONE_ACTION( object );

    g_free( action->private_data->iconProperty );
    g_free( action->private_data->appearance );
    g_free( action->private_data->selection );

    if ( G_OBJECT_CLASS(ege_select_one_action_parent_class)->finalize ) {
        (*G_OBJECT_CLASS(ege_select_one_action_parent_class)->finalize)(object);
    }
}

EgeSelectOneAction* ege_select_one_action_new( const gchar *name,
                                               const gchar *label,
                                               const gchar *tooltip,
                                               const gchar *stock_id,
                                               GtkTreeModel* model )
{
    GObject* obj = (GObject*)g_object_new( EGE_SELECT_ONE_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "stock_id", stock_id,
                                           "model", model,
                                           "active", 0,
                                           "icon-property", "stock-id",
                                           NULL );

    EgeSelectOneAction* action = EGE_SELECT_ONE_ACTION( obj );

    return action;
}


gint ege_select_one_action_get_active( EgeSelectOneAction* action )
{
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );
    return action->private_data->active;
}

gchar *ege_select_one_action_get_active_text( EgeSelectOneAction* action )
{
    GtkTreeIter iter;
    gchar *str = 0;
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );

    if ( action->private_data->active >= 0) {
        if ( gtk_tree_model_iter_nth_child( action->private_data->model, &iter, NULL, action->private_data->active ) ) {
            gtk_tree_model_get( action->private_data->model, &iter,
                                action->private_data->labelColumn, &str,
                                -1 );
        }
    } else if ( (action->private_data->active == -1) && action->private_data->activeText ) {
        str = g_strdup(action->private_data->activeText);
    }

    return str;
}

void ege_select_one_action_set_active_text( EgeSelectOneAction* action, gchar const *text )
{
    g_return_if_fail( IS_EGE_SELECT_ONE_ACTION(action) );

    if (action->private_data->activeText) {
        g_free( action->private_data->activeText );
    }
    action->private_data->activeText = g_strdup(text);

    if (action->private_data->active != -1) {
        g_object_set( G_OBJECT(action), "active", -1, NULL );
    } else {
        resync_active( action, -1, TRUE );
    }
}

void ege_select_one_action_set_active( EgeSelectOneAction* action, gint val )
{
    g_object_set( G_OBJECT(action), "active", val, NULL );
}

void ege_select_one_action_update_sensitive( EgeSelectOneAction* action )
{
    if( action->private_data->sensitiveColumn < 0 ) {
        g_warning( "ege_select_one_action: Attempt to update sensitivity of item without sensitive column\n" );
        return;
    }
    resync_sensitive( action );
}

gint ege_select_one_action_get_label_column( EgeSelectOneAction* action )
{
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );
    return action->private_data->labelColumn;
}

void ege_select_one_action_set_label_column( EgeSelectOneAction* action, gint col )
{
    g_object_set( G_OBJECT(action), "label-column", col, NULL );
}

gint ege_select_one_action_get_icon_column( EgeSelectOneAction* action )
{
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );
    return action->private_data->iconColumn;
}

void ege_select_one_action_set_icon_column( EgeSelectOneAction* action, gint col )
{
    g_object_set( G_OBJECT(action), "icon-column", col, NULL );
}

gint ege_select_one_action_get_icon_size( EgeSelectOneAction* action )
{
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );
    return action->private_data->iconSize;
}

void ege_select_one_action_set_icon_size( EgeSelectOneAction* action, gint size )
{
    g_object_set( G_OBJECT(action), "icon-size", size, NULL );
}

gint ege_select_one_action_get_tooltip_column( EgeSelectOneAction* action )
{
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );
    return action->private_data->tooltipColumn;
}

void ege_select_one_action_set_tooltip_column( EgeSelectOneAction* action, gint col )
{
    g_object_set( G_OBJECT(action), "tooltip-column", col, NULL );
}

gint ege_select_one_action_get_sensitive_column( EgeSelectOneAction* action )
{
    g_return_val_if_fail( IS_EGE_SELECT_ONE_ACTION(action), 0 );
    return action->private_data->sensitiveColumn;
}

void ege_select_one_action_set_sensitive_column( EgeSelectOneAction* action, gint col )
{
    g_object_set( G_OBJECT(action), "sensitive-column", col, NULL );
}

void ege_select_one_action_set_appearance( EgeSelectOneAction* action, gchar const* val )
{
    g_object_set( G_OBJECT(action), "appearance", val, NULL );
}

void ege_select_one_action_set_selection( EgeSelectOneAction* action, gchar const* val )
{
    g_object_set( G_OBJECT(action), "selection", val, NULL );
}

void ege_select_one_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec )
{
    EgeSelectOneAction* action = EGE_SELECT_ONE_ACTION( obj );
    switch ( propId ) {
        case PROP_MODEL:
            g_value_set_object( value, action->private_data->model );
            break;

        case PROP_ACTIVE:
            g_value_set_int( value, action->private_data->active );
            break;

        case PROP_LABEL_COLUMN:
            g_value_set_int( value, action->private_data->labelColumn );
            break;

        case PROP_ICON_COLUMN:
            g_value_set_int( value, action->private_data->iconColumn );
            break;

        case PROP_TOOLTIP_COLUMN:
            g_value_set_int( value, action->private_data->tooltipColumn );
            break;

        case PROP_SENSITIVE_COLUMN:
            g_value_set_int( value, action->private_data->sensitiveColumn );
            break;

        case PROP_ICON_PROP:
            g_value_set_string( value, action->private_data->iconProperty );
            break;

        case PROP_ICON_SIZE:
            g_value_set_int( value, action->private_data->iconSize );
            break;

        case PROP_APPEARANCE:
            g_value_set_string( value, action->private_data->appearance );
            break;

        case PROP_SELECTION:
            g_value_set_string( value, action->private_data->selection );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

void ege_select_one_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec )
{
    EgeSelectOneAction* action = EGE_SELECT_ONE_ACTION( obj );
    switch ( propId ) {
        case PROP_MODEL:
        {
            action->private_data->model = GTK_TREE_MODEL( g_value_get_object( value ) );
        }
        break;

        case PROP_ACTIVE:
        {
            resync_active( action, g_value_get_int( value ), FALSE );
        }
        break;

        case PROP_LABEL_COLUMN:
        {
            action->private_data->labelColumn = g_value_get_int( value );
        }
        break;

        case PROP_ICON_COLUMN:
        {
            action->private_data->iconColumn = g_value_get_int( value );
        }
        break;

        case PROP_TOOLTIP_COLUMN:
        {
            action->private_data->tooltipColumn = g_value_get_int( value );
        }
        break;

        case PROP_SENSITIVE_COLUMN:
        {
            action->private_data->sensitiveColumn = g_value_get_int( value );
        }
        break;

        case PROP_ICON_PROP:
        {
            gchar* tmp = action->private_data->iconProperty;
            gchar* newVal = g_value_dup_string( value );
            action->private_data->iconProperty = newVal;
            g_free( tmp );
        }
        break;

        case PROP_ICON_SIZE:
        {
            action->private_data->iconSize = g_value_get_int( value );
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

        case PROP_SELECTION:
        {
            gchar* tmp = action->private_data->selection;
            gchar* newVal = g_value_dup_string( value );
            action->private_data->selection = newVal;
            g_free( tmp );

            if ( !action->private_data->selection || (strcmp("closed", newVal) == 0) ) {
                action->private_data->selectionMode = SELECTION_CLOSED;
            } else if ( strcmp("open", newVal) == 0 ) {
                action->private_data->selectionMode = SELECTION_OPEN;
            } else {
                action->private_data->selectionMode = SELECTION_UNKNOWN;
            }
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

GtkWidget* create_menu_item( GtkAction* action )
{
    GtkWidget* item = 0;

    if ( IS_EGE_SELECT_ONE_ACTION(action) ) {
        EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION( action );
        gchar*  sss = 0;
        gint index = 0;
        GtkTreeIter iter;
        GSList* group = 0;
        GtkWidget* subby = gtk_menu_new();

        g_object_get( G_OBJECT(action), "label", &sss, NULL );

        item = gtk_menu_item_new_with_label( sss );

        gboolean valid = gtk_tree_model_get_iter_first( act->private_data->model, &iter );
        while ( valid ) {
            gchar* str = 0;
            gtk_tree_model_get( act->private_data->model, &iter,
                                act->private_data->labelColumn, &str,
                                -1 );

            GtkWidget *item = gtk_radio_menu_item_new_with_label( group, str );
            group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(item) );
            gtk_menu_shell_append( GTK_MENU_SHELL(subby), item );
            g_object_set_qdata( G_OBJECT(item), gDataName, act );

            gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(item), index == act->private_data->active );

            g_free(str);
            str = 0;

            g_signal_connect( G_OBJECT(item), "toggled", G_CALLBACK(menu_toggled_cb), GINT_TO_POINTER(index) );

            index++;
            valid = gtk_tree_model_iter_next( act->private_data->model, &iter );
        }

        gtk_menu_item_set_submenu( GTK_MENU_ITEM(item), subby );
        gtk_widget_show_all( subby );

        g_free(sss);
    } else {
        item = GTK_ACTION_CLASS(ege_select_one_action_parent_class)->create_menu_item( action );
    }

    return item;
}


void ege_select_one_action_set_radio_action_type( EgeSelectOneAction* action, GType radioActionType )
{
    (void)action;

    if ( g_type_is_a( radioActionType, GTK_TYPE_RADIO_ACTION ) ) {
        action->private_data->radioActionType = radioActionType;
    } else {
        g_warning("Passed in type '%s' is not derived from '%s'", g_type_name(radioActionType), g_type_name(GTK_TYPE_RADIO_ACTION) );
    }
}

GtkWidget* create_tool_item( GtkAction* action )
{
    GtkWidget* item = 0;

    if ( IS_EGE_SELECT_ONE_ACTION(action) && EGE_SELECT_ONE_ACTION(action)->private_data->model )
    {
        EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION(action);
        item = GTK_WIDGET( gtk_tool_item_new() );

        if ( act->private_data->appearanceMode == APPEARANCE_FULL ) {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget* holder = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_set_homogeneous(GTK_BOX(holder), FALSE);
#else
            GtkWidget* holder = gtk_hbox_new( FALSE, 0 );
#endif

            GtkRadioAction* ract = 0;
            GSList* group = 0;
            GtkTreeIter iter;
            gboolean valid = FALSE;
            gint index = 0;

            {
                gchar*  sss = 0;
                g_object_get( G_OBJECT(action), "short_label", &sss, NULL );
                // If short_label not defined, g_object_get will return label.
                // This hack allows a label to be used with a drop-down menu when
                // no label is used with a set of icons that are self-explanatory.
                if (sss && strcmp( sss, "NotUsed" ) != 0 ) {
                    GtkWidget* lbl = gtk_label_new(sss);
                    gtk_box_pack_start( GTK_BOX(holder), lbl, FALSE, FALSE, 4 );
                }
                g_free( sss );
                sss = 0;
            }

            valid = gtk_tree_model_get_iter_first( act->private_data->model, &iter );
            while ( valid ) {
                gchar* str = 0;
                gchar* tip = 0;
                gchar* iconId = 0;
                gboolean sens = true;
                /*
                gint size = 0;
                */
                gtk_tree_model_get( act->private_data->model, &iter,
                                    act->private_data->labelColumn, &str,
                                    -1 );
                if ( act->private_data->iconColumn >= 0 ) {
                    gtk_tree_model_get( act->private_data->model, &iter,
                                        act->private_data->iconColumn, &iconId,
                                        -1 );
                }
                if ( act->private_data->tooltipColumn >= 0 ) {
                    gtk_tree_model_get( act->private_data->model, &iter,
                                        act->private_data->tooltipColumn, &tip,
                                        -1 );
                }
                if ( act->private_data->sensitiveColumn >= 0 ) {
                    gtk_tree_model_get( act->private_data->model, &iter,
                                        act->private_data->sensitiveColumn, &sens,
                                        -1 );
                }

                if ( act->private_data->radioActionType ) {
                    void* obj = g_object_new( act->private_data->radioActionType,
                                              "name", "Name 1",
                                              "label", str,
                                              "tooltip", tip,
                                              "value", index,
                                              /*
                                              "iconId", iconId,
                                              "iconSize", size,
                                              */
                                              NULL );
                    if ( iconId ) {
                        g_object_set( G_OBJECT(obj), act->private_data->iconProperty, iconId, NULL );
                    }

                    if ( act->private_data->iconProperty ) {
                        /* TODO get this string to be set instead of hardcoded */
                        if ( act->private_data->iconSize >= 0 ) {
                            g_object_set( G_OBJECT(obj), "iconSize", act->private_data->iconSize, NULL );
                        }
                    }

                    ract = GTK_RADIO_ACTION(obj);
                } else {
                    ract = gtk_radio_action_new( "Name 1", str, tip, iconId, index );
                }

                if ( act->private_data->sensitiveColumn >= 0 ) {
                    gtk_action_set_sensitive( GTK_ACTION(ract), sens );
                }

                gtk_radio_action_set_group( ract, group );
                group = gtk_radio_action_get_group( ract );

                if ( index == act->private_data->active ) {
                    gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(ract), TRUE );
                }
                g_signal_connect( G_OBJECT(ract), "changed", G_CALLBACK( proxy_action_chagned_cb ), act );

                GtkWidget* sub = gtk_action_create_tool_item( GTK_ACTION(ract) );
                gtk_activatable_set_related_action( GTK_ACTIVATABLE (sub), GTK_ACTION(ract) );
                gtk_tool_item_set_tooltip_text( GTK_TOOL_ITEM(sub), tip );

                gtk_box_pack_start( GTK_BOX(holder), sub, FALSE, FALSE, 0 );

                g_free( str );
                g_free( tip );
                g_free( iconId );

                index++;
                valid = gtk_tree_model_iter_next( act->private_data->model, &iter );
            }

            g_object_set_data( G_OBJECT(holder), "ege-proxy_action-group", group );

            gtk_container_add( GTK_CONTAINER(item), holder );
        } else {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget* holder = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            gtk_box_set_homogeneous(GTK_BOX(holder), FALSE);
#else
            GtkWidget *holder = gtk_hbox_new( FALSE, 4 );
#endif

            GtkEntry *entry = 0;
            GtkWidget *normal;

            if (act->private_data->selectionMode == SELECTION_OPEN) {
	            normal = gtk_combo_box_new_with_model_and_entry (act->private_data->model);
	            gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (normal), act->private_data->labelColumn);

                GtkWidget *child = gtk_bin_get_child( GTK_BIN(normal) );
                if (GTK_IS_ENTRY(child)) {
                    int maxUsed = scan_max_width( act->private_data->model, act->private_data->labelColumn );
                    GtkEntryCompletion *complete = 0;
                    entry = GTK_ENTRY(child);
                    gtk_entry_set_width_chars(entry, maxUsed); /* replace with property */

                    complete = gtk_entry_completion_new();
                    gtk_entry_completion_set_model( complete, act->private_data->model );
                    gtk_entry_completion_set_text_column( complete, act->private_data->labelColumn );
                    gtk_entry_completion_set_inline_completion( complete, FALSE );
                    gtk_entry_completion_set_inline_selection( complete, FALSE );
                    gtk_entry_completion_set_popup_completion( complete, TRUE );
                    gtk_entry_completion_set_popup_set_width( complete, FALSE );
                    gtk_entry_set_completion( entry, complete );

                    g_signal_connect( G_OBJECT(child), "activate", G_CALLBACK(combo_entry_changed_cb), act );
                    g_signal_connect( G_OBJECT(child), "focus-out-event", G_CALLBACK(combo_entry_focus_lost_cb), act );
                }
            } 
	    else {
                GtkCellRenderer * renderer = NULL;
                normal = gtk_combo_box_new_with_model( act->private_data->model );
                if ( act->private_data->iconColumn >= 0 ) {
                    renderer = gtk_cell_renderer_pixbuf_new();
                    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT(normal), renderer, TRUE );

                    /* "icon-name" */
                    gtk_cell_layout_add_attribute( GTK_CELL_LAYOUT(normal), renderer, "stock-id", act->private_data->iconColumn );
                }

                renderer = gtk_cell_renderer_text_new();
                gtk_cell_layout_pack_start( GTK_CELL_LAYOUT(normal), renderer, TRUE );
                gtk_cell_layout_add_attribute( GTK_CELL_LAYOUT(normal), renderer, "text", act->private_data->labelColumn );
            }

            gtk_combo_box_set_active( GTK_COMBO_BOX(normal), act->private_data->active );
            if ( entry && (act->private_data->active == -1) ) {
                gtk_entry_set_text( entry, act->private_data->activeText );
            }

            g_signal_connect( G_OBJECT(normal), "changed", G_CALLBACK(combo_changed_cb), action );

            g_object_set_data( G_OBJECT(holder), "ege-combo-box", normal );
            g_object_set_data( G_OBJECT(act), "ege-combo-box", normal );

            if (act->private_data->appearanceMode == APPEARANCE_COMPACT) {
                gchar*  sss = 0;
                g_object_get( G_OBJECT(action), "short_label", &sss, NULL );
                if (sss) {
                    GtkWidget* lbl = gtk_label_new(sss);
                    gtk_box_pack_start( GTK_BOX(holder), lbl, FALSE, FALSE, 4 );
                    g_free( sss );
                    sss = 0;
                }
            }

            gtk_box_pack_start( GTK_BOX(holder), normal, FALSE, FALSE, 0 );

            {
#if GTK_CHECK_VERSION(3,0,0)
                gtk_widget_set_halign(holder, GTK_ALIGN_START);
                gtk_container_add(GTK_CONTAINER(item), holder);
#else
                GtkWidget *align = gtk_alignment_new(0, 0.5, 0, 0);
                gtk_container_add( GTK_CONTAINER(align), holder);
                gtk_container_add( GTK_CONTAINER(item), align );
#endif
            }
        }

        gtk_widget_show_all( item );
    } else {
        item = GTK_ACTION_CLASS(ege_select_one_action_parent_class)->create_tool_item( action );
    }

    return item;
}


void connect_proxy( GtkAction *action, GtkWidget *proxy )
{
    GTK_ACTION_CLASS(ege_select_one_action_parent_class)->connect_proxy( action, proxy );
}

void disconnect_proxy( GtkAction *action, GtkWidget *proxy )
{
    GTK_ACTION_CLASS(ege_select_one_action_parent_class)->disconnect_proxy( action, proxy );
}


void resync_active( EgeSelectOneAction* act, gint active, gboolean override )
{
    if ( override || (act->private_data->active != active) ) {
        act->private_data->active = active;
        GSList* proxies = gtk_action_get_proxies( GTK_ACTION(act) );
        while ( proxies ) {
            if ( GTK_IS_TOOL_ITEM(proxies->data) ) {
                /* Search for the things we built up in create_tool_item() */
                GList* children = gtk_container_get_children( GTK_CONTAINER(proxies->data) );
                if ( children && children->data ) {
                    gpointer combodata = g_object_get_data( G_OBJECT(children->data), "ege-combo-box" );

#if !GTK_CHECK_VERSION(3,0,0)
                    if (!combodata && GTK_IS_ALIGNMENT(children->data)) {
                        GList *other = gtk_container_get_children( GTK_CONTAINER(children->data) );
                         combodata = g_object_get_data( G_OBJECT(other->data), "ege-combo-box" );
                    }
#endif

                    if ( GTK_IS_COMBO_BOX(combodata) ) {
                        GtkComboBox* combo = GTK_COMBO_BOX(combodata);
                        if ((active == -1) && (gtk_combo_box_get_has_entry(combo))) {
                            gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo))), act->private_data->activeText);
                        } else if ( gtk_combo_box_get_active(combo) != active ) {
                            gtk_combo_box_set_active( combo, active );
                        }
                    } else if ( GTK_IS_BOX(children->data) ) {
                        gpointer data = g_object_get_data( G_OBJECT(children->data), "ege-proxy_action-group" );
                        if ( data ) {
                            GSList* group = (GSList*)data;
                            GtkRadioAction* oneAction = GTK_RADIO_ACTION(group->data);
                            gint hot = gtk_radio_action_get_current_value( oneAction );
                            if ( hot != active ) {
                                /*gtk_radio_action_set_current_value( oneAction, active );*/
                                gint value = 0;
                                while ( group ) {
                                    GtkRadioAction* possible = GTK_RADIO_ACTION(group->data);
                                    g_object_get( G_OBJECT(possible), "value", &value, NULL );
                                    if ( value == active ) {
                                        /* Found the group member to set active */
                                        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(possible), TRUE );
                                        break;
                                    }

                                    group = g_slist_next(group);
                                }
                            }
                        }
                    }
                }
            } else if ( GTK_IS_MENU_ITEM(proxies->data) ) {
                GtkWidget* subMenu = gtk_menu_item_get_submenu( GTK_MENU_ITEM(proxies->data) );
                GList* children = gtk_container_get_children( GTK_CONTAINER(subMenu) );
                if ( children && (g_list_length(children) > (guint)active) ) {
                    gpointer data = g_list_nth_data( children, active );
                    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(data), TRUE );
                }
            }

            proxies = g_slist_next( proxies );
        }

        g_signal_emit( G_OBJECT(act), signals[CHANGED], 0);
    }
}

void resync_sensitive( EgeSelectOneAction* act )
{
    GSList* proxies = gtk_action_get_proxies( GTK_ACTION(act) );
    while ( proxies ) {
        if ( GTK_IS_TOOL_ITEM(proxies->data) ) {
            /* Search for the things we built up in create_tool_item() */
            GList* children = gtk_container_get_children( GTK_CONTAINER(proxies->data) );
            if ( children && children->data ) {
                gpointer combodata = g_object_get_data( G_OBJECT(children->data), "ege-combo-box" );

#if !GTK_CHECK_VERSION(3,0,0)
                if (!combodata && GTK_IS_ALIGNMENT(children->data)) {
                    GList *other = gtk_container_get_children( GTK_CONTAINER(children->data) );
                    combodata = g_object_get_data( G_OBJECT(other->data), "ege-combo-box" );
                }
#endif

                if ( GTK_IS_COMBO_BOX(combodata) ) {
                    /* Not implemented */
                } else if ( GTK_IS_BOX(children->data) ) {
                    gpointer data = g_object_get_data( G_OBJECT(children->data), "ege-proxy_action-group" );
                    if ( data ) {
                        GSList* group = (GSList*)data;
                        // List is backwards in group as compared to GtkTreeModel, we better do matching.
                        while ( group ) {
                            GtkRadioAction* ract = GTK_RADIO_ACTION(group->data);
                            const gchar* label = gtk_action_get_label( GTK_ACTION( ract ) );

                            // Search for matching GtkTreeModel entry
                            GtkTreeIter iter;
                            gboolean valid;
                            valid = gtk_tree_model_get_iter_first( act->private_data->model, &iter );
                            gboolean sens = true;
 
                            while( valid ) {

                                gchar* str = 0;
                                gtk_tree_model_get( act->private_data->model, &iter,
                                                    act->private_data->labelColumn, &str,
                                                    -1 );

                                if( strcmp( label, str ) == 0 ) {
                                    gtk_tree_model_get( act->private_data->model, &iter,
                                                        act->private_data->sensitiveColumn, &sens,
                                                        -1 );
                                    break;
                                }
                                g_free( str );

                                valid = gtk_tree_model_iter_next( act->private_data->model, &iter );
                            }

                            gtk_action_set_sensitive( GTK_ACTION(ract), sens );
                            group = g_slist_next(group);
                        }
                    }
                }
            }
        } else if ( GTK_IS_MENU_ITEM(proxies->data) ) {
            /* Not implemented */
        }

        proxies = g_slist_next( proxies );
    }

    g_signal_emit( G_OBJECT(act), signals[CHANGED], 0);
}

void combo_changed_cb( GtkComboBox* widget, gpointer user_data )
{
    EgeSelectOneAction *act = EGE_SELECT_ONE_ACTION(user_data);
    gchar              *text = 0;
    GtkComboBox        *cb = GTK_COMBO_BOX (widget);
    gint                newActive = gtk_combo_box_get_active(widget);

    if (gtk_combo_box_get_has_entry (cb)) {
	    GtkBin   *bin = GTK_BIN (cb);
	    GtkEntry *entry = GTK_ENTRY (gtk_bin_get_child (bin));

	    text = g_strdup (gtk_entry_get_text (entry));
    }
    else {
	    GtkTreeIter  iter;

	    if (gtk_combo_box_get_active_iter (cb, &iter)) {
		    GtkTreeModel *model = gtk_combo_box_get_model (cb);

		    gtk_tree_model_get (model, &iter, 0, &text, -1);
	    }
    }

    if (!text) {
        /* User probably deleted the data in the model */
        return;
    }

    if (newActive == -1) {
        /* indicates the user is entering text for a custom aka "open" value */
        if (act->private_data->pendingText && text && (strcmp(act->private_data->pendingText, text) == 0) ) {
            /* The currently entered data matches the last seen */
        } else {
            if (act->private_data->pendingText) {
                g_free(act->private_data->pendingText);
            }
            act->private_data->pendingText = text;
            text = 0;
        }
    } else if (newActive != act->private_data->active) {
        if (act->private_data->pendingText) {
            g_free(act->private_data->pendingText);
            act->private_data->pendingText = 0;
        }
        g_object_set( G_OBJECT(act), "active", newActive, NULL );
    }

    if (text) {
        g_free(text);
        text = 0;
    }
}

gboolean combo_entry_focus_lost_cb( GtkWidget *widget, GdkEventFocus *event, gpointer data )
{
    EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION(data);
    (void)widget;
    (void)event;

    commit_pending_change(act);

    return FALSE;
}

void combo_entry_changed_cb( GtkEntry* widget, gpointer user_data )
{
    EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION(user_data);
    (void)widget;
    commit_pending_change(act);
}

void commit_pending_change(EgeSelectOneAction *act)
{
    if (act->private_data->pendingText) {
        if (act->private_data->activeText && (strcmp(act->private_data->pendingText, act->private_data->activeText) == 0)) {
            /* Was the same value */
            g_free(act->private_data->pendingText);
            act->private_data->pendingText = 0;
        } else {
            gint matching = find_text_index(act, act->private_data->pendingText);

            if (act->private_data->activeText) {
                g_free(act->private_data->activeText);
            }
            act->private_data->activeText = act->private_data->pendingText;
            act->private_data->pendingText = 0;

            if (matching >= 0) {
                g_free(act->private_data->activeText);
                act->private_data->activeText = 0;
                g_object_set( G_OBJECT(act), "active", matching, NULL );
            } else if (act->private_data->active != -1) {
                g_object_set( G_OBJECT(act), "active", -1, NULL );
            } else {
                resync_active( act, -1, TRUE );
            }
        }
    }
}

gint find_text_index(EgeSelectOneAction *act, gchar const* text)
{
    gint index = -1;

    if (text) {
        GtkTreeIter iter;
        gboolean valid = gtk_tree_model_get_iter_first( act->private_data->model, &iter );
        gint curr = 0;
        while ( valid && (index < 0) ) {
            gchar* str = 0;
            gtk_tree_model_get( act->private_data->model, &iter,
                                act->private_data->labelColumn, &str,
                                -1 );

            if (str && (strcmp(text, str) == 0)) {
                index = curr;
            }

            g_free(str);
            str = 0;

            curr++;
            valid = gtk_tree_model_iter_next( act->private_data->model, &iter );
        }
    }

    return index;
}

void menu_toggled_cb( GtkWidget* obj, gpointer data )
{
    GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM(obj);
    EgeSelectOneAction* act = (EgeSelectOneAction*)g_object_get_qdata( G_OBJECT(obj), gDataName );
    gint newActive = GPOINTER_TO_INT(data);
    if ( gtk_check_menu_item_get_active(item) && (newActive != act->private_data->active) ) {
        g_object_set( G_OBJECT(act), "active", newActive, NULL );
    }
}

void proxy_action_chagned_cb( GtkRadioAction* action, GtkRadioAction* current, gpointer user_data )
{
    (void)current;
    if ( gtk_toggle_action_get_active( GTK_TOGGLE_ACTION(action) ) ) {
        EgeSelectOneAction* act = EGE_SELECT_ONE_ACTION(user_data);
        gint newActive = gtk_radio_action_get_current_value( action );
        if ( newActive != act->private_data->active ) {
            g_object_set( G_OBJECT(act), "active", newActive, NULL );
        }
    }
}

int scan_max_width( GtkTreeModel *model, gint labelColumn )
{
    int maxUsed = 0;
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first( model, &iter );
    while ( valid ) {
        gchar* str = NULL;
        gtk_tree_model_get( model, &iter, labelColumn, &str, -1 );
        if (str != NULL){
            int count = strlen(str);
            if (count > maxUsed) {
                maxUsed = count;
            }
            g_free(str);
        }
        valid = gtk_tree_model_iter_next( model, &iter );
    }
    return maxUsed;
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
