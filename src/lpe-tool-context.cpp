/*
 * LPEToolContext: a context for a generic tool composed of subtools that are given by LPEs
 *
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include "forward.h"
#include "pixmaps/cursor-pencil.xpm"
#include <gtk/gtk.h>
#include "desktop.h"
#include "message-context.h"
#include "prefs-utils.h"

/**

#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>
#include <string>
#include <cstring>
#include <numeric>

#include "svg/svg.h"
#include "display/canvas-bpath.h"
#include "display/bezier-utils.h"

#include <glib/gmem.h>
#include "macros.h"
#include "document.h"
#include "selection.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-style.h"
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"
#include "color.h"
#include "rubberband.h"
#include "splivarot.h"
#include "sp-item-group.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "sp-text.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "livarot/Shape.h"
#include <2geom/isnan.h>
#include <2geom/pathvector.h>
**/

#include "lpe-tool-context.h"

static void sp_lpetool_context_class_init(SPLPEToolContextClass *klass);
static void sp_lpetool_context_init(SPLPEToolContext *erc);
static void sp_lpetool_context_dispose(GObject *object);

static void sp_lpetool_context_setup(SPEventContext *ec);
static void sp_lpetool_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_lpetool_context_root_handler(SPEventContext *ec, GdkEvent *event);


static SPPenContextClass *lpetool_parent_class = 0;

GType sp_lpetool_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPLPEToolContextClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_lpetool_context_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPLPEToolContext),
            0, // n_preallocs
            (GInstanceInitFunc)sp_lpetool_context_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_PEN_CONTEXT, "SPLPEToolContext", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_lpetool_context_class_init(SPLPEToolContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    lpetool_parent_class = (SPPenContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_lpetool_context_dispose;

    event_context_class->setup = sp_lpetool_context_setup;
    event_context_class->set = sp_lpetool_context_set;
    //event_context_class->root_handler = sp_lpetool_context_root_handler;
}

static void
sp_lpetool_context_init(SPLPEToolContext *erc)
{
    erc->cursor_shape = cursor_pencil_xpm;
    erc->hot_x = 4;
    erc->hot_y = 4;
}

static void
sp_lpetool_context_dispose(GObject *object)
{
    //SPLPEToolContext *erc = SP_LPETOOL_CONTEXT(object);

    G_OBJECT_CLASS(lpetool_parent_class)->dispose(object);
}

static void
sp_lpetool_context_setup(SPEventContext *ec)
{
    SPLPEToolContext *erc = SP_LPETOOL_CONTEXT(ec);

    if (((SPEventContextClass *) lpetool_parent_class)->setup)
        ((SPEventContextClass *) lpetool_parent_class)->setup(ec);

    erc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    if (prefs_get_int_attribute("tools.lpetool", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }
// TODO temp force:
    ec->enableSelectionCue();
}

static void
sp_lpetool_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    // FIXME: how to set this correcly? the value from preferences-skeleton.h doesn't seem to get
    // read (it wants to set drag = 1)
    lpetool_parent_class->set(ec, key, "drag");

    /**
    //pass on up to parent class to handle common attributes.
    if ( lpetool_parent_class->set ) {
        lpetool_parent_class->set(ec, key, val);
    }
    **/
}

/**
void
sp_erc_update_toolbox (SPDesktop *desktop, const gchar *id, double value)
{
    desktop->setToolboxAdjustmentValue (id, value);
}
**/

gint
sp_lpetool_context_root_handler(SPEventContext *event_context,
                                  GdkEvent *event)
{
    //SPLPEToolContext *dc = SP_LPETOOL_CONTEXT(event_context);
    //SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    /**
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(dc);

                if (Inkscape::have_viable_layer(desktop, dc->_message_context) == false) {
                    return TRUE;
                }

                NR::Point const button_w(event->button.x,
                                         event->button.y);
                NR::Point const button_dt(desktop->w2d(button_w));
                sp_lpetool_reset(dc, button_dt);
                sp_lpetool_extinput(dc, event);
                sp_lpetool_apply(dc, button_dt);
                dc->accumulated->reset();
                if (dc->repr) {
                    dc->repr = NULL;
                }

                Inkscape::Rubberband::get()->start(desktop, button_dt);
                Inkscape::Rubberband::get()->setMode(RUBBERBAND_MODE_TOUCHPATH);

                // initialize first point
                dc->npoints = 0;

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    ( GDK_KEY_PRESS_MASK |
                                      GDK_BUTTON_RELEASE_MASK |
                                      GDK_POINTER_MOTION_MASK |
                                      GDK_BUTTON_PRESS_MASK ),
                                    NULL,
                                    event->button.time);

                ret = TRUE;

                sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 3);
                dc->is_drawing = true;
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point motion_dt(desktop->w2d(motion_w));
            sp_lpetool_extinput(dc, event);

            dc->_message_context->clear();

            if ( dc->is_drawing && (event->motion.state & GDK_BUTTON1_MASK) && !event_context->space_panning) {
                dc->dragging = TRUE;

                dc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Drawing</b> an lpetool stroke"));

                if (!sp_lpetool_apply(dc, motion_dt)) {
                    ret = TRUE;
                    break;
                }

                if ( dc->cur != dc->last ) {
                    sp_lpetool_brush(dc);
                    g_assert( dc->npoints > 0 );
                    fit_and_split(dc, FALSE);
                }
                ret = TRUE;
            }
            Inkscape::Rubberband::get()->move(motion_dt);
        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        NR::Point const motion_w(event->button.x, event->button.y);
        NR::Point const motion_dt(desktop->w2d(motion_w));

        sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
        sp_canvas_end_forced_full_redraws(desktop->canvas);
        dc->is_drawing = false;

        if (dc->dragging && event->button.button == 1 && !event_context->space_panning) {
            dc->dragging = FALSE;

            NR::Maybe<NR::Rect> const b = Inkscape::Rubberband::get()->getRectangle();

            sp_lpetool_apply(dc, motion_dt);
            
            // Remove all temporary line segments
            while (dc->segments) {
                gtk_object_destroy(GTK_OBJECT(dc->segments->data));
                dc->segments = g_slist_remove(dc->segments, dc->segments->data);
            }

            // Create object
            fit_and_split(dc, TRUE);
            accumulate_lpetool(dc);
            set_to_accumulated(dc); // performs document_done

            // reset accumulated curve
            dc->accumulated->reset();

            clear_current(dc);
            if (dc->repr) {
                dc->repr = NULL;
            }

            Inkscape::Rubberband::get()->stop();
            dc->_message_context->clear();
            ret = TRUE;
        }
        break;
    }

    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Up:
        case GDK_KP_Up:
            if (!MOD__CTRL_ONLY) {
                dc->angle += 5.0;
                if (dc->angle > 90.0)
                    dc->angle = 90.0;
                sp_erc_update_toolbox (desktop, "lpetool-angle", dc->angle);
                ret = TRUE;
            }
            break;
        case GDK_Down:
        case GDK_KP_Down:
            if (!MOD__CTRL_ONLY) {
                dc->angle -= 5.0;
                if (dc->angle < -90.0)
                    dc->angle = -90.0;
                sp_erc_update_toolbox (desktop, "lpetool-angle", dc->angle);
                ret = TRUE;
            }
            break;
        case GDK_Right:
        case GDK_KP_Right:
            if (!MOD__CTRL_ONLY) {
                dc->width += 0.01;
                if (dc->width > 1.0)
                    dc->width = 1.0;
                sp_erc_update_toolbox (desktop, "altx-lpetool", dc->width * 100); // the same spinbutton is for alt+x
                ret = TRUE;
            }
            break;
        case GDK_Left:
        case GDK_KP_Left:
            if (!MOD__CTRL_ONLY) {
                dc->width -= 0.01;
                if (dc->width < 0.01)
                    dc->width = 0.01;
                sp_erc_update_toolbox (desktop, "altx-lpetool", dc->width * 100);
                ret = TRUE;
            }
            break;
        case GDK_Home:
        case GDK_KP_Home:
            dc->width = 0.01;
            sp_erc_update_toolbox (desktop, "altx-lpetool", dc->width * 100);
            ret = TRUE;
            break;
        case GDK_End:
        case GDK_KP_End:
            dc->width = 1.0;
            sp_erc_update_toolbox (desktop, "altx-lpetool", dc->width * 100);
            ret = TRUE;
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-lpetool");
                ret = TRUE;
            }
            break;
        case GDK_Escape:
            Inkscape::Rubberband::get()->stop();
            if (dc->is_drawing) {
                // if drawing, cancel, otherwise pass it up for deselecting
                lpetool_cancel (dc);
                ret = TRUE;
            }
            break;
        case GDK_z:
        case GDK_Z:
            if (MOD__CTRL_ONLY && dc->is_drawing) {
                // if drawing, cancel, otherwise pass it up for undo
                lpetool_cancel (dc);
                ret = TRUE;
            }
            break;
        default:
            break;
        }
        break;

    case GDK_KEY_RELEASE:
        switch (get_group0_keyval(&event->key)) {
            case GDK_Control_L:
            case GDK_Control_R:
                dc->_message_context->clear();
                break;
            default:
                break;
        }

    default:
        break;
    }
    **/

    if (!ret) {
        if (((SPEventContextClass *) lpetool_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
