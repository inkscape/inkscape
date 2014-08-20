/** @file
 * @brief SVG flood filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FEFLOOD_H_SEEN
#define SP_FEFLOOD_H_SEEN

#include "sp-filter-primitive.h"
#include "svg/svg-icc-color.h"

#define SP_FEFLOOD(obj) (dynamic_cast<SPFeFlood*>((SPObject*)obj))
#define SP_IS_FEFLOOD(obj) (dynamic_cast<const SPFeFlood*>((SPObject*)obj) != NULL)

class SPFeFlood : public SPFilterPrimitive {
public:
	SPFeFlood();
	virtual ~SPFeFlood();

    guint32 color;
    SVGICCColor *icc;
    double opacity;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, const gchar* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);

	virtual void build_renderer(Inkscape::Filters::Filter* filter);
};

#endif /* !SP_FEFLOOD_H_SEEN */

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
