/**
 * SVG <ellipse> and related implementations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *   Tavmjong Bah
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2013 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SP_ELLIPSE_H
#define SEEN_SP_ELLIPSE_H

#include "svg/svg-length.h"
#include "sp-shape.h"

/* Common parent class */
#define SP_GENERICELLIPSE(obj) (dynamic_cast<SPGenericEllipse*>(obj))
#define SP_IS_GENERICELLIPSE(obj) (dynamic_cast<const SPGenericEllipse*>((obj)) != NULL)

enum GenericEllipseType {
    SP_GENERIC_ELLIPSE_UNDEFINED, // FIXME shouldn't exist
    SP_GENERIC_ELLIPSE_ARC,
    SP_GENERIC_ELLIPSE_CIRCLE,
    SP_GENERIC_ELLIPSE_ELLIPSE
};

class SPGenericEllipse : public SPShape {
public:
    SPGenericEllipse();
    virtual ~SPGenericEllipse();

    // Regardless of type, the ellipse/circle/arc is stored
    // internally with these variables. (Circle radius is rx).
    SVGLength cx;
    SVGLength cy;
    SVGLength rx;
    SVGLength ry;

    /**
     * If we have a slice, returns whether the shape is closed ("pizza slice") or not (arc only).
     */
    bool closed();
    void setClosed(bool value);

    double start, end;
    GenericEllipseType type;

    virtual void build(SPDocument *document, Inkscape::XML::Node *repr);

    virtual void set(unsigned int key, char const *value);
    virtual void update(SPCtx *ctx, unsigned int flags);

    virtual Inkscape::XML::Node *write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
    virtual const char *displayName() const;

    virtual void set_shape();
    virtual Geom::Affine set_transform(Geom::Affine const &xform);

    virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;

    virtual void modified(unsigned int flags);

    virtual void update_patheffect(bool write);

    /**
     * @brief Makes sure that start and end lie between 0 and 2 * PI.
     */
    void normalize();

    Geom::Point getPointAtAngle(double arg) const;

    bool set_elliptical_path_attribute(Inkscape::XML::Node *repr);
    void position_set(double x, double y, double rx, double ry);

protected:
    /**
     * @brief Determines whether the shape is a part of an ellipse.
     */
    bool _isSlice() const;

    bool _closed;
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
