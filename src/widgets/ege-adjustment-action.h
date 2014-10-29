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

/** \file
 *  GtkAction subclass that represents a GtkAdjustment value.
 */

/* Note: this file should be kept compilable as both .cpp and .c */

#include <gtk/gtk.h>

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

namespace Inkscape {
    namespace UI {
        namespace Widget {
            class UnitTracker;
        }
    }
}

/**
 * Instance structure of EgeAdjustmentAction.
 */
struct _EgeAdjustmentAction
{
    /** Parent instance structure. */
    GtkAction action;

    /** Pointer to private instance data. */
    EgeAdjustmentActionPrivate *private_data;
};

/**
 * Class structure of EgeAdjustmentAction.
 */
struct _EgeAdjustmentActionClass
{
    /** Parent class structure. */
    GtkActionClass parent_class;
};

/** Standard Gtk type function */
GType ege_adjustment_action_get_type( void );


/*
 * Note: This normally could be implemented via a GType property for the class to construct,
 * but gtkmm classes implemented in C++ only will often not funciton properly.
 *
 */

/** Callback type for widgets creation factory */
typedef GtkWidget* (*EgeCreateAdjWidgetCB)( GtkAdjustment *adjustment, gdouble climb_rate, guint digits, Inkscape::UI::Widget::UnitTracker *unit_tracker );

/**
 * Sets a factory callback to be used to create the specific widget.
 *
 * @param factoryCb the callback to use to create custom widgets, NULL to use the default.
 */
void ege_adjustment_action_set_compact_tool_factory( EgeCreateAdjWidgetCB factoryCb );


/**
 * Creates a new EgeAdjustmentAction instance.
 * This is a GtkAction subclass that manages a value stored in a
 * GtkAdjustment.
 *
 * @param adjustment The GtkAdjustment to manage.
 * @param name Functional name for the action.
 * @param label Display label for the action.
 * @param tooltip Tooltip for the action.
 * @param stock_id Icon id to use.
 * @param climb_rate Used for created widgets.
 * @param digits Used for created widgets.
 * @param unit_tracker Used to store unit.
 */
EgeAdjustmentAction* ege_adjustment_action_new( GtkAdjustment* adjustment,
                                                const gchar *name,
                                                const gchar *label,
                                                const gchar *tooltip,
                                                const gchar *stock_id,
                                                gdouble climb_rate,
                                                guint digits,
                                                Inkscape::UI::Widget::UnitTracker *unit_tracker
                                                );
/**
 * Returns a pointer to the GtkAdjustment represented by the given
 * EgeAdjustmentAction.
 *
 * @param action The action to fetch the GtkAdjustment for.
 */
GtkAdjustment* ege_adjustment_action_get_adjustment( EgeAdjustmentAction* action );

/**
 * Sets the GtkWidget to return focus to.
 * This is used to be able to transfer focus back out of a toolbar.
 *
 * @param action The action to set the widget for.
 * @param widget The widget to return focus to after editing.
 * @see ege_adjustment_action_get_focuswidget
 */
void ege_adjustment_action_set_focuswidget( EgeAdjustmentAction* action, GtkWidget* widget );

/**
 * Returns a pointer to the GtkWidget to return focus to after changing
 * the value.
 *
 * @param action The action to fetch the focus widget for.
 * @returns A pointer to the widget to return focus to, NULL if none set.
 * @see ege_adjustment_action_set_focuswidget
 */
GtkWidget* ege_adjustment_action_get_focuswidget( EgeAdjustmentAction* action );

/**
 * Set a list of values with labels to explicitly include in menus.
 *
 * @param action The action to set explicit entries for.
 * @param descriptions Array of descriptions to include.
 *          Descriptions will be matched one-for-one with numbers in the 'values' array.
 * @param values Array of values to include.
 *          Values will be matched one-for-one with numbers in the 'descriptions' array.
 * @param count Number of items in the 'descriptions' and 'values' arrays.
 */
void ege_adjustment_action_set_descriptions( EgeAdjustmentAction* action, gchar const** descriptions, gdouble const* values, guint count );

/**
 * Sets a hint to be used in determining the display form.
 * This is the XForms style 'appearance' hint: "full", "compact", "minimal".
 *
 * @param action The action to set the tooltip column for.
 * @param val The value of the appearance hint.
 */
void ege_adjustment_action_set_appearance( EgeAdjustmentAction* action, gchar const* val );

/** Callback type for post-creation 'fixup' pass on generated widgets */
typedef void (*EgeWidgetFixup)(GtkWidget *widget);


G_END_DECLS

#endif /* SEEN_EGE_ADJUSTMENT_ACTION */
