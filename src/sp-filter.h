#ifndef SP_FILTER_H_SEEN
#define SP_FILTER_H_SEEN

/** \file
 * SVG <filter> implementation, see sp-filter.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>

#include <gtkmm.h>

#include "number-opt-number.h"
#include "sp-object.h"
#include "sp-filter-units.h"
#include "sp-filter-fns.h"
#include "svg/svg-length.h"
#include "display/nr-filter.h"

/* Filter base class */

/* MACROS DEFINED IN FILE sp-filter-fns.h */

struct SPFilterReference;

class SPFilter;
class SPFilterClass;

struct ltstr {
    bool operator()(const char* s1, const char* s2) const;
};

struct SPFilter : public SPObject {

    /** filterUnits attribute */
    SPFilterUnits filterUnits;
    guint filterUnits_set : 1;
    /** primitiveUnits attribute */
    SPFilterUnits primitiveUnits;
    guint primitiveUnits_set : 1;
    /** X attribute */    
    SVGLength x;
    /** Y attribute */
    SVGLength y;
    /** WIDTH attribute */
    SVGLength width;
    /** HEIGHT attribute */
    SVGLength height;
    /** FILTERRES attribute */
    NumberOptNumber filterRes;
    /** HREF attribute */
    SPFilterReference *href;
    sigc::connection modified_connection;

    Inkscape::Filters::Filter *_renderer;

    std::map<gchar *, int, ltstr>* _image_name;
    int _image_number_next;
};

struct SPFilterClass {
    SPObjectClass parent_class;
};

/*
 * Initializes the given Inkscape::Filters::Filter object as a renderer for this
 * SPFilter object.
 */
void sp_filter_build_renderer(SPFilter *sp_filter, Inkscape::Filters::Filter *nr_filter);

/*
 * Returns the number of filter primitives in this SPFilter object.
 */
int sp_filter_primitive_count(SPFilter *filter);

/**
 * Returns a slot number for given image name, or -1 for unknown name.
 */
int sp_filter_get_image_name(SPFilter *filter, gchar const *name);
/**
 * Returns slot number for given image name, even if it's unknown.
 */
int sp_filter_set_image_name(SPFilter *filter, gchar const *name);
/**
 * Finds image name based on it's slot number. Returns 0 for unknown slot
 * numbers.
 */
gchar const *sp_filter_name_for_image(SPFilter const *filter, int const image);

/*
 * Returns a result image name that is not in use inside this filter.
 */
Glib::ustring sp_filter_get_new_result_name(SPFilter *filter);

#endif /* !SP_FILTER_H_SEEN */

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
