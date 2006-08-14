#ifndef __SP_FILTER_PRIMITIVE_H__
#define __SP_FILTER_PRIMITIVE_H__


#include "sp-object.h"


#define SP_TYPE_FILTER_PRIMITIVE (sp_filter_primitive_get_type ())
#define SP_FILTER_PRIMITIVE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FILTER_PRIMITIVE, SPFilterPrimitive))
#define SP_FILTER_PRIMITIVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FILTER_PRIMITIVE, SPFilterPrimitiveClass))
#define SP_IS_FILTER_PRIMITIVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FILTER_PRIMITIVE))
#define SP_IS_FILTER_PRIMITIVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FILTER_PRIMITIVE))

class SPFilterPrimitive;
class SPFilterPrimitiveClass;

struct SPFilterPrimitive : public SPObject {
};

struct SPFilterPrimitiveClass {
	SPObjectClass sp_object_class;
};

GType sp_filter_primitive_get_type (void);

#endif
