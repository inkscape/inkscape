#ifndef SEEN_SP_GRADIENT_H
#define SEEN_SP_GRADIENT_H
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyrigt  (C) 2010 Jon A. Cruz
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include <gdk/gdk.h>
#include <glibmm/ustring.h>
#include <2geom/affine.h>
#include "sp-paint-server.h"
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"
#include "sp-gradient-vector.h"
#include "sp-mesh-array.h"

#include <stddef.h>
#include <sigc++/connection.h>

struct SPGradientReference;
class SPStop;

#define SP_TYPE_GRADIENT (SPGradient::getType())
#define SP_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GRADIENT, SPGradient))
#define SP_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GRADIENT, SPGradientClass))
#define SP_IS_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GRADIENT))
#define SP_IS_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GRADIENT))

enum SPGradientType {
    SP_GRADIENT_TYPE_UNKNOWN,
    SP_GRADIENT_TYPE_LINEAR,
    SP_GRADIENT_TYPE_RADIAL,
    SP_GRADIENT_TYPE_MESH
};

enum SPGradientMeshType {
    SP_GRADIENT_MESH_TYPE_UNKNOWN,
    SP_GRADIENT_MESH_TYPE_NORMAL,
    SP_GRADIENT_MESH_TYPE_CONICAL
};

enum SPGradientState {
    SP_GRADIENT_STATE_UNKNOWN,
    SP_GRADIENT_STATE_VECTOR,
    SP_GRADIENT_STATE_PRIVATE
};

enum GrPointType {
    POINT_LG_BEGIN = 0, //start enum at 0 (for indexing into gr_knot_shapes array for example)
    POINT_LG_END,
    POINT_LG_MID,
    POINT_RG_CENTER,
    POINT_RG_R1,
    POINT_RG_R2,
    POINT_RG_FOCUS,
    POINT_RG_MID1,
    POINT_RG_MID2,
    POINT_MG_CORNER,
    POINT_MG_HANDLE,
    POINT_MG_TENSOR,
    // insert new point types here.

    POINT_G_INVALID
};

namespace Inkscape {

enum PaintTarget {
    FOR_FILL,
    FOR_STROKE
};

/**
 * Convenience function to access a common vector of all enum values.
 */
std::vector<PaintTarget> const &allPaintTargets();

} // namespace Inkscape


/**
 * Gradient
 *
 * Implement spread, stops list
 * \todo fixme: Implement more here (Lauris)
 */
struct SPGradient : public SPPaintServer {

    /** Reference (href) */
    SPGradientReference *ref;

    /** State in Inkscape gradient system */
    guint state : 2;

private:
    /** gradientUnits attribute */
    SPGradientUnits units;
    guint units_set : 1;
public:

    /** gradientTransform attribute */
    Geom::Affine gradientTransform;
    guint gradientTransform_set : 1;

private:
    /** spreadMethod attribute */
    SPGradientSpread spread;
    guint spread_set : 1;

    /** Gradient stops */
    guint has_stops : 1;

    /** Gradient patches */
    guint has_patches : 1;

public:

    /** Linear and Radial Gradients */

    /** Composed vector */
    SPGradientVector vector;

    sigc::connection modified_connection;

    bool hasStops() const;

    SPStop* getFirstStop();
    int getStopCount() const;


    /** Mesh Gradients **************/

    /** Composed array (for mesh gradients) */
    SPMeshNodeArray array;

    bool hasPatches() const;


    /** All Gradients **************/
    bool isUnitsSet() const;
    SPGradientUnits getUnits() const;
    void setUnits(SPGradientUnits units);


    bool isSpreadSet() const;
    SPGradientSpread getSpread() const;

/**
 * Returns private vector of given gradient (the gradient at the end of the href chain which has
 * stops), optionally normalizing it.
 *
 * \pre SP_IS_GRADIENT(gradient).
 * \pre There exists a gradient in the chain that has stops.
 */
    SPGradient *getVector(bool force_private = false);

    static GType getType();

    /** Forces vector to be built, if not present (i.e. changed) */
    void ensureVector();

    /** Forces array (mesh) to be built, if not present (i.e. changed) */
    void ensureArray();

    /** Ensures that color array is populated */
    void ensureColors();

    /**
     * Set spread property of gradient and emit modified.
     */
    void setSpread(SPGradientSpread spread);

    SPGradientSpread fetchSpread();
    SPGradientUnits fetchUnits();

    void setSwatch(bool swatch = true);

private:
    bool invalidateVector();
    bool invalidateArray();
    void rebuildVector();
    void rebuildArray();

    friend class SPGradientImpl;
//    friend class SPLGPainter;
//    friend class SPRGPainter;
};

/**
 * The SPGradient vtable.
 */
struct SPGradientClass {
    SPPaintServerClass parent_class;
};


#include "sp-gradient-fns.h"

#endif // SEEN_SP_GRADIENT_H

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
