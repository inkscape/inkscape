/**
 *  \file geom.h
 *  \brief Various geometrical calculations
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-forward.h"

enum IntersectorKind {
    INTERSECTS = 0,
    PARALLEL,
    COINCIDENT,
    NO_INTERSECTION
};

/* Define here various primatives, such as line, line segment, circle, bezier path etc. */



/* intersectors */

IntersectorKind intersector_line_intersection(NR::Point const &n0, double const d0,
					      NR::Point const &n1, double const d1,
					      NR::Point &result);
