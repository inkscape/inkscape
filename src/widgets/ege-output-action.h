#ifndef SEEN_EGE_OUTPUT_ACTION
#define SEEN_EGE_OUTPUT_ACTION
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

/** \file
 *  GtkAction subclass that represents a string for output.
 */

/* Note: this file should be kept compilable as both .cpp and .c */

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define EGE_OUTPUT_ACTION_TYPE                ( ege_output_action_get_type() )
#define EGE_OUTPUT_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), EGE_OUTPUT_ACTION_TYPE, EgeOutputAction) )
#define EGE_OUTPUT_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), EGE_OUTPUT_ACTION_TYPE, EgeOutputActionClass) )
#define IS_EGE_OUTPUT_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), EGE_OUTPUT_ACTION_TYPE) )
#define IS_EGE_OUTPUT_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), EGE_OUTPUT_ACTION_TYPE) )
#define EGE_OUTPUT_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), EGE_OUTPUT_ACTION_TYPE, EgeOutputActionClass) )

typedef struct _EgeOutputAction      EgeOutputAction;
typedef struct _EgeOutputActionClass EgeOutputActionClass;
typedef struct _EgeOutputActionPrivate EgeOutputActionPrivate;

/**
 * Instance structure of EgeOutputAction.
 */
struct _EgeOutputAction
{
    /** Parent instance structure. */
    GtkAction action;

    /** Pointer to private instance data. */
    EgeOutputActionPrivate *private_data;
};

/**
 * Class structure of EgeOutputAction.
 */
struct _EgeOutputActionClass
{
    /** Parent class structure. */
    GtkActionClass parent_class;
};

/** Standard Gtk type function */
GType ege_output_action_get_type( void );

/**
 * Creates a new EgeOutputAction instance.
 * This is a GtkAction subclass that displays a string.
 *
 * @param name Functional name for the action.
 * @param label Display label for the action.
 * @param tooltip Tooltip for the action.
 * @param stock_id Icon id to use.
 */
EgeOutputAction* ege_output_action_new( const gchar *name,
                                        const gchar *label,
                                        const gchar *tooltip,
                                        const gchar *stock_id );

/**
 * Return whether or not the displayed text is interpreted as markup.
 *
 * @param action The action to fetch the markup state for.
 * @return True if the text is to be interpreted as markup, false otherwise.
 */
gboolean ege_output_action_get_use_markup( EgeOutputAction* action );

/**
 * Sets whether or not the displayed text is interpreted as markup.
 *
 * @param action The action to set the markup state for.
 * @param setting True if the text is to be interpreted as markup, false otherwise.
 */
void ege_output_action_set_use_markup( EgeOutputAction* action, gboolean setting );

G_END_DECLS

#endif /* SEEN_EGE_OUTPUT_ACTION */
