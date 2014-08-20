/** @file
 * @brief SVG component transferfilter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SP_FECOMPONENTTRANSFER_H_SEEN
#define SP_FECOMPONENTTRANSFER_H_SEEN

#include "sp-filter-primitive.h"

#define SP_FECOMPONENTTRANSFER(obj) (dynamic_cast<SPFeComponentTransfer*>((SPObject*)obj))
#define SP_IS_FECOMPONENTTRANSFER(obj) (dynamic_cast<const SPFeComponentTransfer*>((SPObject*)obj) != NULL)

namespace Inkscape {
namespace Filters {
class FilterComponentTransfer;
} }

class SPFeComponentTransfer : public SPFilterPrimitive {
public:
	SPFeComponentTransfer();
	virtual ~SPFeComponentTransfer();

    Inkscape::Filters::FilterComponentTransfer *renderer;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);

	virtual void build_renderer(Inkscape::Filters::Filter* filter);
};

#endif /* !SP_FECOMPONENTTRANSFER_H_SEEN */

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
