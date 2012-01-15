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
 * The Original Code is Appear Time Tracker.
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


#include "ege-appear-time-tracker.h"
#include <glib-object.h>
#include <gtk/gtk.h>


namespace ege
{

namespace {

void unhookHandler( gulong &id, GtkWidget *obj )
{
   if ( id ) {
        if ( obj ) {
            g_signal_handler_disconnect( G_OBJECT(obj), id );
        }
        id = 0;
    }
}

} // namespace


AppearTimeTracker::AppearTimeTracker(GTimer *timer, GtkWidget *widget, gchar const* name) : 
    _name(name ? name : ""),
    _timer(timer),
    _widget(widget),
    _topMost(widget),
    _autodelete(false),
    _mapId(0),
    _realizeId(0),
    _hierarchyId(0)

{
    while (gtk_widget_get_parent(_topMost)) {
        _topMost = gtk_widget_get_parent(_topMost);
    }
    _mapId = g_signal_connect( G_OBJECT(_topMost), "map-event", G_CALLBACK(mapCB), this );
    _realizeId = g_signal_connect( G_OBJECT(_topMost), "realize", G_CALLBACK(realizeCB), this );
    _hierarchyId = g_signal_connect( G_OBJECT(_widget), "hierarchy-changed", G_CALLBACK(hierarchyCB), this );
}

AppearTimeTracker::~AppearTimeTracker()
{
    if ( _timer ) {
        g_timer_destroy(_timer);
        _timer = 0;
    }

    unhookHandler( _mapId, _topMost );
    unhookHandler( _realizeId, _topMost );
    unhookHandler( _hierarchyId, _widget );
}

void AppearTimeTracker::stop() {
    if (_timer) {
        g_timer_stop(_timer);
    }
}

void AppearTimeTracker::setAutodelete(bool autodelete)
{
    if ( autodelete != _autodelete ) {
        _autodelete = autodelete;
    }
}

void AppearTimeTracker::report(gchar const* msg)
{
    gulong msCount = 0;
    gdouble secs = g_timer_elapsed( _timer, &msCount );
    g_message("Time ended at %2.3f with [%s] on [%s]", secs, msg, _name.c_str());
}

void AppearTimeTracker::handleHierarchyChange( GtkWidget * /*prevTop*/ )
{
    GtkWidget *newTop = _widget;
    while (gtk_widget_get_parent(newTop)) {
        newTop = gtk_widget_get_parent(newTop);
    }

    if ( newTop != _topMost ) {
        unhookHandler( _mapId, _topMost );
        unhookHandler( _realizeId, _topMost );

        _topMost = newTop;
        _mapId = g_signal_connect( G_OBJECT(_topMost), "map-event", G_CALLBACK(mapCB), this );
        _realizeId = g_signal_connect( G_OBJECT(_topMost), "realize", G_CALLBACK(realizeCB), this );
    }
}

gboolean AppearTimeTracker::mapCB(GtkWidget * /*widget*/, GdkEvent * /*event*/, gpointer userData)
{
    AppearTimeTracker *tracker = reinterpret_cast<AppearTimeTracker*>(userData);
    tracker->report("MAP");
    if ( tracker->_autodelete ) {
        delete tracker;
    }
    return FALSE;
}

void AppearTimeTracker::realizeCB(GtkWidget * /*widget*/, gpointer userData)
{
    AppearTimeTracker *tracker = reinterpret_cast<AppearTimeTracker*>(userData);
    tracker->report("REALIZE");
}

void AppearTimeTracker::hierarchyCB(GtkWidget * /*widget*/, GtkWidget *prevTop, gpointer userData)
{
    AppearTimeTracker *tracker = reinterpret_cast<AppearTimeTracker*>(userData);
    tracker->handleHierarchyChange( prevTop );
}

} // namespace ege

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
