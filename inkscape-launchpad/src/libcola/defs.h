/* $Id: defs.h,v 1.5 2005/10/18 18:42:59 ellson Exp $ $Revision: 1.5 $ */
/* vim:set shiftwidth=4 ts=8: */

/* ********************************************************
*      This software is part of the graphviz package      *
*                http://www.graphviz.org/                 *
*                                                         *
*            Copyright (c) 1994-2004 AT&T Corp.           *
*                and is licensed under the                *
*            Common Public License, Version 1.0           *
*                      by AT&T Corp.                      *
*                                                         *
*        Information and Software Systems Research        *
*              AT&T Research, Florham Park NJ             *
**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _DEFS_H_
#define _DEFS_H_

#include "neato.h"

#ifdef __cplusplus
    enum Style { regular, invisible };
    struct vtx_data {
	int nedges;
	int *edges;
	float *ewgts;
	Style *styles;
	float *edists; /* directed dist reflecting the direction of the edge */
    };

    struct cluster_data {
	int nvars;         /* total count of vars in clusters */
        int nclusters;     /* number of clusters */
        int *clustersizes; /* number of vars in each cluster */
        int **clusters;    /* list of var indices for constituents of each c */
	int ntoplevel;     /* number of nodes not in any cluster */
	int *toplevel;     /* array of nodes not in any cluster */
	boxf *bb;	   /* bounding box of each cluster */
    };

    typedef int DistType;	/* must be signed!! */

    inline double max(double x, double y) {
	if (x >= y)
	    return x;
	else
	    return y;
    } inline double min(double x, double y) {
	if (x <= y)
	    return x;
	else
	    return y;
    }

    inline int max(int x, int y) {
	if (x >= y)
	    return x;
	else
	    return y;
    }

    inline int min(int x, int y) {
	if (x <= y)
	    return x;
	else
	    return y;
    }

    struct Point {
	double x;
	double y;
	int operator==(Point other) {
	    return x == other.x && y == other.y;
    }};
#else
#undef inline
#define inline
#define NOTUSED(var)      (void) var

#include <macros.h>
    extern void *gmalloc(size_t);
#define DIGCOLA 1

#ifdef USE_STYLES
    typedef enum { regular, invisible } Style;
#endif
    typedef struct {
	int nedges;		/* no. of neighbors, including self */
	int *edges;		/* edges[0..(nedges-1)] are neighbors; edges[0] is self */
	float *ewgts;		/* preferred edge lengths */
	float *eweights;	/* edge weights */
	node_t *np;		/* original node */
#ifdef USE_STYLES
	Style *styles;
#endif
#ifdef DIGCOLA
	float *edists; /* directed dist reflecting the direction of the edge */
#endif
    } vtx_data;

    typedef struct cluster_data {
	int nvars;         /* total count of vars in clusters */
        int nclusters;     /* number of clusters */
        int *clustersizes; /* number of vars in each cluster */
        int **clusters;    /* list of var indices for constituents of each c */
	int ntoplevel;     /* number of nodes not in any cluster */
	int *toplevel;     /* array of nodes not in any cluster */
	boxf *bb;	   /* bounding box of each cluster */
    } cluster_data;


    typedef int DistType;	/* must be signed!! */

#ifdef UNUSED
    typedef struct {
	double x;
	double y;
    } Point;
#endif

#endif

#endif

#ifdef __cplusplus
}
#endif
