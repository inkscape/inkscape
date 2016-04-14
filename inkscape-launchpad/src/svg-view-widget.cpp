/*
 * Functions and callbacks for generic SVG view and widget.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 authors
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/sp-canvas.h"
#include "display/sp-canvas-group.h"
#include "display/canvas-arena.h"
#include "document.h"
#include "svg-view.h"
#include "svg-view-widget.h"
#include "util/units.h"

static void sp_svg_view_widget_dispose(GObject *object);

static void sp_svg_view_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void sp_svg_view_widget_size_request (GtkWidget *widget, GtkRequisition *req);

#if GTK_CHECK_VERSION(3,0,0)
static void sp_svg_view_widget_get_preferred_width(GtkWidget *widget, 
                                                   gint *minimal_width,
						   gint *natural_width);

static void sp_svg_view_widget_get_preferred_height(GtkWidget *widget, 
                                                    gint *minimal_height,
						    gint *natural_height);
#endif

static void sp_svg_view_widget_view_resized (SPViewWidget *vw, Inkscape::UI::View::View *view, gdouble width, gdouble height);

G_DEFINE_TYPE(SPSVGSPViewWidget, sp_svg_view_widget, SP_TYPE_VIEW_WIDGET);

/**
 * Callback to initialize SPSVGSPViewWidget vtable.
 */
static void sp_svg_view_widget_class_init(SPSVGSPViewWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	SPViewWidgetClass *vw_class = SP_VIEW_WIDGET_CLASS (klass);

	object_class->dispose = sp_svg_view_widget_dispose;

	widget_class->size_allocate = sp_svg_view_widget_size_allocate;
#if GTK_CHECK_VERSION(3,0,0)
	widget_class->get_preferred_width = sp_svg_view_widget_get_preferred_width;
	widget_class->get_preferred_height = sp_svg_view_widget_get_preferred_height;
#else
	widget_class->size_request = sp_svg_view_widget_size_request;
#endif

	vw_class->view_resized = sp_svg_view_widget_view_resized;
}

/**
 * Callback to initialize SPSVGSPViewWidget object.
 */
static void sp_svg_view_widget_init(SPSVGSPViewWidget *vw)
{
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
#if !GTK_CHECK_VERSION(3,0,0)
	GdkColormap *cmap = gdk_colormap_get_system();
	gtk_widget_push_colormap(cmap);
#endif

	vw->canvas = SPCanvas::createAA();

#if GTK_CHECK_VERSION(3,0,0)
        GtkCssProvider  *css_provider  = gtk_css_provider_new();
        GtkStyleContext *style_context = gtk_widget_get_style_context(GTK_WIDGET(vw->canvas));

        gtk_css_provider_load_from_data(css_provider,
                                        "SPCanvas {\n"
                                        " background-color: white;\n"
                                        "}\n",
                                        -1, NULL);

        gtk_style_context_add_provider(style_context,
                                       GTK_STYLE_PROVIDER(css_provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_USER);
#else
	gtk_widget_pop_colormap ();
	GtkStyle *style = gtk_style_copy (gtk_widget_get_style (vw->canvas));
	style->bg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (vw->canvas, style);
#endif

#if GTK_CHECK_VERSION(3,0,0)
	gtk_container_add (GTK_CONTAINER (vw->sw), vw->canvas);
#else
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (vw->sw), vw->canvas);
#endif

	gtk_widget_show (vw->canvas);

	/* View */
	parent = sp_canvas_item_new(SP_CANVAS(vw->canvas)->getRoot(), SP_TYPE_CANVAS_GROUP, NULL);
	Inkscape::UI::View::View *view = Inkscape::GC::release(new SPSVGView (SP_CANVAS_GROUP (parent)));
	sp_view_widget_set_view (SP_VIEW_WIDGET (vw), view);
}

/*
 * Destructor callback for SPSVGSPViewWidget objects.
 */
static void sp_svg_view_widget_dispose(GObject *object)
{
	SPSVGSPViewWidget *vw = SP_SVG_VIEW_WIDGET (object);

	vw->canvas = NULL;

	if (G_OBJECT_CLASS(sp_svg_view_widget_parent_class)->dispose) {
            G_OBJECT_CLASS(sp_svg_view_widget_parent_class)->dispose(object);
        }
}

/**
 * Callback connected with size_request signal.
 */
static void sp_svg_view_widget_size_request(GtkWidget *widget, GtkRequisition *req)
{
	SPSVGSPViewWidget *vw = SP_SVG_VIEW_WIDGET (widget);
	Inkscape::UI::View::View *v = SP_VIEW_WIDGET_VIEW (widget);

#if GTK_CHECK_VERSION(3,0,0)
	if (GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->get_preferred_width &&
            GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->get_preferred_height) {
		gint width_min, height_min, width_nat, height_nat;

		GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->get_preferred_width(widget, &width_min, &width_nat);
		GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->get_preferred_height(widget, &height_min, &height_nat);
		req->width=width_min;
		req->height=height_min;
        }
#else
	if (GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->size_request) {
            GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->size_request(widget, req);
        }
#endif

	if (v->doc()) {
		SPSVGView *svgv;
		GtkPolicyType hpol, vpol;
		gdouble width, height;

		svgv = static_cast<SPSVGView*> (v);
		width = (v->doc())->getWidth().value("px") * svgv->_hscale;
		height = (v->doc())->getHeight().value("px") * svgv->_vscale;

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

#if GTK_CHECK_VERSION(3,0,0)
static void sp_svg_view_widget_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	sp_svg_view_widget_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void sp_svg_view_widget_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	sp_svg_view_widget_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

/**
 * Callback connected with size_allocate signal.
 */
static void sp_svg_view_widget_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	SPSVGSPViewWidget *svgvw = SP_SVG_VIEW_WIDGET (widget);

	if (GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->size_allocate) {
            GTK_WIDGET_CLASS(sp_svg_view_widget_parent_class)->size_allocate(widget, allocation);
        }

	if (!svgvw->resize) {
		static_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (svgvw))->setRescale (TRUE, TRUE,
					 (gdouble) allocation->width - 1.0, (gdouble) allocation->height - 1.0);
	}
}

/**
 * Callback connected with view_resized signal.
 */
static void sp_svg_view_widget_view_resized(SPViewWidget *vw, Inkscape::UI::View::View */*view*/, gdouble width, gdouble height)
{
	SPSVGSPViewWidget *svgvw = SP_SVG_VIEW_WIDGET (vw);

	if (svgvw->resize) {
		gtk_widget_set_size_request (svgvw->canvas, (int)width, (int)height);
		gtk_widget_queue_resize (GTK_WIDGET (vw));
	}
}

GtkWidget *sp_svg_view_widget_new(SPDocument *doc)
{
	GtkWidget *widget;

	g_return_val_if_fail (doc != NULL, NULL);

	widget = (GtkWidget*)g_object_new (SP_TYPE_SVG_VIEW_WIDGET, NULL);

	reinterpret_cast<SPSVGView*>(SP_VIEW_WIDGET_VIEW (widget))->setDocument (doc);

	return widget;
}

void SPSVGSPViewWidget::setResize(bool resize, gdouble width, gdouble height)
{
    g_return_if_fail( !resize || (width > 0.0) );
    g_return_if_fail( !resize || (height > 0.0) );

    this->resize = resize;
    this->maxwidth = width;
    this->maxheight = height;

    if ( resize ) {
        gtk_widget_queue_resize( GTK_WIDGET(this) );
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
