/*
 * Authors:
 *     Ralf Stephan <ralf@ark.in-berlin.de>
 *     John Bintz <jcoswell@coswellproductions.org>
 *
 * Copyright (C) 2006 John Bintz
 * Copyright (C) 2005 Ralf Stephan
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_VIEW_EDIT_WIDGET_IFACE_H
#define INKSCAPE_UI_VIEW_EDIT_WIDGET_IFACE_H

#include "message.h"
#include <2geom/point.h>

namespace Gtk {
class Window;
}

namespace Glib {
class ustring;
}

namespace Inkscape { namespace UI { namespace Widget { class Dock; } } }

namespace Inkscape {
namespace UI {
namespace View {

/**
 * Abstract base class for all EditWidget implementations.
 */
struct EditWidgetInterface
{
    EditWidgetInterface() {}
    virtual ~EditWidgetInterface() {}

    /// Returns pointer to window UI object as void*
    virtual Gtk::Window *getWindow() = 0;

    /// Set the widget's title
    virtual void setTitle (gchar const*) = 0;

    /// Show all parts of widget the user wants to see.
    virtual void layout() = 0;

    /// Present widget to user
    virtual void present() = 0;

    /// Returns geometry of widget
    virtual void getGeometry (gint &x, gint &y, gint &w, gint &h) = 0;

    /// Change the widget's size
    virtual void setSize (gint w, gint h) = 0;

    /// Move widget to specified position
    virtual void setPosition (Geom::Point p) = 0;

    /// Transientize widget
    virtual void setTransient (void*, int) = 0;

    /// Return mouse position in widget
    virtual Geom::Point getPointer() = 0;

    /// Make widget iconified
    virtual void setIconified() = 0;

    /// Make widget maximized on screen
    virtual void setMaximized() = 0;

    /// Make widget fill screen and show it if possible.
    virtual void setFullscreen() = 0;

    /// Shuts down the desktop object for the view being closed.  It checks
    /// to see if the document has been edited, and if so prompts the user
    /// to save, discard, or cancel.  Returns TRUE if the shutdown operation
    /// is cancelled or if the save is cancelled or fails, FALSE otherwise.
    virtual bool shutdown() = 0;

    /// Destroy and delete widget.
    virtual void destroy() = 0;


    /// Queue a redraw request with the canvas
    virtual void requestCanvasUpdate() = 0;

    /// Force a redraw of the canvas
    virtual void requestCanvasUpdateAndWait() = 0;

    /// Enable interaction on this desktop
    virtual void enableInteraction() = 0;

    /// Disable interaction on this desktop
    virtual void disableInteraction() = 0;

    /// Update the "active desktop" indicator
    virtual void activateDesktop() = 0;

    /// Update the "inactive desktop" indicator
    virtual void deactivateDesktop() = 0;

    /// Update rulers from current values
    virtual void updateRulers() = 0;

    /// Update scrollbars from current values
    virtual void updateScrollbars (double scale) = 0;

    /// Toggle rulers on/off and set preference value accordingly
    virtual void toggleRulers() = 0;

    /// Toggle scrollbars on/off and set preference value accordingly
    virtual void toggleScrollbars() = 0;

    /// Toggle CMS on/off and set preference value accordingly
    virtual void toggleColorProfAdjust() = 0;

    /// Toggle lock guides on/off and set namedview value accordingly
    virtual void toggleGuidesLock() = 0;

    /// Is CMS on/off
    virtual bool colorProfAdjustEnabled() = 0;

    /// Temporarily block signals and update zoom display
    virtual void updateZoom() = 0;

    /// The zoom display will get the keyboard focus.
    virtual void letZoomGrabFocus() = 0;

    /// In auxiliary toolbox, set focus to widget having specific id
    virtual void setToolboxFocusTo (const gchar *) = 0;

    /// In auxiliary toolbox, set value of adjustment with specific id
    virtual void setToolboxAdjustmentValue (const gchar *, double) = 0;

    /// In auxiliary toolbox, select one of the "select one" options (usually radio toggles)
    virtual void setToolboxSelectOneValue (const gchar *, gint) = 0;

    /// In auxiliary toolbox, return true if specific togglebutton is active
    virtual bool isToolboxButtonActive (gchar const*) = 0;

    /// Set the coordinate display
    virtual void setCoordinateStatus (Geom::Point p) = 0;

    /// Message widget will get no content
    virtual void setMessage (Inkscape::MessageType type, gchar const* msg) = 0;


    /** Show an info dialog with the given message */
    virtual bool showInfoDialog( Glib::ustring const &message ) = 0;

    /// Open yes/no dialog with warning text and confirmation question.
    virtual bool warnDialog (Glib::ustring const &) = 0;

    virtual Inkscape::UI::Widget::Dock* getDock () = 0;
};

} // namespace View
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_VIEW_EDIT_WIDGET_IFACE_H

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
