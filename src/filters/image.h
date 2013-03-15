/** @file
 * @brief SVG image filter effect
 *//*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FEIMAGE_H_SEEN
#define SP_FEIMAGE_H_SEEN

#include "sp-filter-primitive.h"
#include "svg/svg-length.h"
#include "sp-item.h"
#include "uri-references.h"

#define SP_TYPE_FEIMAGE (sp_feImage_get_type())
#define SP_FEIMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEIMAGE, SPFeImage))
#define SP_FEIMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEIMAGE, SPFeImageClass))
#define SP_IS_FEIMAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEIMAGE))
#define SP_IS_FEIMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEIMAGE))

struct SPFeImage : public SPFilterPrimitive {
    gchar *href;

    /* preserveAspectRatio */
    unsigned int aspect_align : 4;
    unsigned int aspect_clip : 1;

    SPDocument *document;
    bool from_element;
    SPItem* SVGElem;
    Inkscape::URIReference* SVGElemRef;
    sigc::connection _image_modified_connection;
    sigc::connection _href_modified_connection;
};

struct SPFeImageClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feImage_get_type();

#endif /* !SP_FEIMAGE_H_SEEN */

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
