#ifndef SP_CURSOR_H
#define SP_CURSOR_H

#include <gdk/gdk.h>

GdkPixbuf* sp_cursor_pixbuf_from_xpm(gchar const *const *xpm, GdkColor const& black, GdkColor const& white, guint32 fill, guint32 stroke);
GdkCursor *sp_cursor_new_from_xpm(gchar const *const *xpm, gint hot_x, gint hot_y);

#endif

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
