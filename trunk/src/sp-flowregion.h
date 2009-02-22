#ifndef __SP_ITEM_FLOWREGION_H__
#define __SP_ITEM_FLOWREGION_H__

/*
 */

#include "sp-item.h"

#define SP_TYPE_FLOWREGION            (sp_flowregion_get_type ())
#define SP_FLOWREGION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWREGION, SPFlowregion))
#define SP_FLOWREGION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWREGION, SPFlowregionClass))
#define SP_IS_FLOWREGION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWREGION))
#define SP_IS_FLOWREGION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWREGION))

#define SP_TYPE_FLOWREGIONEXCLUDE            (sp_flowregionexclude_get_type ())
#define SP_FLOWREGIONEXCLUDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWREGIONEXCLUDE, SPFlowregionExclude))
#define SP_FLOWREGIONEXCLUDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWREGIONEXCLUDE, SPFlowregionExcludeClass))
#define SP_IS_FLOWREGIONEXCLUDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWREGIONEXCLUDE))
#define SP_IS_FLOWREGIONEXCLUDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWREGIONEXCLUDE))

class Path;
class Shape;
class flow_dest;
class FloatLigne;

struct SPFlowregion : public SPItem {
	std::vector<Shape*>     computed;
	
	void             UpdateComputed(void);
};

struct SPFlowregionClass {
	SPItemClass parent_class;
};

GType sp_flowregion_get_type (void);

struct SPFlowregionExclude : public SPItem {
	Shape            *computed;
	
	void             UpdateComputed(void);
};

struct SPFlowregionExcludeClass {
	SPItemClass parent_class;
};

GType sp_flowregionexclude_get_type (void);

#endif
