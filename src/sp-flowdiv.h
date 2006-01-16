#ifndef __SP_ITEM_FLOWDIV_H__
#define __SP_ITEM_FLOWDIV_H__

/*
 */

#include "sp-object.h"
#include "sp-item.h"

#define SP_TYPE_FLOWDIV            (sp_flowdiv_get_type ())
#define SP_FLOWDIV(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWDIV, SPFlowdiv))
#define SP_FLOWDIV_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWDIV, SPFlowdivClass))
#define SP_IS_FLOWDIV(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWDIV))
#define SP_IS_FLOWDIV_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWDIV))

#define SP_TYPE_FLOWTSPAN            (sp_flowtspan_get_type ())
#define SP_FLOWTSPAN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWTSPAN, SPFlowtspan))
#define SP_FLOWTSPAN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWTSPAN, SPFlowtspanClass))
#define SP_IS_FLOWTSPAN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWTSPAN))
#define SP_IS_FLOWTSPAN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWTSPAN))

#define SP_TYPE_FLOWPARA            (sp_flowpara_get_type ())
#define SP_FLOWPARA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWPARA, SPFlowpara))
#define SP_FLOWPARA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWPARA, SPFlowparaClass))
#define SP_IS_FLOWPARA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWPARA))
#define SP_IS_FLOWPARA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWPARA))

#define SP_TYPE_FLOWLINE            (sp_flowline_get_type ())
#define SP_FLOWLINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWLINE, SPFlowline))
#define SP_FLOWLINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWLINE, SPFlowlineClass))
#define SP_IS_FLOWLINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWLINE))
#define SP_IS_FLOWLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWLINE))

#define SP_TYPE_FLOWREGIONBREAK            (sp_flowregionbreak_get_type ())
#define SP_FLOWREGIONBREAK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_FLOWREGIONBREAK, SPFlowregionbreak))
#define SP_FLOWREGIONBREAK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_FLOWREGIONBREAK, SPFlowregionbreakClass))
#define SP_IS_FLOWREGIONBREAK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_FLOWREGIONBREAK))
#define SP_IS_FLOWREGIONBREAK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_FLOWREGIONBREAK))

// these 3 are derivatives of SPItem to get the automatic style handling
struct SPFlowdiv : public SPItem {
};

struct SPFlowtspan : public SPItem {
};

struct SPFlowpara : public SPItem {
};

// these do not need any style
struct SPFlowline : public SPObject {
};

struct SPFlowregionbreak : public SPObject {
};


struct SPFlowdivClass {
	SPItemClass parent_class;
};

struct SPFlowtspanClass {
	SPItemClass parent_class;
};

struct SPFlowparaClass {
	SPItemClass parent_class;
};

struct SPFlowlineClass {
	SPObjectClass parent_class;
};

struct SPFlowregionbreakClass {
	SPObjectClass parent_class;
};

GType sp_flowdiv_get_type (void);
GType sp_flowtspan_get_type (void);
GType sp_flowpara_get_type (void);
GType sp_flowline_get_type (void);
GType sp_flowregionbreak_get_type (void);

#endif
