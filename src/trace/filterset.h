/*
 * Some filters for Potrace in Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *   Stéphane Gimenez <dev@gim.name>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __FILTERSET_H__
#define __FILTERSET_H__

#include "imagemap.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Apply gaussian blur to an GrayMap
 */
GrayMap *grayMapGaussian(GrayMap *gmap);

/**
 *  Apply gaussian bluf to an RgbMap
 */
RgbMap *rgbMapGaussian(RgbMap *rgbmap);

/**
 *
 */
GrayMap *grayMapCanny(GrayMap *gmap,
             double lowThreshold, double highThreshold);

/**
 *
 */
GrayMap *quantizeBand(RgbMap *rgbmap, int nrColors);



#ifdef __cplusplus
}
#endif


#endif /* __FILTERSET_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
