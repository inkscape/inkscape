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
#include "inkscape-stock.h"
#include "sp-rect.h"
#include "document-private.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

#include "ui/cache/svg_preview_cache.h"

GdkPixbuf* render_pixbuf(NRArenaItem* root, double scale_factor, const Geom::Rect& dbox, unsigned psize) {
    NRGC gc(NULL);

    Geom::Matrix t(Geom::Scale(scale_factor, scale_factor));
    nr_arena_item_set_transform(root, t);

    gc.transform.set_identity();
    nr_arena_item_invoke_update( root, NULL, &gc,
                                 NR_ARENA_ITEM_STATE_ALL,
                                 NR_ARENA_ITEM_STATE_NONE );

    /* Item integer bbox in points */
    NRRectL ibox;
    ibox.x0 = (int) floor(scale_factor * dbox.min()[Geom::X] + 0.5);
    ibox.y0 = (int) floor(scale_factor * dbox.min()[Geom::Y] + 0.5);
    ibox.x1 = (int) floor(scale_factor * dbox.max()[Geom::X] + 0.5);
    ibox.y1 = (int) floor(scale_factor * dbox.max()[Geom::Y] + 0.5);

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

    /* Actual renderable area */
    NRRectL ua;
    ua.x0 = std::max(ibox.x0, area.x0);
    ua.y0 = std::max(ibox.y0, area.y0);
    ua.x1 = std::min(ibox.x1, area.x1);
    ua.y1 = std::min(ibox.y1, area.y1);

    /* Set up pixblock */
    guchar *px = g_new(guchar, 4 * psize * psize);
    memset(px, 0x00, 4 * psize * psize);

    /* Render */
    NRPixBlock B;
    nr_pixblock_setup_extern( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                              ua.x0, ua.y0, ua.x1, ua.y1,
                              px + 4 * psize * (ua.y0 - area.y0) +
                              4 * (ua.x0 - area.x0),
                              4 * psize, FALSE, FALSE );
    nr_arena_item_invoke_render(NULL, root, &ua, &B,
                                 NR_ARENA_ITEM_RENDER_NO_CACHE );
    nr_pixblock_release(&B);

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(px,
                                      GDK_COLORSPACE_RGB,
                                      TRUE,
                                      8, psize, psize, psize * 4,
                                      (GdkPixbufDestroyNotify)g_free,
                                      NULL);

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
