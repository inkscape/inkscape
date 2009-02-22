#ifndef __SP_SVG_VIEW_WIDGET_H__
#define __SP_SVG_VIEW_WIDGET_H__

/** \file
 * SPSVGView, SPSVGSPViewWidget: Generic SVG view and widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"
#include "ui/view/view-widget.h"

class SPDocument;
class SPSVGSPViewWidget;
class SPSVGSPViewWidgetClass;

#define SP_TYPE_SVG_VIEW_WIDGET (sp_svg_view_widget_get_type ())
#define SP_SVG_VIEW_WIDGET(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_SVG_VIEW_WIDGET, SPSVGSPViewWidget))
#define SP_SVG_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_SVG_VIEW_WIDGET, SPSVGSPViewWidgetClass))
#define SP_IS_SVG_VIEW_WIDGET(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_SVG_VIEW_WIDGET))
#define SP_IS_SVG_VIEW_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_SVG_VIEW_WIDGET))

GtkType sp_svg_view_widget_get_type (void);

GtkWidget *sp_svg_view_widget_new (SPDocument *doc);

void sp_svg_view_widget_set_resize (SPSVGSPViewWidget *vw, bool resize, gdouble width, gdouble height);

/**
 * An SPSVGSPViewWidget is an SVG view together with a canvas.
 */
struct SPSVGSPViewWidget {
    public:
	SPViewWidget widget;

	GtkWidget *sw;
	GtkWidget *canvas;

	/// Whether to resize automatically
	bool resize;
	gdouble maxwidth, maxheight;

    // C++ Wrappers
    /// Flags the SPSVGSPViewWidget to have its size changed with Gtk.
    void setResize(bool resize, gdouble width, gdouble height) {
	sp_svg_view_widget_set_resize(this, resize, width, height);
    }
};

/// The SPSVGSPViewWidget vtable.
struct SPSVGSPViewWidgetClass {
	SPViewWidgetClass parent_class;
};


#endif
