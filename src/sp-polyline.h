#ifndef SP_POLYLINE_H
#define SP_POLYLINE_H

#include "sp-shape.h"



#define SP_TYPE_POLYLINE            (sp_polyline_get_type ())
#define SP_POLYLINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_POLYLINE, SPPolyLine))
#define SP_POLYLINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_POLYLINE, SPPolyLineClass))
#define SP_IS_POLYLINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_POLYLINE))
#define SP_IS_POLYLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_POLYLINE))

class SPPolyLine;
class SPPolyLineClass;

struct SPPolyLine : public SPShape {
};

struct SPPolyLineClass {
	SPShapeClass parent_class;
};

GType sp_polyline_get_type (void);



#endif
