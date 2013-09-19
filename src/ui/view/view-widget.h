#ifndef INKSCAPE_UI_VIEW_VIEWWIDGET_H
#define INKSCAPE_UI_VIEW_VIEWWIDGET_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>

namespace Inkscape {
namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI
} // namespace Inkscape

class SPViewWidget;
class SPNamedView;

#define SP_TYPE_VIEW_WIDGET (sp_view_widget_get_type ())
#define SP_VIEW_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_VIEW_WIDGET, SPViewWidget))
#define SP_VIEW_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_VIEW_WIDGET, SPViewWidgetClass))
#define SP_IS_VIEW_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_VIEW_WIDGET))
#define SP_IS_VIEW_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_VIEW_WIDGET))
#define SP_VIEW_WIDGET_VIEW(w) (SP_VIEW_WIDGET (w)->view)
#define SP_VIEW_WIDGET_DOCUMENT(w) (SP_VIEW_WIDGET (w)->view ? ((SPViewWidget *) (w))->view->doc : NULL)

/**
 * Registers the SPViewWidget class with Glib and returns its type number.
 */
GType sp_view_widget_get_type(void);

/**
 * Connects widget to view's 'resized' signal and calls virtual set_view()
 * function.
 */
void sp_view_widget_set_view(SPViewWidget *vw, Inkscape::UI::View::View *view);

/**
 * Allows presenting 'save changes' dialog, FALSE - continue, TRUE - cancel.
 * Calls the virtual shutdown() function of the SPViewWidget.
 */
bool sp_view_widget_shutdown(SPViewWidget *vw);

/**
 * Create a new SPViewWidget (which happens to be a SPDesktopWidget).
 */
SPViewWidget *sp_desktop_widget_new(SPNamedView *namedview);

/**
 * SPViewWidget is a GUI widget that contain a single View. It is also
 * an abstract base class with little functionality of its own.
 */
class SPViewWidget {
 public:
	GtkEventBox eventbox;

        Inkscape::UI::View::View *view;

    // C++ Wrappers
    GType getType() const {
	return sp_view_widget_get_type();
    }

    void setView(Inkscape::UI::View::View *view) {
	sp_view_widget_set_view(this, view);
    }

    gboolean shutdown() {
	return sp_view_widget_shutdown(this);
    }

//    void resized (double x, double y) = 0;
};

/**
 * The Glib-style vtable for the SPViewWidget class.
 */
class SPViewWidgetClass {
 public:
    GtkEventBoxClass parent_class;

    /* Virtual method to set/change/remove view */
    void (* set_view) (SPViewWidget *vw, Inkscape::UI::View::View *view);
    /// Virtual method about view size change
    void (* view_resized) (SPViewWidget *vw, Inkscape::UI::View::View *view, gdouble width, gdouble height);

    gboolean (* shutdown) (SPViewWidget *vw);
};

#endif

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
