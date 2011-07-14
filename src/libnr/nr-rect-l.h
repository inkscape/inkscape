#ifndef SEEN_NR_RECT_L_H
#define SEEN_NR_RECT_L_H

#include <glib.h>
#include <2geom/int-rect.h>

struct NRRectL {
    gint32 x0, y0, x1, y1;
    NRRectL();
    NRRectL(gint32 xmin, gint32 ymin, gint32 xmax, gint32 ymax);
    explicit NRRectL(Geom::IntRect const &r);
    explicit NRRectL(Geom::OptIntRect const &r);
    operator Geom::OptIntRect() const { Geom::OptIntRect r = upgrade_2geom(); return r; }
    Geom::OptIntRect upgrade_2geom() const;
};

#endif /* !SEEN_NR_RECT_L_H */

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
