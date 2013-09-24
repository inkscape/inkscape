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
#include "sp-shape.h"

#define SP_IMAGE(obj) (dynamic_cast<SPImage*>((SPObject*)obj))
#define SP_IS_IMAGE(obj) (dynamic_cast<const SPImage*>((SPObject*)obj) != NULL)

#define SP_IMAGE_HREF_MODIFIED_FLAG SP_OBJECT_USER_MODIFIED_FLAG_A

namespace Inkscape { class Pixbuf; }
class SPImage : public SPItem {
public:
    SPImage();
    virtual ~SPImage();

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

    virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
    virtual void release();
    virtual void set(unsigned int key, gchar const* value);
    virtual void update(SPCtx *ctx, guint flags);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);
    virtual void modified(unsigned int flags);

    virtual Geom::OptRect bbox(Geom::Affine const &transform, SPItem::BBoxType type);
    virtual void print(SPPrintContext *ctx);
    virtual const char* displayName();
    virtual gchar* description();
    virtual Inkscape::DrawingItem* show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags);
    virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs);
    virtual Geom::Affine set_transform(Geom::Affine const &transform);
};

/* Return duplicate of curve or NULL */
SPCurve *sp_image_get_curve (SPImage *image);
void sp_embed_image(Inkscape::XML::Node *imgnode, Inkscape::Pixbuf *pb);
void sp_image_refresh_if_outdated( SPImage* image );

#endif
