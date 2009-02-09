/** \file
 * SPViewWidget implementation.
 *
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

/* SPViewWidget */

static void sp_view_widget_class_init(SPViewWidgetClass *vwc);
static void sp_view_widget_init(SPViewWidget *widget);
static void sp_view_widget_destroy(GtkObject *object);

static GtkEventBoxClass *widget_parent_class;

/**
 * Registers the SPViewWidget class with Glib and returns its type number.
 */
GtkType sp_view_widget_get_type(void)
{
    static GtkType type = 0;
    
    if (!type) {
        GtkTypeInfo info = {
            (gchar*) "SPViewWidget",
            sizeof(SPViewWidget),
            sizeof(SPViewWidgetClass),
            (GtkClassInitFunc) sp_view_widget_class_init,
            (GtkObjectInitFunc) sp_view_widget_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique(GTK_TYPE_EVENT_BOX, &info);
    }
    
    return type;
}

/**
 * Callback to initialize the SPViewWidget vtable.
 */
static void sp_view_widget_class_init(SPViewWidgetClass *vwc)
{
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(vwc);

    widget_parent_class = (GtkEventBoxClass*) gtk_type_class(GTK_TYPE_EVENT_BOX);
    
    object_class->destroy = sp_view_widget_destroy;
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
static void sp_view_widget_destroy(GtkObject *object)
{
    SPViewWidget *vw = SP_VIEW_WIDGET(object);

    if (vw->view) {
        vw->view->close();
        Inkscape::GC::release(vw->view);
        vw->view = NULL;
    }

    if (((GtkObjectClass *) (widget_parent_class))->destroy) {
        (* ((GtkObjectClass *) (widget_parent_class))->destroy)(object);
    }

    Inkscape::GC::request_early_collection();
}

/**
 * Connects widget to view's 'resized' signal and calls virtual set_view()
 * function.
 */
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

/**
 * Calls the virtual shutdown() function of the SPViewWidget.
 */
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
