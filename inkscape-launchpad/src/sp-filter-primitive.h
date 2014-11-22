#ifndef SEEN_SP_FILTER_PRIMITIVE_H
#define SEEN_SP_FILTER_PRIMITIVE_H

/** \file
 * Document level base class for all SVG filter primitives.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"
#include "svg/svg-length.h"

#define SP_FILTER_PRIMITIVE(obj) (dynamic_cast<SPFilterPrimitive*>((SPObject*)obj))
#define SP_IS_FILTER_PRIMITIVE(obj) (dynamic_cast<const SPFilterPrimitive*>((SPObject*)obj) != NULL)

namespace Inkscape {
namespace Filters {
class Filter;
class FilterPrimitive;
} }

class SPFilterPrimitive : public SPObject {
public:
	SPFilterPrimitive();
	virtual ~SPFilterPrimitive();

    int image_in, image_out;

    /* filter primitive subregion */
    SVGLength x, y, height, width;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, char const* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);

public:
	virtual void build_renderer(Inkscape::Filters::Filter* filter) = 0;
};

/* Common initialization for filter primitives */
void sp_filter_primitive_renderer_common(SPFilterPrimitive *sp_prim, Inkscape::Filters::FilterPrimitive *nr_prim);

int sp_filter_primitive_name_previous_out(SPFilterPrimitive *prim);
int sp_filter_primitive_read_in(SPFilterPrimitive *prim, char const *name);
int sp_filter_primitive_read_result(SPFilterPrimitive *prim, char const *name);

#endif
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
