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
#include "display/sp-canvas.h"

enum {
    ARG_0,
    ARG_SHAPE,
    ARG_MODE,
    ARG_ANCHOR,
    ARG_SIZE,
    ARG_ANGLE,
    ARG_FILLED,
    ARG_FILL_COLOR,
    ARG_STROKED,
    ARG_STROKE_COLOR,
    ARG_PIXBUF
};

static void sp_ctrl_destroy(SPCanvasItem *object);
static void sp_ctrl_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void sp_ctrl_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void sp_ctrl_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_ctrl_render (SPCanvasItem *item, SPCanvasBuf *buf);

static double sp_ctrl_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);

G_DEFINE_TYPE(SPCtrl, sp_ctrl, SP_TYPE_CANVAS_ITEM);

static void
sp_ctrl_class_init (SPCtrlClass *klass)
{
    SPCanvasItemClass *item_class = SP_CANVAS_ITEM_CLASS(klass);
    GObjectClass *g_object_class = (GObjectClass *) klass;

    g_object_class->set_property = sp_ctrl_set_property;
    g_object_class->get_property = sp_ctrl_get_property;

    g_object_class_install_property (g_object_class,
            ARG_SHAPE, g_param_spec_int ("shape", "shape", "Shape", 0, G_MAXINT, SP_CTRL_SHAPE_SQUARE, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_MODE, g_param_spec_int ("mode", "mode", "Mode", 0, G_MAXINT, SP_CTRL_MODE_COLOR, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_ANCHOR, g_param_spec_int ("anchor", "anchor", "Anchor", 0, G_MAXINT, SP_ANCHOR_CENTER, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_SIZE, g_param_spec_double ("size", "size", "Size", 0.0, G_MAXDOUBLE, 8.0, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_ANGLE, g_param_spec_double ("angle", "angle", "Angle", -G_MAXDOUBLE, G_MAXDOUBLE, 0.0, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_FILLED, g_param_spec_boolean ("filled", "filled", "Filled", TRUE, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_FILL_COLOR, g_param_spec_int ("fill_color", "fill_color", "Fill Color", G_MININT, G_MAXINT, 0x000000ff, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_STROKED, g_param_spec_boolean ("stroked", "stroked", "Stroked", FALSE, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_STROKE_COLOR, g_param_spec_int ("stroke_color", "stroke_color", "Stroke Color", G_MININT, G_MAXINT, 0x000000ff, (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property (g_object_class,
            ARG_PIXBUF, g_param_spec_pointer ("pixbuf", "pixbuf", "Pixbuf", (GParamFlags) G_PARAM_READWRITE));

    item_class->destroy = sp_ctrl_destroy;
    item_class->update = sp_ctrl_update;
    item_class->render = sp_ctrl_render;
    item_class->point = sp_ctrl_point;
}

static void
sp_ctrl_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{

    SPCanvasItem *item;
    SPCtrl *ctrl;
    GdkPixbuf * pixbuf = NULL;

    item = SP_CANVAS_ITEM (object);
    ctrl = SP_CTRL (object);

    switch (prop_id) {
    case ARG_SHAPE:
        ctrl->shape = (SPCtrlShapeType) g_value_get_int(value);
        break;
    case ARG_MODE:
        ctrl->mode = (SPCtrlModeType) g_value_get_int(value);
        break;
    case ARG_ANCHOR:
        ctrl->anchor = (SPAnchorType) g_value_get_int(value);
        break;
    case ARG_SIZE:
        ctrl->width = (gint)(g_value_get_double(value) / 2.0);
        ctrl->height = ctrl->width;
        ctrl->defined = (ctrl->width > 0);
        break;
    case ARG_ANGLE:
        ctrl->angle = (double)g_value_get_double(value);
        break;
    case ARG_FILLED:
        ctrl->filled = g_value_get_boolean(value);
        break;
    case ARG_FILL_COLOR:
        ctrl->fill_color = (guint32)g_value_get_int(value);
        break;
    case ARG_STROKED:
        ctrl->stroked = g_value_get_boolean(value);
        break;
    case ARG_STROKE_COLOR:
        ctrl->stroke_color = (guint32)g_value_get_int(value);
        break;
    case ARG_PIXBUF:
        pixbuf = (GdkPixbuf*) g_value_get_pointer(value);
        // A pixbuf defines it's own size, don't mess about with size.
        ctrl->width = gdk_pixbuf_get_width(pixbuf) / 2.0;
        ctrl->height = gdk_pixbuf_get_height(pixbuf) / 2.0;
        if (gdk_pixbuf_get_has_alpha(pixbuf)) {
            ctrl->pixbuf = pixbuf;
        } else {
            ctrl->pixbuf = gdk_pixbuf_add_alpha(pixbuf, FALSE, 0, 0, 0);
            g_object_unref(pixbuf);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        return; // Do not do an update
    }
    ctrl->build = FALSE;
    sp_canvas_item_request_update(item);
}

static void
sp_ctrl_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    SPCtrl *ctrl;
    ctrl = SP_CTRL (object);

    switch (prop_id) {

        case ARG_SHAPE:
            g_value_set_int(value, ctrl->shape);
            break;

        case ARG_MODE:
            g_value_set_int(value, ctrl->mode);
            break;

        case ARG_ANCHOR:
            g_value_set_int(value, ctrl->anchor);
            break;

        case ARG_SIZE:
            g_value_set_double(value, ctrl->width);
            break;

        case ARG_ANGLE:
            g_value_set_double(value, ctrl->angle);
            break;

        case ARG_FILLED:
            g_value_set_boolean(value, ctrl->filled);
            break;

        case ARG_FILL_COLOR:
        	g_value_set_int(value, ctrl->fill_color);
            break;

        case ARG_STROKED:
            g_value_set_boolean(value, ctrl->stroked);
            break;

        case ARG_STROKE_COLOR:
            g_value_set_int(value, ctrl->stroke_color);
            break;

        case ARG_PIXBUF:
            g_value_set_pointer(value, ctrl->pixbuf);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }

}
static void
sp_ctrl_init (SPCtrl *ctrl)
{
    ctrl->shape = SP_CTRL_SHAPE_SQUARE;
    ctrl->mode = SP_CTRL_MODE_COLOR;
    ctrl->anchor = SP_ANCHOR_CENTER;
    ctrl->width = 3;
    ctrl->height = 3;
    ctrl->defined = TRUE;
    ctrl->shown = FALSE;
    ctrl->build = FALSE;
    ctrl->filled = 1;
    ctrl->stroked = 0;
    ctrl->fill_color = 0x000000ff;
    ctrl->stroke_color = 0x000000ff;
    ctrl->angle = 0.0;

    new (&ctrl->box) Geom::IntRect(0,0,0,0);
    ctrl->cache = NULL;
    ctrl->pixbuf = NULL;

    ctrl->_point = Geom::Point(0,0);
}

static void sp_ctrl_destroy(SPCanvasItem *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRL (object));

    SPCtrl *ctrl = SP_CTRL (object);

    if (ctrl->cache) {
        delete[] ctrl->cache;
        ctrl->cache = NULL;
    }

    if (SP_CANVAS_ITEM_CLASS(sp_ctrl_parent_class)->destroy)
        SP_CANVAS_ITEM_CLASS(sp_ctrl_parent_class)->destroy(object);
}

static void
sp_ctrl_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCtrl *ctrl;
    gint x, y;

    ctrl = SP_CTRL (item);

    if (SP_CANVAS_ITEM_CLASS(sp_ctrl_parent_class)->update)
        SP_CANVAS_ITEM_CLASS(sp_ctrl_parent_class)->update(item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    if (ctrl->shown) {
        item->canvas->requestRedraw(ctrl->box.left(), ctrl->box.top(), ctrl->box.right() + 1, ctrl->box.bottom() + 1);
    }

    if (!ctrl->defined) return;

    x = (gint) ((affine[4] > 0) ? (affine[4] + 0.5) : (affine[4] - 0.5)) - ctrl->width;
    y = (gint) ((affine[5] > 0) ? (affine[5] + 0.5) : (affine[5] - 0.5)) - ctrl->height;

    switch (ctrl->anchor) {
        case SP_ANCHOR_N:
        case SP_ANCHOR_CENTER:
        case SP_ANCHOR_S:
            break;

        case SP_ANCHOR_NW:
        case SP_ANCHOR_W:
        case SP_ANCHOR_SW:
            x += ctrl->width;
            break;

        case SP_ANCHOR_NE:
        case SP_ANCHOR_E:
        case SP_ANCHOR_SE:
            x -= (ctrl->width + 1);
            break;
    }

    switch (ctrl->anchor) {
        case SP_ANCHOR_W:
        case SP_ANCHOR_CENTER:
        case SP_ANCHOR_E:
            break;

        case SP_ANCHOR_NW:
        case SP_ANCHOR_N:
        case SP_ANCHOR_NE:
            y += ctrl->height;
            break;

        case SP_ANCHOR_SW:
        case SP_ANCHOR_S:
        case SP_ANCHOR_SE:
            y -= (ctrl->height + 1);
            break;
    }

    ctrl->box = Geom::IntRect::from_xywh(x, y, 2*ctrl->width, 2*ctrl->height);
    sp_canvas_update_bbox (item, ctrl->box.left(), ctrl->box.top(), ctrl->box.right() + 1, ctrl->box.bottom() + 1);
}

static double
sp_ctrl_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item)
{
    SPCtrl *ctrl = SP_CTRL (item);

    *actual_item = item;

    if (ctrl->box.contains(p.floor())) return 0.0;
    return 1e18;
}

bool 
sp_point_inside_line(Geom::Point a, Geom::Point b, Geom::Point c, double tolerance = 0.1){
    //http://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
    return Geom::are_near(Geom::distance(a,c) + Geom::distance(c,b) , Geom::distance(a,b), tolerance);
}

bool 
sp_point_inside_triangle(Geom::Point p1,Geom::Point p2,Geom::Point p3, Geom::Point point){
    using Geom::X;
    using Geom::Y;
    double denominator = (p1[X]*(p2[Y] - p3[Y]) + p1[Y]*(p3[X] - p2[X]) + p2[X]*p3[Y] - p2[Y]*p3[X]);
    double t1 = (point[X]*(p3[Y] - p1[Y]) + point[Y]*(p1[X] - p3[X]) - p1[X]*p3[Y] + p1[Y]*p3[X]) / denominator;
    double t2 = (point[X]*(p2[Y] - p1[Y]) + point[Y]*(p1[X] - p2[X]) - p1[X]*p2[Y] + p1[Y]*p2[X]) / -denominator;
    double see = t1 + t2;
    return 0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1 && see <= 1;
}

static void
sp_ctrl_build_cache (SPCtrl *ctrl)
{
    guint32 *p, *q;
    gint size, x, y, z, s, a, width, height, c;
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
    gint32 stroke_color_smooth =  SP_RGBA32_F_COMPOSE(SP_RGBA32_R_F(stroke_color), SP_RGBA32_G_F(stroke_color), SP_RGBA32_B_F(stroke_color), 0.15);
    width = (ctrl->width * 2 +1);
    height = (ctrl->height * 2 +1);
    c = ctrl->width; // Only used for pre-set square drawing
    size = width * height;
    if (width < 2) return;

    if (ctrl->cache) delete[] ctrl->cache;
    ctrl->cache = new guint32[size];
    Geom::Point point;
    Geom::Point p1;
    Geom::Point p2;
    Geom::Point p3;
    if(ctrl->shape == SP_CTRL_SHAPE_TRIANGLE){
        Geom::Affine m = Geom::Translate(Geom::Point(-width/2.0,-height/2.0));
        m *= Geom::Rotate(-ctrl->angle);
        m *= Geom::Translate(Geom::Point(width/2.0, height/2.0));
        p1 = Geom::Point(0,height/2);
        p2 = Geom::Point(width - (width/M_PI), height/M_PI);
        p3 = Geom::Point(width - (width/M_PI), height-(height/M_PI));
        p1 *= m;
        p2 *= m;
        p3 *= m;
        p1 = p1.floor();
        p2 = p2.floor();
        p3 = p3.floor();
    }
    switch (ctrl->shape) {
        case SP_CTRL_SHAPE_SQUARE:
            p = ctrl->cache;
            // top edge
            for (x=0; x < width; x++) {
                *p++ = stroke_color;
            }
            // middle
            for (y = 2; y < height; y++) {
                *p++ = stroke_color; // stroke at first and last pixel
                for (x=2; x < width; x++) {
                    *p++ = fill_color; // fill in the middle
                }
                *p++ = stroke_color;
            }
            // bottom edge
            for (x=0; x < width; x++) {
                *p++ = stroke_color;
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_DIAMOND:
            p = ctrl->cache;
            for (y = 0; y < height; y++) {
                z = abs (c - y);
                for (x = 0; x < z; x++) {
                    *p++ = 0;
                }
                *p++ = stroke_color; x++;
                for (; x < width - z -1; x++) {
                    *p++ = fill_color;
                }
                if (z != c) {
                    *p++ = stroke_color; x++;
                }
                for (; x < width; x++) {
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
                while (x < width) {
                    *p++ = 0;
                    *q-- = 0;
                    x++;
                }
                s = z;
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_TRIANGLE:
            p = ctrl->cache;
            for(y = 0; y < height; y++) {
                for(x = 0; x < width; x++) {
                    point = Geom::Point(x,y);
                    if (sp_point_inside_triangle(p1, p2, p3, point)) {
                        p[(y*width)+x] = fill_color;
                    } else if (point == p1 || point == p2 || point == p3 || sp_point_inside_line(p1, p2, point, 0.2) ||
                               sp_point_inside_line(p3, p1, point, 0.2))
                    {
                        p[(y*width)+x] = stroke_color;
                    } else if (sp_point_inside_line(p1, p2, point, 0.5) ||
                               sp_point_inside_line(p3, p1, point, 0.5))
                    {
                        p[(y*width)+x] = stroke_color_smooth;
                    } else {
                        p[(y*width)+x] = 0;
                    }
                }
            }
            ctrl->build = TRUE;
            break;

        case SP_CTRL_SHAPE_CROSS:
            p = ctrl->cache;
            for (y = 0; y < height; y++) {
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
                for (; x < width; x++) {
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
                for (y = 0; y < height; y++){
                    guint32 *d;
                    unsigned char *s;
                    s = px + y * rs;
                    d = ctrl->cache + height * y;
                    for (x = 0; x < width; x++) {
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
                for (y = 0; y < height; y++){
                    px = reinterpret_cast<guint32*>(data + y * r);
                    for (x = 0; x < width; x++) {
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

    int w = (ctrl->width * 2 + 1);
    int h = (ctrl->height * 2 + 1);

    // The code below works even when the target is not an image surface
    if (ctrl->mode == SP_CTRL_MODE_XOR) {
        // 1. Copy the affected part of output to a temporary surface
        cairo_surface_t *work = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_t *cr = cairo_create(work);
        cairo_translate(cr, -ctrl->box.left(), -ctrl->box.top());
        cairo_set_source_surface(cr, cairo_get_target(buf->ct), buf->rect.left(), buf->rect.top());
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
            ctrl->box.left() - buf->rect.left(), ctrl->box.top() - buf->rect.top());
        cairo_rectangle(buf->ct, ctrl->box.left() - buf->rect.left(), ctrl->box.top() - buf->rect.top(), w, h);
        cairo_clip(buf->ct);
        cairo_set_operator(buf->ct, CAIRO_OPERATOR_SOURCE);
        cairo_paint(buf->ct);
        cairo_restore(buf->ct);
        cairo_surface_destroy(work);
    } else {
        cairo_surface_t *cache = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char*>(ctrl->cache), CAIRO_FORMAT_ARGB32, w, h, w*4);
        cairo_set_source_surface(buf->ct, cache,
            ctrl->box.left() - buf->rect.left(), ctrl->box.top() - buf->rect.top());
        cairo_paint(buf->ct);
        cairo_surface_destroy(cache);
    }
    ctrl->shown = TRUE;
}

void SPCtrl::moveto (Geom::Point const p) {
    if (p != _point) {
        sp_canvas_item_affine_absolute (SP_CANVAS_ITEM (this), Geom::Affine(Geom::Translate (p)));
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
