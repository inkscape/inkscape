#ifndef INKSCAPE_SPIRO_bezctx_H
#define INKSCAPE_SPIRO_bezctx_H

#include "bezctx_intf.h"

class _bezctx {
public:
    void (*moveto)(bezctx *bc, double x, double y, int is_open);
    void (*lineto)(bezctx *bc, double x, double y);
    void (*quadto)(bezctx *bc, double x1, double y1, double x2, double y2);
    void (*curveto)(bezctx *bc, double x1, double y1, double x2, double y2,
                    double x3, double y3);
};

#endif
