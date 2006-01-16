#define __SP_GUIDELINE_C__

/*
 * Infinite horizontal/vertical line
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
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
        static const GTypeInfo guideline_info =
            {
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
    
    gl->vertical = 0;
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
    
    int p0, p1, step;
    unsigned char *d;

    if (gl->vertical) {

        if (gl->position < buf->rect.x0 || gl->position >= buf->rect.x1) {
            return;
        }

        p0 = buf->rect.y0;
        p1 = buf->rect.y1;
        step = buf->buf_rowstride;
        d = buf->buf + 3 * (gl->position - buf->rect.x0);
        
    } else {

	if (gl->position < buf->rect.y0 || gl->position >= buf->rect.y1) {
            return;
        }
        
        p0 = buf->rect.x0;
        p1 = buf->rect.x1;
        step = 3;
        d = buf->buf + (gl->position - buf->rect.y0) * buf->buf_rowstride;
    }

    for (int p = p0; p < p1; p++) {
        d[0] = NR_COMPOSEN11(r, a, d[0]);
        d[1] = NR_COMPOSEN11(g, a, d[1]);
        d[2] = NR_COMPOSEN11(b, a, d[2]);
        d += step;
    }
}

static void sp_guideline_update(SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPGuideLine *gl = SP_GUIDELINE(item);

    if (((SPCanvasItemClass *) parent_class)->update) {
        ((SPCanvasItemClass *) parent_class)->update(item, affine, flags);
    }

    if (gl->vertical) {
        gl->position = (int) (affine[4] + 0.5);
        sp_canvas_update_bbox (item, gl->position, -1000000, gl->position + 1, 1000000);
    } else {
        gl->position = (int) (affine[5] - 0.5);
        sp_canvas_update_bbox (item, -1000000, gl->position, 1000000, gl->position + 1);
    }
}

static double sp_guideline_point(SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item)
{
    SPGuideLine *gl = SP_GUIDELINE (item);

    if (!gl->sensitive) {
        return NR_HUGE;
    }

    *actual_item = item;

    if (gl->vertical) {
        return MAX(fabs(gl->position - p[NR::X])-1, 0);
    } else {
        return MAX(fabs(gl->position - p[NR::Y])-1, 0);
    }
}

SPCanvasItem *sp_guideline_new(SPCanvasGroup *parent, double position, unsigned int vertical)
{
    SPCanvasItem *item = sp_canvas_item_new(parent, SP_TYPE_GUIDELINE, NULL);

    SPGuideLine *gl = SP_GUIDELINE(item);

    gl->vertical = vertical;
    sp_guideline_set_position(gl, position);

    return item;
}

void sp_guideline_set_position(SPGuideLine *gl, double position)
{
    sp_canvas_item_affine_absolute(SP_CANVAS_ITEM (gl), 
                                   NR::Matrix(NR::translate(position, position)));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
