/*
 * Tool for picking colors from drawing
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm.h>
#include <gdk/gdk.h>
#include <2geom/transforms.h>

#include "macros.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "display/curve.h"
#include "display/cairo-utils.h"
#include "svg/svg-color.h"
#include "color.h"
#include "color-rgba.h"
#include "desktop-style.h"
#include "preferences.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "document.h"

#include "pixmaps/cursor-dropper.xpm"

#include "dropper-context.h"
#include "message-context.h"

using Inkscape::DocumentUndo;

static void sp_dropper_context_class_init(SPDropperContextClass *klass);
static void sp_dropper_context_init(SPDropperContext *dc);

static void sp_dropper_context_setup(SPEventContext *ec);
static void sp_dropper_context_finish(SPEventContext *ec);

static gint sp_dropper_context_root_handler(SPEventContext *ec, GdkEvent * event);

static SPEventContextClass *parent_class;

GType sp_dropper_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPDropperContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_dropper_context_class_init,
            NULL, NULL,
            sizeof(SPDropperContext),
            4,
            (GInstanceInitFunc) sp_dropper_context_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPDropperContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_dropper_context_class_init(SPDropperContextClass *klass)
{
    SPEventContextClass *ec_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    ec_class->setup = sp_dropper_context_setup;
    ec_class->finish = sp_dropper_context_finish;
    ec_class->root_handler = sp_dropper_context_root_handler;
}

static void sp_dropper_context_init(SPDropperContext *dc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(dc);
    event_context->cursor_shape = cursor_dropper_xpm;
    event_context->hot_x = 7;
    event_context->hot_y = 7;
}

static void sp_dropper_context_setup(SPEventContext *ec)
{
    SPDropperContext *dc = SP_DROPPER_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    /* TODO: have a look at sp_dyna_draw_context_setup where the same is done.. generalize? at least make it an arcto! */
    SPCurve *c = new SPCurve();
    const double C1 = 0.552;
    c->moveto(-1,0);
    c->curveto(-1, C1, -C1, 1, 0, 1 );
    c->curveto(C1, 1, 1, C1, 1, 0 );
    c->curveto(1, -C1, C1, -1, 0, -1 );
    c->curveto(-C1, -1, -1, -C1, -1, 0 );
    c->closepath();
    dc->area = sp_canvas_bpath_new(sp_desktop_controls(ec->desktop), c);
    c->unref();
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(dc->area), 0x00000000,(SPWindRule)0);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(dc->area), 0x0000007f, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_item_hide(dc->area);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/dropper/selcue")) {
        ec->enableSelectionCue();
    }

    if (prefs->getBool("/tools/dropper/gradientdrag")) {
        ec->enableGrDrag();
    }
}

static void sp_dropper_context_finish(SPEventContext *ec)
{
    SPDropperContext *dc = SP_DROPPER_CONTEXT(ec);

    ec->enableGrDrag(false);
	
    if (dc->grabbed) {
        sp_canvas_item_ungrab(dc->grabbed, GDK_CURRENT_TIME);
        dc->grabbed = NULL;
    }

    if (dc->area) {
        gtk_object_destroy(GTK_OBJECT(dc->area));
        dc->area = NULL;
    }
}


/**
 * Returns the current dropper context icc-color.
 */
SPColor* sp_dropper_context_get_icc_color(SPEventContext */*ec*/)
{
    //TODO: implement-me!

    return 0; // At least we will cause a clean crash, instead of random corruption.
}

/**
 * Returns the current dropper context color.
 */
guint32 sp_dropper_context_get_color(SPEventContext *ec)
{
    SPDropperContext *dc = SP_DROPPER_CONTEXT(ec);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    int pick = prefs->getInt("/tools/dropper/pick",
                                       SP_DROPPER_PICK_VISIBLE);
    bool setalpha = prefs->getBool("/tools/dropper/setalpha", true);

    return SP_RGBA32_F_COMPOSE(dc->R, dc->G, dc->B,
        (pick == SP_DROPPER_PICK_ACTUAL && setalpha) ? dc->alpha : 1.0);
}


static gint sp_dropper_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPDropperContext *dc = (SPDropperContext *) event_context;
    int ret = FALSE;
    SPDesktop *desktop = event_context->desktop;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    int pick = prefs->getInt("/tools/dropper/pick", SP_DROPPER_PICK_VISIBLE);
    bool setalpha = prefs->getBool("/tools/dropper/setalpha", true);

    switch (event->type) {
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
                dc->centre = Geom::Point(event->button.x, event->button.y);
                dc->dragging = TRUE;
                ret = TRUE;
            }
			
			sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
								GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
								NULL, event->button.time);
			dc->grabbed = SP_CANVAS_ITEM(desktop->acetate);
			
            break;
	case GDK_MOTION_NOTIFY:
            if (event->motion.state & GDK_BUTTON2_MASK) {
                // pass on middle-drag
                ret = FALSE;
                break;
            } else if (!event_context->space_panning) {
                // otherwise, constantly calculate color no matter is any button pressed or not

                double rw = 0.0;
                double R(0), G(0), B(0), A(0);

                if (dc->dragging) {
                    // calculate average

                    // radius
                    rw = std::min(Geom::L2(Geom::Point(event->button.x, event->button.y) - dc->centre), 400.0);

                    if (rw == 0) { // happens sometimes, little idea why...
                        break;
                    }

                    Geom::Point const cd = desktop->w2d(dc->centre);
                    Geom::Affine const w2dt = desktop->w2d();
                    const double scale = rw * w2dt.descrim();
                    Geom::Affine const sm( Geom::Scale(scale, scale) * Geom::Translate(cd) );
                    sp_canvas_item_affine_absolute(dc->area, sm);
                    sp_canvas_item_show(dc->area);

                    /* Get buffer */
                    Geom::Rect r(dc->centre, dc->centre);
                    r.expandBy(rw);
                    if (!r.hasZeroArea()) {
                        Geom::IntRect area = r.roundOutwards();
                        cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, area.width(), area.height());
                        sp_canvas_arena_render_surface(SP_CANVAS_ARENA(sp_desktop_drawing(desktop)), s, area);
                        ink_cairo_surface_average_color_premul(s, R, G, B, A);
                        cairo_surface_destroy(s);
                    }
                } else {
                    // pick single pixel
                    Geom::IntRect area = Geom::IntRect::from_xywh(floor(event->button.x), floor(event->button.y), 1, 1);
                    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
                    sp_canvas_arena_render_surface(SP_CANVAS_ARENA(sp_desktop_drawing(desktop)), s, area);
                    ink_cairo_surface_average_color_premul(s, R, G, B, A);
                    cairo_surface_destroy(s);
                }

                if (pick == SP_DROPPER_PICK_VISIBLE) {
                    // compose with page color
                    guint32 bg = sp_desktop_namedview(desktop)->pagecolor;
                    R = R + (SP_RGBA32_R_F(bg)) * (1 - A);
                    G = G + (SP_RGBA32_G_F(bg)) * (1 - A);
                    B = B + (SP_RGBA32_B_F(bg)) * (1 - A);
                    A = 1.0;
                } else {
                    // un-premultiply color channels
                    if (A > 0) {
                        R /= A;
                        G /= A;
                        B /= A;
                    }
                }

                if (fabs(A) < 1e-4) {
                    A = 0; // suppress exponentials, CSS does not allow that
                }

                // remember color
                dc->R = R;
                dc->G = G;
                dc->B = B;
                dc->alpha = A;

                // status message
                double alpha_to_set = setalpha? dc->alpha : 1.0;
                guint32 c32 = SP_RGBA32_F_COMPOSE(R, G, B, alpha_to_set);

                gchar c[64];
                sp_svg_write_color(c, sizeof(c), c32);

                // alpha of color under cursor, to show in the statusbar
                // locale-sensitive printf is OK, since this goes to the UI, not into SVG
                gchar *alpha = g_strdup_printf(_(" alpha %.3g"), alpha_to_set);
                // where the color is picked, to show in the statusbar
                gchar *where = dc->dragging ? g_strdup_printf(_(", averaged with radius %d"), (int) rw) : g_strdup_printf(_(" under cursor"));
                // message, to show in the statusbar
                const gchar *message = dc->dragging ? _("<b>Release mouse</b> to set color.") : _("<b>Click</b> to set fill, <b>Shift+click</b> to set stroke; <b>drag</b> to average color in area; with <b>Alt</b> to pick inverse color; <b>Ctrl+C</b> to copy the color under mouse to clipboard");
                event_context->defaultMessageContext()->setF(
                    Inkscape::NORMAL_MESSAGE,
                    "<b>%s%s</b>%s. %s", c,
                    (pick == SP_DROPPER_PICK_VISIBLE)? "" : alpha,
                    where, message
                    );

                g_free(where);
                g_free(alpha);

                ret = TRUE;
            }
            break;
	case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && !event_context->space_panning)
            {
                sp_canvas_item_hide(dc->area);
                dc->dragging = FALSE;
				
				if (dc->grabbed) {
					sp_canvas_item_ungrab(dc->grabbed, event->button.time);
					dc->grabbed = NULL;
				}

                double alpha_to_set = setalpha? dc->alpha : 1.0;

                // do the actual color setting
                sp_desktop_set_color(desktop,
                                     (event->button.state & GDK_MOD1_MASK)?
                                     ColorRGBA(1 - dc->R, 1 - dc->G, 1 - dc->B, alpha_to_set) : ColorRGBA(dc->R, dc->G, dc->B, alpha_to_set),
                                     false,  !(event->button.state & GDK_SHIFT_MASK));

                // REJON: set aux. toolbar input to hex color!


                if (!(sp_desktop_selection(desktop)->isEmpty())) {
                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_DROPPER,
                                       _("Set picked color"));
                }

                ret = TRUE;
            }
            break;
	case GDK_KEY_PRESS:
            switch (get_group0_keyval(&event->key)) {
		case GDK_Up:
		case GDK_Down:
		case GDK_KP_Up:
		case GDK_KP_Down:
                    // prevent the zoom field from activation
                    if (!MOD__CTRL_ONLY) {
                        ret = TRUE;
                    }
                    break;
		case GDK_Escape:
                    sp_desktop_selection(desktop)->clear();
		default:
                    break;
            }
            break;
	default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
