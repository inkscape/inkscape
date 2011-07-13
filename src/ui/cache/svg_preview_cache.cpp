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

#include <glib/gmem.h>
#include <gtk/gtk.h>
#include "sp-namedview.h"
#include "selection.h"
#include "inkscape.h"
#include "sp-rect.h"
#include "document-private.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "display/cairo-utils.h"

#include "ui/cache/svg_preview_cache.h"

GdkPixbuf* render_pixbuf(NRArenaItem* root, double scale_factor, const Geom::Rect& dbox, unsigned psize) {
    NRGC gc(NULL);

    Geom::Affine t(Geom::Scale(scale_factor, scale_factor));
    nr_arena_item_set_transform(root, t);

    gc.transform.setIdentity();
    nr_arena_item_invoke_update( root, NULL, &gc,
                                 NR_ARENA_ITEM_STATE_ALL,
                                 NR_ARENA_ITEM_STATE_NONE );

    /* Item integer bbox in points */
    NRRectL ibox;
    ibox.x0 = floor(scale_factor * dbox.min()[Geom::X]);
    ibox.y0 = floor(scale_factor * dbox.min()[Geom::Y]);
    ibox.x1 = ceil(scale_factor * dbox.max()[Geom::X]);
    ibox.y1 = ceil(scale_factor * dbox.max()[Geom::Y]);

    /* Find visible area */
    int width = ibox.x1 - ibox.x0;
    int height = ibox.y1 - ibox.y0;
    int dx = psize;
    int dy = psize;
    dx = (dx - width)/2; // watch out for size, since 'unsigned'-'signed' can cause problems if the result is negative
    dy = (dy - height)/2;

    NRRectL area;
    area.x0 = ibox.x0 - dx;
    area.y0 = ibox.y0 - dy;
    area.x1 = area.x0 + psize;
    area.y1 = area.y0 + psize;

    /* Render */
    cairo_surface_t *s = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, psize, psize);
    cairo_t *ct = cairo_create(s);
    cairo_translate(ct, -area.x0, -area.y0);

    nr_arena_item_invoke_render(ct, root, &area, NULL,
                                 NR_ARENA_ITEM_RENDER_NO_CACHE );
    cairo_surface_flush(s);
    cairo_destroy(ct);

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

GdkPixbuf* SvgPreview::get_preview(const gchar* uri, const gchar* id, NRArenaItem */*root*/,
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

};
};
};
