#ifndef SEEN_UI_UXMANAGER_H
#define SEEN_UI_UXMANAGER_H
/*
 * A simple interface for previewing representations.
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <vector>

extern "C"
{
    typedef struct _GObject GObject;
    typedef struct _GtkWidget GtkWidget;
}

class SPDesktop;

struct SPDesktopWidget;


namespace Inkscape {
namespace UI {

class UXManager
{
public:
    static UXManager* getInstance();
    virtual ~UXManager();

    void addTrack( SPDesktopWidget* dtw );
    void delTrack( SPDesktopWidget* dtw );

    void connectToDesktop( std::vector<GtkWidget *> const & toolboxes, SPDesktop *desktop );

    void setTask(SPDesktop* dt, gint val);

private:
    UXManager();
};

} // namespace UI
} // namespace Inkscape

#endif // SEEN_UI_UXMANAGER_H
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
