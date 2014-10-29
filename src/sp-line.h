#ifndef SEEN_SP_LINE_H
#define SEEN_SP_LINE_H

/*
 * SVG <line> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg-length.h"
#include "sp-shape.h"

#define SP_LINE(obj) (dynamic_cast<SPLine*>((SPObject*)obj))
#define SP_IS_LINE(obj) (dynamic_cast<const SPLine*>((SPObject*)obj) != NULL)

class SPLine : public SPShape {
public:
	SPLine();
	virtual ~SPLine();

    SVGLength x1;
    SVGLength y1;
    SVGLength x2;
    SVGLength y2;

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
	virtual void set(unsigned int key, char const* value);

	virtual const char* displayName() const;
	virtual Geom::Affine set_transform(Geom::Affine const &transform);
	virtual void convert_to_guides() const;
	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual void set_shape();
};

#endif // SEEN_SP_LINE_H
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
