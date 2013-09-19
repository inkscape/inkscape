#ifndef SP_FECOMPONENTTRANSFER_FUNCNODE_H_SEEN
#define SP_FECOMPONENTTRANSFER_FUNCNODE_H_SEEN

/** \file
 * SVG <filter> implementation, see sp-filter.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"
#include "display/nr-filter-component-transfer.h"

//#define SP_TYPE_FEFUNCR (sp_fefuncR_get_type())
//#define SP_TYPE_FEFUNCG (sp_fefuncG_get_type())
//#define SP_TYPE_FEFUNCB (sp_fefuncB_get_type())
//#define SP_TYPE_FEFUNCA (sp_fefuncA_get_type())

// CPPIFY: Casting macros buggy, as these aren't classes.
//#define SP_IS_FEFUNCR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCR))
//#define SP_IS_FEFUNCG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCG))
//#define SP_IS_FEFUNCB(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCB))
//#define SP_IS_FEFUNCA(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCA))

#define SP_FEFUNCNODE(obj) (dynamic_cast<SPFeFuncNode*>((SPObject*)obj))

//#define SP_IS_FEFUNCR(obj) (obj != NULL && static_cast<const SPObject*>(obj)->typeHierarchy.count(typeid(SPFeFuncNode)))
//#define SP_IS_FEFUNCG(obj) (obj != NULL && static_cast<const SPObject*>(obj)->typeHierarchy.count(typeid(SPFeFuncNode)))
//#define SP_IS_FEFUNCB(obj) (obj != NULL && static_cast<const SPObject*>(obj)->typeHierarchy.count(typeid(SPFeFuncNode)))
//#define SP_IS_FEFUNCA(obj) (obj != NULL && static_cast<const SPObject*>(obj)->typeHierarchy.count(typeid(SPFeFuncNode)))

#define SP_IS_FEFUNCR(obj) (dynamic_cast<const SPFeFuncNode*>((SPObject*)obj) != NULL)
#define SP_IS_FEFUNCG(obj) (dynamic_cast<const SPFeFuncNode*>((SPObject*)obj) != NULL)
#define SP_IS_FEFUNCB(obj) (dynamic_cast<const SPFeFuncNode*>((SPObject*)obj) != NULL)
#define SP_IS_FEFUNCA(obj) (dynamic_cast<const SPFeFuncNode*>((SPObject*)obj) != NULL)

class SPFeFuncNode : public SPObject {
public:
	SPFeFuncNode();
	virtual ~SPFeFuncNode();

    Inkscape::Filters::FilterComponentTransferType type;
    std::vector<double> tableValues;
    double slope;
    double intercept;
    double amplitude;
    double exponent;
    double offset;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

#endif /* !SP_FECOMPONENTTRANSFER_FUNCNODE_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
