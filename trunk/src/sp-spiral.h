#ifndef __SP_SPIRAL_H__
#define __SP_SPIRAL_H__

/** \file
 * SPSpiral: <sodipodi:spiral> implementation
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"

#define noSPIRAL_VERBOSE

#define SP_EPSILON       1e-5
#define SP_EPSILON_2     (SP_EPSILON * SP_EPSILON)
#define SP_HUGE          1e5

#define SPIRAL_TOLERANCE 3.0
#define SAMPLE_STEP      (1.0/4.0) ///< step per 2PI 
#define SAMPLE_SIZE      8         ///< sample size per one bezier 

#define SP_TYPE_SPIRAL            (sp_spiral_get_type ())
#define SP_SPIRAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_SPIRAL, SPSpiral))
#define SP_SPIRAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_SPIRAL, SPSpiralClass))
#define SP_IS_SPIRAL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_SPIRAL))
#define SP_IS_SPIRAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_SPIRAL))

class SPSpiral;
class SPSpiralClass;

/**
 * A spiral Shape.
 *
 * The Spiral shape is defined as:
 * \verbatim
   x(t) = rad * t^exp cos(2 * Pi * revo*t + arg) + cx
   y(t) = rad * t^exp sin(2 * Pi * revo*t + arg) + cy    \endverbatim
 * where spiral curve is drawn for {t | t0 <= t <= 1}. The  rad and arg 
 * parameters can also be represented by transformation. 
 *
 * \todo Should I remove these attributes?
 */
struct SPSpiral : public SPShape {
	float cx, cy;
	float exp;  ///< Spiral expansion factor
	float revo; ///< Spiral revolution factor
	float rad;  ///< Spiral radius
	float arg;  ///< Spiral argument
	float t0;
};

/// The SPSpiral vtable.
struct SPSpiralClass {
	SPShapeClass parent_class;
};


/* Standard Gtk function */
GType sp_spiral_get_type  (void);

/* Lowlevel interface */
void    sp_spiral_position_set		(SPSpiral      *spiral,
				 gdouble	cx,
				 gdouble	cy,
				 gdouble	exp,
				 gdouble	revo,
				 gdouble	rad,
				 gdouble	arg,
				 gdouble	t0);

Geom::Point    sp_spiral_get_xy	(SPSpiral const *spiral,
				 gdouble	t);

void    sp_spiral_get_polar	(SPSpiral const *spiral,
				 gdouble	t,
				 gdouble       *rad,
				 gdouble       *arg);

bool sp_spiral_is_invalid   (SPSpiral const *spiral);




#endif
