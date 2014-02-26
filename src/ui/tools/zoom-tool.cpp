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

#include "ui/tools/zoom-tool.h"
#include "tool-factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

namespace {
	ToolBase* createZoomContext() {
		return new ZoomTool();
	}

	bool zoomContextRegistered = ToolFactory::instance().registerObject("/tools/zoom", createZoomContext);
}

const std::string& ZoomTool::getPrefsPath() {
	return ZoomTool::prefsPath;
}

const std::string ZoomTool::prefsPath = "/tools/zoom";

ZoomTool::ZoomTool()
    : ToolBase(cursor_zoom_xpm, 6, 6)
    , grabbed(NULL)
    , escaped(false)
{
}

ZoomTool::~ZoomTool() {
}

void ZoomTool::finish() {
	this->enableGrDrag(false);
	
    if (this->grabbed) {
        sp_canvas_item_ungrab(this->grabbed, GDK_CURRENT_TIME);
        this->grabbed = NULL;
    }

    ToolBase::finish();
}

void ZoomTool::setup() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool("/tools/zoom/selcue")) {
        this->enableSelectionCue();
    }

    if (prefs->getBool("/tools/zoom/gradientdrag")) {
        this->enableGrDrag();
    }

    ToolBase::setup();
}

bool ZoomTool::root_handler(GdkEvent* event) {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
    double const zoom_inc = prefs->getDoubleLimited("/options/zoomincrement/value", M_SQRT2, 1.01, 10);

    bool ret = false;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
        {
            Geom::Point const button_w(event->button.x, event->button.y);
            Geom::Point const button_dt(desktop->w2d(button_w));

            if (event->button.button == 1 && !this->space_panning) {
                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                Inkscape::Rubberband::get(desktop)->start(desktop, button_dt);

                escaped = false;

                ret = true;
            } else if (event->button.button == 3) {
                double const zoom_rel( (event->button.state & GDK_SHIFT_MASK)
                                       ? zoom_inc
                                       : 1 / zoom_inc );

                desktop->zoom_relative_keep_point(button_dt, zoom_rel);
                ret = true;
            }
			
			sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
								GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
								NULL, event->button.time);

			this->grabbed = SP_CANVAS_ITEM(desktop->acetate);
            break;
        }

	case GDK_MOTION_NOTIFY:
            if ((event->motion.state & GDK_BUTTON1_MASK) && !this->space_panning) {
                ret = true;

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

            if ( event->button.button == 1  && !this->space_panning) {
                Geom::OptRect const b = Inkscape::Rubberband::get(desktop)->getRectangle();

                if (b && !within_tolerance) {
                    desktop->set_display_area(*b, 10);
                } else if (!escaped) {
                    double const zoom_rel( (event->button.state & GDK_SHIFT_MASK)
                                           ? 1 / zoom_inc
                                           : zoom_inc );

                    desktop->zoom_relative_keep_point(button_dt, zoom_rel);
                }

                ret = true;
            }

            Inkscape::Rubberband::get(desktop)->stop();
			
			if (this->grabbed) {
				sp_canvas_item_ungrab(this->grabbed, event->button.time);
				this->grabbed = NULL;
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
                    ret = true;
                    break;

                case GDK_KEY_Up:
                case GDK_KEY_Down:
                case GDK_KEY_KP_Up:
                case GDK_KEY_KP_Down:
                    // prevent the zoom field from activation
                    if (!MOD__CTRL_ONLY(event))
                        ret = true;
                    break;

                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    this->cursor_shape = cursor_zoom_out_xpm;
                    this->sp_event_context_update_cursor();
                    break;

                case GDK_KEY_Delete:
                case GDK_KEY_KP_Delete:
                case GDK_KEY_BackSpace:
                    ret = this->deleteSelectedDrag(MOD__CTRL_ONLY(event));
                    break;

                default:
			break;
		}
		break;
	case GDK_KEY_RELEASE:
            switch (get_group0_keyval (&event->key)) {
            	case GDK_KEY_Shift_L:
            	case GDK_KEY_Shift_R:
                    this->cursor_shape = cursor_zoom_xpm;
                    this->sp_event_context_update_cursor();
                    break;
            	default:
                    break;
            }
            break;
	default:
            break;
    }

    if (!ret) {
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
