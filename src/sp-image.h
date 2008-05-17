#ifndef __SP_IMAGE_H__
#define __SP_IMAGE_H__

/*
 * SVG <image> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Edward Flick (EAF)
 *
 * Copyright (C) 1999-2005 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_IMAGE (sp_image_get_type ())
#define SP_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_IMAGE, SPImage))
#define SP_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_IMAGE, SPImageClass))
#define SP_IS_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_IMAGE))
#define SP_IS_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_IMAGE))

class SPImage;
class SPImageClass;

/* SPImage */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "svg/svg-length.h"
#include "sp-item.h"
#include "display/display-forward.h"

#define SP_IMAGE_HREF_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A

struct SPImage : public SPItem {
    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;

    // Added by EAF
    /* preserveAspectRatio */
    unsigned int aspect_align : 4;
    unsigned int aspect_clip : 1;
    int trimx, trimy, trimwidth, trimheight;
    double viewx, viewy, viewwidth, viewheight;

    SPCurve *curve; // This curve is at the image's boundary for snapping

    gchar *href;
#if ENABLE_LCMS
    gchar *color_profile;
#endif // ENABLE_LCMS

    GdkPixbuf *pixbuf;
    gchar *pixPath;
    time_t lastMod;
};

struct SPImageClass {
    SPItemClass parent_class;
};

GType sp_image_get_type (void);

/* Return duplicate of curve or NULL */
SPCurve *sp_image_get_curve (SPImage *image);

void sp_image_refresh_if_outdated( SPImage* image );

#endif
