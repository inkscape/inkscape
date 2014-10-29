#ifndef SEEN_SP_RECT_H
#define SEEN_SP_RECT_H

/*
 * SVG <rect> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/forward.h>

#include "svg/svg-length.h"
#include "sp-shape.h"

#define SP_RECT(obj) (dynamic_cast<SPRect*>((SPObject*)obj))
#define SP_IS_RECT(obj) (dynamic_cast<const SPRect*>((SPObject*)obj) != NULL)

class SPRect : public SPShape {
public:
	SPRect();
	virtual ~SPRect();

	void setPosition(double x, double y, double width, double height);

	/* If SET if FALSE, VALUE is just ignored */
	void setRx(bool set, double value);
	void setRy(bool set, double value);

	double getVisibleRx() const;
	void setVisibleRx(double rx);

	double getVisibleRy() const;
	void setVisibleRy(double ry);

	Geom::Rect getRect() const;

	double getVisibleWidth() const;
	void setVisibleWidth(double rx);

	double getVisibleHeight() const;
	void setVisibleHeight(double ry);

	void compensateRxRy(Geom::Affine xform);

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);

	virtual void set(unsigned key, char const *value);
	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
        virtual const char* displayName() const;

	virtual void set_shape();
	virtual Geom::Affine set_transform(Geom::Affine const& xform);

	virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;
	virtual void convert_to_guides() const;

	SVGLength x;
	SVGLength y;
	SVGLength width;
	SVGLength height;
	SVGLength rx;
	SVGLength ry;

private:
	static double vectorStretch(Geom::Point p0, Geom::Point p1, Geom::Affine xform);
};

#endif // SEEN_SP_RECT_H

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
