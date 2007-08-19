#define __SP_TWEAK_CONTEXT_C__

/*
 * tweaking paths without node editing
 *
 * Authors:
 *   bulia byak 
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noTWEAK_VERBOSE

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

#include <numeric>

#include "svg/svg.h"
#include "display/canvas-bpath.h"
#include "display/bezier-utils.h"

#include <glib/gmem.h>
#include "macros.h"
#include "document.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-thin.xpm"
#include "pixmaps/cursor-thicken.xpm"
#include "pixmaps/cursor-push.xpm"
#include "pixmaps/cursor-roughen.xpm"
#include "libnr/n-art-bpath.h"
#include "libnr/nr-path.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"
#include "color.h"
#include "splivarot.h"
#include "sp-item-group.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "path-chemistry.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "livarot/Shape.h"
#include "isnan.h"
#include "prefs-utils.h"

#include "tweak-context.h"

#define DDC_RED_RGBA 0xff0000ff

#define DYNA_MIN_WIDTH 1.0e-6

// FIXME: move it to some shared file to be reused by both calligraphy and dropper
#define C1 0.552
static NArtBpath const hatch_area_circle[] = {
    { NR_MOVETO, 0, 0, 0, 0, -1, 0 },
    { NR_CURVETO, -1, C1, -C1, 1, 0, 1 },
    { NR_CURVETO, C1, 1, 1, C1, 1, 0 },
    { NR_CURVETO, 1, -C1, C1, -1, 0, -1 },
    { NR_CURVETO, -C1, -1, -1, -C1, -1, 0 },
    { NR_END, 0, 0, 0, 0, 0, 0 }
};
#undef C1


static void sp_tweak_context_class_init(SPTweakContextClass *klass);
static void sp_tweak_context_init(SPTweakContext *ddc);
static void sp_tweak_context_dispose(GObject *object);

static void sp_tweak_context_setup(SPEventContext *ec);
static void sp_tweak_context_set(SPEventContext *ec, gchar const *key, gchar const *val);
static gint sp_tweak_context_root_handler(SPEventContext *ec, GdkEvent *event);

static SPEventContextClass *parent_class;

GtkType
sp_tweak_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPTweakContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_tweak_context_class_init,
            NULL, NULL,
            sizeof(SPTweakContext),
            4,
            (GInstanceInitFunc) sp_tweak_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPTweakContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_tweak_context_class_init(SPTweakContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_tweak_context_dispose;

    event_context_class->setup = sp_tweak_context_setup;
    event_context_class->set = sp_tweak_context_set;
    event_context_class->root_handler = sp_tweak_context_root_handler;
}

static void
sp_tweak_context_init(SPTweakContext *tc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(tc);

    event_context->cursor_shape = cursor_push_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;

    /* attributes */
    tc->dragging = FALSE;

    tc->width = 0.2;
    tc->force = 0.2;
    tc->pressure = TC_DEFAULT_PRESSURE;

    tc->is_dilating = false;
    tc->has_dilated = false;
}

static void
sp_tweak_context_dispose(GObject *object)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(object);

    if (tc->dilate_area) {
        gtk_object_destroy(GTK_OBJECT(tc->dilate_area));
        tc->dilate_area = NULL;
    }

    if (tc->_message_context) {
        delete tc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

void
sp_tweak_update_cursor (SPTweakContext *tc, gint mode)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(tc);
   switch (mode) {
       case TWEAK_MODE_PUSH:
           event_context->cursor_shape = cursor_push_xpm;
           break;
       case TWEAK_MODE_MELT:
           event_context->cursor_shape = cursor_thin_xpm;
           break;
       case TWEAK_MODE_INFLATE:
           event_context->cursor_shape = cursor_thicken_xpm;
           break;
       case TWEAK_MODE_ROUGHEN:
           event_context->cursor_shape = cursor_roughen_xpm;
           break;
   }
   sp_event_context_update_cursor(event_context);
}

static void
sp_tweak_context_setup(SPEventContext *ec)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup(ec);

    {
        SPCurve *c = sp_curve_new_from_foreign_bpath(hatch_area_circle);
        tc->dilate_area = sp_canvas_bpath_new(sp_desktop_controls(ec->desktop), c);
        sp_curve_unref(c);
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(tc->dilate_area), 0x00000000,(SPWindRule)0);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(tc->dilate_area), 0xff9900ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_item_hide(tc->dilate_area);
    }

    sp_event_context_read(ec, "width");
    sp_event_context_read(ec, "mode");
    sp_event_context_read(ec, "fidelity");
    sp_event_context_read(ec, "force");
    sp_event_context_read(ec, "usepressure");

    tc->is_drawing = false;

    tc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static void
sp_tweak_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(ec);

    if (!strcmp(key, "width")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.1 );
        tc->width = CLAMP(dval, -1000.0, 1000.0);
    } else if (!strcmp(key, "mode")) {
        gint64 const dval = ( val ? g_ascii_strtoll (val, NULL, 10) : 0 );
        tc->mode = dval;
        sp_tweak_update_cursor(tc, tc->mode);
    } else if (!strcmp(key, "fidelity")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 0.0 );
        tc->fidelity = CLAMP(dval, 0.0, 1.0);
    } else if (!strcmp(key, "force")) {
        double const dval = ( val ? g_ascii_strtod (val, NULL) : 1.0 );
        tc->force = CLAMP(dval, 0, 1.0);
    } else if (!strcmp(key, "usepressure")) {
        tc->usepressure = (val && strcmp(val, "0"));
    } 
}

static void
sp_tweak_extinput(SPTweakContext *tc, GdkEvent *event)
{
    if (gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &tc->pressure))
        tc->pressure = CLAMP (tc->pressure, TC_MIN_PRESSURE, TC_MAX_PRESSURE);
    else
        tc->pressure = TC_DEFAULT_PRESSURE;
}

double
get_dilate_radius (SPTweakContext *tc)
{
    // 10 times the pen width:
    return 500 * tc->width/SP_EVENT_CONTEXT(tc)->desktop->current_zoom(); 
}

double
get_dilate_force (SPTweakContext *tc)
{
    double force = 8 * (tc->usepressure? tc->pressure : TC_DEFAULT_PRESSURE)
        /sqrt(SP_EVENT_CONTEXT(tc)->desktop->current_zoom()); 
    if (force > 3) {
        force += 4 * (force - 3);
    }
    return force * tc->force;
}

bool
sp_tweak_dilate_recursive (Inkscape::Selection *selection, SPItem *item, NR::Point p, NR::Point vector, gint mode, double radius, double force, double fidelity)
{
    bool did = false;

    if (SP_IS_GROUP(item)) {
        for (SPObject *child = sp_object_first_child(SP_OBJECT(item)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_ITEM(child)) {
                if (sp_tweak_dilate_recursive (selection, SP_ITEM(child), p, vector, mode, radius, force, fidelity))
                    did = true;
            }
        }

    } else if (SP_IS_PATH(item) || SP_IS_SHAPE(item) || SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {

        Inkscape::XML::Node *newrepr = NULL;
        gint pos = 0;
        Inkscape::XML::Node *parent = NULL;
        char const *id = NULL;
        if (!SP_IS_PATH(item)) {
            newrepr = sp_selected_item_to_curved_repr(item, 0);
            if (!newrepr)
                return false;

            // remember the position of the item
            pos = SP_OBJECT_REPR(item)->position();
            // remember parent
            parent = SP_OBJECT_REPR(item)->parent();
            // remember id
            id = SP_OBJECT_REPR(item)->attribute("id");
        }


        // skip those paths whose bboxes are entirely out of reach with our radius
        NR::Maybe<NR::Rect> bbox = item->getBounds(sp_item_i2doc_affine(item));
        if (bbox) {
            bbox->growBy(radius);
            if (!bbox->contains(p)) {
                return false;
            }
        }

        Path *orig = Path_for_item(item, false);
        if (orig == NULL) {
            return false;
        }
        Path *res = new Path;
        res->SetBackData(false);

        Shape *theShape = new Shape;
        Shape *theRes = new Shape;

        orig->ConvertWithBackData(0.08 - (0.07 * fidelity)); // default 0.059
        orig->Fill(theShape, 0);

        SPCSSAttr *css = sp_repr_css_attr(SP_OBJECT_REPR(item), "style");
        gchar const *val = sp_repr_css_property(css, "fill-rule", NULL);
        if (val && strcmp(val, "nonzero") == 0)
        {
            theRes->ConvertToShape(theShape, fill_nonZero);
        }
        else if (val && strcmp(val, "evenodd") == 0)
        {
            theRes->ConvertToShape(theShape, fill_oddEven);
        }
        else
        {
            theRes->ConvertToShape(theShape, fill_nonZero);
        }

        if (NR::L2(vector) != 0)
            vector = 1/NR::L2(vector) * vector;

        bool did_this = false;
        NR::Matrix i2doc(sp_item_i2doc_affine(item));
        if (mode == TWEAK_MODE_MELT || mode == TWEAK_MODE_INFLATE) {
            if (theShape->MakeOffset(theRes, 
                                 mode == TWEAK_MODE_INFLATE? force : -force,
                                 join_straight, 4.0,
                                 true, p[NR::X], p[NR::Y], radius, &i2doc) == 0) // 0 means the shape was actually changed
              did_this = true;
        } else if (mode == TWEAK_MODE_PUSH) {
            if (theShape->MakePush(theRes, 
                                 join_straight, 4.0,
                                 true, p, force*2*vector, radius, &i2doc) == 0)
              did_this = true;
        } else if (mode == TWEAK_MODE_ROUGHEN) {
            if (theShape->MakeJitter(theRes, 
                                 join_straight, 4.0,
                                 true, p, force, radius, &i2doc) == 0)
              did_this = true;
        }

        // the rest only makes sense if we actually changed the path
        if (did_this) {
            theRes->ConvertToShape(theShape, fill_positive);

            res->Reset();
            theRes->ConvertToForme(res);

            double th_max = 0.6 - 0.59*sqrt(fidelity);
            double threshold = MAX(th_max, th_max*force);
            res->ConvertEvenLines(threshold);
            res->Simplify(threshold);

            if (newrepr) { // converting to path, need to replace the repr
                bool is_selected = selection->includes(item);
                if (is_selected)
                    selection->remove(item);

                // It's going to resurrect, so we delete without notifying listeners.
                SP_OBJECT(item)->deleteObject(false);

                // restore id
                newrepr->setAttribute("id", id);
                // add the new repr to the parent
                parent->appendChild(newrepr);
                // move to the saved position
                newrepr->setPosition(pos > 0 ? pos : 0);

                if (is_selected)
                    selection->add(newrepr);
            }

            if (res->descr_cmd.size() > 1) { 
                gchar *str = res->svg_dump_path();
                if (newrepr)
                    newrepr->setAttribute("d", str);
                else
                    SP_OBJECT_REPR(item)->setAttribute("d", str);
                g_free(str);
            } else {
                // TODO: if there's 0 or 1 node left, delete this path altogether
            }

            if (newrepr) {
                Inkscape::GC::release(newrepr);
                newrepr = NULL;
            }
        }

        delete theShape;
        delete theRes;
        delete orig;
        delete res;

        if (did_this) 
            did = true;
    }

    return did;
}


bool
sp_tweak_dilate (SPTweakContext *tc, NR::Point p, NR::Point vector)
{
    Inkscape::Selection *selection = sp_desktop_selection(SP_EVENT_CONTEXT(tc)->desktop);

    if (selection->isEmpty()) {
        return false;
    }

    bool did = false;
    double radius = get_dilate_radius(tc); 
    double force = get_dilate_force(tc); 
    if (radius == 0 || force == 0) {
        return false;
    }

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (sp_tweak_dilate_recursive (selection, item, p, vector, tc->mode, radius, force, tc->fidelity))
            did = true;

    }

    return did;
}

void
sp_tweak_update_area (SPTweakContext *tc)
{
        double radius = get_dilate_radius(tc);
        NR::Matrix const sm (NR::scale(radius, radius) * NR::translate(SP_EVENT_CONTEXT(tc)->desktop->point()));
        sp_canvas_item_affine_absolute(tc->dilate_area, sm);
        sp_canvas_item_show(tc->dilate_area);
}

void
sp_tweak_switch_mode (SPTweakContext *tc, gint mode)
{
    SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue ("tweak_tool_mode", mode);
    // need to set explicitly, because the prefs may not have changed by the previous
    tc->mode = mode;
    sp_tweak_update_cursor (tc, mode);
}

void
sp_tweak_switch_mode_temporarily (SPTweakContext *tc, gint mode)
{
   // Juggling about so that prefs have the old value but tc->mode and the button show new mode:
   gint now_mode = prefs_get_int_attribute("tools.tweak", "mode", 0);
   SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue ("tweak_tool_mode", mode);
   sp_tweak_update_cursor (tc, mode);
   // button has changed prefs, restore
   prefs_set_int_attribute("tools.tweak", "mode", now_mode);
   // changing prefs changed tc->mode, restore back :)
   tc->mode = mode;
}

gint
sp_tweak_context_root_handler(SPEventContext *event_context,
                                  GdkEvent *event)
{
    SPTweakContext *tc = SP_TWEAK_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                if (Inkscape::have_viable_layer(desktop, tc->_message_context) == false) {
                    return TRUE;
                }

                NR::Point const button_w(event->button.x,
                                         event->button.y);
                NR::Point const button_dt(desktop->w2d(button_w));
                tc->last_push = desktop->dt2doc(button_dt);

                sp_tweak_extinput(tc, event);

                sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 3);
                tc->is_drawing = true;
                tc->is_dilating = true;
                tc->has_dilated = false;

                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point motion_dt(desktop->w2d(motion_w));
            NR::Point motion_doc(desktop->dt2doc(motion_dt));
            sp_tweak_extinput(tc, event);

            // draw the dilating cursor
                double radius = get_dilate_radius(tc);
                NR::Matrix const sm (NR::scale(radius, radius) * NR::translate(desktop->w2d(motion_w)));
                sp_canvas_item_affine_absolute(tc->dilate_area, sm);
                sp_canvas_item_show(tc->dilate_area);

                guint num = 0;
                if (!desktop->selection->isEmpty()) {
                    num = g_slist_length((GSList *) desktop->selection->itemList());
                }
                if (num == 0) {
                    tc->_message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Select objects</b> to tweak"));
                } else {
                    switch (tc->mode) {
                        case TWEAK_MODE_PUSH:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      _("<b>Pushing %d</b> selected object(s)"), num);  
                           break;
                        case TWEAK_MODE_MELT:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      _("<b>Melting %d</b> selected object(s)"), num);
                           break;
                        case TWEAK_MODE_INFLATE:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      _("<b>Inflating %d</b> selected object(s)"), num);
                           break;
                        case TWEAK_MODE_ROUGHEN:
                           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE,
                                                      _("<b>Roughening %d</b> selected object(s)"), num);
                           break;
                    }
                }


            // dilating:
            if (tc->is_drawing && ( event->motion.state & GDK_BUTTON1_MASK )) {  
                sp_tweak_dilate (tc, motion_doc, motion_doc - tc->last_push);
                //tc->last_push = motion_doc;
                tc->has_dilated = true;
                // it's slow, so prevent clogging up with events
                gobble_motion_events(GDK_BUTTON1_MASK);
                return TRUE;
            }

        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        NR::Point const motion_w(event->button.x, event->button.y);
        NR::Point const motion_dt(desktop->w2d(motion_w));

        sp_canvas_end_forced_full_redraws(desktop->canvas);
        tc->is_drawing = false;

        if (tc->is_dilating && event->button.button == 1 && !event_context->space_panning) {
            if (!tc->has_dilated) {
                // if we did not rub, do a light tap
                tc->pressure = 0.03;
                sp_tweak_dilate (tc, desktop->dt2doc(motion_dt), NR::Point(0,0));
            }
            tc->is_dilating = false;
            tc->has_dilated = false;
            sp_document_done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop), 
                         SP_VERB_CONTEXT_TWEAK,
                             (tc->mode==TWEAK_MODE_INFLATE ? _("Inflate tweak") : (tc->mode==TWEAK_MODE_MELT ? _("Melt tweak") : (tc->mode==TWEAK_MODE_PUSH ? _("Push tweak") : _("Roughen tweak")))));
            ret = TRUE;

        } 
        break;
    }

    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_p:
        case GDK_P:
        case GDK_1:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_PUSH);
                ret = TRUE;
            }
            break;
        case GDK_m:
        case GDK_M:
        case GDK_2:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_MELT);
                ret = TRUE;
            }
            break;
        case GDK_b:
        case GDK_B:
        case GDK_3:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_INFLATE);
                ret = TRUE;
            }
            break;
        case GDK_r:
        case GDK_R:
        case GDK_4:
            if (MOD__SHIFT_ONLY) {
                sp_tweak_switch_mode(tc, TWEAK_MODE_ROUGHEN);
                ret = TRUE;
            }
            break;

        case GDK_Up:
        case GDK_KP_Up:
            if (!MOD__CTRL_ONLY) {
                tc->force += 0.05;
                if (tc->force > 1.0)
                    tc->force = 1.0;
                desktop->setToolboxAdjustmentValue ("tweak-force", tc->force * 100);
                ret = TRUE;
            }
            break;
        case GDK_Down:
        case GDK_KP_Down:
            if (!MOD__CTRL_ONLY) {
                tc->force -= 0.05;
                if (tc->force < 0.0)
                    tc->force = 0.0;
                desktop->setToolboxAdjustmentValue ("tweak-force", tc->force * 100);
                ret = TRUE;
            }
            break;
        case GDK_Right:
        case GDK_KP_Right:
            if (!MOD__CTRL_ONLY) {
                tc->width += 0.01;
                if (tc->width > 1.0)
                    tc->width = 1.0;
                desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100); // the same spinbutton is for alt+x
                sp_tweak_update_area(tc);
                ret = TRUE;
            }
            break;
        case GDK_Left:
        case GDK_KP_Left:
            if (!MOD__CTRL_ONLY) {
                tc->width -= 0.01;
                if (tc->width < 0.01)
                    tc->width = 0.01;
                desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100);
                sp_tweak_update_area(tc);
                ret = TRUE;
            }
            break;
        case GDK_Home:
        case GDK_KP_Home:
            tc->width = 0.01;
            desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100);
            sp_tweak_update_area(tc);
            ret = TRUE;
            break;
        case GDK_End:
        case GDK_KP_End:
            tc->width = 1.0;
            desktop->setToolboxAdjustmentValue ("altx-tweak", tc->width * 100);
            sp_tweak_update_area(tc);
            ret = TRUE;
            break;
        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-tweak");
                ret = TRUE;
            }
            break;

        case GDK_Control_L:
        case GDK_Control_R:
            if (MOD__SHIFT) {
                sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_INFLATE);
            } else {
                sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_MELT);
            }
            break;
        case GDK_Shift_L:
        case GDK_Shift_R:
            if (MOD__CTRL) {
                sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_INFLATE);
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
                sp_tweak_switch_mode (tc, prefs_get_int_attribute("tools.tweak", "mode", 0));
                tc->_message_context->clear();
                break;
            case GDK_Shift_L:
            case GDK_Shift_R:
                if (MOD__CTRL) {
                    sp_tweak_switch_mode_temporarily(tc, TWEAK_MODE_MELT);
                }
                break;
            break;
            default:
                sp_tweak_switch_mode (tc, prefs_get_int_attribute("tools.tweak", "mode", 0));
                break;
        }

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
