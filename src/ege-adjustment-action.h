#ifndef SEEN_EGE_ADJUSTMENT_ACTION
#define SEEN_EGE_ADJUSTMENT_ACTION
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

#include <glib.h>
#include <gtk/gtkaction.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define EGE_ADJUSTMENT_ACTION_TYPE                ( ege_adjustment_action_get_type() )
#define EGE_ADJUSTMENT_ACTION( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), EGE_ADJUSTMENT_ACTION_TYPE, EgeAdjustmentAction) )
#define EGE_ADJUSTMENT_ACTION_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), EGE_ADJUSTMENT_ACTION_TYPE, EgeAdjustmentActionClass) )
#define IS_EGE_ADJUSTMENT_ACTION( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), EGE_ADJUSTMENT_ACTION_TYPE) )
#define IS_EGE_ADJUSTMENT_ACTION_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), EGE_ADJUSTMENT_ACTION_TYPE) )
#define EGE_ADJUSTMENT_ACTION_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), EGE_ADJUSTMENT_ACTION_TYPE, EgeAdjustmentActionClass) )

typedef struct _EgeAdjustmentAction      EgeAdjustmentAction;
typedef struct _EgeAdjustmentActionClass EgeAdjustmentActionClass;
typedef struct _EgeAdjustmentActionPrivate EgeAdjustmentActionPrivate;

struct _EgeAdjustmentAction
{
    GtkAction action;
    EgeAdjustmentActionPrivate *private_data;
};

struct _EgeAdjustmentActionClass
{
    GtkActionClass parent_class;
};

GType ege_adjustment_action_get_type( void );

EgeAdjustmentAction* ege_adjustment_action_new( GtkAdjustment* adjustment,
                                                const gchar *name,
                                                const gchar *label,
                                                const gchar *tooltip,
                                                const gchar *stock_id,
                                                gdouble climb_rate,
                                                guint digits );

GtkAdjustment* ege_adjustment_action_get_adjustment( EgeAdjustmentAction* action );

void ege_adjustment_action_set_focuswidget( EgeAdjustmentAction* action, GtkWidget* widget );
GtkWidget* ege_adjustment_action_get_focuswidget( EgeAdjustmentAction* action );


typedef void (*EgeWidgetFixup)(GtkWidget *widget);


G_END_DECLS

#endif /* SEEN_EGE_ADJUSTMENT_ACTION */
