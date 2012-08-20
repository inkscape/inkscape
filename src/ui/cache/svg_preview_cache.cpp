/** \file
 * SPIcon: Generic icon widget
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <brycehar@bryceharrington.org>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <2geom/transforms.h>
#include "sp-namedview.h"
#include "selection.h"
#include "inkscape.h"
#include "sp-rect.h"
#include "document-private.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "display/drawing.h"

#include "ui/cache/svg_preview_cache.h"

GdkPixbuf* render_pixbuf(Inkscape::Drawing &drawing, double scale_factor, const Geom::Rect& dbox, unsigned psize)
{
    Geom::Affine t(Geom::Scale(scale_factor, scale_factor));
    drawing.root()->setTransform(Geom::Scale(scale_factor));

    Geom::IntRect ibox = (dbox * Geom::Scale(scale_factor)).roundOutwards();

    drawing.update(ibox);

    /* Find visible area */
    int width = ibox.width();
    int height = ibox.height();
    int dx = psize;
    int dy = psize;
    dx = (dx - width)/2; // watch out for size, since 'unsigned'-'signed' can cause problems if the result is negative
    dy = (dy - height)/2;

    Geom::IntRect area = Geom::IntRect::from_xywh(
        ibox.min() - Geom::IntPoint(dx, dy), Geom::IntPoint(psize, psize));

    /* Render */
    cairo_surface_t *s = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, psize, psize);
    Inkscape::DrawingContext ct(s, area.min());

    drawing.render(ct, area, Inkscape::DrawingItem::RENDER_BYPASS_CACHE);
    cairo_surface_flush(s);

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(cairo_image_surface_get_data(s),
                                      GDK_COLORSPACE_RGB,
                                      TRUE,
                                      8, psize, psize, cairo_image_surface_get_stride(s),
                                      ink_cairo_pixbuf_cleanup, s);
    convert_pixbuf_argb32_to_normal(pixbuf);

    return pixbuf;
}

namespace Inkscape {
namespace UI {
namespace Cache {

SvgPreview::SvgPreview()
{
}

SvgPreview::~SvgPreview()
{
}

Glib::ustring SvgPreview::cache_key(gchar const *uri, gchar const *name, unsigned psize) const {
    Glib::ustring key;
    key += (uri!=NULL)  ? uri  : "";
    key += ":";
    key += (name!=NULL) ? name : "unknown";
    key += ":";
    key += psize;
    return key;
}

GdkPixbuf* SvgPreview::get_preview_from_cache(const Glib::ustring& key) {
    std::map<Glib::ustring, GdkPixbuf *>::iterator found = _pixmap_cache.find(key);
    if ( found != _pixmap_cache.end() ) {
        return found->second;
    }
    return NULL;
}

void SvgPreview::set_preview_in_cache(const Glib::ustring& key, GdkPixbuf* px) {
    _pixmap_cache[key] = px;
}

GdkPixbuf* SvgPreview::get_preview(const gchar* uri, const gchar* id, Inkscape::DrawingItem */*root*/,
                                   double /*scale_factor*/, unsigned int psize) {
    // First try looking up the cached preview in the cache map
    Glib::ustring key = cache_key(uri, id, psize);
    GdkPixbuf* px = get_preview_from_cache(key);

    if (px == NULL) {
        /*
            px = render_pixbuf(root, scale_factor, dbox, psize);
            set_preview_in_cache(key, px);
        */
    }
    return px;
}

void SvgPreview::remove_preview_from_cache(const Glib::ustring& key) {
    std::map<Glib::ustring, GdkPixbuf *>::iterator found = _pixmap_cache.find(key);
    if ( found != _pixmap_cache.end() ) {
        _pixmap_cache.erase(key);
    }
}


}
}
}

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
