/** @file
 * @brief SVG blend filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FEBLEND_H_SEEN
#define SP_FEBLEND_H_SEEN

#include "sp-filter-primitive.h"
#include "display/nr-filter-blend.h"

#define SP_FEBLEND(obj) (dynamic_cast<SPFeBlend*>((SPObject*)obj))
#define SP_IS_FEBLEND(obj) (dynamic_cast<const SPFeBlend*>((SPObject*)obj) != NULL)

class SPFeBlend : public SPFilterPrimitive {
public:
	SPFeBlend();
	virtual ~SPFeBlend();

    Inkscape::Filters::FilterBlendMode blend_mode;
    int in2;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);

	virtual void build_renderer(Inkscape::Filters::Filter* filter);
};

#endif /* !SP_FEBLEND_H_SEEN */

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
