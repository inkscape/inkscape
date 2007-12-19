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

    if (gl->normal_to_line[Geom::Y] == 0.) {
        int position = gl->point_on_line[Geom::X];
        if (position < buf->rect.x0 || position >= buf->rect.x1) {
            return;
        }

        int p0 = buf->rect.y0;
        int p1 = buf->rect.y1;
        int step = buf->buf_rowstride;
        unsigned char *d = buf->buf + 3 * (position - buf->rect.x0);

        for (int p = p0; p < p1; p++) {
            d[0] = NR_COMPOSEN11_1111(r, a, d[0]);
            d[1] = NR_COMPOSEN11_1111(g, a, d[1]);
            d[2] = NR_COMPOSEN11_1111(b, a, d[2]);
            d += step;
        }
    } else if (gl->normal_to_line[Geom::X] == 0.) {
        int position = gl->point_on_line[Geom::Y];
        if (position < buf->rect.y0 || position >= buf->rect.y1) {
            return;
        }

        int p0 = buf->rect.x0;
        int p1 = buf->rect.x1;
        int step = 3;
        unsigned char *d = buf->buf + (position - buf->rect.y0) * buf->buf_rowstride;

        for (int p = p0; p < p1; p++) {
            d[0] = NR_COMPOSEN11_1111(r, a, d[0]);
            d[1] = NR_COMPOSEN11_1111(g, a, d[1]);
            d[2] = NR_COMPOSEN11_1111(b, a, d[2]);
            d += step;
        }
    } else {
        // render angled line
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

    if (gl->normal_to_line[Geom::Y] == 1.) {
        sp_canvas_update_bbox (item, -1000000, -1000000, 1000000, 1000000);
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

    double distance = Geom::dot((p.to_2geom() - gl->point_on_line), gl->normal_to_line);
    return MAX(fabs(distance)-1, 0);
}

SPCanvasItem *sp_guideline_new(SPCanvasGroup *parent, Geom::Point point_on_line, Geom::Point normal)
{
    SPCanvasItem *item = sp_canvas_item_new(parent, SP_TYPE_GUIDELINE, NULL);

    SPGuideLine *gl = SP_GUIDELINE(item);

    normal.normalize();
    gl->normal_to_line = normal;
    sp_guideline_set_position(gl, point_on_line);

    return item;
}

void sp_guideline_set_position(SPGuideLine *gl, Geom::Point point_on_line)
{
    sp_canvas_item_affine_absolute(SP_CANVAS_ITEM (gl),
                                   NR::Matrix(NR::translate(point_on_line)));
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
