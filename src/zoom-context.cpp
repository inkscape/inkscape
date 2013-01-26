/*
 * Handy zooming tool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "rubberband.h"
#include "display/sp-canvas-item.h"
#include "display/sp-canvas-util.h"
#include "desktop.h"
#include "pixmaps/cursor-zoom.xpm"
#include "pixmaps/cursor-zoom-out.xpm"
#include "preferences.h"
#include "selection-chemistry.h"

#include "zoom-context.h"

static void sp_zoom_context_setup(SPEventContext *ec);
static void sp_zoom_context_finish (SPEventContext *ec);

static gint sp_zoom_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_zoom_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;
static bool escaped;

G_DEFINE_TYPE(SPZoomContext, sp_zoom_context, SP_TYPE_EVENT_CONTEXT);

static void sp_zoom_context_class_init(SPZoomContextClass *klass)
{
    SPEventContextClass *event_context_class = SP_EVENT_CONTEXT_CLASS(klass);

    event_context_class->setup = sp_zoom_context_setup;
    event_context_class->finish = sp_zoom_context_finish;

    event_context_class->root_handler = sp_zoom_context_root_handler;
    event_context_class->item_handler = sp_zoom_context_item_handler;
}

static void sp_zoom_context_init (SPZoomContext *zoom_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(zoom_context);

    event_context->cursor_shape = cursor_zoom_xpm;
    event_context->hot_x = 6;
    event_context->hot_y = 6;
}

static void
sp_zoom_context_finish (SPEventContext *ec)
{
	SPZoomContext *zc = SP_ZOOM_CONTEXT(ec);
	
	ec->enableGrDrag(false);
	
    if (zc->grabbed) {
        sp_canvas_item_ungrab(zc->grabbed, GDK_CURRENT_TIME);
        zc->grabbed = NULL;
    }
}

static void sp_zoom_context_setup(SPEventContext *ec)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/zoom/selcue")) {
        ec->enableSelectionCue();
    }
    if (prefs->getBool("/tools/zoom/gradientdrag")) {
        ec->enableGrDrag();
    }

    if ((SP_EVENT_CONTEXT_CLASS(sp_zoom_context_parent_class))->setup) {
        (SP_EVENT_CONTEXT_CLASS(sp_zoom_context_parent_class))->setup(ec);
    }
}

static gint sp_zoom_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    if ((SP_EVENT_CONTEXT_CLASS(sp_zoom_context_parent_class))->item_handler) {
        ret = (SP_EVENT_CONTEXT_CLASS(sp_zoom_context_parent_class))->item_handler (event_context, item, event);
    }

    return ret;
}

static gint sp_zoom_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	
	SPZoomContext *zc = SP_ZOOM_CONTEXT(event_context);
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
    double const zoom_inc = prefs->getDoubleLimited("/options/zoomincrement/value", M_SQRT2, 1.01, 10);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
        {
            Geom::Point const button_w(event->button.x, event->button.y);
            Geom::Point const button_dt(desktop->w2d(button_w));
            if (event->button.button == 1 && !event_context->space_panning) {
                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                Inkscape::Rubberband::get(desktop)->start(desktop, button_dt);

                escaped = false;

                ret = TRUE;
            } else if (event->button.button == 3) {
                double const zoom_rel( (event->button.state & GDK_SHIFT_MASK)
                                       ? zoom_inc
                                       : 1 / zoom_inc );
                desktop->zoom_relative_keep_point(button_dt, zoom_rel);
                ret = TRUE;
            }
			
			sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
								GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
								NULL, event->button.time);
			zc->grabbed = SP_CANVAS_ITEM(desktop->acetate);
				
            break;
        }

	case GDK_MOTION_NOTIFY:
            if (event->motion.state & GDK_BUTTON1_MASK && !event_context->space_panning) {
                ret = TRUE;

                if ( within_tolerance
                     && ( abs( (gint) event->motion.x - xp ) < tolerance )
                     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to move the object, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                within_tolerance = false;

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const motion_dt(desktop->w2d(motion_w));
                Inkscape::Rubberband::get(desktop)->move(motion_dt);
                gobble_motion_events(GDK_BUTTON1_MASK);
            }
            break;

      	case GDK_BUTTON_RELEASE:
        {
            Geom::Point const button_w(event->button.x, event->button.y);
            Geom::Point const button_dt(desktop->w2d(button_w));
            if ( event->button.button == 1  && !event_context->space_panning) {
                Geom::OptRect const b = Inkscape::Rubberband::get(desktop)->getRectangle();
                if (b && !within_tolerance) {
                    desktop->set_display_area(*b, 10);
                } else if (!escaped) {
                    double const zoom_rel( (event->button.state & GDK_SHIFT_MASK)
                                           ? 1 / zoom_inc
                                           : zoom_inc );
                    desktop->zoom_relative_keep_point(button_dt, zoom_rel);
                }
                ret = TRUE;
            } 
            Inkscape::Rubberband::get(desktop)->stop();
			
			if (zc->grabbed) {
				sp_canvas_item_ungrab(zc->grabbed, event->button.time);
				zc->grabbed = NULL;
			}
			
            xp = yp = 0;
            escaped = false;
            break;
        }
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_KEY_Escape:
                    if (!Inkscape::Rubberband::get(desktop)->is_started()) {
                        Inkscape::SelectionHelper::selectNone(desktop);
                    }
                    Inkscape::Rubberband::get(desktop)->stop();
                    xp = yp = 0;
                    escaped = true;
                    ret = TRUE;
                    break;
                case GDK_KEY_Up:
                case GDK_KEY_Down:
                case GDK_KEY_KP_Up:
                case GDK_KEY_KP_Down:
                    // prevent the zoom field from activation
                    if (!MOD__CTRL_ONLY)
                        ret = TRUE;
                    break;
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    event_context->cursor_shape = cursor_zoom_out_xpm;
                    sp_event_context_update_cursor(event_context);
                    break;
                case GDK_KEY_Delete:
                case GDK_KEY_KP_Delete:
                case GDK_KEY_BackSpace:
                    ret = event_context->deleteSelectedDrag(MOD__CTRL_ONLY);
                    break;

                default:
			break;
		}
		break;
	case GDK_KEY_RELEASE:
            switch (get_group0_keyval (&event->key)) {
		case GDK_KEY_Shift_L:
		case GDK_KEY_Shift_R:
                    event_context->cursor_shape = cursor_zoom_xpm;
                    sp_event_context_update_cursor(event_context);
                    break;
		default:
                    break;
		}
            break;
	default:
            break;
    }

    if (!ret) {
        if ((SP_EVENT_CONTEXT_CLASS(sp_zoom_context_parent_class))->root_handler) {
            ret = (SP_EVENT_CONTEXT_CLASS(sp_zoom_context_parent_class))->root_handler(event_context, event);
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
