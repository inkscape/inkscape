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

using std::map;
using std::vector;

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


UXManager::UXManager()
{
    ege::TagSet tags;
    tags.setLang("en");

    tags.addTag(ege::Tag("General"));
    tags.addTag(ege::Tag("Icons"));
}

UXManager::~UXManager()
{
}

void UXManager::setTask(SPDesktop* dt, gint val)
{
    GtkOrientation orientation = (val == 0)? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    for (vector<SPDesktopWidget*>::iterator it = dtws.begin(); it != dtws.end(); ++it) {
        if ((*it)->desktop == dt) {
            vector<GtkWidget*>& boxes = trackedBoxes[dt];
            for (vector<GtkWidget*>::iterator it2 = boxes.begin(); it2 != boxes.end(); ++it2) {
                gint id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(*it2), "BarIdValue"));
                if (id != 1) {
                    //ToolboxFactory::setOrientation(*it2, orientation);
                }
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
