#ifndef SP_CANVAS_GRID_H
#define SP_CANVAS_GRID_H

/*
 * SPCGrid
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <display/sp-canvas.h>



#define SP_TYPE_CGRID            (sp_cgrid_get_type ())
#define SP_CGRID(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_CGRID, SPCGrid))
#define SP_CGRID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CGRID, SPCGridClass))
#define SP_IS_CGRID(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_CGRID))
#define SP_IS_CGRID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CGRID))


/** \brief  All the variables that are tracked for a grid specific
            canvas item. */
struct SPCGrid : public SPCanvasItem{
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

struct SPCGridClass {
	SPCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType sp_cgrid_get_type (void);



#endif
