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

#include <2geom/forward.h>
#include <cstddef>
#include <sigc++/connection.h>

#include "sp-lpe-item.h"
#include "sp-marker-loc.h"

#define SP_SHAPE(obj) (dynamic_cast<SPShape*>((SPObject*)obj))
#define SP_IS_SHAPE(obj) (dynamic_cast<const SPShape*>((SPObject*)obj) != NULL)

#define SP_SHAPE_WRITE_PATH (1 << 2)

class SPDesktop;
class SPMarker;
namespace Inkscape { class DrawingItem; }

/**
 * Base class for shapes, including <path> element
 */
class SPShape : public SPLPEItem {
public:
	SPShape();
	virtual ~SPShape();

    SPCurve * getCurve () const;
    SPCurve * getCurveBeforeLPE () const;
    void setCurve (SPCurve *curve, unsigned int owner);
    void setCurveInsync (SPCurve *curve, unsigned int owner);
    void setCurveBeforeLPE (SPCurve *curve);
    int hasMarkers () const;
    int numberOfMarkers (int type) const;

public: // temporarily public, until SPPath is properly classed, etc.
    SPCurve *_curve_before_lpe;
    SPCurve *_curve;

public:
    SPMarker *_marker[SP_MARKER_LOC_QTY];
    sigc::connection _release_connect [SP_MARKER_LOC_QTY];
    sigc::connection _modified_connect [SP_MARKER_LOC_QTY];

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

	virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType bboxtype) const;
	virtual void print(SPPrintContext* ctx);

	virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
	virtual void hide(unsigned int key);

	virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;

	virtual void set_shape();
};


void sp_shape_set_marker (SPObject *object, unsigned int key, const char *value);

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
