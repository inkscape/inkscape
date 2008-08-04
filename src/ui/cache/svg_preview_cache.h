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

GdkPixbuf* render_pixbuf(NRArenaItem* root, double scale_factor, const Geom::Rect& dbox, unsigned psize);

namespace Inkscape {
namespace UI {
namespace Cache {

class SvgPreview {
 protected:
    std::map<Glib::ustring, GdkPixbuf*>   _pixmap_cache;

 public:
    SvgPreview();
    ~SvgPreview();

    Glib::ustring cache_key(gchar const *uri, gchar const *name, unsigned psize) const;
    GdkPixbuf*    get_preview_from_cache(const Glib::ustring& key);
    void          set_preview_in_cache(const Glib::ustring& key, GdkPixbuf* px);
    GdkPixbuf*    get_preview(const gchar* uri, const gchar* id, NRArenaItem *root, double scale_factor, unsigned int psize);
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


