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

#include <2geom/affine.h>
#include <cstddef>
#include <glibmm/ustring.h>
#include <sigc++/connection.h>
#include <vector>

#include "sp-paint-server.h"
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"
#include "sp-gradient-vector.h"
#include "sp-mesh-array.h"

class SPGradientReference;
class SPStop;

#define SP_GRADIENT(obj) (dynamic_cast<SPGradient*>((SPObject*)obj))
#define SP_IS_GRADIENT(obj) (dynamic_cast<const SPGradient*>((SPObject*)obj) != NULL)

enum SPGradientType {
    SP_GRADIENT_TYPE_UNKNOWN,
    SP_GRADIENT_TYPE_LINEAR,
    SP_GRADIENT_TYPE_RADIAL,
    SP_GRADIENT_TYPE_MESH
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
class SPGradient : public SPPaintServer {
public:
	SPGradient();
	virtual ~SPGradient();

private:
    /** gradientUnits attribute */
    SPGradientUnits units;
    unsigned int units_set : 1;
public:

    /** gradientTransform attribute */
    Geom::Affine gradientTransform;
    unsigned int gradientTransform_set : 1;

private:
    /** spreadMethod attribute */
    SPGradientSpread spread;
    unsigned int spread_set : 1;

    /** Gradient stops */
    unsigned int has_stops : 1;

    /** Gradient patches */
    unsigned int has_patches : 1;

public:
    /** Reference (href) */
    SPGradientReference *ref;

    /** State in Inkscape gradient system */
    unsigned int state;
    
    /** Linear and Radial Gradients */

    /** Composed vector */
    SPGradientVector vector;

    sigc::connection modified_connection;

    bool hasStops() const;

    SPStop* getFirstStop();
    int getStopCount() const;

    bool isEquivalent(SPGradient *b);
    bool isAligned(SPGradient *b);

    /** Mesh Gradients **************/

    /** Composed array (for mesh gradients) */
    SPMeshNodeArray array;
    SPMeshNodeArray array_smoothed; // Smoothed version of array
    
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

 /**
 * Returns private mesh of given gradient (the gradient at the end of the href chain which has
 * patches), optionally normalizing it.
 */
    SPGradient *getArray(bool force_private = false);

    //static GType getType();

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

    static void gradientRefModified(SPObject *href, unsigned int flags, SPGradient *gradient);
    static void gradientRefChanged(SPObject *old_ref, SPObject *ref, SPGradient *gr);

private:
    bool invalidateVector();
    bool invalidateArray();
    void rebuildVector();
    void rebuildArray();

protected:
    virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
    virtual void release();
    virtual void modified(unsigned int flags);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);

    virtual void child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
    virtual void remove_child(Inkscape::XML::Node *child);

    virtual void set(unsigned key, char const *value);
};

void
sp_gradient_pattern_common_setup(cairo_pattern_t *cp,
                                 SPGradient *gr,
                                 Geom::OptRect const &bbox,
                                 double opacity);

/* Gradient repr methods */
void sp_gradient_repr_write_vector(SPGradient *gr);
void sp_gradient_repr_clear_vector(SPGradient *gr);

cairo_pattern_t *sp_gradient_create_preview_pattern(SPGradient *gradient, double width);

/** Transforms to/from gradient position space in given environment */
Geom::Affine sp_gradient_get_g2d_matrix(SPGradient const *gr, Geom::Affine const &ctm,
                                      Geom::Rect const &bbox);
Geom::Affine sp_gradient_get_gs2d_matrix(SPGradient const *gr, Geom::Affine const &ctm,
                                       Geom::Rect const &bbox);
void sp_gradient_set_gs2d_matrix(SPGradient *gr, Geom::Affine const &ctm, Geom::Rect const &bbox,
                                 Geom::Affine const &gs2d);


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
