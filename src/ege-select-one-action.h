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

#include <glib.h>
#include <gtk/gtkaction.h>
#include <gtk/gtktreemodel.h>
#include <glib-object.h>

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

struct _EgeSelectOneAction
{
    GtkAction action;
    EgeSelectOneActionPrivate *private_data;
};

struct _EgeSelectOneActionClass
{
    GtkActionClass parent_class;
    void (*changed) (EgeSelectOneAction* action);
};

GType ege_select_one_action_get_type( void );

EgeSelectOneAction* ege_select_one_action_new( const gchar *name,
                                               const gchar *label,
                                               const gchar *tooltip,
                                               const gchar *stock_id,
                                               GtkTreeModel* model );

gint ege_select_one_action_get_active( EgeSelectOneAction* action );
void ege_select_one_action_set_active( EgeSelectOneAction* action, gint val );

gint ege_select_one_action_get_label_column( EgeSelectOneAction* action );
void ege_select_one_action_set_label_column( EgeSelectOneAction* action, gint col );

gint ege_select_one_action_get_icon_column( EgeSelectOneAction* action );
void ege_select_one_action_set_icon_column( EgeSelectOneAction* action, gint col );

gint ege_select_one_action_get_tooltip_column( EgeSelectOneAction* action );
void ege_select_one_action_set_tooltip_column( EgeSelectOneAction* action, gint col );

void ege_select_one_action_set_appearance( EgeSelectOneAction* action, gchar const* val );


/* bit of a work-around */
void ege_select_one_action_set_radio_action_type( EgeSelectOneAction* action, GType radioActionType );

G_END_DECLS

#endif /* SEEN_EGE_SELECT_ONE_ACTION */
