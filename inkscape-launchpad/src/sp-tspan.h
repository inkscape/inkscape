#ifndef INKSCAPE_SP_TSPAN_H
#define INKSCAPE_SP_TSPAN_H

/*
 * tspan and textpath, based on the flowtext routines
 */

#include "sp-item.h"
#include "text-tag-attributes.h"

#define SP_TSPAN(obj) (dynamic_cast<SPTSpan*>((SPObject*)obj))
#define SP_IS_TSPAN(obj) (dynamic_cast<const SPTSpan*>((SPObject*)obj) != NULL)

enum {
    SP_TSPAN_ROLE_UNSPECIFIED,
    SP_TSPAN_ROLE_PARAGRAPH,
    SP_TSPAN_ROLE_LINE
};

class SPTSpan : public SPItem {
public:
	SPTSpan();
	virtual ~SPTSpan();

    unsigned int role : 2;
    TextTagAttributes attributes;

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();
	virtual void set(unsigned int key, const char* value);
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);

	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type) const;
        virtual const char* displayName() const;
};

#endif /* !INKSCAPE_SP_TSPAN_H */

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
