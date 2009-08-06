#define __SP_SVG_VIEW_C__

/** \file
 * Functions and callbacks for generic SVG view and widget
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

#include <gtk/gtkscrolledwindow.h>
#include "display/canvas-arena.h"
#include "document.h"
#include "svg-view.h"
#include "svg-view-widget.h"

static void sp_svg_view_widget_class_init (SPSVGSPViewWidgetClass *klass);
static void sp_svg_view_widget_init (SPSVGSPViewWidget *widget);
static void sp_svg_view_widget_destroy (GtkObject *object);

static void sp_svg_view_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void sp_svg_view_widget_size_request (GtkWidget *widget, GtkRequisition *req);

static void sp_svg_view_widget_view_resized (SPViewWidget *vw, Inkscape::UI::View::View *view, gdouble width, gdouble height);

static SPViewWidgetClass *widget_parent_class;

/**
 * Registers SPSVGSPViewWidget class with Gtk and returns its type number.
 */
GType sp_svg_view_widget_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPSVGSPViewWidgetClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_svg_view_widget_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPSVGSPViewWidget),
            0, // n_preallocs
            (GInstanceInitFunc)sp_svg_view_widget_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_VIEW_WIDGET, "SPSVGSPViewWidget", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

/**
 * Callback to initialize SPSVGSPViewWidget vtable.
 */
static void
sp_svg_view_widget_class_init (SPSVGSPViewWidgetClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPViewWidgetClass *vw_class;

	object_class = GTK_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);
	vw_class = SP_VIEW_WIDGET_CLASS (klass);

	widget_parent_class = (SPViewWidgetClass*)gtk_type_class (SP_TYPE_VIEW_WIDGET);

	object_class->destroy = sp_svg_view_widget_destroy;

	widget_class->size_allocate = sp_svg_view_widget_size_allocate;
	widget_class->size_request = sp_svg_view_widget_size_request;

	vw_class->view_resized = sp_svg_view_widget_view_resized;
}

/**
 * Callback to initialize SPSVGSPViewWidget object.
 */
static void
sp_svg_view_widget_init (SPSVGSPViewWidget *vw)
{
	GtkStyle *style;
	SPCanvasItem *parent;

	/* Settings */
	vw->resize = FALSE;
	vw->maxwidth = 400.0;
	vw->maxheight = 400.0;

	/* ScrolledWindow */
	vw->sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(vw->sw), GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (vw->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (vw), vw->sw);
	gtk_widget_show (vw->sw);

	/* Canvas */
	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	vw->canvas = sp_canvas_new_aa ();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	style = gtk_style_copy (vw->canvas->style);
	style->bg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (vw->canvas, style);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (vw->sw), vw->canvas);
	gtk_widget_show (vw->canvas);

	/* View */
	parent = sp_canvas_item_new (sp_canvas_root (SP_CANVAS (vw->canvas)), SP_TYPE_CANVAS_GROUP, NULL);
	Inkscape::UI::View::View *view = Inkscape::GC::release(new SPSVGView (SP_CANVAS_GROUP (parent)));
	sp_view_widget_set_view (SP_VIEW_WIDGET (vw), view);
}

/*
 * Destructor callback for SPSVGSPViewWidget objects.
 */
static void
sp_svg_view_widget_destroy (GtkObject *object)
{
	SPSVGSPViewWidget *vw = SP_SVG_VIEW_WIDGET (object);

	vw->canvas = NULL;

	if (((GtkObjectClass *) (widget_parent_class))->destroy)
		(* ((GtkObjectClass *) (widget_parent_class))->destroy) (object);
}

/**
 * Callback connected with size_request signal.
 */
static void
sp_svg_view_widget_size_request (GtkWidget *widget, GtkRequisition *req)
{
	SPSVGSPViewWidget *vw = SP_SVG_VIEW_WIDGET (widget);
	Inkscape::UI::View::View *v = SP_VIEW_WIDGET_VIEW (widget);

	if (((GtkWidgetClass *) (widget_parent_class))->size_request)
		(* ((GtkWidgetClass *) (widget_parent_class))->size_request) (widget, req);

	if (v->doc()) {
		SPSVGView *svgv;
		GtkPolicyType hpol, vpol;
		gdouble width, height;

		svgv = static_cast<SPSVGView*> (v);
		width = sp_document_width (v->doc()) * svgv->_hscale;
		height = sp_document_height (v->doc()) * svgv->_vscale;

		if (width <= vw->maxwidth) {
			hpol = GTK_POLICY_NEVER;
			req->width = (gint) (width + 0.5);
		} else {
			hpol = GTK_POLICY_AUTOMATIC;
			req->width = (gint) (vw->maxwidth + 0.5);
		}
		if (height <= vw->maxheight) {
			vpol = GTK_POLICY_NEVER;
			req->height = (gint) (height + 8.0);
		} else {
			vpol = GTK_POLICY_AUTOMATIC;
			req->height = (gint) (vw->maxheight + 2.0);
		}
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (vw->sw), hpol, vpol);
	}
}

/**
 * Callback connected with size_allocate signal.
 */
static void
sp_svg_view_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	SPSVGSPViewWidget *svgvw = SP_SVG_VIEW_WIDGET (widget);

	if (((GtkWidgetClass *) (widget_parent_class))->size_allocate)
		(* ((GtkWidgetClass *) (widget_parent_class))->size_allocate) (widget, allocation);

	if (!svgvw->resize) {
		static_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (svgvw))->setRescale (TRUE, TRUE,
					 (gdouble) allocation->width - 1.0, (gdouble) allocation->height - 1.0);
	}
}

/**
 * Callback connected with view_resized signal.
 */
static void
sp_svg_view_widget_view_resized (SPViewWidget *vw, Inkscape::UI::View::View */*view*/, gdouble width, gdouble height)
{
	SPSVGSPViewWidget *svgvw = SP_SVG_VIEW_WIDGET (vw);

	if (svgvw->resize) {
		gtk_widget_set_size_request (svgvw->canvas, (int)width, (int)height);
		gtk_widget_queue_resize (GTK_WIDGET (vw));
	}
}

/**
 * Constructs new SPSVGSPViewWidget object and returns pointer to it.
 */
GtkWidget *
sp_svg_view_widget_new (SPDocument *doc)
{
	GtkWidget *widget;

	g_return_val_if_fail (doc != NULL, NULL);

	widget = (GtkWidget*)gtk_type_new (SP_TYPE_SVG_VIEW_WIDGET);

	reinterpret_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (widget))->setDocument (doc);

	return widget;
}

/**
 * Flags the SPSVGSPViewWidget to have its size renegotiated with Gtk.
 */
void
sp_svg_view_widget_set_resize (SPSVGSPViewWidget *vw, bool resize, gdouble width, gdouble height)
{
	g_return_if_fail (vw != NULL);

	g_return_if_fail (SP_IS_SVG_VIEW_WIDGET (vw));
	g_return_if_fail (!resize || (width > 0.0));
	g_return_if_fail (!resize || (height > 0.0));

	vw->resize = resize;
	vw->maxwidth = width;
	vw->maxheight = height;

	if (resize) {
		gtk_widget_queue_resize (GTK_WIDGET (vw));
	}
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
