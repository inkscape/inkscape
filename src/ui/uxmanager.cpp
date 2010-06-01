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
#include "preferences.h"
#include "gdkmm/screen.h"

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

class UXManagerImpl : public UXManager
{
public:
    UXManagerImpl();
    virtual ~UXManagerImpl();

    virtual void setTask(SPDesktop* dt, gint val);
    virtual void addTrack( SPDesktopWidget* dtw );
    virtual void delTrack( SPDesktopWidget* dtw );
    virtual void connectToDesktop( vector<GtkWidget *> const & toolboxes, SPDesktop *desktop );

    virtual bool isFloatWindowProblem() const;
    virtual bool isWidescreen() const;

private:
    bool _floatwindowIssues;
    bool _widescreen;
};

UXManager* UXManager::getInstance()
{
    if (!instance) {
        instance = new UXManagerImpl();
    }
    return instance;
}


UXManager::UXManager()
{
}

UXManager::~UXManager()
{
}

UXManagerImpl::UXManagerImpl() :
    _floatwindowIssues(false),
    _widescreen(false)
{
    ege::TagSet tags;
    tags.setLang("en");

    tags.addTag(ege::Tag("General"));
    tags.addTag(ege::Tag("Icons"));

#if defined(GDK_WINDOWING_X11)
    char const* wmName = gdk_x11_screen_get_window_manager_name( gdk_screen_get_default() );
    //g_message("Window manager is [%s]", wmName);

    //if (g_ascii_strcasecmp( wmName, UNKOWN_WINDOW_MANAGER_NAME ) == 0) {
    if (g_ascii_strcasecmp( wmName, KDE_WINDOW_MANAGER_NAME ) == 0) {
        _floatwindowIssues = true;
    }
#elif defined(GDK_WINDOWING_WIN32)
    _floatwindowIssues = true;
#endif // GDK_WINDOWING_WIN32


    Glib::RefPtr<Gdk::Screen> defaultScreen = Gdk::Screen::get_default();
    if (defaultScreen) {
        int width = defaultScreen->get_width();
        int height = defaultScreen->get_height();
        gdouble aspect = static_cast<gdouble>(width) / static_cast<gdouble>(height);
        if (aspect > 1.4) {
            _widescreen = true;
        }
    }
}

UXManagerImpl::~UXManagerImpl()
{
}

bool UXManagerImpl::isFloatWindowProblem() const
{
    return _floatwindowIssues;
}

bool UXManagerImpl::isWidescreen() const
{
    return _widescreen;
}

void UXManagerImpl::setTask(SPDesktop* dt, gint val)
{
    for (vector<SPDesktopWidget*>::iterator it = dtws.begin(); it != dtws.end(); ++it) {
        SPDesktopWidget* dtw = *it;

        gboolean notDone = Inkscape::Preferences::get()->getBool("/options/workarounds/dynamicnotdone", false);

        if (dtw->desktop == dt) {
            switch (val) {
                default:
                case 0:
                    dtw->setToolboxPosition("ToolToolbar", GTK_POS_LEFT);
                    dtw->setToolboxPosition("CommandsToolbar", GTK_POS_TOP);
                    if (notDone) {
                        dtw->setToolboxPosition("AuxToolbar", GTK_POS_TOP);
                    }
                    dtw->setToolboxPosition("SnapToolbar", GTK_POS_TOP);
                    break;
                case 1:
                    dtw->setToolboxPosition("ToolToolbar", GTK_POS_TOP);
                    dtw->setToolboxPosition("CommandsToolbar", GTK_POS_LEFT);
                    if (notDone) {
                        dtw->setToolboxPosition("AuxToolbar", GTK_POS_TOP);
                    }
                    dtw->setToolboxPosition("SnapToolbar", GTK_POS_RIGHT);
                    break;
                case 2:
                    dtw->setToolboxPosition("ToolToolbar", GTK_POS_LEFT);
                    dtw->setToolboxPosition("CommandsToolbar", GTK_POS_RIGHT);
                    dtw->setToolboxPosition("SnapToolbar", GTK_POS_RIGHT);
                    if (notDone) {
                        dtw->setToolboxPosition("AuxToolbar", GTK_POS_RIGHT);
                    }
            }
        }
    }
}


void UXManagerImpl::addTrack( SPDesktopWidget* dtw )
{
    if (std::find(dtws.begin(), dtws.end(), dtw) == dtws.end()) {
        dtws.push_back(dtw);
    }
}

void UXManagerImpl::delTrack( SPDesktopWidget* dtw )
{
    vector<SPDesktopWidget*>::iterator iter = std::find(dtws.begin(), dtws.end(), dtw);
    if (iter != dtws.end()) {
        dtws.erase(iter);
    }
}

void UXManagerImpl::connectToDesktop( vector<GtkWidget *> const & toolboxes, SPDesktop *desktop )
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
