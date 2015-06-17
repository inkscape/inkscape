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
 * The Original Code is EGE Output Action.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2007
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

#include "widgets/ege-output-action.h"


static void ege_output_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec );
static void ege_output_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec );
static void fixup_labels( GObject *gobject, GParamSpec *arg1, gpointer user_data );

/* static GtkWidget* create_menu_item( GtkAction* action ); */
static GtkWidget* create_tool_item( GtkAction* action );

struct _EgeOutputActionPrivate
{
    gboolean useMarkup;
};

#define EGE_OUTPUT_ACTION_GET_PRIVATE( o ) ( G_TYPE_INSTANCE_GET_PRIVATE( (o), EGE_OUTPUT_ACTION_TYPE, EgeOutputActionPrivate ) )

enum {
    PROP_USE_MARKUP = 1,
};

G_DEFINE_TYPE(EgeOutputAction, ege_output_action, GTK_TYPE_ACTION);

void ege_output_action_class_init( EgeOutputActionClass* klass )
{
    if ( klass ) {
        GObjectClass* objClass = G_OBJECT_CLASS( klass );

        objClass->get_property = ege_output_action_get_property;
        objClass->set_property = ege_output_action_set_property;

/*         klass->parent_class.create_menu_item = create_menu_item; */
        klass->parent_class.create_tool_item = create_tool_item;

        g_object_class_install_property( objClass,
                                         PROP_USE_MARKUP,
                                         g_param_spec_boolean( "use-markup",
                                                               "UseMarkup",
                                                               "If markup should be used",
                                                               FALSE,
                                                               (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

        g_type_class_add_private( klass, sizeof(EgeOutputActionClass) );
    }
}


void ege_output_action_init( EgeOutputAction* action )
{
    action->private_data = EGE_OUTPUT_ACTION_GET_PRIVATE( action );
    action->private_data->useMarkup = FALSE;

    g_signal_connect( action, "notify", G_CALLBACK( fixup_labels ), NULL );
}

EgeOutputAction* ege_output_action_new( const gchar *name,
                                        const gchar *label,
                                        const gchar *tooltip,
                                        const gchar *stock_id )
{
    GObject* obj = (GObject*)g_object_new( EGE_OUTPUT_ACTION_TYPE,
                                           "name", name,
                                           "label", label,
                                           "tooltip", tooltip,
                                           "stock_id", stock_id,
                                           "use-markup", FALSE,
                                           NULL );

    EgeOutputAction* action = EGE_OUTPUT_ACTION( obj );

    return action;
}

gboolean ege_output_action_get_use_markup( EgeOutputAction* action )
{
    g_return_val_if_fail( IS_EGE_OUTPUT_ACTION(action), FALSE );

    return action->private_data->useMarkup;
}

void ege_output_action_set_use_markup( EgeOutputAction* action, gboolean setting )
{
    g_object_set( G_OBJECT(action), "use-markup", setting, NULL );
}

void ege_output_action_get_property( GObject* obj, guint propId, GValue* value, GParamSpec * pspec )
{
    EgeOutputAction* action = EGE_OUTPUT_ACTION( obj );
    switch ( propId ) {
        case PROP_USE_MARKUP:
            g_value_set_boolean( value, action->private_data->useMarkup );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}

void ege_output_action_set_property( GObject* obj, guint propId, const GValue *value, GParamSpec* pspec )
{
    EgeOutputAction* action = EGE_OUTPUT_ACTION( obj );
    switch ( propId ) {
        case PROP_USE_MARKUP:
        {
            action->private_data->useMarkup = g_value_get_boolean( value );
        }
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( obj, propId, pspec );
    }
}


/* static GtkWidget* create_menu_item( GtkAction* action ) */

GtkWidget* create_tool_item( GtkAction* action )
{
    GtkWidget* item = 0;

    if ( IS_EGE_OUTPUT_ACTION(action) )
    {
        GValue value;
#if GTK_CHECK_VERSION(3,0,0)
        GtkWidget* hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
        GtkWidget* hb = gtk_hbox_new( FALSE, 5 );
#endif
        GtkWidget* lbl = 0;
        memset( &value, 0, sizeof(value) );

        g_value_init( &value, G_TYPE_STRING );
        g_object_get_property( G_OBJECT(action), "short_label", &value );
        const gchar* sss = g_value_get_string( &value );

        item = GTK_WIDGET( gtk_tool_item_new() );

        lbl = gtk_label_new( " " );
        gtk_container_add( GTK_CONTAINER(hb), lbl );

        if ( EGE_OUTPUT_ACTION(action)->private_data->useMarkup ) {
            lbl = gtk_label_new(NULL);
            gtk_label_set_markup( GTK_LABEL(lbl), sss ? sss : " " );
        } else {
            lbl = gtk_label_new( sss ? sss : " " );
        }
        gtk_container_add( GTK_CONTAINER(hb), lbl );

        lbl = gtk_label_new( " " );
        gtk_container_add( GTK_CONTAINER(hb), lbl );

        gtk_container_add( GTK_CONTAINER(item), hb );

        gtk_widget_show_all( item );

        g_value_unset( &value );
    } else {
        item = GTK_ACTION_CLASS(ege_output_action_parent_class)->create_tool_item( action );
    }

    return item;
}

void fixup_labels( GObject *gobject, GParamSpec *arg1, gpointer user_data )
{
    /* TODO: handle 'use-markup' getting changed also */

    if ( arg1 && arg1->name && (strcmp("label", arg1->name) == 0) ) {
        GSList* proxies = gtk_action_get_proxies( GTK_ACTION(gobject) );
        gchar* str = 0;
        g_object_get( gobject, "label", &str, NULL );
        (void)user_data;
        while ( proxies ) {
            if ( GTK_IS_TOOL_ITEM(proxies->data) ) {
                /* Search for the things we built up in create_tool_item() */
                GList* children = gtk_container_get_children( GTK_CONTAINER(proxies->data) );
                if ( children && children->data ) {
                    if ( GTK_IS_BOX(children->data) ) {
                        children = gtk_container_get_children( GTK_CONTAINER(children->data) );
                        if ( children && g_list_next(children) ) {
                            GtkWidget* child = GTK_WIDGET( g_list_next(children)->data );
                            if ( GTK_IS_LABEL(child) ) {
                                GtkLabel* lbl = GTK_LABEL(child);
                                if ( EGE_OUTPUT_ACTION(gobject)->private_data->useMarkup ) {
                                    gtk_label_set_markup( lbl, str );
                                } else {
                                    gtk_label_set_text( lbl, str );
                                }
                            }
                        }
                    }
                }
            }
            proxies = g_slist_next( proxies );
        }
        g_free( str );
    }
}

