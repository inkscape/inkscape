typedef struct _bezctx bezctx;

bezctx *
new_bezctx(void);

void
bezctx_moveto(bezctx *bc, double x, double y, int is_open);

void
bezctx_lineto(bezctx *bc, double x, double y);

void
bezctx_quadto(bezctx *bc, double x1, double y1, double x2, double y2);

void
bezctx_curveto(bezctx *bc, double x1, double y1, double x2, double y2,
	       double x3, double y3);

void
bezctx_mark_knot(bezctx *bc, int knot_idx);
