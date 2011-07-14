#include "libnr/nr-rect-l.h"

NRRectL::NRRectL()
{
    x0 = G_MAXINT32;
    y0 = G_MAXINT32;
    x1 = G_MININT32;
    y1 = G_MININT32;
}

NRRectL::NRRectL(gint32 xmin, gint32 ymin, gint32 xmax, gint32 ymax)
{
    x0 = xmin;
    y0 = ymin;
    x1 = xmax;
    y1 = ymax;
}

NRRectL::NRRectL(Geom::OptIntRect const &r)
{
    if (r) {
        x0 = r->left();
        y0 = r->top();
        x1 = r->right();
        y1 = r->bottom();
    } else {
        x0 = G_MAXINT32;
        y0 = G_MAXINT32;
        x1 = G_MININT32;
        y1 = G_MININT32;
    }
}

NRRectL::NRRectL(Geom::IntRect const &r)
{
    x0 = r.left();
    y0 = r.top();
    x1 = r.right();
    y1 = r.bottom();
}

Geom::OptIntRect NRRectL::upgrade_2geom() const
{
    Geom::OptIntRect ret;
    if (x0 > x1 || y0 > y1) return ret;
    ret = Geom::IntRect(x0, y0, x1, y1);
    return ret;
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
