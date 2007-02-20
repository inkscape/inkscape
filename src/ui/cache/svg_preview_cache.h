#ifndef __SVG_PREVIEW_CACHE_H__
#define __SVG_PREVIEW_CACHE_H__

/** \file
 * SPIcon: Generic icon widget
 */
/*
 * Copyright (C) 2007 Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

GdkPixbuf* render_pixbuf(NRArenaItem* root, double scale_factor, const NR::Rect& dbox, unsigned psize);

namespace Inkscape {
namespace UI {
namespace Cache {

class SvgPreview {
 protected:
    std::map<Glib::ustring, GdkPixbuf*>   _pixmap_cache;

 public:
    SvgPreview() {}
    ~SvgPreview() {}

    Glib::ustring cache_key(gchar const *name, unsigned psize) const {
        Glib::ustring key = name;
        key += ":";
        key += psize;
        return key;
    }

    GdkPixbuf* get_preview_from_cache(const Glib::ustring& key) {
        std::map<Glib::ustring, GdkPixbuf *>::iterator found = _pixmap_cache.find(key);
        if ( found != _pixmap_cache.end() ) {
            return found->second;
        }
        return NULL;
    }

    void set_preview_in_cache(const Glib::ustring& key, GdkPixbuf* px) {
        _pixmap_cache[key] = px;
    }

    GdkPixbuf* get_preview(const gchar* id, NRArenaItem *root, double scale_factor, unsigned int psize) {
        // First try looking up the cached preview in the cache map
        Glib::ustring key = cache_key(id, psize);
        GdkPixbuf* px = get_preview_from_cache(key);

        if (px == NULL) {
/*
            px = render_pixbuf(root, scale_factor, dbox, psize);
            set_preview_in_cache(key, px);
*/
        }

    }
};

}; // namespace Cache
}; // namespace UI
}; // namespace Inkscape



#endif // __SVG_PREVIEW_CACHE_H__
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


