#ifndef SP_CANVAS_AXONOMGRID_H
#define SP_CANVAS_AXONOMGRID_H

/*
 * SPCAxonomGrid
 *
 * Generic (and quite unintelligent) modified copy of the grid item for gnome canvas
 *
 * Copyright (C) 2006 Johan Engelen  <johan@shouraizou.nl>
 * Copyright (C) 2000 Lauris Kaplinski 2000
 *
 */

#include <display/sp-canvas.h>
#include <libnr/nr-coord.h>


#define SP_TYPE_CAXONOMGRID            (sp_caxonomgrid_get_type ())
#define SP_CAXONOMGRID(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_CAXONOMGRID, SPCAxonomGrid))
#define SP_CAXONOMGRID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CAXONOMGRID, SPCAxonomGridClass))
#define SP_IS_CAXONOMGRID(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_CAXONOMGRID))
#define SP_IS_CAXONOMGRID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CAXONOMGRID))


/** \brief  All the variables that are tracked for a axonometric grid specific
            canvas item. */
struct SPCAxonomGrid : public SPCanvasItem{
	NR::Point origin;     /**< Origin of the grid */
	double lengthy;       /**< The lengths of the primary y-axis */
	double angle_deg[3];  /**< Angle of each axis (note that angle[2] == 0) */
	double angle_rad[3];  /**< Angle of each axis (note that angle[2] == 0) */
	double tan_angle[3];  /**< tan(angle[.]) */
	guint32 color;        /**< Color for normal lines */
	guint32 empcolor;     /**< Color for emphasis lines */
	gint empspacing;      /**< Spacing between emphasis lines */
	bool scaled;          /**< Whether the grid is in scaled mode */
	
	NR::Point ow;         /**< Transformed origin by the affine for the zoom */
	double lyw;           /**< Transformed length y by the affine for the zoom */
	double lxw_x;
	double lxw_z;
	double spacing_ylines;
                          
    NR::Point sw;          /**< the scaling factors of the affine transform */
};

struct SPCAxonomGridClass {
	SPCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType sp_caxonomgrid_get_type (void);



#endif    


