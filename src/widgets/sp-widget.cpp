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
static void sp_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static void sp_widget_modify_selection (Inkscape::Application *inkscape, Inkscape::Selection *selection, guint flags, SPWidget *spw);
static void sp_widget_change_selection (Inkscape::Application *inkscape, Inkscape::Selection *selection, SPWidget *spw);
static void sp_widget_set_selection (Inkscape::Application *inkscape, Inkscape::Selection *selection, SPWidget *spw);

static GtkBinClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GtkType
sp_widget_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		static const GtkTypeInfo info = {
			(gchar*) "SPWidget",
			sizeof (SPWidget),
			sizeof (SPWidgetClass),
			(GtkClassInitFunc) sp_widget_class_init,
			(GtkObjectInitFunc) sp_widget_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_BIN, &info);
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

	parent_class = (GtkBinClass*)gtk_type_class (GTK_TYPE_BIN);

	object_class->destroy = sp_widget_destroy;

	signals[CONSTRUCT] =        gtk_signal_new ("construct",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, construct),
						    gtk_marshal_NONE__NONE,
						    GTK_TYPE_NONE, 0);
	signals[CHANGE_SELECTION] = gtk_signal_new ("change_selection",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, change_selection),
						    gtk_marshal_NONE__POINTER,
						    GTK_TYPE_NONE, 1,
						    GTK_TYPE_POINTER);
	signals[MODIFY_SELECTION] = gtk_signal_new ("modify_selection",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, modify_selection),
						    gtk_marshal_NONE__POINTER_UINT,
						    GTK_TYPE_NONE, 2,
						    GTK_TYPE_POINTER, GTK_TYPE_UINT);
	signals[SET_SELECTION] =    gtk_signal_new ("set_selection",
						    GTK_RUN_FIRST,
						    GTK_CLASS_TYPE(object_class),
						    GTK_SIGNAL_OFFSET (SPWidgetClass, set_selection),
						    gtk_marshal_NONE__POINTER,
						    GTK_TYPE_NONE, 1,
						    GTK_TYPE_POINTER);

	widget_class->show = sp_widget_show;
	widget_class->hide = sp_widget_hide;
	widget_class->expose_event = sp_widget_expose;
	widget_class->size_request = sp_widget_size_request;
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
	GtkBin *bin;

	bin = GTK_BIN (widget);

        if ( bin->child ) {
            gtk_container_propagate_expose (GTK_CONTAINER(widget), bin->child, event);
        }
	/*
	if ((bin->child) && (GTK_WIDGET_NO_WINDOW (bin->child))) {
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
	if (((GtkBin *) widget)->child)
		gtk_widget_size_request (((GtkBin *) widget)->child, requisition);
}

static void
sp_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	widget->allocation = *allocation;

	if (((GtkBin *) widget)->child)
		gtk_widget_size_allocate (((GtkBin *) widget)->child, allocation);
}

/* Methods */

GtkWidget *
sp_widget_new_global (Inkscape::Application *inkscape)
{
	SPWidget *spw;

	spw = (SPWidget*)gtk_type_new (SP_TYPE_WIDGET);

	if (!sp_widget_construct_global (spw, inkscape)) {
		gtk_object_unref (GTK_OBJECT (spw));
		return NULL;
	}

	return (GtkWidget *) spw;
}

GtkWidget *
sp_widget_construct_global (SPWidget *spw, Inkscape::Application *inkscape)
{
	g_return_val_if_fail (!spw->inkscape, NULL);

	spw->inkscape = inkscape;
	if (GTK_WIDGET_VISIBLE (spw)) {
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
