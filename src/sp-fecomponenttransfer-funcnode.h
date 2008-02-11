#ifndef SP_FECOMPONENTTRANSFER_FUNCNODE_H_SEEN
#define SP_FECOMPONENTTRANSFER_FUNCNODE_H_SEEN

/** \file
 * SVG <filter> implementation, see sp-filter.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"
#include "display/nr-filter-component-transfer.h"

#define SP_TYPE_FEFUNCR (sp_fefuncR_get_type())
#define SP_TYPE_FEFUNCG (sp_fefuncG_get_type())
#define SP_TYPE_FEFUNCB (sp_fefuncB_get_type())
#define SP_TYPE_FEFUNCA (sp_fefuncA_get_type())

#define SP_IS_FEFUNCR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCR))
#define SP_IS_FEFUNCG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCG))
#define SP_IS_FEFUNCB(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCB))
#define SP_IS_FEFUNCA(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFUNCA))

#define SP_FEFUNCNODE(obj) (SP_IS_FEFUNCR(obj) ? G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEFUNCR, SPFeFuncNode) : (SP_IS_FEFUNCG(obj) ? G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEFUNCG, SPFeFuncNode) : (SP_IS_FEFUNCB(obj) ? G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEFUNCB, SPFeFuncNode):(SP_IS_FEFUNCA(obj) ? G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEFUNCA, SPFeFuncNode): NULL))))

#define SP_FEFUNCNODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEFUNCNODE, SPFeFuncNodeClass))

#define SP_IS_FEFUNCNODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEFUNCNODE))

/* Component Transfer funcNode class */

class SPFeFuncNode;
class SPFeFuncNodeClass;

struct SPFeFuncNode : public SPObject {
    NR::FilterComponentTransferType type;
    std::vector<double> tableValues;
    double slope;
    double intercept;
    double amplitude;
    double exponent;
    double offset;
};

struct SPFeFuncNodeClass {
    SPObjectClass parent_class;
};

GType sp_fefuncR_get_type();
GType sp_fefuncG_get_type();
GType sp_fefuncB_get_type();
GType sp_fefuncA_get_type();

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
