#ifndef __SP_FILTER_CHEMISTRY_H__
#define __SP_FILTER_CHEMISTRY_H__

/*
 * Various utility methods for filters
 *
 * Authors:
 *   Hugo Rodrigues
 *   bulia byak
 *   Niko Kiirala
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "sp-filter.h"

SPFilterPrimitive *filter_add_primitive(SPFilter *filter, Inkscape::Filters::FilterPrimitiveType);
SPFilter *new_filter (Document *document);
SPFilter *new_filter_gaussian_blur (Document *document, gdouble stdDeviation, double expansion, double expansionX, double expansionY, double width, double height);
SPFilter *new_filter_simple_from_item (Document *document, SPItem *item, const char *mode, gdouble stdDeviation);
SPFilter *modify_filter_gaussian_blur_from_item (Document *document, SPItem *item, gdouble stdDeviation);
void remove_filter (SPObject *item, bool recursive);
void remove_filter_gaussian_blur (SPObject *item);
bool filter_is_single_gaussian_blur(SPFilter *filter);
double get_single_gaussian_blur_radius(SPFilter *filter);

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
