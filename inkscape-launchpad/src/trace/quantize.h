/*
 *  Quantization for Inkscape
 *
 * Authors:
 *   Stéphane Gimenez <dev@gim.name>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "imagemap.h"

/**
 * Quantize an RGB image to a reduced number of colors.
 */
IndexedMap *rgbMapQuantize(RgbMap *rgbmap, int nrColors);

#endif /* __QUANTIZE_H__ */
