/** @file
 * SVG <image> implementation
 *//*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Edward Flick (EAF)
 *
 * Copyright (C) 1999-2005 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_SP_IMAGE_H
#define SEEN_INKSCAPE_SP_IMAGE_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glibmm/ustring.h>
#include "svg/svg-length.h"
#include "sp-item.h"

#define SP_TYPE_IMAGE (sp_image_get_type ())
#define SP_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_IMAGE, SPImage))
#define SP_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_IMAGE, SPImageClass))
#define SP_IS_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_IMAGE))
#define SP_IS_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_IMAGE))

#define SP_IMAGE_HREF_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A

namespace Inkscape { class Pixbuf; }

struct SPImage : public SPItem {
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;

    Geom::Rect clipbox;
    double sx, sy;
    double ox, oy;

    // Added by EAF
    /* preserveAspectRatio */
    unsigned int aspect_align : 4;
    unsigned int aspect_clip : 1;
    //int trimx, trimy, trimwidth, trimheight;
    //double viewx, viewy, viewwidth, viewheight;

    SPCurve *curve; // This curve is at the image's boundary for snapping

    gchar *href;
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    gchar *color_profile;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    Inkscape::Pixbuf *pixbuf;
};

struct SPImageClass {
    SPItemClass parent_class;
};

GType sp_image_get_type (void);

/* Return duplicate of curve or NULL */
SPCurve *sp_image_get_curve (SPImage *image);
void sp_embed_image(Inkscape::XML::Node *imgnode, Inkscape::Pixbuf *pb);
void sp_image_refresh_if_outdated( SPImage* image );

#endif
