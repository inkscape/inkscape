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

#include "view.h"
#include "view-widget.h"

//using namespace Inkscape::UI::View;

// SPViewWidget
static void sp_view_widget_dispose(GObject *object);

G_DEFINE_TYPE(SPViewWidget, sp_view_widget, GTK_TYPE_EVENT_BOX);

/**
 * Callback to initialize the SPViewWidget vtable.
 */
static void sp_view_widget_class_init(SPViewWidgetClass *vwc)
{
    GObjectClass *object_class = G_OBJECT_CLASS(vwc);
    
    object_class->dispose = sp_view_widget_dispose;
}

/**
 * Callback to initialize the SPViewWidget.
 */
static void sp_view_widget_init(SPViewWidget *vw)
{
    vw->view = NULL;
}

/**
 * Callback to disconnect from view and destroy SPViewWidget.
 *
 * Apparently, this gets only called when a desktop is closed, but then twice!
 */
static void sp_view_widget_dispose(GObject *object)
{
    SPViewWidget *vw = SP_VIEW_WIDGET(object);

    if (vw->view) {
        vw->view->close();
        Inkscape::GC::release(vw->view);
        vw->view = NULL;
    }

    if (G_OBJECT_CLASS(sp_view_widget_parent_class)->dispose) {
        G_OBJECT_CLASS(sp_view_widget_parent_class)->dispose(object);
    }

    Inkscape::GC::request_early_collection();
}

void sp_view_widget_set_view(SPViewWidget *vw, Inkscape::UI::View::View *view)
{
    g_return_if_fail(vw != NULL);
    g_return_if_fail(SP_IS_VIEW_WIDGET(vw));
    g_return_if_fail(view != NULL);
    
    g_return_if_fail(vw->view == NULL);
    
    vw->view = view;
    Inkscape::GC::anchor(view);

    if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->set_view) {
        ((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->set_view(vw, view);
    }
}

bool sp_view_widget_shutdown(SPViewWidget *vw)
{
    g_return_val_if_fail(vw != NULL, TRUE);
    g_return_val_if_fail(SP_IS_VIEW_WIDGET(vw), TRUE);

    if (((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->shutdown) {
        return ((SPViewWidgetClass *) G_OBJECT_GET_CLASS(vw))->shutdown(vw);
    }

    return FALSE;
}



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
