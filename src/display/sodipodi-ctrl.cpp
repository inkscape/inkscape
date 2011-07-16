#define INKSCAPE_CTRL_C

/*
 * SPCtrl
 *
 * We render it by hand to reduce allocing/freeing svps & to get clean
 *    (non-aa) images
 *
 */

#include <2geom/transforms.h>
#include "sp-canvas-util.h"
#include "sodipodi-ctrl.h"
#include "display/cairo-utils.h"

enum {
    ARG_0,
    ARG_SHAPE,
    ARG_MODE,
    ARG_ANCHOR,
    ARG_SIZE,
    ARG_FILLED,
    ARG_FILL_COLOR,
    ARG_STROKED,
    ARG_STROKE_COLOR,
    ARG_PIXBUF
};


static void sp_ctrl_class_init (SPCtrlClass *klass);
static void sp_ctrl_init (SPCtrl *ctrl);
static void sp_ctrl_destroy (GtkObject *object);
static void sp_ctrl_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void sp_ctrl_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_ctrl_render (SPCanvasItem *item, SPCanvasBuf *buf);

static double sp_ctrl_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);


static SPCanvasItemClass *parent_class;

GType
sp_ctrl_get_type (void)
{
    static GType ctrl_type = 0;
    if (!ctrl_type) {
        static GTypeInfo const ctrl_info = {
            sizeof (SPCtrlClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_ctrl_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof (SPCtrl),
            0,   /* n_preallocs */
            (GInstanceInitFunc) sp_ctrl_init,
            NULL
        };
        ctrl_type = g_type_register_static (SP_TYPE_CANVAS_ITEM, "SPCtrl", &ctrl_info, (GTypeFlags)0);
    }
    return ctrl_type;
}

static void
sp_ctrl_class_init (SPCtrlClass *klass)
{
    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = (GtkObjectClass *) klass;
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass *)g_type_class_peek_parent (klass);

    gtk_object_add_arg_type ("SPCtrl::shape", GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_SHAPE);
    gtk_object_add_arg_type ("SPCtrl::mode", GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_MODE);
    gtk_object_add_arg_type ("SPCtrl::anchor", GTK_TYPE_ANCHOR_TYPE, GTK_ARG_READWRITE, ARG_ANCHOR);
    gtk_object_add_arg_type ("SPCtrl::size", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_SIZE);
    gtk_object_add_arg_type ("SPCtrl::pixbuf", GTK_TYPE_POINTER, GTK_ARG_READWRITE, ARG_PIXBUF);
    gtk_object_add_arg_type ("SPCtrl::filled", GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_FILLED);
    gtk_object_add_arg_type ("SPCtrl::fill_color", GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_FILL_COLOR);
    gtk_object_add_arg_type ("SPCtrl::stroked", GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_STROKED);
    gtk_object_add_arg_type ("SPCtrl::stroke_color", GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_STROKE_COLOR);

    object_class->destroy = sp_ctrl_destroy;
    object_class->set_arg = sp_ctrl_set_arg;

    item_class->update = sp_ctrl_update;
    item_class->render = sp_ctrl_render;
    item_class->point = sp_ctrl_point;
}

static void
sp_ctrl_init (SPCtrl *ctrl)
{
    ctrl->shape = SP_CTRL_SHAPE_SQUARE;
    ctrl->mode = SP_CTRL_MODE_COLOR;
    ctrl->anchor = GTK_ANCHOR_CENTER;
    ctrl->span = 3;
    ctrl->defined = TRUE;
    ctrl->shown = FALSE;
    ctrl->build = FALSE;
    ctrl->filled = 1;
    ctrl->stroked = 0;
    ctrl->fill_color = 0x000000ff;
    ctrl->stroke_color = 0x000000ff;

    // This way we make sure that the first sp_ctrl_update() call finishes properly;
    // in subsequent calls it will not update anything it the control hasn't moved
    // Consider for example the case in which a snap indicator is drawn at (0, 0);
    // If moveto() is called then it will not set _moved to true because we're initially already at (0, 0)
    ctrl->_moved = true; // Is this flag ever going to be set back to false? I can't find where that is supposed to happen

    ctrl->box.x0 = ctrl->box.y0 = ctrl->box.x1 = ctrl->box.y1 = 0;
    ctrl->cache = NULL;
    ctrl->pixbuf = NULL;

    ctrl->_point = Geom::Point(0,0);
}

static void
sp_ctrl_destroy (GtkObject *object)
{
    SPCtrl *ctrl;

    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRL (object));

    ctrl = SP_CTRL (object);

    if (ctrl->cache) {
        delete[] ctrl->cache;
        ctrl->cache = NULL;
    }

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_ctrl_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
    SPCanvasItem *item;
    SPCtrl *ctrl;
    GdkPixbuf * pixbuf = NULL;

    item = SP_CANVAS_ITEM (object);
    ctrl = SP_CTRL (object);

    switch (arg_id) {
        case ARG_SHAPE:
            ctrl->shape = (SPCtrlShapeType)(GTK_VALUE_INT (*arg));
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            break;

        case ARG_MODE:
            ctrl->mode = (SPCtrlModeType)(GTK_VALUE_INT (*arg));
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            break;

        case ARG_ANCHOR:
            ctrl->anchor = (GtkAnchorType)(GTK_VALUE_INT (*arg));
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            break;

        case ARG_SIZE:
            ctrl->span = (gint) ((GTK_VALUE_DOUBLE (*arg) - 1.0) / 2.0 + 0.5);
            ctrl->defined = (ctrl->span > 0);
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            break;

        case ARG_FILLED:
            ctrl->filled = GTK_VALUE_BOOL (*arg);
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            break;

        case ARG_FILL_COLOR: {
            guint32 fill = GTK_VALUE_INT (*arg);
            ctrl->fill_color = fill;
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            } break;

        case ARG_STROKED:
            ctrl->stroked = GTK_VALUE_BOOL (*arg);
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            break;

        case ARG_STROKE_COLOR: {
            guint32 stroke = GTK_VALUE_INT (*arg);
            ctrl->stroke_color = stroke;
            ctrl->build = FALSE;
            sp_canvas_item_request_update (item);
            } break;

        case ARG_PIXBUF:
            pixbuf  = (GdkPixbuf*)(GTK_VALUE_POINTER (*arg));
            if (gdk_pixbuf_get_has_alpha (pixbuf)) {
                ctrl->pixbuf = pixbuf;
            } else {
                ctrl->pixbuf = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
                gdk_pixbuf_unref (pixbuf);
            }
            ctrl->build = FALSE;
            break;

        default:
            break;
    }
}

static void
sp_ctrl_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCtrl *ctrl;
    gint x, y;

    ctrl = SP_CTRL (item);

    if (((SPCanvasItemClass *) parent_class)->update)
        (* ((SPCanvasItemClass *) parent_class)->update) (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    if (!ctrl->_moved) return;

    if (ctrl->shown) {
        sp_canvas_request_redraw (item->canvas, ctrl->box.x0, ctrl->box.y0, ctrl->box.x1 + 1, ctrl->box.y1 + 1);
    }

    if (!ctrl->defined) return;

    x = (gint) ((affine[4] > 0) ? (affine[4] + 0.5) : (affine[4] - 0.5)) - ctrl->span;
    y = (gint) ((affine[5] > 0) ? (affine[5] + 0.5) : (affine[5] - 0.5)) - ctrl->span;

    switch (ctrl->anchor) {
        case GTK_ANCHOR_N:
        case GTK_ANCHOR_CENTER:
        case GTK_ANCHOR_S:
            break;

        case GTK_ANCHOR_NW:
        case GTK_ANCHOR_W:
        case GTK_ANCHOR_SW:
            x += ctrl->span;
            break;

        case GTK_ANCHOR_NE:
        case GTK_ANCHOR_E:
        case GTK_ANCHOR_SE:
            x -= (ctrl->span + 1);
            break;
    }

    switch (ctrl->anchor) {
        case GTK_ANCHOR_W:
        case GTK_ANCHOR_CENTER:
        case GTK_ANCHOR_E:
            break;

        case GTK_ANCHOR_NW:
        case GTK_ANCHOR_N:
        case GTK_ANCHOR_NE:
            y += ctrl->span;
            break;

        case GTK_ANCHOR_SW:
        case GTK_ANCHOR_S:
        case GTK_ANCHOR_SE:
            y -= (ctrl->span + 1);
            break;
    }

    ctrl->box.x0 = x;
    ctrl->box.y0 = y;
    ctrl->box.x1 = ctrl->box.x0 + 2 * ctrl->span;
    ctrl->box.y1 = ctrl->box.y0 + 2 * ctrl->span;

    sp_canvas_update_bbox (item, ctrl->box.x0, ctrl->box.y0, ctrl->box.x1 + 1, ctrl->box.y1 + 1);
}

static double
sp_ctrl_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item)
{
    SPCtrl *ctrl = SP_CTRL (item);

    *actual_item = item;

    double const x = p[Geom::X];
    double const y = p[Geom::Y];

    if ((x >= ctrl->box.x0) && (x <= ctrl->box.x1) && (y >= ctrl->box.y0) && (y <= ctrl->box.y1)) return 0.0;

    return 1e18;
}

static void
sp_ctrl_build_cache (SPCtrl *ctrl)
{
    guint32 *p, *q;
    gint size, x, y, z, s, a, side, c;
    guint32 stroke_color, fill_color;

    if (ctrl->filled) {
        if (ctrl->mode == SP_CTRL_MODE_XOR) {
            fill_color = ctrl->fill_color;
        } else {
            fill_color = argb32_from_rgba(ctrl->fill_color);
        }
    } else {
        fill_color = 0;
    }
    if (ctrl->stroked) {
        if (ctrl->mode == SP_CTRL_MODE_XOR) {
            stroke_color = ctrl->stroke_color;
        } else {
            stroke_color = argb32_from_rgba(ctrl->stroke_color);
        }
    } else {
        stroke_color = fill_color;
    }


    side = (ctrl->span * 2 +1);
    c = ctrl->span;
    size = side * side;
    if (side < 2) return;

    if (ctrl->cache) delete[] ctrl->cache;
    ctrl->cache = new guint32[size];

    switch (ctrl->shape) {
        case SP_CTRL_SHAPE_SQUARE:
            p = ctrl->cache;
            // top edge
            for (x=0; x < side; x++) {
                *p++ = stroke_color;
            }
            // middle
            for (y = 2; y < side; y++) {
                *p++ = stroke_color; // stroke at first and last pixel
                for (x=2; x < side; x++) {
                    *p++ = fill_color; // fill in the middle
                }
                *p++ = stroke_color;
            }
            // bottom edge
            for (x=0; x < side; x++) {
                *p++ = stroke_color;
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_DIAMOND:
            p = ctrl->cache;
            for (y = 0; y < side; y++) {
                z = abs (c - y);
                for (x = 0; x < z; x++) {
                    *p++ = 0;
                }
                *p++ = stroke_color; x++;
                for (; x < side - z -1; x++) {
                    *p++ = fill_color;
                }
                if (z != c) {
                    *p++ = stroke_color; x++;
                }
                for (; x < side; x++) {
                    *p++ = 0;
                }
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_CIRCLE:
            p = ctrl->cache;
            q = p + size -1;
            s = -1;
            for (y = 0; y <= c ; y++) {
                a = abs (c - y);
                z = (gint)(0.0 + sqrt ((c+.4)*(c+.4) - a*a));
                x = 0;
                while (x < c-z) {
                    *p++ = 0;
                    *q-- = 0;
                    x++;
                }
                do {
                    *p++ = stroke_color;
                    *q-- = stroke_color;
                    x++;
                } while (x < c-s);
                while (x < MIN(c+s+1, c+z)) {
                    *p++ = fill_color;
                    *q-- = fill_color;
                    x++;
                }
                do {
                    *p++ = stroke_color;
                    *q-- = stroke_color;
                    x++;
                } while (x <= c+z);
                while (x < side) {
                    *p++ = 0;
                    *q-- = 0;
                    x++;
                }
                s = z;
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_CROSS:
            p = ctrl->cache;
            for (y = 0; y < side; y++) {
                z = abs (c - y);
                for (x = 0; x < c-z; x++) {
                    *p++ = 0;
                }
                *p++ = stroke_color; x++;
                for (; x < c + z; x++) {
                    *p++ = 0;
                }
                if (z != 0) {
                    *p++ = stroke_color; x++;
                }
                for (; x < side; x++) {
                    *p++ = 0;
                }
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_BITMAP:
            if (ctrl->pixbuf) {
                unsigned char *px;
                unsigned int rs;
                px = gdk_pixbuf_get_pixels (ctrl->pixbuf);
                rs = gdk_pixbuf_get_rowstride (ctrl->pixbuf);
                for (y = 0; y < side; y++){
                    guint32 *d;
                    unsigned char *s;
                    s = px + y * rs;
                    d = ctrl->cache + side * y;
                    for (x = 0; x < side; x++) {
                        if (s[3] < 0x80) {
                            *d++ = 0;
                        } else if (s[0] < 0x80) {
                            *d++ = stroke_color;
                        } else {
                            *d++ = fill_color;
                        }
                        s += 4;
                    }
                }
            } else {
                g_print ("control has no pixmap\n");
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_IMAGE:
            if (ctrl->pixbuf) {
                guint r = gdk_pixbuf_get_rowstride (ctrl->pixbuf);
                guint32 *px;
                guchar *data = gdk_pixbuf_get_pixels (ctrl->pixbuf);
                p = ctrl->cache;
                for (y = 0; y < side; y++){
                    px = reinterpret_cast<guint32*>(data + y * r);
                    for (x = 0; x < side; x++) {
                        *p++ = *px++;
                    }
                }
            } else {
                g_print ("control has no pixmap\n");
            }
            ctrl->build = TRUE;
            break;

        default:
            break;
    }
}

static inline guint32 compose_xor(guint32 bg, guint32 fg, guint32 a)
{
    guint32 c = bg * (255-a) + (((bg ^ ~fg) + (bg >> 2) - (bg > 127 ? 63 : 0)) & 255) * a;
    return (c + 127) / 255;
}

static void
sp_ctrl_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    //gint y0, y1, y, x0,x1,x;
    //guchar *p, *q, a;

    SPCtrl *ctrl = SP_CTRL (item);

    if (!ctrl->defined) return;
    if ((!ctrl->filled) && (!ctrl->stroked)) return;

    // the control-image is rendered into ctrl->cache
    if (!ctrl->build) {
        sp_ctrl_build_cache (ctrl);
    }

    int w, h;
    w = h = (ctrl->span * 2 +1);

    // The code below works even when the target is not an image surface
    if (ctrl->mode == SP_CTRL_MODE_XOR) {
        // 1. Copy the affected part of output to a temporary surface
        cairo_surface_t *work = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_t *cr = cairo_create(work);
        cairo_translate(cr, -ctrl->box.x0, -ctrl->box.y0);
        cairo_set_source_surface(cr, cairo_get_target(buf->ct), buf->rect.x0, buf->rect.y0);
        cairo_paint(cr);
        cairo_destroy(cr);

        // 2. Composite the control on a temporary surface
        cairo_surface_flush(work);
        int strideb = cairo_image_surface_get_stride(work);
        unsigned char *pxb = cairo_image_surface_get_data(work);
        guint32 *p = ctrl->cache;
        for (int i=0; i<h; ++i) {
            guint32 *pb = reinterpret_cast<guint32*>(pxb + i*strideb);
            for (int j=0; j<w; ++j) {
                guint32 cc = *p++;
                guint32 ac = cc & 0xff;
                if (ac == 0 && cc != 0) {
                    *pb++ = argb32_from_rgba(cc | 0x000000ff);
                } else {
                    EXTRACT_ARGB32(*pb, ab,rb,gb,bb)
                    guint32 ro = compose_xor(rb, (cc & 0xff000000) >> 24, ac);
                    guint32 go = compose_xor(gb, (cc & 0x00ff0000) >> 16, ac);
                    guint32 bo = compose_xor(bb, (cc & 0x0000ff00) >>  8, ac);
                    ASSEMBLE_ARGB32(px, ab,ro,go,bo)
                    *pb++ = px;
                }
            }
        }
        cairo_surface_mark_dirty(work);

        // 3. Replace the affected part of output with contents of temporary surface
        cairo_save(buf->ct);
        cairo_set_source_surface(buf->ct, work,
            ctrl->box.x0 - buf->rect.x0, ctrl->box.y0 - buf->rect.y0);
        cairo_rectangle(buf->ct, ctrl->box.x0 - buf->rect.x0, ctrl->box.y0 - buf->rect.y0, w, h);
        cairo_clip(buf->ct);
        cairo_set_operator(buf->ct, CAIRO_OPERATOR_SOURCE);
        cairo_paint(buf->ct);
        cairo_restore(buf->ct);
        cairo_surface_destroy(work);
    } else {
        cairo_surface_t *cache = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char*>(ctrl->cache), CAIRO_FORMAT_ARGB32, w, h, w*4);
        cairo_set_source_surface(buf->ct, cache,
            ctrl->box.x0 - buf->rect.x0, ctrl->box.y0 - buf->rect.y0);
        cairo_paint(buf->ct);
        cairo_surface_destroy(cache);
    }
    ctrl->shown = TRUE;
}

void SPCtrl::moveto (Geom::Point const p) {
    if (p != _point) {
        sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (this), Geom::Affine(Geom::Translate (p)));
        _moved = true;
    }
    _point = p;
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
