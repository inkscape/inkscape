#ifndef SEEN_SP_SHAPE_H
#define SEEN_SP_SHAPE_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *   Johan Engelen
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 1999-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-lpe-item.h"
#include "sp-marker-loc.h"
#include <2geom/forward.h>

#include <stddef.h>
#include <sigc++/connection.h>

#define SP_TYPE_SHAPE (sp_shape_get_type ())
#define SP_SHAPE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_SHAPE, SPShape))
#define SP_SHAPE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_SHAPE, SPShapeClass))
#define SP_IS_SHAPE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_SHAPE))
#define SP_IS_SHAPE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_SHAPE))

#define SP_SHAPE_WRITE_PATH (1 << 2)

class SPDesktop;
namespace Inkscape { class DrawingItem; }
GType sp_shape_get_type (void) G_GNUC_CONST;

/**
 * Base class for shapes, including <path> element
 */
class SPShape : public SPLPEItem {
public:
    void setShape ();
    SPCurve * getCurve () const;
    SPCurve * getCurveBeforeLPE () const;
    void setCurve (SPCurve *curve, unsigned int owner);
    void setCurveInsync (SPCurve *curve, unsigned int owner);
    void setCurveBeforeLPE (SPCurve *curve);
    int hasMarkers () const;
    int numberOfMarkers (int type);

public: // temporarily public, until SPPath is properly classed, etc.
    SPCurve *_curve_before_lpe;
    SPCurve *_curve;

public:
    SPObject *_marker[SP_MARKER_LOC_QTY];
    sigc::connection _release_connect [SP_MARKER_LOC_QTY];
    sigc::connection _modified_connect [SP_MARKER_LOC_QTY];

private:
    friend class SPShapeClass;	
};

class SPShapeClass {
public:
    SPLPEItemClass item_class;

    /* Build bpath from extra shape attributes */
    void (* set_shape) (SPShape *shape);

private:
    friend class SPShape;
};

void sp_shape_set_marker (SPObject *object, unsigned int key, const gchar *value);

Geom::Affine sp_shape_marker_get_transform(Geom::Curve const & c1, Geom::Curve const & c2);
Geom::Affine sp_shape_marker_get_transform_at_start(Geom::Curve const & c);
Geom::Affine sp_shape_marker_get_transform_at_end(Geom::Curve const & c);

#endif // SEEN_SP_SHAPE_H

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
