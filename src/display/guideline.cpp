#define __SP_GUIDELINE_C__

/*
 * Horizontal/vertical but can also be angled line
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <libnr/nr-pixops.h>
#include "display-forward.h"
#include "sp-canvas-util.h"
#include "guideline.h"

static void sp_guideline_class_init(SPGuideLineClass *c);
static void sp_guideline_init(SPGuideLine *guideline);
static void sp_guideline_destroy(GtkObject *object);

static void sp_guideline_update(SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_guideline_render(SPCanvasItem *item, SPCanvasBuf *buf);

static double sp_guideline_point(SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item);

static void sp_guideline_drawline (SPCanvasBuf *buf, gint x0, gint y0, gint x1, gint y1, guint32 rgba);

static SPCanvasItemClass *parent_class;

GType sp_guideline_get_type()
{
    static GType guideline_type = 0;

    if (!guideline_type) {
        static GTypeInfo const guideline_info = {
            sizeof (SPGuideLineClass),
            NULL, NULL,
            (GClassInitFunc) sp_guideline_class_init,
            NULL, NULL,
            sizeof (SPGuideLine),
            16,
            (GInstanceInitFunc) sp_guideline_init,
            NULL,
        };

        guideline_type = g_type_register_static(SP_TYPE_CANVAS_ITEM, "SPGuideLine", &guideline_info, (GTypeFlags) 0);
    }

    return guideline_type;
}

static void sp_guideline_class_init(SPGuideLineClass *c)
{
    parent_class = (SPCanvasItemClass*) g_type_class_peek_parent(c);

    GtkObjectClass *object_class = (GtkObjectClass *) c;
    object_class->destroy = sp_guideline_destroy;

    SPCanvasItemClass *item_class = (SPCanvasItemClass *) c;
    item_class->update = sp_guideline_update;
    item_class->render = sp_guideline_render;
    item_class->point = sp_guideline_point;
}

static void sp_guideline_init(SPGuideLine *gl)
{
    gl->rgba = 0x0000ff7f;

    gl->normal_to_line = Geom::Point(0,1);
    gl->angle = 3.14159265358979323846/2;
    gl->point_on_line = Geom::Point(0,0);
    gl->sensitive = 0;
}

static void sp_guideline_destroy(GtkObject *object)
{
    GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static void sp_guideline_render(SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPGuideLine const *gl = SP_GUIDELINE (item);

    sp_canvas_prepare_buffer(buf);

    unsigned int const r = NR_RGBA32_R (gl->rgba);
    unsigned int const g = NR_RGBA32_G (gl->rgba);
    unsigned int const b = NR_RGBA32_B (gl->rgba);
    unsigned int const a = NR_RGBA32_A (gl->rgba);

    if (gl->is_vertical()) {
        int position = (int) Inkscape::round(gl->point_on_line[Geom::X]);
        if (position < buf->rect.x0 || position >= buf->rect.x1) {
            return;
        }

        int p0 = buf->rect.y0;
        int p1 = buf->rect.y1;
        int step = buf->buf_rowstride;
        unsigned char *d = buf->buf + 4 * (position - buf->rect.x0);

        for (int p = p0; p < p1; p++) {
            d[0] = NR_COMPOSEN11_1111(r, a, d[0]);
            d[1] = NR_COMPOSEN11_1111(g, a, d[1]);
            d[2] = NR_COMPOSEN11_1111(b, a, d[2]);
            d += step;
        }
    } else if (gl->is_horizontal()) {
        int position = (int) Inkscape::round(gl->point_on_line[Geom::Y]);
        if (position < buf->rect.y0 || position >= buf->rect.y1) {
            return;
        }

        int p0 = buf->rect.x0;
        int p1 = buf->rect.x1;
        int step = 4;
        unsigned char *d = buf->buf + (position - buf->rect.y0) * buf->buf_rowstride;

        for (int p = p0; p < p1; p++) {
            d[0] = NR_COMPOSEN11_1111(r, a, d[0]);
            d[1] = NR_COMPOSEN11_1111(g, a, d[1]);
            d[2] = NR_COMPOSEN11_1111(b, a, d[2]);
            d += step;
        }
    } else {
        // render angled line, once intersection has been detected, draw from there.
        Geom::Point parallel_to_line( gl->normal_to_line[Geom::Y],
                                      /*should be minus, but inverted y axis*/ gl->normal_to_line[Geom::X]);

        //try to intersect with left vertical of rect
        double y_intersect_left = (buf->rect.x0 - gl->point_on_line[Geom::X]) * parallel_to_line[Geom::Y] / parallel_to_line[Geom::X] + gl->point_on_line[Geom::Y];
        if ( (y_intersect_left >= buf->rect.y0) && (y_intersect_left <= buf->rect.y1) ) {
            // intersects with left vertical!
            double y_intersect_right = (buf->rect.x1 - gl->point_on_line[Geom::X]) * parallel_to_line[Geom::Y] / parallel_to_line[Geom::X] + gl->point_on_line[Geom::Y];
            sp_guideline_drawline (buf, buf->rect.x0, static_cast<gint>(round(y_intersect_left)), buf->rect.x1, static_cast<gint>(round(y_intersect_right)), gl->rgba);
            return;
        }

        //try to intersect with right vertical of rect
        double y_intersect_right = (buf->rect.x1 - gl->point_on_line[Geom::X]) * parallel_to_line[Geom::Y] / parallel_to_line[Geom::X] + gl->point_on_line[Geom::Y];
        if ( (y_intersect_right >= buf->rect.y0) && (y_intersect_right <= buf->rect.y1) ) {
            // intersects with right vertical!
            sp_guideline_drawline (buf, buf->rect.x1, static_cast<gint>(round(y_intersect_right)), buf->rect.x0, static_cast<gint>(round(y_intersect_left)), gl->rgba);
            return;
        }

        //try to intersect with top horizontal of rect
        double x_intersect_top = (buf->rect.y0 - gl->point_on_line[Geom::Y]) * parallel_to_line[Geom::X] / parallel_to_line[Geom::Y] + gl->point_on_line[Geom::X];
        if ( (x_intersect_top >= buf->rect.x0) && (x_intersect_top <= buf->rect.x1) ) {
            // intersects with top horizontal!
            double x_intersect_bottom = (buf->rect.y1 - gl->point_on_line[Geom::Y]) * parallel_to_line[Geom::X] / parallel_to_line[Geom::Y] + gl->point_on_line[Geom::X];
            sp_guideline_drawline (buf, static_cast<gint>(round(x_intersect_top)), buf->rect.y0, static_cast<gint>(round(x_intersect_bottom)), buf->rect.y1, gl->rgba);
            return;
        }

        //try to intersect with bottom horizontal of rect
        double x_intersect_bottom = (buf->rect.y1 - gl->point_on_line[Geom::Y]) * parallel_to_line[Geom::X] / parallel_to_line[Geom::Y] + gl->point_on_line[Geom::X];
        if ( (x_intersect_top >= buf->rect.x0) && (x_intersect_top <= buf->rect.x1) ) {
            // intersects with bottom horizontal!
            sp_guideline_drawline (buf, static_cast<gint>(round(x_intersect_bottom)), buf->rect.y1, static_cast<gint>(round(x_intersect_top)), buf->rect.y0, gl->rgba);
            return;
        }
    }
}

static void sp_guideline_update(SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPGuideLine *gl = SP_GUIDELINE(item);

    if (((SPCanvasItemClass *) parent_class)->update) {
        ((SPCanvasItemClass *) parent_class)->update(item, affine, flags);
    }

    gl->point_on_line[Geom::X] = affine[4];
    gl->point_on_line[Geom::Y] = affine[5];

    if (gl->is_horizontal()) {
        sp_canvas_update_bbox (item, -1000000, (int) Inkscape::round(gl->point_on_line[Geom::Y]), 1000000, (int) Inkscape::round(gl->point_on_line[Geom::Y] + 1));
    } else if (gl->is_vertical()) {
        sp_canvas_update_bbox (item, (int) Inkscape::round(gl->point_on_line[Geom::X]), -1000000, (int) Inkscape::round(gl->point_on_line[Geom::X] + 1), 1000000);
    } else {
        sp_canvas_update_bbox (item, -1000000, -1000000, 1000000, 1000000);
    }
}

// Returns 0.0 if point is on the guideline
static double sp_guideline_point(SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item)
{
    SPGuideLine *gl = SP_GUIDELINE (item);

    if (!gl->sensitive) {
        return NR_HUGE;
    }

    *actual_item = item;

    Geom::Point vec(gl->normal_to_line[Geom::X], - gl->normal_to_line[Geom::Y]);
    double distance = Geom::dot((p - gl->point_on_line), vec);
    return MAX(fabs(distance)-1, 0);
}

SPCanvasItem *sp_guideline_new(SPCanvasGroup *parent, Geom::Point point_on_line, Geom::Point normal)
{
    SPCanvasItem *item = sp_canvas_item_new(parent, SP_TYPE_GUIDELINE, NULL);

    SPGuideLine *gl = SP_GUIDELINE(item);

    normal.normalize();
    gl->normal_to_line = normal;
    gl->angle = tan( -gl->normal_to_line[Geom::X] / gl->normal_to_line[Geom::Y]);
    sp_guideline_set_position(gl, point_on_line);

    return item;
}

void sp_guideline_set_position(SPGuideLine *gl, Geom::Point point_on_line)
{
    sp_canvas_item_affine_absolute(SP_CANVAS_ITEM (gl),
                                   NR::Matrix(NR::translate(point_on_line)));
}

void sp_guideline_set_normal(SPGuideLine *gl, Geom::Point normal_to_line)
{
    gl->normal_to_line = normal_to_line;
    gl->angle = tan( -normal_to_line[Geom::X] / normal_to_line[Geom::Y]);

    sp_canvas_item_request_update(SP_CANVAS_ITEM (gl));
}

void sp_guideline_set_color(SPGuideLine *gl, unsigned int rgba)
{
    gl->rgba = rgba;

    sp_canvas_item_request_update(SP_CANVAS_ITEM(gl));
}

void sp_guideline_set_sensitive(SPGuideLine *gl, int sensitive)
{
    gl->sensitive = sensitive;
}

//##########################################################
// Line rendering
#define SAFE_SETPIXEL   //undefine this when it is certain that setpixel is never called with invalid params

/**
    \brief  This function renders a pixel on a particular buffer.

    The topleft of the buffer equals
                        ( rect.x0 , rect.y0 )  in screen coordinates
                        ( 0 , 0 )  in setpixel coordinates
    The bottomright of the buffer equals
                        ( rect.x1 , rect,y1 )  in screen coordinates
                        ( rect.x1 - rect.x0 , rect.y1 - rect.y0 )  in setpixel coordinates
*/
static void
sp_guideline_setpixel (SPCanvasBuf *buf, gint x, gint y, guint32 rgba)
{
#ifdef SAFE_SETPIXEL
    if ( (x >= buf->rect.x0) && (x < buf->rect.x1) && (y >= buf->rect.y0) && (y < buf->rect.y1) ) {
#endif
        guint r, g, b, a;
        r = NR_RGBA32_R (rgba);
        g = NR_RGBA32_G (rgba);
        b = NR_RGBA32_B (rgba);
        a = NR_RGBA32_A (rgba);
        guchar * p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 4;
        p[0] = NR_COMPOSEN11_1111 (r, a, p[0]);
        p[1] = NR_COMPOSEN11_1111 (g, a, p[1]);
        p[2] = NR_COMPOSEN11_1111 (b, a, p[2]);
#ifdef SAFE_SETPIXEL
    }
#endif
}

/**
    \brief  This function renders a line on a particular canvas buffer,
            using Bresenham's line drawing function.
            http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
            Coordinates are interpreted as SCREENcoordinates
*/
static void
sp_guideline_drawline (SPCanvasBuf *buf, gint x0, gint y0, gint x1, gint y1, guint32 rgba)
{
    int dy = y1 - y0;
    int dx = x1 - x0;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;                                                  // dy is now 2*dy
    dx <<= 1;                                                  // dx is now 2*dx

    sp_guideline_setpixel(buf, x0, y0, rgba);
    if (dx > dy) {
        int fraction = dy - (dx >> 1);                         // same as 2*dy - dx
        while (x0 != x1) {
            if (fraction >= 0) {
                y0 += stepy;
                fraction -= dx;                                // same as fraction -= 2*dx
            }
            x0 += stepx;
            fraction += dy;                                    // same as fraction -= 2*dy
            sp_guideline_setpixel(buf, x0, y0, rgba);
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y0 != y1) {
            if (fraction >= 0) {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            sp_guideline_setpixel(buf, x0, y0, rgba);
        }
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
