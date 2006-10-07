#ifndef __SP_FILTER_CHEMISTRY_H__
#define __SP_FILTER_CHEMISTRY_H__

/*
 * Various utility methods for filters
 *
 * Authors:
 *   Hugo Rodrigues
 *   bulia byak
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "sp-filter.h"

SPFilter *new_filter_gaussian_blur (SPDocument *document, gdouble stdDeviation);
void remove_filter (SPObject *item, bool recursive);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
