#ifndef SEEN_EGE_COLOR_PROF_TRACKER
#define SEEN_EGE_COLOR_PROF_TRACKER
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
 * The Original Code is EGE Color Profile Tracker.
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
 * Object used to track ICC profiles attached to monitors and update as they change.
 */

/* Note: this file should be kept compilable as both .cpp and .c */

#include <glib-object.h>

typedef struct _GtkWidget GtkWidget;

G_BEGIN_DECLS

#define EGE_COLOR_PROF_TRACKER_TYPE                ( ege_color_prof_tracker_get_type() )
#define EGE_COLOR_PROF_TRACKER( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( (obj), EGE_COLOR_PROF_TRACKER_TYPE, EgeColorProfTracker) )
#define EGE_COLOR_PROF_TRACKER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), EGE_COLOR_PROF_TRACKER_TYPE, EgeColorProfTrackerClass) )
#define IS_EGE_COLOR_PROF_TRACKER( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), EGE_COLOR_PROF_TRACKER_TYPE) )
#define IS_EGE_COLOR_PROF_TRACKER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), EGE_COLOR_PROF_TRACKER_TYPE) )
#define EGE_COLOR_PROF_TRACKER_GET_CLASS( obj )    ( G_TYPE_INSTANCE_GET_CLASS( (obj), EGE_COLOR_PROF_TRACKER_TYPE, EgeColorProfTrackerClass) )

typedef struct _EgeColorProfTracker        EgeColorProfTracker;
typedef struct _EgeColorProfTrackerClass   EgeColorProfTrackerClass;
typedef struct _EgeColorProfTrackerPrivate EgeColorProfTrackerPrivate;

/**
 * Instance structure of EgeColorProfTracker.
 */
struct _EgeColorProfTracker
{
    /** Parent instance structure. */
    GObject object;

    /** Pointer to private instance data. */
    EgeColorProfTrackerPrivate *private_data;
};

/**
 * Class structure of EgeColorProfTracker.
 */
struct _EgeColorProfTrackerClass
{
    /** Parent class structure. */
    GObjectClass parent_class;

    void (*changed) (EgeColorProfTracker* tracker);
};

/** Standard Gtk type function */
GType ege_color_prof_tracker_get_type( void );

/**
 * Creates a new EgeColorProfTracker instance.
 */
EgeColorProfTracker* ege_color_prof_tracker_new( GtkWidget* target );

void ege_color_prof_tracker_get_profile( EgeColorProfTracker const * tracker, gpointer* ptr, guint* len );
void ege_color_prof_tracker_get_profile_for( guint screen, guint monitor, gpointer* ptr, guint* len );

G_END_DECLS

#endif /* SEEN_EGE_COLOR_PROF_TRACKER */
