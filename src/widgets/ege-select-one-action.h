#ifndef SEEN_EGE_SELECT_ONE_ACTION
#define SEEN_EGE_SELECT_ONE_ACTION
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

/** \file
 * GtkAction subclass that represents a set of values the user may select
 *  one from at a given time.
 * This can manifest as a popup menu, a ComboBox, a set of toggle buttons,
 *  etc.
 */

/* Note: this file should be kept compilable as both .cpp and .c */

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define EGE_SELECT_ONE_ACTION_TYPE                ( ege_select_one_action_get_type() )
#define EGE_SELECT_ONE_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), EGE_SELECT_ONE_ACTION_TYPE, EgeSelectOneAction) )
#define EGE_SELECT_ONE_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), EGE_SELECT_ONE_ACTION_TYPE, EgeSelectOneActionClass) )
#define IS_EGE_SELECT_ONE_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), EGE_SELECT_ONE_ACTION_TYPE) )
#define IS_EGE_SELECT_ONE_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), EGE_SELECT_ONE_ACTION_TYPE) )
#define EGE_SELECT_ONE_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), EGE_SELECT_ONE_ACTION_TYPE, EgeSelectOneActionClass) )

typedef struct _EgeSelectOneAction      EgeSelectOneAction;
typedef struct _EgeSelectOneActionClass EgeSelectOneActionClass;
typedef struct _EgeSelectOneActionPrivate EgeSelectOneActionPrivate;

/**
 * Instance structure of EgeSelectOneAction.
 */
struct _EgeSelectOneAction
{
    /** Parent instance structure. */
    GtkAction action;

    /** Pointer to private instance data. */
    EgeSelectOneActionPrivate *private_data;
};

/**
 * Class structure of EgeSelectOneAction.
 */
struct _EgeSelectOneActionClass
{
    /** Parent class structure. */
    GtkActionClass parent_class;

    void (*changed) (EgeSelectOneAction* action);
};

/** Standard Gtk type function */
GType ege_select_one_action_get_type( void );

/**
 * Creates a new EgeSelectOneAction instance.
 * This is a GtkAction subclass that represents a set of values the user
 *  may select one from at a given time.
 * This can manifest as a popup menu, a ComboBox, a set of toggle buttons,
 *  etc.
 *
 * @param name Functional name for the action.
 * @param label Display label for the action.
 * @param tooltip Tooltip for the action.
 * @param stock_id Icon id to use.
 * @param model the source of choices to present.
 */
EgeSelectOneAction* ege_select_one_action_new( const gchar *name,
                                               const gchar *label,
                                               const gchar *tooltip,
                                               const gchar *stock_id,
                                               GtkTreeModel* model );

GtkTreeModel *ege_select_one_action_get_model(EgeSelectOneAction* action );

/**
 * Returns the index of the currently selected item.
 *
 * @param action The action to fetch the selected index for.
 */
gint ege_select_one_action_get_active( EgeSelectOneAction* action );

/**
 * Returns the text of the currently selected item.
 *
 * @param action The action to fetch the text for.
 * @return the selected text. The caller is responsible to call g_free() on it when done.
 */
gchar *ege_select_one_action_get_active_text( EgeSelectOneAction* action );

/**
 * Sets the text of the currently selected item.
 *
 * @param action The action to fetch the text for.
 * @param text the text to set.
 */
void ege_select_one_action_set_active_text( EgeSelectOneAction* action, gchar const *text );

/**
 * Sets the  currently selected item.
 *
 * @param action The action to fetch the selected index for.
 * @param val index of the item to make selected.
 */
void ege_select_one_action_set_active( EgeSelectOneAction* action, gint val );

//void ege_select_one_action_set_sensitive( EgeSelectOneAction *action, gint val, gboolean sensitive );

/**
 * Update sensitive parameters.
 * @param action The action to update.
 */
void ege_select_one_action_update_sensitive( EgeSelectOneAction *action );

/**
 * Returns the column used for the display label.
 *
 * @param action The action to fetch the label column for.
 */
gint ege_select_one_action_get_label_column( EgeSelectOneAction* action );

/**
 * Sets the column used for the display label.
 *
 * @param action The action to set the label column for.
 * @param col column to use.
 */
void ege_select_one_action_set_label_column( EgeSelectOneAction* action, gint col );


/**
 * Returns the column used for the display icon.
 *
 * @param action The action to fetch the icon column for.
 */
gint ege_select_one_action_get_icon_column( EgeSelectOneAction* action );

/**
 * Sets the column used for the display icon.
 *
 * @param action The action to set the icon column for.
 * @param col column to use.
 */
void ege_select_one_action_set_icon_column( EgeSelectOneAction* action, gint col );

gint ege_select_one_action_get_icon_size( EgeSelectOneAction* action );

void ege_select_one_action_set_icon_size( EgeSelectOneAction* action, gint size );


/**
 * Returns the column used for the tooltip.
 *
 * @param action The action to fetch the tooltip column for.
 */
gint ege_select_one_action_get_tooltip_column( EgeSelectOneAction* action );

/**
 * Sets the column used for the tooltip.
 *
 * @param action The action to set the tooltip column for.
 * @param col column to use.
 */
void ege_select_one_action_set_tooltip_column( EgeSelectOneAction* action, gint col );


/**
 * Returns the column used for tracking sensitivity.
 *
 * @param action The action to fetch the sensitive column for.
 */
gint ege_select_one_action_get_sensitive_column( EgeSelectOneAction* action );

/**
 * Sets the column used for sensitivity (if any).
 *
 * @param action The action to set the sensitive column for.
 * @param col column to use.
 */
void ege_select_one_action_set_sensitive_column( EgeSelectOneAction* action, gint col );


/**
 * Sets a hint to be used in determining the display form.
 * This is the XForms style 'appearance' hint: "full", "compact", "minimal".
 *
 * @param action The action to set the tooltip column for.
 * @param val The value of the appearance hint.
 */
void ege_select_one_action_set_appearance( EgeSelectOneAction* action, gchar const* val );

/**
 * Sets to allow or disallow free entry additions to the list.
 * The default is "closed" selections that do not allow additions/edits.
 * This is the XForms functional 'selection' attribute: "open", "closed".
 *
 * @param action The action to set the tooltip column for.
 * @param val The value of the selection attribute.
 */
void ege_select_one_action_set_selection( EgeSelectOneAction *action, gchar const* val );

/* bit of a work-around */
void ege_select_one_action_set_radio_action_type( EgeSelectOneAction* action, GType radioActionType );

G_END_DECLS

#endif /* SEEN_EGE_SELECT_ONE_ACTION */
