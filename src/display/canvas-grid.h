#ifndef INKSCAPE_CANVAS_GRID_H
#define INKSCAPE_CANVAS_GRID_H

/*
 * Inkscape::CXYGrid
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Johan Engelen 2006 <johan@shouraizou.nl>
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <display/sp-canvas.h>

namespace Inkscape {

#define INKSCAPE_TYPE_CXYGRID            (Inkscape::cxygrid_get_type ())
#define INKSCAPE_CXYGRID(obj)            (GTK_CHECK_CAST ((obj), INKSCAPE_TYPE_CXYGRID, CXYGrid))
#define INKSCAPE_CXYGRID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), INKSCAPE_TYPE_CXYGRID, CXYGridClass))
#define INKSCAPE_IS_CXYGRID(obj)         (GTK_CHECK_TYPE ((obj), INKSCAPE_TYPE_CXYGRID))
#define INKSCAPE_IS_CXYGRID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), INKSCAPE_TYPE_CXYGRID))


/** \brief  All the variables that are tracked for a grid specific
            canvas item. */
struct CXYGrid : public SPCanvasItem{
	NR::Point origin;  /**< Origin of the grid */
	NR::Point spacing; /**< Spacing between elements of the grid */
	guint32 color;     /**< Color for normal lines */
	guint32 empcolor;  /**< Color for emphisis lines */
	gint empspacing;   /**< Spacing between emphisis lines */
	bool scaled[2];    /**< Whether the grid is in scaled mode, which can
					        be different in the X or Y direction, hense two
						    variables */
	NR::Point ow;      /**< Transformed origin by the affine for the zoom */
	NR::Point sw;      /**< Transformed spacing by the affine for the zoom */
};

struct CXYGridClass {
	SPCanvasItemClass parent_class;
};

/* Standard Gtk function */
GtkType cxygrid_get_type (void);

}; /* namespace Inkscape */




#endif
