/** @file
 * @brief Preview cache
 */
/*
 * Copyright (C) 2007 Bryce W. Harrington <bryce@bryceharrington.org>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UI_SVG_PREVIEW_CACHE_H
#define SEEN_INKSCAPE_UI_SVG_PREVIEW_CACHE_H

#include <map>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glibmm/ustring.h>
#include <2geom/rect.h>

namespace Inkscape {

class Drawing;
class DrawingItem;

} // namespace Inkscape


GdkPixbuf* render_pixbuf(Inkscape::Drawing &drawing, double scale_factor, const Geom::Rect& dbox, unsigned psize);

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
    GdkPixbuf*    get_preview(const gchar* uri, const gchar* id, Inkscape::DrawingItem *root, double scale_factor, unsigned int psize);
    void          remove_preview_from_cache(const Glib::ustring& key);
};

}; // namespace Cache
}; // namespace UI
}; // namespace Inkscape



#endif // SEEN_INKSCAPE_UI_SVG_PREVIEW_CACHE_H
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


