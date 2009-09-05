#ifndef SP_ROOT_H_SEEN
#define SP_ROOT_H_SEEN

/** \file
 * SPRoot: SVG \<svg\> implementation.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_ROOT (sp_root_get_type())
#define SP_ROOT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_ROOT, SPRoot))
#define SP_ROOT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_ROOT, SPRootClass))
#define SP_IS_ROOT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_ROOT))
#define SP_IS_ROOT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_ROOT))

#include "version.h"
#include "svg/svg-length.h"
#include "enums.h"
#include "sp-item-group.h"

/** \<svg\> element */
struct SPRoot : public SPGroup {
    struct {
        Inkscape::Version svg;
        Inkscape::Version inkscape;
    } version, original;

    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;

    /* viewBox; */
    unsigned int viewBox_set : 1;
    NRRect viewBox;

    /* preserveAspectRatio */
    unsigned int aspect_set : 1;
    unsigned int aspect_align : 4;
    unsigned int aspect_clip : 1;

    /** Child to parent additional transform. */
    Geom::Matrix c2p;

    gchar *onload;

    /**
     * Primary \<defs\> element where we put new defs (patterns, gradients etc.).
     *
     * At the time of writing, this is chosen as the first \<defs\> child of
     * this \<svg\> element: see writers of this member in sp-root.cpp.
     */
    SPDefs *defs;
};

struct SPRootClass {
    SPGroupClass parent_class;
};

GType sp_root_get_type();


#endif /* !SP_ROOT_H_SEEN */

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
