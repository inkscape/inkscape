/*
 * Spray Tool
 *
 * Authors:
 *   Pierre-Antoine MARC
 *   Pierre CACLIN
 *   Aurel-Aimé MARMION
 *   Julien LERAY
 *   Benoît LAVORATA
 *   Vincent MONTAGNE
 *   Pierre BARBRY-BLOT
 *   Steren GIANNINI (steren.giannini@gmail.com)
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2009 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <numeric>

#include "ui/dialog/dialog-manager.h"

#include "svg/svg.h"

#include <glib.h>
#include "macros.h"
#include "document.h"
#include "document-undo.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-events.h"
#include "desktop-handles.h"
#include "message-context.h"
#include "pixmaps/cursor-spray.xpm"
#include <boost/optional.hpp>
#include "xml/repr.h"
#include "context-fns.h"
#include "sp-item.h"
#include "inkscape.h"

#include "splivarot.h"
#include "sp-item-group.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "path-chemistry.h"

#include "sp-text.h"
#include "sp-flowtext.h"
#include "display/sp-canvas.h"
#include "display/canvas-bpath.h"
#include "display/canvas-arena.h"
#include "display/curve.h"
#include "livarot/Shape.h"
#include <2geom/transforms.h>
#include "preferences.h"
#include "style.h"
#include "box3d.h"
#include "sp-item-transform.h"
#include "filter-chemistry.h"

#include "spray-context.h"
#include "helper/action.h"
#include "verbs.h"

#include <iostream>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

#if !GTK_CHECK_VERSION(2,22,0)
#include "compat-key-syms.h"
#endif

using Inkscape::DocumentUndo;
using namespace std;

#define DDC_RED_RGBA 0xff0000ff
#define DYNA_MIN_WIDTH 1.0e-6

static void sp_spray_context_class_init(SPSprayContextClass *klass);
static void sp_spray_context_init(SPSprayContext *ddc);
static void sp_spray_context_dispose(GObject *object);

static void sp_spray_context_setup(SPEventContext *ec);
static void sp_spray_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val);
static gint sp_spray_context_root_handler(SPEventContext *ec, GdkEvent *event);

static SPEventContextClass *parent_class = 0;


/**
 * This function returns pseudo-random numbers from a normal distribution
 * @param mu : mean
 * @param sigma : standard deviation ( > 0 )
 */
inline double NormalDistribution(double mu, double sigma)
{
  // use Box Muller's algorithm
  return mu + sigma * sqrt( -2.0 * log(g_random_double_range(0, 1)) ) * cos( 2.0*M_PI*g_random_double_range(0, 1) );
}

GType sp_spray_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPSprayContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_spray_context_class_init,
            NULL, NULL,
            sizeof(SPSprayContext),
            4,
            (GInstanceInitFunc) sp_spray_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPSprayContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void sp_spray_context_class_init(SPSprayContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_spray_context_dispose;

    event_context_class->setup = sp_spray_context_setup;
    event_context_class->set = sp_spray_context_set;
    event_context_class->root_handler = sp_spray_context_root_handler;
}

/* Method to rotate items */
void sp_spray_rotate_rel(Geom::Point c, SPDesktop */*desktop*/, SPItem *item, Geom::Rotate const &rotation)
{
    Geom::Translate const s(c);
    Geom::Affine affine = Geom::Affine(s).inverse() * Geom::Affine(rotation) * Geom::Affine(s);
    // Rotate item.
    item->set_i2d_affine(item->i2dt_affine() * (Geom::Affine)affine);
    // Use each item's own transform writer, consistent with sp_selection_apply_affine()
    item->doWriteTransform(item->getRepr(), item->transform);
    // Restore the center position (it's changed because the bbox center changed)
    if (item->isCenterSet()) {
        item->setCenter(c);
        item->updateRepr();
    }
}

/* Method to scale items */
void sp_spray_scale_rel(Geom::Point c, SPDesktop */*desktop*/, SPItem *item, Geom::Scale const &scale)
{
    Geom::Translate const s(c);
    item->set_i2d_affine(item->i2dt_affine() * s.inverse() * scale * s);
    item->doWriteTransform(item->getRepr(), item->transform);
}

static void sp_spray_context_init(SPSprayContext *tc)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(tc);

    event_context->cursor_shape = cursor_spray_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;

    /* attributes */
    tc->dragging = FALSE;
    tc->distrib = 1;
    tc->width = 0.2;
    tc->force = 0.2;
    tc->ratio = 0;
    tc->tilt = 0;
    tc->mean = 0.2;
    tc->rotation_variation = 0;
    tc->standard_deviation = 0.2;
    tc->scale = 1;
    tc->scale_variation = 1;
    tc->pressure = TC_DEFAULT_PRESSURE;

    tc->is_dilating = false;
    tc->has_dilated = false;

    new (&tc->style_set_connection) sigc::connection();
}

static void sp_spray_context_dispose(GObject *object)
{
    SPSprayContext *tc = SP_SPRAY_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    tc->style_set_connection.disconnect();
    tc->style_set_connection.~connection();

    if (tc->dilate_area) {
        gtk_object_destroy(GTK_OBJECT(tc->dilate_area));
        tc->dilate_area = NULL;
    }

    if (tc->_message_context) {
        delete tc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

bool is_transform_modes(gint mode)
{
    return (mode == SPRAY_MODE_COPY ||
            mode == SPRAY_MODE_CLONE ||
            mode == SPRAY_MODE_SINGLE_PATH ||
            mode == SPRAY_OPTION);
}

void sp_spray_update_cursor(SPSprayContext *tc, bool /*with_shift*/)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(tc);
    SPDesktop *desktop = event_context->desktop;

    guint num = 0;
    gchar *sel_message = NULL;
    if (!desktop->selection->isEmpty()) {
        num = g_slist_length((GSList *) desktop->selection->itemList());
        sel_message = g_strdup_printf(ngettext("<b>%i</b> object selected","<b>%i</b> objects selected",num), num);
    } else {
        sel_message = g_strdup_printf(_("<b>Nothing</b> selected"));
    }


   switch (tc->mode) {
       case SPRAY_MODE_COPY:
           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag, click or scroll to spray <b>copies</b> of the initial selection."), sel_message);
           break;
       case SPRAY_MODE_CLONE:
           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag, click or scroll to spray <b>clones</b> of the initial selection."), sel_message);
           break;
       case SPRAY_MODE_SINGLE_PATH:
           tc->_message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag, click or scroll to spray in a <b>single path</b> of the initial selection."), sel_message);
           break;
       default:
           break;
   }
   sp_event_context_update_cursor(event_context);
   g_free(sel_message);
}

static void sp_spray_context_setup(SPEventContext *ec)
{
    SPSprayContext *tc = SP_SPRAY_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    {
        /* TODO: have a look at sp_dyna_draw_context_setup where the same is done.. generalize? at least make it an arcto! */
        SPCurve *c = new SPCurve();
        const double C1 = 0.552;
        c->moveto(-1,0);
        c->curveto(-1, C1, -C1, 1, 0, 1 );
        c->curveto(C1, 1, 1, C1, 1, 0 );
        c->curveto(1, -C1, C1, -1, 0, -1 );
        c->curveto(-C1, -1, -1, -C1, -1, 0 );
        c->closepath();
        tc->dilate_area = sp_canvas_bpath_new(sp_desktop_controls(ec->desktop), c);
        c->unref();
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(tc->dilate_area), 0x00000000,(SPWindRule)0);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(tc->dilate_area), 0xff9900ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_item_hide(tc->dilate_area);
    }

    tc->is_drawing = false;

    tc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    sp_event_context_read(ec, "distrib");
    sp_event_context_read(ec, "width");
    sp_event_context_read(ec, "ratio");
    sp_event_context_read(ec, "tilt");
    sp_event_context_read(ec, "rotation_variation");
    sp_event_context_read(ec, "scale_variation");
    sp_event_context_read(ec, "mode");
    sp_event_context_read(ec, "population");
    sp_event_context_read(ec, "force");
    sp_event_context_read(ec, "mean");
    sp_event_context_read(ec, "standard_deviation");
    sp_event_context_read(ec, "usepressure");
    sp_event_context_read(ec, "Scale");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/spray/selcue")) {
        ec->enableSelectionCue();
    }
    if (prefs->getBool("/tools/spray/gradientdrag")) {
        ec->enableGrDrag();
    }
}

static void sp_spray_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    SPSprayContext *tc = SP_SPRAY_CONTEXT(ec);
    Glib::ustring path = val->getEntryName();

    if (path == "mode") {
        tc->mode = val->getInt();
        sp_spray_update_cursor(tc, false);
    } else if (path == "width") {
        tc->width = 0.01 * CLAMP(val->getInt(10), 1, 100);
    } else if (path == "usepressure") {
        tc->usepressure = val->getBool();
    } else if (path == "population") {
        tc->population = 0.01 * CLAMP(val->getInt(10), 1, 100);
    } else if (path == "rotation_variation") {
        tc->rotation_variation = CLAMP(val->getDouble(0.0), 0, 100.0);
    } else if (path == "scale_variation") {
        tc->scale_variation = CLAMP(val->getDouble(1.0), 0, 100.0);
    } else if (path == "standard_deviation") {
        tc->standard_deviation = 0.01 * CLAMP(val->getInt(10), 1, 100);
    } else if (path == "mean") {
        tc->mean = 0.01 * CLAMP(val->getInt(10), 1, 100);
// Not implemented in the toolbar and preferences yet
    } else if (path == "distribution") {
        tc->distrib = val->getInt(1);
    } else if (path == "tilt") {
        tc->tilt = CLAMP(val->getDouble(0.1), 0, 1000.0);
    } else if (path == "ratio") {
        tc->ratio = CLAMP(val->getDouble(), 0.0, 0.9);
    } else if (path == "force") {
        tc->force = CLAMP(val->getDouble(1.0), 0, 1.0);
    } 
}

static void sp_spray_extinput(SPSprayContext *tc, GdkEvent *event)
{
    if (gdk_event_get_axis(event, GDK_AXIS_PRESSURE, &tc->pressure)) {
        tc->pressure = CLAMP(tc->pressure, TC_MIN_PRESSURE, TC_MAX_PRESSURE);
    } else {
        tc->pressure = TC_DEFAULT_PRESSURE;
    }
}

double get_dilate_radius(SPSprayContext *tc)
{
    return 250 * tc->width/SP_EVENT_CONTEXT(tc)->desktop->current_zoom();
}

double get_path_force(SPSprayContext *tc)
{
    double force = 8 * (tc->usepressure? tc->pressure : TC_DEFAULT_PRESSURE)
        /sqrt(SP_EVENT_CONTEXT(tc)->desktop->current_zoom());
    if (force > 3) {
        force += 4 * (force - 3);
    }
    return force * tc->force;
}

double get_path_mean(SPSprayContext *tc)
{
    return tc->mean;
}

double get_path_standard_deviation(SPSprayContext *tc)
{
    return tc->standard_deviation;
}

double get_move_force(SPSprayContext *tc)
{
    double force = (tc->usepressure? tc->pressure : TC_DEFAULT_PRESSURE);
    return force * tc->force;
}

double get_move_mean(SPSprayContext *tc)
{
    return tc->mean;
}

double get_move_standard_deviation(SPSprayContext *tc)
{
    return tc->standard_deviation;
}

/**
 * Method to handle the distribution of the items
 * @param[out]  radius : radius of the position of the sprayed object
 * @param[out]  angle : angle of the position of the sprayed object
 * @param[in]   a : mean
 * @param[in]   s : standard deviation
 * @param[in]   choice : 

 */
void random_position(double &radius, double &angle, double &a, double &s, int /*choice*/)
{
    // angle is taken from an uniform distribution
    angle = g_random_double_range(0, M_PI*2.0);

    // radius is taken from a Normal Distribution
    double radius_temp =-1;
    while(!((radius_temp >= 0) && (radius_temp <=1 )))
    {
        radius_temp = NormalDistribution(a, s);
    }
    // Because we are in polar coordinates, a special treatment has to be done to the radius.
    // Otherwise, positions taken from an uniform repartition on radius and angle will not seam to 
    // be uniformily distributed on the disk (more at the center and less at the boundary).
    // We counter this effect with a 0.5 exponent. This is empiric.
    radius = pow(radius_temp, 0.5);

}

bool sp_spray_recursive(SPDesktop *desktop,
                               Inkscape::Selection *selection,
                               SPItem *item,
                               Geom::Point p,
                               Geom::Point /*vector*/,
                               gint mode,
                               double radius,
                               double /*force*/,
                               double population,
                               double &scale,
                               double scale_variation,
                               bool /*reverse*/,
                               double mean,
                               double standard_deviation,
                               double ratio,
                               double tilt,
                               double rotation_variation,
                               gint _distrib)
{
    bool did = false;
    
    if (SP_IS_BOX3D(item) ) {
        // convert 3D boxes to ordinary groups before spraying their shapes
        item = box3d_convert_to_group(SP_BOX3D(item));
        selection->add(item);
    }

    double _fid = g_random_double_range(0, 1);
    double angle = g_random_double_range( - rotation_variation / 100.0 * M_PI , rotation_variation / 100.0 * M_PI );
    double _scale = g_random_double_range( 1.0 - scale_variation / 100.0, 1.0 + scale_variation / 100.0 );
    double dr; double dp;
    random_position( dr, dp, mean, standard_deviation, _distrib );
    dr=dr*radius;

    if (mode == SPRAY_MODE_COPY) {
        Geom::OptRect a = item->documentVisualBounds();
        if (a) {
            SPItem *item_copied;
            if(_fid <= population)
            {
                // duplicate
                SPDocument *doc = item->document;
                Inkscape::XML::Document* xml_doc = doc->getReprDoc();
                Inkscape::XML::Node *old_repr = item->getRepr();
                Inkscape::XML::Node *parent = old_repr->parent();
                Inkscape::XML::Node *copy = old_repr->duplicate(xml_doc);
                parent->appendChild(copy);

                SPObject *new_obj = doc->getObjectByRepr(copy);
                item_copied = (SPItem *) new_obj;   //convertion object->item
                Geom::Point center=item->getCenter();
                sp_spray_scale_rel(center,desktop, item_copied, Geom::Scale(_scale,_scale));
                sp_spray_scale_rel(center,desktop, item_copied, Geom::Scale(scale,scale));

                sp_spray_rotate_rel(center,desktop,item_copied, Geom::Rotate(angle));
                //Move the cursor p
                Geom::Point move = (Geom::Point(cos(tilt)*cos(dp)*dr/(1-ratio)+sin(tilt)*sin(dp)*dr/(1+ratio), -sin(tilt)*cos(dp)*dr/(1-ratio)+cos(tilt)*sin(dp)*dr/(1+ratio)))+(p-a->midpoint());
                sp_item_move_rel(item_copied, Geom::Translate(move[Geom::X], -move[Geom::Y]));
                did = true;
            }
        }
    } else if (mode == SPRAY_MODE_SINGLE_PATH) {

        SPItem *father;         //initial Object
        SPItem *item_copied;    //Projected Object
        SPItem *unionResult;    //previous union
        SPItem *son;            //father copy

        int i=1;
        for (GSList *items = g_slist_copy((GSList *) selection->itemList());
                items != NULL;
                items = items->next) {

            SPItem *item1 = (SPItem *) items->data;
            if (i == 1) {
                father = item1;
            }
            if (i == 2) {
                unionResult = item1;
            }
            i++;
        }
        SPDocument *doc = father->document;
        Inkscape::XML::Document* xml_doc = doc->getReprDoc();
        Inkscape::XML::Node *old_repr = father->getRepr();
        Inkscape::XML::Node *parent = old_repr->parent();

        Geom::OptRect a = father->documentVisualBounds();
        if (a) {
            if (i == 2) {
                Inkscape::XML::Node *copy1 = old_repr->duplicate(xml_doc);
                parent->appendChild(copy1);
                SPObject *new_obj1 = doc->getObjectByRepr(copy1);
                son = (SPItem *) new_obj1;   // conversion object->item
                unionResult = son;
                Inkscape::GC::release(copy1);
            }

            if (_fid <= population) { // Rules the population of objects sprayed
                // duplicates the father
                Inkscape::XML::Node *copy2 = old_repr->duplicate(xml_doc);
                parent->appendChild(copy2);
                SPObject *new_obj2 = doc->getObjectByRepr(copy2);
                item_copied = (SPItem *) new_obj2;

                // Move around the cursor
                Geom::Point move = (Geom::Point(cos(tilt)*cos(dp)*dr/(1-ratio)+sin(tilt)*sin(dp)*dr/(1+ratio), -sin(tilt)*cos(dp)*dr/(1-ratio)+cos(tilt)*sin(dp)*dr/(1+ratio)))+(p-a->midpoint()); 

                Geom::Point center=father->getCenter();
                sp_spray_scale_rel(center, desktop, item_copied, Geom::Scale(_scale, _scale));
                sp_spray_scale_rel(center, desktop, item_copied, Geom::Scale(scale, scale));
                sp_spray_rotate_rel(center, desktop, item_copied, Geom::Rotate(angle));
                sp_item_move_rel(item_copied, Geom::Translate(move[Geom::X], -move[Geom::Y]));

                // union and duplication
                selection->clear();
                selection->add(item_copied);
                selection->add(unionResult);
                sp_selected_path_union_skip_undo(selection->desktop());
                selection->add(father);
                Inkscape::GC::release(copy2);
                did = true;
            }
        }
    } else if (mode == SPRAY_MODE_CLONE) {
        Geom::OptRect a = item->documentVisualBounds();
        if (a) {
            if(_fid <= population) {
                SPItem *item_copied;
                SPDocument *doc = item->document;
                Inkscape::XML::Document* xml_doc = doc->getReprDoc();
                Inkscape::XML::Node *old_repr = item->getRepr();
                Inkscape::XML::Node *parent = old_repr->parent();

                // Creation of the clone
                Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");
                // Ad the clone to the list of the father's sons
                parent->appendChild(clone);
                // Generates the link between father and son attributes
                clone->setAttribute("xlink:href", g_strdup_printf("#%s", old_repr->attribute("id")), false); 

                SPObject *clone_object = doc->getObjectByRepr(clone);
                // conversion object->item
                item_copied = (SPItem *) clone_object;
                Geom::Point center = item->getCenter();
                sp_spray_scale_rel(center, desktop, item_copied, Geom::Scale(_scale, _scale));
                sp_spray_scale_rel(center, desktop, item_copied, Geom::Scale(scale, scale));
                sp_spray_rotate_rel(center, desktop, item_copied, Geom::Rotate(angle));
                Geom::Point move = (Geom::Point(cos(tilt)*cos(dp)*dr/(1-ratio)+sin(tilt)*sin(dp)*dr/(1+ratio), -sin(tilt)*cos(dp)*dr/(1-ratio)+cos(tilt)*sin(dp)*dr/(1+ratio)))+(p-a->midpoint());
                sp_item_move_rel(item_copied, Geom::Translate(move[Geom::X], -move[Geom::Y]));

                Inkscape::GC::release(clone);

                did = true;
            }
        }
    }

    return did;
}

bool sp_spray_dilate(SPSprayContext *tc, Geom::Point /*event_p*/, Geom::Point p, Geom::Point vector, bool reverse)
{
    Inkscape::Selection *selection = sp_desktop_selection(SP_EVENT_CONTEXT(tc)->desktop);
    SPDesktop *desktop = SP_EVENT_CONTEXT(tc)->desktop;

    if (selection->isEmpty()) {
        return false;
    }

    bool did = false;
    double radius = get_dilate_radius(tc);
    double path_force = get_path_force(tc);
    if (radius == 0 || path_force == 0) {
        return false;
    }
    double path_mean = get_path_mean(tc);
    if (radius == 0 || path_mean == 0) {
        return false;
    }
    double path_standard_deviation = get_path_standard_deviation(tc);
    if (radius == 0 || path_standard_deviation == 0) {
        return false;
    }
    double move_force = get_move_force(tc);
    double move_mean = get_move_mean(tc);
    double move_standard_deviation = get_move_standard_deviation(tc);

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (is_transform_modes(tc->mode)) {
            if (sp_spray_recursive(desktop, selection, item, p, vector, tc->mode, radius, move_force, tc->population, tc->scale, tc->scale_variation, reverse, move_mean, move_standard_deviation, tc->ratio, tc->tilt, tc->rotation_variation, tc->distrib))
                did = true;
        } else {
            if (sp_spray_recursive(desktop, selection, item, p, vector, tc->mode, radius, path_force, tc->population, tc->scale, tc->scale_variation, reverse, path_mean, path_standard_deviation, tc->ratio, tc->tilt, tc->rotation_variation, tc->distrib))
                did = true;
        }
    }

    return did;
}

void sp_spray_update_area(SPSprayContext *tc)
{
    double radius = get_dilate_radius(tc);
    Geom::Affine const sm ( Geom::Scale(radius/(1-tc->ratio), radius/(1+tc->ratio)) );
    sp_canvas_item_affine_absolute(tc->dilate_area, (sm* Geom::Rotate(tc->tilt))* Geom::Translate(SP_EVENT_CONTEXT(tc)->desktop->point()));
    sp_canvas_item_show(tc->dilate_area);
}

void sp_spray_switch_mode(SPSprayContext *tc, gint mode, bool with_shift)
{
    // select the button mode
    SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue("spray_tool_mode", mode); 
    // need to set explicitly, because the prefs may not have changed by the previous
    tc->mode = mode;
    sp_spray_update_cursor(tc, with_shift);
}

void sp_spray_switch_mode_temporarily(SPSprayContext *tc, gint mode, bool with_shift)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    // Juggling about so that prefs have the old value but tc->mode and the button show new mode:
    gint now_mode = prefs->getInt("/tools/spray/mode", 0);
    SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue("spray_tool_mode", mode);
    // button has changed prefs, restore
    prefs->setInt("/tools/spray/mode", now_mode);
    // changing prefs changed tc->mode, restore back :)
    tc->mode = mode;
    sp_spray_update_cursor(tc, with_shift);
}

gint sp_spray_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPSprayContext *tc = SP_SPRAY_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            sp_canvas_item_show(tc->dilate_area);
            break;
        case GDK_LEAVE_NOTIFY:
            sp_canvas_item_hide(tc->dilate_area);
            break;
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
                if (Inkscape::have_viable_layer(desktop, tc->_message_context) == false) {
                    return TRUE;
                }

                Geom::Point const motion_w(event->button.x, event->button.y);
                Geom::Point const motion_dt(desktop->w2d(motion_w));
                tc->last_push = desktop->dt2doc(motion_dt);

                sp_spray_extinput(tc, event);

                desktop->canvas->forceFullRedrawAfterInterruptions(3);
                tc->is_drawing = true;
                tc->is_dilating = true;
                tc->has_dilated = false;

                if(tc->is_dilating && event->button.button == 1 && !event_context->space_panning) {
                    sp_spray_dilate(tc, motion_w, desktop->dt2doc(motion_dt), Geom::Point(0,0), MOD__SHIFT);
                }

                tc->has_dilated = true;
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY: {
            Geom::Point const motion_w(event->motion.x,
                                     event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));
            Geom::Point motion_doc(desktop->dt2doc(motion_dt));
            sp_spray_extinput(tc, event);

            // draw the dilating cursor
            double radius = get_dilate_radius(tc);
            Geom::Affine const sm (Geom::Scale(radius/(1-tc->ratio), radius/(1+tc->ratio)) );
            sp_canvas_item_affine_absolute(tc->dilate_area, (sm*Geom::Rotate(tc->tilt))*Geom::Translate(desktop->w2d(motion_w)));
            sp_canvas_item_show(tc->dilate_area);

            guint num = 0;
            if (!desktop->selection->isEmpty()) {
                num = g_slist_length((GSList *) desktop->selection->itemList());
            }
            if (num == 0) {
                tc->_message_context->flash(Inkscape::ERROR_MESSAGE, _("<b>Nothing selected!</b> Select objects to spray."));
            }

            // dilating:
            if (tc->is_drawing && ( event->motion.state & GDK_BUTTON1_MASK )) {
                sp_spray_dilate(tc, motion_w, motion_doc, motion_doc - tc->last_push, event->button.state & GDK_SHIFT_MASK? true : false);
                //tc->last_push = motion_doc;
                tc->has_dilated = true;

                // it's slow, so prevent clogging up with events
                gobble_motion_events(GDK_BUTTON1_MASK);
                return TRUE;
            }
        }
        break;
        /*Spray with the scroll*/
        case GDK_SCROLL: {
            if (event->scroll.state & GDK_BUTTON1_MASK) {
                double temp ;
                temp = tc->population;
                tc->population = 1.0;
                desktop->setToolboxAdjustmentValue("population", tc->population * 100);
                Geom::Point const scroll_w(event->button.x, event->button.y);
                Geom::Point const scroll_dt = desktop->point();;
                Geom::Point motion_doc(desktop->dt2doc(scroll_dt));
                switch (event->scroll.direction) {
                    case GDK_SCROLL_DOWN:
                    case GDK_SCROLL_UP: {
                        if (Inkscape::have_viable_layer(desktop, tc->_message_context) == false) {
                            return TRUE;
                        }
                        tc->last_push = desktop->dt2doc(scroll_dt);
                        sp_spray_extinput(tc, event);
                        desktop->canvas->forceFullRedrawAfterInterruptions(3);
                        tc->is_drawing = true;
                        tc->is_dilating = true;
                        tc->has_dilated = false;
                        if(tc->is_dilating && !event_context->space_panning) {
                            sp_spray_dilate(tc, scroll_w, desktop->dt2doc(scroll_dt), Geom::Point(0,0), false);
                        }
                        tc->has_dilated = true;
                        
                        tc->population = temp;
                        desktop->setToolboxAdjustmentValue("population", tc->population * 100);

                        ret = TRUE;
                    }
                    break;
                    case GDK_SCROLL_RIGHT:
                       {} break;
                    case GDK_SCROLL_LEFT:
                       {} break;
                }
            }
            break;
        }
        
        case GDK_BUTTON_RELEASE: {
            Geom::Point const motion_w(event->button.x, event->button.y);
            Geom::Point const motion_dt(desktop->w2d(motion_w));

            desktop->canvas->endForcedFullRedraws();
            tc->is_drawing = false;

            if (tc->is_dilating && event->button.button == 1 && !event_context->space_panning) {
                if (!tc->has_dilated) {
                    // if we did not rub, do a light tap
                    tc->pressure = 0.03;
                    sp_spray_dilate(tc, motion_w, desktop->dt2doc(motion_dt), Geom::Point(0,0), MOD__SHIFT);
                }
                tc->is_dilating = false;
                tc->has_dilated = false;
                switch (tc->mode) {
                    case SPRAY_MODE_COPY:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                           SP_VERB_CONTEXT_SPRAY, _("Spray with copies"));
                        break;
                    case SPRAY_MODE_CLONE:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                           SP_VERB_CONTEXT_SPRAY, _("Spray with clones"));
                        break;
                    case SPRAY_MODE_SINGLE_PATH:
                        DocumentUndo::done(sp_desktop_document(SP_EVENT_CONTEXT(tc)->desktop),
                                           SP_VERB_CONTEXT_SPRAY, _("Spray in single path"));
                        break;
                }
            }
            break;
        }

        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_KEY_j:
                case GDK_KEY_J:
                    if (MOD__SHIFT_ONLY) {
                        sp_spray_switch_mode(tc, SPRAY_MODE_COPY, MOD__SHIFT);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_k:
                case GDK_KEY_K:
                    if (MOD__SHIFT_ONLY) {
                        sp_spray_switch_mode(tc, SPRAY_MODE_CLONE, MOD__SHIFT);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_l:
                case GDK_KEY_L:
                    if (MOD__SHIFT_ONLY) {
                        sp_spray_switch_mode(tc, SPRAY_MODE_SINGLE_PATH, MOD__SHIFT);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Up:
                case GDK_KEY_KP_Up:
                    if (!MOD__CTRL_ONLY) {
                        tc->population += 0.01;
                        if (tc->population > 1.0) {
                            tc->population = 1.0;
                        }
                        desktop->setToolboxAdjustmentValue("spray-population", tc->population * 100);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Down:
                case GDK_KEY_KP_Down:
                    if (!MOD__CTRL_ONLY) {
                        tc->population -= 0.01;
                        if (tc->population < 0.0) {
                            tc->population = 0.0;
                        }
                        desktop->setToolboxAdjustmentValue("spray-population", tc->population * 100);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Right:
                case GDK_KEY_KP_Right:
                    if (!MOD__CTRL_ONLY) {
                        tc->width += 0.01;
                        if (tc->width > 1.0) {
                            tc->width = 1.0;
                        }
                        // the same spinbutton is for alt+x
                        desktop->setToolboxAdjustmentValue("altx-spray", tc->width * 100);
                        sp_spray_update_area(tc);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Left:
                case GDK_KEY_KP_Left:
                    if (!MOD__CTRL_ONLY) {
                        tc->width -= 0.01;
                        if (tc->width < 0.01) {
                            tc->width = 0.01;
                        }
                        desktop->setToolboxAdjustmentValue("altx-spray", tc->width * 100);
                        sp_spray_update_area(tc);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Home:
                case GDK_KEY_KP_Home:
                    tc->width = 0.01;
                    desktop->setToolboxAdjustmentValue("altx-spray", tc->width * 100);
                    sp_spray_update_area(tc);
                    ret = TRUE;
                    break;
                case GDK_KEY_End:
                case GDK_KEY_KP_End:
                    tc->width = 1.0;
                    desktop->setToolboxAdjustmentValue("altx-spray", tc->width * 100);
                    sp_spray_update_area(tc);
                    ret = TRUE;
                    break;
                case GDK_KEY_x:
                case GDK_KEY_X:
                    if (MOD__ALT_ONLY) {
                        desktop->setToolboxFocusTo("altx-spray");
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    sp_spray_update_cursor(tc, true);
                    break;
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                    break;
                default:
                    break;
            }
            break;

        case GDK_KEY_RELEASE: {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            switch (get_group0_keyval(&event->key)) {
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    sp_spray_update_cursor(tc, false);
                    break;
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                    sp_spray_switch_mode (tc, prefs->getInt("/tools/spray/mode"), MOD__SHIFT);
                    tc->_message_context->clear();
                    break;
                default:
                    sp_spray_switch_mode (tc, prefs->getInt("/tools/spray/mode"), MOD__SHIFT);
                    break;
            }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

