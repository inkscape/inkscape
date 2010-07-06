#ifndef SEEN_SP_GRADIENT_H
#define SEEN_SP_GRADIENT_H

/** \file
 * SVG <stop> <linearGradient> and <radialGradient> implementation
 *
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

#include <gdk/gdktypes.h>
#include "libnr/nr-matrix.h"
#include "sp-paint-server.h"
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"
#include "sp-gradient-vector.h"

#include <sigc++/connection.h>

struct SPGradientReference;


#define SP_TYPE_GRADIENT (SPGradient::getType())
#define SP_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_GRADIENT, SPGradient))
#define SP_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_GRADIENT, SPGradientClass))
#define SP_IS_GRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_GRADIENT))
#define SP_IS_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_GRADIENT))

typedef enum {
    SP_GRADIENT_TYPE_UNKNOWN,
    SP_GRADIENT_TYPE_LINEAR,
    SP_GRADIENT_TYPE_RADIAL
} SPGradientType;

typedef enum {
    SP_GRADIENT_STATE_UNKNOWN,
    SP_GRADIENT_STATE_VECTOR,
    SP_GRADIENT_STATE_PRIVATE
} SPGradientState;

typedef enum {
    POINT_LG_BEGIN =0, //start enum at 0 (for indexing into gr_knot_shapes array for example)
    POINT_LG_END,
    POINT_LG_MID,
    POINT_RG_CENTER,
    POINT_RG_R1,
    POINT_RG_R2,
    POINT_RG_FOCUS,
    POINT_RG_MID1,
    POINT_RG_MID2,
    // insert new point types here.

    POINT_G_INVALID
} GrPointType;

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
    Geom::Matrix gradientTransform;
    guint gradientTransform_set : 1;

private:
    /** spreadMethod attribute */
    SPGradientSpread spread;
    guint spread_set : 1;

    /** Gradient stops */
    guint has_stops : 1;
public:

    /** Composed vector */
    SPGradientVector vector;

    /** Rendered color array (4 * 1024 bytes) */
    guchar *color;

    sigc::connection modified_connection;

    bool hasStops() const;

    SPStop* getFirstStop();
    int getStopCount() const;


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
    void rebuildVector();

    friend class SPGradientImpl;
    friend class SPLGPainter;
    friend class SPRGPainter;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
