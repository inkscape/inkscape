#ifndef SEEN_NR_FILTER_IMAGE_H
#define SEEN_NR_FILTER_IMAGE_H

/*
 * feImage filter primitive renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"

class SPDocument;
class SPItem;

namespace Inkscape {
class Pixbuf;

namespace Filters {
class FilterSlot;

class FilterImage : public FilterPrimitive {
public:
    FilterImage();
    static FilterPrimitive *create();
    virtual ~FilterImage();

    virtual void render_cairo(FilterSlot &slot);
    virtual bool can_handle_affine(Geom::Affine const &);
    virtual double complexity(Geom::Affine const &ctm);

    void set_document( SPDocument *document );
    void set_href(char const *href);
    void set_align( unsigned int align );
    void set_clip( unsigned int clip );
    bool from_element;
    SPItem* SVGElem;

private:
    SPDocument *document;
    char *feImageHref;
    Inkscape::Pixbuf *image;
    float feImageX, feImageY, feImageWidth, feImageHeight;
    unsigned int aspect_align, aspect_clip;
    bool broken_ref;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_IMAGE_H__ */
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
