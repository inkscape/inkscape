/*
 * Some filters for Potrace in Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __FILTERSET_H__
#define __FILTERSET_H__

#include "imagemap.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*#########################################################################
### C A N N Y    E D G E    D E T E C T I O N
#########################################################################*/



#ifdef __cplusplus
extern "C" {
#endif


/**
 *
 */
GrayMap *grayMapGaussian(GrayMap *me);

/**
 *
 */
RgbMap *rgbMapGaussian(RgbMap *me);

/**
 *
 */
GrayMap *grayMapCanny(GrayMap *gm, 
             double lowThreshold, double highThreshold);

/**
 *
 */
GdkPixbuf *gdkCanny(GdkPixbuf *img,
            double lowThreshold, double highThreshold);

/**
 * Quantize an RGB image to a reduced number of colors.  bitsPerSample
 * is usually 3 - 5 out of 8 to conserve cpu and memory
 */
IndexedMap *rgbMapQuantize(RgbMap *rgbMap, int bitsPerSample, int nrColors);

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
