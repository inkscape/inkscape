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

#include <vector>
#include <glib.h>

extern "C" typedef struct _GtkWidget GtkWidget;
class SPDesktop;
struct SPDesktopWidget;

namespace Inkscape {
namespace UI {

class UXManager
{
public:
    static UXManager* getInstance();
    virtual ~UXManager();

    virtual void addTrack( SPDesktopWidget* dtw ) = 0;
    virtual void delTrack( SPDesktopWidget* dtw ) = 0;

    virtual void connectToDesktop( std::vector<GtkWidget *> const & toolboxes, SPDesktop *desktop ) = 0;

    virtual gint getDefaultTask( SPDesktop *desktop ) = 0;
    virtual void setTask( SPDesktop* dt, gint val ) = 0;

    virtual bool isFloatWindowProblem() const = 0;
    virtual bool isWidescreen() const = 0;

protected:
    UXManager();

private:
    UXManager( UXManager const & );
    UXManager & operator=( UXManager const & );
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
