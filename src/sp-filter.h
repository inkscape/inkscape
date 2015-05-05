/** \file
 * SVG <filter> element
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SP_FILTER_H_SEEN
#define SP_FILTER_H_SEEN

#include <glibmm/ustring.h>
#include <map>

#include "number-opt-number.h"
#include "sp-object.h"
#include "sp-filter-units.h"
#include "svg/svg-length.h"

#define SP_FILTER(obj) (dynamic_cast<SPFilter*>((SPObject*)obj))
#define SP_IS_FILTER(obj) (dynamic_cast<const SPFilter*>((SPObject*)obj) != NULL)

#define SP_FILTER_FILTER_UNITS(f) (SP_FILTER(f)->filterUnits)
#define SP_FILTER_PRIMITIVE_UNITS(f) (SP_FILTER(f)->primitiveUnits)

namespace Inkscape {
namespace Filters {
class Filter;
} }

class SPFilterReference;
class SPFilterPrimitive;

struct ltstr {
    bool operator()(const char* s1, const char* s2) const;
};

class SPFilter : public SPObject {
public:
	SPFilter();
	virtual ~SPFilter();

    SPFilterUnits filterUnits;
    unsigned int filterUnits_set : 1;
    SPFilterUnits primitiveUnits;
    unsigned int primitiveUnits_set : 1;
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;
    NumberOptNumber filterRes;
    SPFilterReference *href;
    sigc::connection modified_connection;
    
    guint getRefCount();
    guint _refcount;

    Inkscape::Filters::Filter *_renderer;

    std::map<gchar *, int, ltstr>* _image_name;
    int _image_number_next;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node* child);

	virtual void set(unsigned int key, const char* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};

void sp_filter_set_filter_units(SPFilter *filter, SPFilterUnits filterUnits);
void sp_filter_set_primitive_units(SPFilter *filter, SPFilterUnits filterUnits);
SPFilterPrimitive *add_primitive(SPFilter *filter, SPFilterPrimitive *primitive);
SPFilterPrimitive *get_primitive(SPFilter *filter, int index);

/* Initializes the given Inkscape::Filters::Filter object as a renderer for this
 * SPFilter object. */
void sp_filter_build_renderer(SPFilter *sp_filter, Inkscape::Filters::Filter *nr_filter);

/// Returns the number of filter primitives in this SPFilter object.
int sp_filter_primitive_count(SPFilter *filter);

/// Returns a slot number for given image name, or -1 for unknown name.
int sp_filter_get_image_name(SPFilter *filter, char const *name);

/// Returns slot number for given image name, even if it's unknown.
int sp_filter_set_image_name(SPFilter *filter, char const *name);

/** Finds image name based on it's slot number. Returns 0 for unknown slot
 * numbers. */
char const *sp_filter_name_for_image(SPFilter const *filter, int const image);

/// Returns a result image name that is not in use inside this filter.
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
