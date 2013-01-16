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
#include <glibmm/i18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
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
#include "sp-cursor.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "document.h"
#include "document-undo.h"

#include "pixmaps/cursor-dropper.xpm"
#include "pixmaps/cursor-dropper-f.xpm"
#include "pixmaps/cursor-dropper-s.xpm"

#include "dropper-context.h"
#include "message-context.h"
#include "verbs.h"
#include "event-context.h"

using Inkscape::DocumentUndo;

typedef struct
{
    double        R;
    double        G;
    double        B;
    double        alpha;

    unsigned int  dragging : 1;
    
    SPCanvasItem *grabbed;
    SPCanvasItem *area;
    Geom::Point   centre;
} SPDropperContextPrivate;

#define SP_DROPPER_CONTEXT_GET_PRIVATE(dc) \
    G_TYPE_INSTANCE_GET_PRIVATE(dc, SP_TYPE_DROPPER_CONTEXT, SPDropperContextPrivate)

static void sp_dropper_context_class_init(SPDropperContextClass *klass);
static void sp_dropper_context_init(SPDropperContext *dc);

static void sp_dropper_context_setup(SPEventContext *ec);
static void sp_dropper_context_finish(SPEventContext *ec);

static gint sp_dropper_context_root_handler(SPEventContext *ec, GdkEvent * event);

static SPEventContextClass *parent_class;

static GdkCursor *cursor_dropper_fill = NULL;
static GdkCursor *cursor_dropper_stroke = NULL;

G_DEFINE_TYPE(SPDropperContext, sp_dropper_context, SP_TYPE_EVENT_CONTEXT);

static void
sp_dropper_context_class_init(SPDropperContextClass *klass)
{
    GObjectClass        *object_class = G_OBJECT_CLASS (klass);
    SPEventContextClass *ec_class     = SP_EVENT_CONTEXT_CLASS(klass);

    parent_class = SP_EVENT_CONTEXT_CLASS(g_type_class_peek_parent(klass));

    ec_class->setup = sp_dropper_context_setup;
    ec_class->finish = sp_dropper_context_finish;
    ec_class->root_handler = sp_dropper_context_root_handler;
    
    g_type_class_add_private(object_class, sizeof(SPDropperContext));
}

static void sp_dropper_context_init(SPDropperContext *dc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(dc);
    event_context->cursor_shape = cursor_dropper_f_xpm;
    event_context->hot_x = 7;
    event_context->hot_y = 7;

    cursor_dropper_fill = sp_cursor_new_from_xpm(cursor_dropper_f_xpm , 7, 7);
    cursor_dropper_stroke = sp_cursor_new_from_xpm(cursor_dropper_s_xpm , 7, 7);

}

static void
sp_dropper_context_setup(SPEventContext *ec)
{
    SPDropperContext        *dc   = SP_DROPPER_CONTEXT(ec);
    SPDropperContextPrivate *priv = SP_DROPPER_CONTEXT_GET_PRIVATE(dc);

    if ((SP_EVENT_CONTEXT_CLASS(parent_class))->setup) {
        (SP_EVENT_CONTEXT_CLASS(parent_class))->setup(ec);
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
    priv->area = sp_canvas_bpath_new(sp_desktop_controls(ec->desktop), c);
    c->unref();
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(priv->area), 0x00000000,(SPWindRule)0);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(priv->area), 0x0000007f, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_item_hide(priv->area);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/dropper/selcue")) {
        ec->enableSelectionCue();
    }

    if (prefs->getBool("/tools/dropper/gradientdrag")) {
        ec->enableGrDrag();
    }
}

static void
sp_dropper_context_finish(SPEventContext *ec)
{
    SPDropperContext        *dc   = SP_DROPPER_CONTEXT(ec);
    SPDropperContextPrivate *priv = SP_DROPPER_CONTEXT_GET_PRIVATE(dc);

    ec->enableGrDrag(false);
	
    if (priv->grabbed) {
        sp_canvas_item_ungrab(priv->grabbed, GDK_CURRENT_TIME);
        priv->grabbed = NULL;
    }

    if (priv->area) {
        sp_canvas_item_destroy(priv->area);
        priv->area = NULL;
    }

    if (cursor_dropper_fill) {
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(cursor_dropper_fill);
#else
        gdk_cursor_unref (cursor_dropper_fill);
#endif
        cursor_dropper_fill = NULL;
    }
    if (cursor_dropper_stroke) {
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(cursor_dropper_stroke);
#else
        gdk_cursor_unref (cursor_dropper_stroke);
#endif
        cursor_dropper_fill = NULL;
    }

    //Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    //prefs->setBool("/tools/dropper/onetimepick", false);

}

/**
 * Returns the current dropper context color.
 */
guint32
sp_dropper_context_get_color(SPEventContext *ec)
{
    SPDropperContext        *dc    = SP_DROPPER_CONTEXT(ec);
    SPDropperContextPrivate *priv  = SP_DROPPER_CONTEXT_GET_PRIVATE(dc);
    Inkscape::Preferences   *prefs = Inkscape::Preferences::get();

    int pick = prefs->getInt("/tools/dropper/pick",
                             SP_DROPPER_PICK_VISIBLE);

    bool setalpha = prefs->getBool("/tools/dropper/setalpha", true);

    return SP_RGBA32_F_COMPOSE(priv->R,
                               priv->G,
                               priv->B,
                               (pick == SP_DROPPER_PICK_ACTUAL && setalpha) ? priv->alpha 
                                                                              : 1.0);
}

static gint
sp_dropper_context_root_handler(SPEventContext *event_context,
                                GdkEvent       *event)
{
    SPDropperContext        *dc      = SP_DROPPER_CONTEXT(event_context);
    SPDropperContextPrivate *priv    = SP_DROPPER_CONTEXT_GET_PRIVATE(dc);
    SPDesktop               *desktop = event_context->desktop;
    Inkscape::Preferences   *prefs   = Inkscape::Preferences::get();

    int ret = FALSE;

    int pick = prefs->getInt("/tools/dropper/pick", SP_DROPPER_PICK_VISIBLE);
    bool setalpha = prefs->getBool("/tools/dropper/setalpha", true);

    switch (event->type) {
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
                priv->centre = Geom::Point(event->button.x, event->button.y);
                priv->dragging = TRUE;
                ret = TRUE;
            }
			
			sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
								GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
								NULL, event->button.time);
			priv->grabbed = SP_CANVAS_ITEM(desktop->acetate);

            break;
	case GDK_MOTION_NOTIFY:
            if (event->motion.state & GDK_BUTTON2_MASK || event->motion.state & GDK_BUTTON3_MASK) {
                // pass on middle and right drag
                ret = FALSE;
                break;
            } else if (!event_context->space_panning) {
                // otherwise, constantly calculate color no matter is any button pressed or not

                // If one time pick with stroke set the pixmap
                if (prefs->getBool("/tools/dropper/onetimepick", false) && prefs->getInt("/dialogs/fillstroke/page", 0) == 1) {
                    //TODO Only set when not set already
                    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(sp_desktop_canvas(desktop)));
                    gdk_window_set_cursor(window, cursor_dropper_stroke);
                }

                double rw = 0.0;
                double R(0), G(0), B(0), A(0);

                if (priv->dragging) {
                    // calculate average

                    // radius
                    rw = std::min(Geom::L2(Geom::Point(event->button.x, event->button.y) - priv->centre), 400.0);

                    if (rw == 0) { // happens sometimes, little idea why...
                        break;
                    }

                    Geom::Point const cd = desktop->w2d(priv->centre);
                    Geom::Affine const w2dt = desktop->w2d();
                    const double scale = rw * w2dt.descrim();
                    Geom::Affine const sm( Geom::Scale(scale, scale) * Geom::Translate(cd) );
                    sp_canvas_item_affine_absolute(priv->area, sm);
                    sp_canvas_item_show(priv->area);

                    /* Get buffer */
                    Geom::Rect r(priv->centre, priv->centre);
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
                priv->R = R;
                priv->G = G;
                priv->B = B;
                priv->alpha = A;

                // status message
                double alpha_to_set = setalpha? priv->alpha : 1.0;
                guint32 c32 = SP_RGBA32_F_COMPOSE(R, G, B, alpha_to_set);

                gchar c[64];
                sp_svg_write_color(c, sizeof(c), c32);

                // alpha of color under cursor, to show in the statusbar
                // locale-sensitive printf is OK, since this goes to the UI, not into SVG
                gchar *alpha = g_strdup_printf(_(" alpha %.3g"), alpha_to_set);
                // where the color is picked, to show in the statusbar
                gchar *where = priv->dragging ? g_strdup_printf(_(", averaged with radius %d"), (int) rw) : g_strdup_printf(_(" under cursor"));
                // message, to show in the statusbar
                const gchar *message = priv->dragging ? _("<b>Release mouse</b> to set color.") : _("<b>Click</b> to set fill, <b>Shift+click</b> to set stroke; <b>drag</b> to average color in area; with <b>Alt</b> to pick inverse color; <b>Ctrl+C</b> to copy the color under mouse to clipboard");
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
                sp_canvas_item_hide(priv->area);
                priv->dragging = FALSE;
				
				if (priv->grabbed) {
					sp_canvas_item_ungrab(priv->grabbed, event->button.time);
					priv->grabbed = NULL;
				}

                double alpha_to_set = setalpha? priv->alpha : 1.0;

                bool fill = !(event->button.state & GDK_SHIFT_MASK); // Stroke if Shift key held
                if (prefs->getBool("/tools/dropper/onetimepick", false)) {
                    // "One time" pick from Fill/Stroke dialog stroke page, always apply fill or stroke (ignore <Shift> key)
                    fill = (prefs->getInt("/dialogs/fillstroke/page", 0) == 0)  ? true : false;
                }

                // do the actual color setting
                sp_desktop_set_color(desktop,
                                     (event->button.state & GDK_MOD1_MASK)?
                                     ColorRGBA(1 - priv->R, 1 - priv->G, 1 - priv->B, alpha_to_set) : ColorRGBA(priv->R, priv->G, priv->B, alpha_to_set),
                                     false,  fill);

                // REJON: set aux. toolbar input to hex color!

                if (event->button.state & GDK_SHIFT_MASK) {
                    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(sp_desktop_canvas(desktop)));
                    gdk_window_set_cursor(window, cursor_dropper_stroke);
                }

                if (!(sp_desktop_selection(desktop)->isEmpty())) {
                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_DROPPER,
                                       _("Set picked color"));
                }

                if (prefs->getBool("/tools/dropper/onetimepick", false)) {
                    prefs->setBool("/tools/dropper/onetimepick", false);
                    sp_toggle_dropper(desktop);
                }

                ret = TRUE;
            }
            break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval(&event->key)) {
        case GDK_KEY_Up:
        case GDK_KEY_Down:
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY) {
                ret = TRUE;
            }
            break;
        case GDK_KEY_Escape:
            sp_desktop_selection(desktop)->clear();
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            if (!desktop->isWaitingCursor() && !prefs->getBool("/tools/dropper/onetimepick", false)) {
                GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(sp_desktop_canvas(desktop)));
                gdk_window_set_cursor(window, cursor_dropper_stroke);
            }

            break;
        default:
            break;
        }
        break;
    case GDK_KEY_RELEASE:
        switch (get_group0_keyval(&event->key)) {
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            if (!desktop->isWaitingCursor() && !prefs->getBool("/tools/dropper/onetimepick", false)) {
                GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(sp_desktop_canvas(desktop)));
                gdk_window_set_cursor(window, cursor_dropper_fill);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if ((SP_EVENT_CONTEXT_CLASS(parent_class))->root_handler) {
            ret = (SP_EVENT_CONTEXT_CLASS(parent_class))->root_handler(event_context, event);
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
