#define __SP_WIDGET_C__

/*
 * Abstract base class for dynamic control widgets
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "macros.h"
#include "../document.h"
#include "sp-widget.h"
#include "helper/sp-marshal.h"

enum {
	CONSTRUCT,
	MODIFY_SELECTION,
	CHANGE_SELECTION,
	SET_SELECTION,
	LAST_SIGNAL
};

static void sp_widget_class_init (SPWidgetClass *klass);
static void sp_widget_init (SPWidget *widget);

static void sp_widget_destroy (GtkObject *object);

static void sp_widget_show (GtkWidget *widget);
static void sp_widget_hide (GtkWidget *widget);
static gint sp_widget_expose (GtkWidget *widget, GdkEventExpose *event);
static void sp_widget_size_request (GtkWidget *widget, GtkRequisition *requisition);

#if GTK_CHECK_VERSION(3,0,0)
static void sp_widget_get_preferred_width(GtkWidget *widget, 
                                                   gint *minimal_width,
						   gint *natural_width);

static void sp_widget_get_preferred_height(GtkWidget *widget, 
                                                    gint *minimal_height,
						    gint *natural_height);
#endif

static void sp_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static void sp_widget_modify_selection (Inkscape::Application *inkscape, Inkscape::Selection *selection, guint flags, SPWidget *spw);
static void sp_widget_change_selection (Inkscape::Application *inkscape, Inkscape::Selection *selection, SPWidget *spw);
static void sp_widget_set_selection (Inkscape::Application *inkscape, Inkscape::Selection *selection, SPWidget *spw);

static GtkBinClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GType
sp_widget_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPWidgetClass),
			NULL, NULL,
			(GClassInitFunc) sp_widget_class_init,
			NULL, NULL,
			sizeof (SPWidget),
			0,
			(GInstanceInitFunc) sp_widget_init,
			NULL			
		};
		type = g_type_register_static (GTK_TYPE_BIN, 
				              "SPWidget",
					      &info,
					      (GTypeFlags)0);
	}
	return type;
}

static void
sp_widget_class_init (SPWidgetClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = (GtkBinClass*)g_type_class_peek_parent (klass);

	object_class->destroy = sp_widget_destroy;

	signals[CONSTRUCT] =        g_signal_new ("construct",
						    G_TYPE_FROM_CLASS(object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET (SPWidgetClass, construct),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__VOID,
						    G_TYPE_NONE, 0);
	signals[CHANGE_SELECTION] = g_signal_new ("change_selection",
						    G_TYPE_FROM_CLASS(object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET (SPWidgetClass, change_selection),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__POINTER,
						    G_TYPE_NONE, 1,
						    G_TYPE_POINTER);
	signals[MODIFY_SELECTION] = g_signal_new ("modify_selection",
						    G_TYPE_FROM_CLASS(object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET (SPWidgetClass, modify_selection),
						    NULL, NULL,
						    sp_marshal_VOID__POINTER_UINT,
						    G_TYPE_NONE, 2,
						    G_TYPE_POINTER, G_TYPE_UINT);
	signals[SET_SELECTION] =    g_signal_new ("set_selection",
						    G_TYPE_FROM_CLASS(object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET (SPWidgetClass, set_selection),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__POINTER,
						    G_TYPE_NONE, 1,
						    G_TYPE_POINTER);

	widget_class->show = sp_widget_show;
	widget_class->hide = sp_widget_hide;
	widget_class->expose_event = sp_widget_expose;
#if GTK_CHECK_VERSION(3,0,0)
	widget_class->get_preferred_width = sp_widget_get_preferred_width;
	widget_class->get_preferred_height = sp_widget_get_preferred_height;
#else
	widget_class->size_request = sp_widget_size_request;
#endif
	widget_class->size_allocate = sp_widget_size_allocate;
}

static void
sp_widget_init (SPWidget *spw)
{
	spw->inkscape = NULL;
}

static void
sp_widget_destroy (GtkObject *object)
{
	SPWidget *spw;

	spw = (SPWidget *) object;

	if (spw->inkscape) {
		/* Disconnect signals */
		// the checks are necessary because when destroy is caused by the the program shutting down,
		// the inkscape object may already be (partly?) invalid --bb
		if (G_IS_OBJECT(spw->inkscape) && G_OBJECT_GET_CLASS(G_OBJECT(spw->inkscape)))
  			sp_signal_disconnect_by_data (spw->inkscape, spw);
		spw->inkscape = NULL;
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

static void
sp_widget_show (GtkWidget *widget)
{
	SPWidget *spw;

	spw = SP_WIDGET (widget);

	if (spw->inkscape) {
		/* Connect signals */
		g_signal_connect (G_OBJECT (spw->inkscape), "modify_selection", G_CALLBACK (sp_widget_modify_selection), spw);
		g_signal_connect (G_OBJECT (spw->inkscape), "change_selection", G_CALLBACK (sp_widget_change_selection), spw);
		g_signal_connect (G_OBJECT (spw->inkscape), "set_selection", G_CALLBACK (sp_widget_set_selection), spw);
	}

	if (((GtkWidgetClass *) parent_class)->show)
		(* ((GtkWidgetClass *) parent_class)->show) (widget);
}

static void
sp_widget_hide (GtkWidget *widget)
{
	SPWidget *spw;

	spw = SP_WIDGET (widget);

	if (spw->inkscape) {
		/* Disconnect signals */
		sp_signal_disconnect_by_data (spw->inkscape, spw);
	}

	if (((GtkWidgetClass *) parent_class)->hide)
		(* ((GtkWidgetClass *) parent_class)->hide) (widget);
}

static gint
sp_widget_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkBin    *bin;
	GtkWidget *child;

	bin = GTK_BIN (widget);
	child = gtk_bin_get_child (bin);

        if (child) {
            gtk_container_propagate_expose (GTK_CONTAINER(widget), child, event);
        }
	/*
	if ((bin->child) && (!gtk_widget_get_has_window (bin->child))) {
		GdkEventExpose ce;
		ce = *event;
		gtk_widget_event (bin->child, (GdkEvent *) &ce);
	}
	*/

	return FALSE;
}

static void
sp_widget_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkBin    *bin   = GTK_BIN (widget);
	GtkWidget *child = gtk_bin_get_child (bin);

	if (child)
		gtk_widget_size_request (child, requisition);
}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_widget_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	sp_widget_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void sp_widget_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	sp_widget_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

static void
sp_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkBin        *bin   = GTK_BIN (widget);
	GtkWidget     *child = gtk_bin_get_child (bin);

	gtk_widget_set_allocation (widget, allocation);

	if (child)
		gtk_widget_size_allocate (child, allocation);
}

/* Methods */

GtkWidget *
sp_widget_new_global (Inkscape::Application *inkscape)
{
	SPWidget *spw;

	spw = (SPWidget*)g_object_new (SP_TYPE_WIDGET, NULL);

	if (!sp_widget_construct_global (spw, inkscape)) {
	    g_object_unref (spw);
		return NULL;
	}

	return (GtkWidget *) spw;
}

GtkWidget *
sp_widget_construct_global (SPWidget *spw, Inkscape::Application *inkscape)
{
	g_return_val_if_fail (!spw->inkscape, NULL);

	spw->inkscape = inkscape;
	if (gtk_widget_get_visible (GTK_WIDGET(spw))) {
		g_signal_connect (G_OBJECT (inkscape), "modify_selection", G_CALLBACK (sp_widget_modify_selection), spw);
		g_signal_connect (G_OBJECT (inkscape), "change_selection", G_CALLBACK (sp_widget_change_selection), spw);
		g_signal_connect (G_OBJECT (inkscape), "set_selection", G_CALLBACK (sp_widget_set_selection), spw);
	}

	g_signal_emit (G_OBJECT (spw), signals[CONSTRUCT], 0);

	return (GtkWidget *) spw;
}

static void
sp_widget_modify_selection (Inkscape::Application */*inkscape*/, Inkscape::Selection *selection, guint flags, SPWidget *spw)
{
	g_signal_emit (G_OBJECT (spw), signals[MODIFY_SELECTION], 0, selection, flags);
}

static void
sp_widget_change_selection (Inkscape::Application */*inkscape*/, Inkscape::Selection *selection, SPWidget *spw)
{
	g_signal_emit (G_OBJECT (spw), signals[CHANGE_SELECTION], 0, selection);
}

static void
sp_widget_set_selection (Inkscape::Application */*inkscape*/, Inkscape::Selection *selection, SPWidget *spw)
{
	/* Emit "set_selection" signal */
	g_signal_emit (G_OBJECT (spw), signals[SET_SELECTION], 0, selection);
	/* Inkscape will force "change_selection" anyways */
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
