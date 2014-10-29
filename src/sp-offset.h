#ifndef SEEN_SP_OFFSET_H
#define SEEN_SP_OFFSET_H
/*
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 * (of the sp-spiral.h upon which this file was created)
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstddef>
#include <sigc++/sigc++.h>

#include "sp-shape.h"

#define SP_OFFSET(obj) (dynamic_cast<SPOffset*>((SPObject*)obj))
#define SP_IS_OFFSET(obj) (dynamic_cast<const SPOffset*>((SPObject*)obj) != NULL)

class SPUseReference;

/**
 * SPOffset class.
 *
 * An offset is defined by curve and radius. The original curve is kept as
 * a path in a sodipodi:original attribute. It's not possible to change
 * the original curve.
 *
 * SPOffset is a derivative of SPShape, much like the SPSpiral or SPRect.
 * The goal is to have a source shape (= originalPath), an offset (= radius)
 * and compute the offset of the source by the radius. To get it to work,
 * one needs to know what the source is and what the radius is, and how it's
 * stored in the xml representation. The object itself is a "path" element,
 * to get lots of shape functionality for free. The source is the easy part:
 * it's stored in a "inkscape:original" attribute in the path. In case of
 * "linked" offset, as they've been dubbed, there is an additional
 * "inkscape:href" that contains the id of an element of the svg.
 * When built, the object will attach a listener vector to that object and
 * rebuild the "inkscape:original" whenever the href'd object changes. This
 * is of course grossly inefficient, and also does not react to changes
 * to the href'd during context stuff (like changing the shape of a star by
 * dragging control points) unless the path of that object is changed during
 * the context (seems to be the case for SPEllipse). The computation of the
 * offset is done in sp_offset_set_shape(), a function that is called whenever
 * a change occurs to the offset (change of source or change of radius).
 * just like the sp-star and other, this path derivative can make control
 * points, or more precisely one control point, that's enough to define the
 * radius (look in object-edit).
 */
class SPOffset : public SPShape {
public:
	SPOffset();
	virtual ~SPOffset();

    void *originalPath; ///< will be a livarot Path, just don't declare it here to please the gcc linker FIXME what?
    char *original;     ///< SVG description of the source path
    float rad;          ///< offset radius

    /// for interactive setting of the radius
    bool knotSet;
    Geom::Point knot;

    bool sourceDirty;
    bool isUpdating;

    char                 *sourceHref;
    SPUseReference       *sourceRef;
    Inkscape::XML::Node  *sourceRepr; ///< the repr associated with that id
    SPObject             *sourceObject;

    sigc::connection _modified_connection;
    sigc::connection _delete_connection;
    sigc::connection _changed_connection;
    sigc::connection _transformed_connection;

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void set(unsigned int key, char const* value);
	virtual void update(SPCtx *ctx, unsigned int flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned flags);
	virtual void release();

	virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;
        virtual const char* displayName() const;
	virtual char* description() const;

	virtual void set_shape();
};

double sp_offset_distance_to_original (SPOffset * offset, Geom::Point px);
void sp_offset_top_point (SPOffset const *offset, Geom::Point *px);

SPItem *sp_offset_get_source (SPOffset *offset);

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
