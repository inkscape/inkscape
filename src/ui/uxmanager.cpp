/** \file
 * Desktop widget implementation
 */
/* Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <algorithm>

#include "uxmanager.h"
#include "util/ege-tags.h"
#include "widgets/toolbox.h"
#include "widgets/desktop-widget.h"

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif // GDK_WINDOWING_X11

using std::map;
using std::vector;


gchar const* KDE_WINDOW_MANAGER_NAME = "KWin";
gchar const* UNKOWN_WINDOW_MANAGER_NAME = "unknown";


static vector<SPDesktop*> desktops;
static vector<SPDesktopWidget*> dtws;
static map<SPDesktop*, vector<GtkWidget*> > trackedBoxes;



namespace Inkscape {
namespace UI {

UXManager* instance = 0;

UXManager* UXManager::getInstance()
{
    if (!instance) {
        instance = new UXManager();
    }
    return instance;
}


UXManager::UXManager() :
    floatwindowIssues(false)
{
    ege::TagSet tags;
    tags.setLang("en");

    tags.addTag(ege::Tag("General"));
    tags.addTag(ege::Tag("Icons"));

#ifdef GDK_WINDOWING_X11
    char const* wmName = gdk_x11_screen_get_window_manager_name( gdk_screen_get_default() );
    //g_message("Window manager is [%s]", wmName);

    //if (g_ascii_strcasecmp( wmName, UNKOWN_WINDOW_MANAGER_NAME ) == 0) {
    if (g_ascii_strcasecmp( wmName, KDE_WINDOW_MANAGER_NAME ) == 0) {
        floatwindowIssues = true;
    }
#elif GDK_WINDOWING_WIN32
    floatwindowIssues = true;
#endif // GDK_WINDOWING_WIN32
}

UXManager::~UXManager()
{
}


bool UXManager::isFloatWindowProblem() const
{
    return floatwindowIssues;
}

void UXManager::setTask(SPDesktop* dt, gint val)
{
    for (vector<SPDesktopWidget*>::iterator it = dtws.begin(); it != dtws.end(); ++it) {
        SPDesktopWidget* dtw = *it;
        if (dtw->desktop == dt) {
            if (val == 0) {
                dtw->setToolboxPosition("ToolToolbar", GTK_POS_LEFT);
                dtw->setToolboxPosition("CommandsToolbar", GTK_POS_TOP);
                dtw->setToolboxPosition("SnapToolbar", GTK_POS_TOP);
                // for now skip "AuxToolbar";
            } else {
                dtw->setToolboxPosition("ToolToolbar", GTK_POS_TOP);
                dtw->setToolboxPosition("CommandsToolbar", GTK_POS_LEFT);
                dtw->setToolboxPosition("SnapToolbar", GTK_POS_RIGHT);
                // for now skip "AuxToolbar";
            }
            break;
        }
    }
}


void UXManager::addTrack( SPDesktopWidget* dtw )
{
    if (std::find(dtws.begin(), dtws.end(), dtw) == dtws.end()) {
        dtws.push_back(dtw);
    }
}

void UXManager::delTrack( SPDesktopWidget* dtw )
{
    vector<SPDesktopWidget*>::iterator iter = std::find(dtws.begin(), dtws.end(), dtw);
    if (iter != dtws.end()) {
        dtws.erase(iter);
    }
}

void UXManager::connectToDesktop( vector<GtkWidget *> const & toolboxes, SPDesktop *desktop )
{
//static map<SPDesktop*, vector<GtkWidget*> > trackedBoxes;

    for (vector<GtkWidget*>::const_iterator it = toolboxes.begin(); it != toolboxes.end(); ++it ) {
        GtkWidget* toolbox = *it;

        ToolboxFactory::setToolboxDesktop( toolbox, desktop );
        vector<GtkWidget*>& tracked = trackedBoxes[desktop];
        if (find(tracked.begin(), tracked.end(), toolbox) == tracked.end()) {
            tracked.push_back(toolbox);
        }
    }

    if (std::find(desktops.begin(), desktops.end(), desktop) == desktops.end()) {
        desktops.push_back(desktop);
    }
}


} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
