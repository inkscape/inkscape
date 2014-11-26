/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "widgets/desktop-widget.h"
#include <glib-object.h>

#include "desktop-tracker.h"

#include "inkscape.h"
#include "desktop.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

DesktopTracker::DesktopTracker() :
    base(0),
    desktop(0),
    widget(0),
    hierID(0),
    trackActive(false),
    desktopChangedSig()
{
}

DesktopTracker::~DesktopTracker()
{
    disconnect();
}

void DesktopTracker::connect(GtkWidget *widget)
{
    disconnect();

    this->widget = widget;

    // Use C/gobject callbacks to avoid gtkmm rewrap-during-destruct issues:
    hierID = g_signal_connect( G_OBJECT(widget), "hierarchy-changed", G_CALLBACK(hierarchyChangeCB), this );
    inkID = INKSCAPE.signal_activate_desktop.connect(
              sigc::bind(
              sigc::ptr_fun(&DesktopTracker::activateDesktopCB), this)
    );

    GtkWidget *wdgt = gtk_widget_get_ancestor(widget, SP_TYPE_DESKTOP_WIDGET);
    if (wdgt && !base) {
        SPDesktopWidget *dtw = SP_DESKTOP_WIDGET(wdgt);
        if (dtw && dtw->desktop) {
            setBase(dtw->desktop); // may also set desktop
        }
    }
}

void DesktopTracker::disconnect()
{
    if (hierID) {
        if (widget) {
            g_signal_handler_disconnect(G_OBJECT(widget), hierID);
        }
        hierID = 0;
    }
    if (inkID.connected()) {
        inkID.disconnect();
    }
}

void DesktopTracker::setBase(SPDesktop *desktop)
{
    if (this->base != desktop) {
        base = desktop;
        // Do not override an existing target desktop
        if (!this->desktop) {
            setDesktop(desktop);
        }
    }
}

SPDesktop *DesktopTracker::getBase() const
{
    return base;
}

SPDesktop *DesktopTracker::getDesktop() const
{
    return desktop;
}

sigc::connection DesktopTracker::connectDesktopChanged( const sigc::slot<void, SPDesktop*> & slot )
{
    return desktopChangedSig.connect(slot);
}

void DesktopTracker::activateDesktopCB(SPDesktop *desktop, DesktopTracker *self )
{
    if (self && self->trackActive) {
        self->setDesktop(desktop);
    }
    //return FALSE;
}

bool DesktopTracker::hierarchyChangeCB(GtkWidget * /*widget*/, GtkWidget* /*prev*/, DesktopTracker *self)
{
    if (self) {
        self->handleHierarchyChange();
    }
    return false;
}

void DesktopTracker::handleHierarchyChange()
{
    GtkWidget *wdgt = gtk_widget_get_ancestor(widget, SP_TYPE_DESKTOP_WIDGET);
    bool newFlag = (wdgt == 0); // true means not in an SPDesktopWidget, thus floating.
    if (wdgt && !base) {
        SPDesktopWidget *dtw = SP_DESKTOP_WIDGET(wdgt);
        if (dtw && dtw->desktop) {
            setBase(dtw->desktop); // may also set desktop
        }
    }
    if (newFlag != trackActive) {
        trackActive = newFlag;
        if (trackActive) {
            setDesktop(SP_ACTIVE_DESKTOP);
        } else if (desktop != base) {
            setDesktop(getBase());
        }
    }
}

void DesktopTracker::setDesktop(SPDesktop *desktop)
{
    if (desktop != this->desktop) {
        this->desktop = desktop;
        desktopChangedSig.emit(desktop);
    }
}


} // namespace Dialog
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
