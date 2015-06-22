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
#include <2geom/circle.h>
#include <2geom/transforms.h>
#include "preferences.h"
#include "style.h"
#include "box3d.h"
#include "sp-item-transform.h"
#include "filter-chemistry.h"

#include "ui/tools/spray-tool.h"
#include "helper/action.h"
#include "verbs.h"

#include <iostream>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

using Inkscape::DocumentUndo;
using namespace std;

#define DDC_RED_RGBA 0xff0000ff
#define DYNA_MIN_WIDTH 1.0e-6

// Disabled in 0.91 because of Bug #1274831 (crash, spraying an object 
// with the mode: spray object in single path)
// Please enable again when working on 1.0
#define ENABLE_SPRAY_MODE_SINGLE_PATH

namespace Inkscape {
namespace UI {
namespace Tools {

const std::string& SprayTool::getPrefsPath() {
    return SprayTool::prefsPath;
}

const std::string SprayTool::prefsPath = "/tools/spray";

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

/* Method to rotate items */
static void sp_spray_rotate_rel(Geom::Point c, SPDesktop */*desktop*/, SPItem *item, Geom::Rotate const &rotation)
{
    Geom::Translate const s(c);
    Geom::Affine affine = s.inverse() * rotation * s;
    // Rotate item.
    item->set_i2d_affine(item->i2dt_affine() * affine);
    // Use each item's own transform writer, consistent with sp_selection_apply_affine()
    item->doWriteTransform(item->getRepr(), item->transform);
    // Restore the center position (it's changed because the bbox center changed)
    if (item->isCenterSet()) {
        item->setCenter(c);
        item->updateRepr();
    }
}

/* Method to scale items */
static void sp_spray_scale_rel(Geom::Point c, SPDesktop */*desktop*/, SPItem *item, Geom::Scale const &scale)
{
    Geom::Translate const s(c);
    item->set_i2d_affine(item->i2dt_affine() * s.inverse() * scale * s);
    item->doWriteTransform(item->getRepr(), item->transform);
}

SprayTool::SprayTool()
    : ToolBase(cursor_spray_xpm, 4, 4, false)
    , pressure(TC_DEFAULT_PRESSURE)
    , dragging(false)
    , usepressure(false)
    , usetilt(false)
    , usetext(false)
    , width(0.2)
    , ratio(0)
    , tilt(0)
    , rotation_variation(0)
    , population(0)
    , scale_variation(1)
    , scale(1)
    , mean(0.2)
    , standard_deviation(0.2)
    , distrib(1)
    , mode(0)
    , is_drawing(false)
    , is_dilating(false)
    , has_dilated(false)
    , dilate_area(NULL)
{
}

SprayTool::~SprayTool() {
    this->enableGrDrag(false);
    this->style_set_connection.disconnect();

    if (this->dilate_area) {
        sp_canvas_item_destroy(this->dilate_area);
        this->dilate_area = NULL;
    }
}

void SprayTool::update_cursor(bool /*with_shift*/) {
    guint num = 0;
    gchar *sel_message = NULL;

    if (!desktop->selection->isEmpty()) {
        num = desktop->selection->itemList().size();
        sel_message = g_strdup_printf(ngettext("<b>%i</b> object selected","<b>%i</b> objects selected",num), num);
    } else {
        sel_message = g_strdup_printf("%s", _("<b>Nothing</b> selected"));
    }

    switch (this->mode) {
        case SPRAY_MODE_COPY:
            this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag, click or click and scroll to spray <b>copies</b> of the initial selection."), sel_message);
            break;
        case SPRAY_MODE_CLONE:
            this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag, click or click and scroll to spray <b>clones</b> of the initial selection."), sel_message);
            break;
        case SPRAY_MODE_SINGLE_PATH:
            this->message_context->setF(Inkscape::NORMAL_MESSAGE, _("%s. Drag, click or click and scroll to spray in a <b>single path</b> of the initial selection."), sel_message);
            break;
        default:
            break;
    }

    this->sp_event_context_update_cursor();
    g_free(sel_message);
}

void SprayTool::setup() {
    ToolBase::setup();

    {
        /* TODO: have a look at sp_dyna_draw_context_setup where the same is done.. generalize? at least make it an arcto! */
        Geom::PathVector path;
        Geom::Circle(0, 0, 1).getPath(path);

        SPCurve *c = new SPCurve(path);

        this->dilate_area = sp_canvas_bpath_new(this->desktop->getControls(), c);
        c->unref();
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(this->dilate_area), 0x00000000,(SPWindRule)0);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(this->dilate_area), 0xff9900ff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_item_hide(this->dilate_area);
    }

    this->is_drawing = false;

    sp_event_context_read(this, "distrib");
    sp_event_context_read(this, "width");
    sp_event_context_read(this, "ratio");
    sp_event_context_read(this, "tilt");
    sp_event_context_read(this, "rotation_variation");
    sp_event_context_read(this, "scale_variation");
    sp_event_context_read(this, "mode");
    sp_event_context_read(this, "population");
    sp_event_context_read(this, "mean");
    sp_event_context_read(this, "standard_deviation");
    sp_event_context_read(this, "usepressure");
    sp_event_context_read(this, "Scale");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/spray/selcue")) {
        this->enableSelectionCue();
    }
    if (prefs->getBool("/tools/spray/gradientdrag")) {
        this->enableGrDrag();
    }
}

void SprayTool::set(const Inkscape::Preferences::Entry& val) {
    Glib::ustring path = val.getEntryName();

    if (path == "mode") {
        this->mode = val.getInt();
        this->update_cursor(false);
    } else if (path == "width") {
        this->width = 0.01 * CLAMP(val.getInt(10), 1, 100);
    } else if (path == "usepressure") {
        this->usepressure = val.getBool();
    } else if (path == "population") {
        this->population = 0.01 * CLAMP(val.getInt(10), 1, 100);
    } else if (path == "rotation_variation") {
        this->rotation_variation = CLAMP(val.getDouble(0.0), 0, 100.0);
    } else if (path == "scale_variation") {
        this->scale_variation = CLAMP(val.getDouble(1.0), 0, 100.0);
    } else if (path == "standard_deviation") {
        this->standard_deviation = 0.01 * CLAMP(val.getInt(10), 1, 100);
    } else if (path == "mean") {
        this->mean = 0.01 * CLAMP(val.getInt(10), 1, 100);
// Not implemented in the toolbar and preferences yet
    } else if (path == "distribution") {
        this->distrib = val.getInt(1);
    } else if (path == "tilt") {
        this->tilt = CLAMP(val.getDouble(0.1), 0, 1000.0);
    } else if (path == "ratio") {
        this->ratio = CLAMP(val.getDouble(), 0.0, 0.9);
    }
}

static void sp_spray_extinput(SprayTool *tc, GdkEvent *event)
{
    if (gdk_event_get_axis(event, GDK_AXIS_PRESSURE, &tc->pressure)) {
        tc->pressure = CLAMP(tc->pressure, TC_MIN_PRESSURE, TC_MAX_PRESSURE);
    } else {
        tc->pressure = TC_DEFAULT_PRESSURE;
    }
}

static double get_dilate_radius(SprayTool *tc)
{
    return 250 * tc->width/SP_EVENT_CONTEXT(tc)->desktop->current_zoom();
}

static double get_path_mean(SprayTool *tc)
{
    return tc->mean;
}

static double get_path_standard_deviation(SprayTool *tc)
{
    return tc->standard_deviation;
}

static double get_population(SprayTool *tc)
{
    double pressure = (tc->usepressure? tc->pressure / TC_DEFAULT_PRESSURE : 1);
    //g_warning("Pressure, population: %f, %f", pressure, pressure * tc->population);
    return pressure * tc->population;
}

static double get_move_mean(SprayTool *tc)
{
    return tc->mean;
}

static double get_move_standard_deviation(SprayTool *tc)
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
static void random_position(double &radius, double &angle, double &a, double &s, int /*choice*/)
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

static bool sp_spray_recursive(SPDesktop *desktop,
                               Inkscape::Selection *selection,
                               SPItem *item,
                               Geom::Point p,
                               Geom::Point /*vector*/,
                               gint mode,
                               double radius,
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

    {
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box) {
            // convert 3D boxes to ordinary groups before spraying their shapes
            item = box3d_convert_to_group(box);
            selection->add(item);
        }
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
                // Duplicate
                SPDocument *doc = item->document;
                Inkscape::XML::Document* xml_doc = doc->getReprDoc();
                Inkscape::XML::Node *old_repr = item->getRepr();
                Inkscape::XML::Node *parent = old_repr->parent();
                Inkscape::XML::Node *copy = old_repr->duplicate(xml_doc);
                parent->appendChild(copy);

                SPObject *new_obj = doc->getObjectByRepr(copy);
                item_copied = dynamic_cast<SPItem *>(new_obj);   // Conversion object->item
                Geom::Point center=item->getCenter();
                sp_spray_scale_rel(center,desktop, item_copied, Geom::Scale(_scale,_scale));
                sp_spray_scale_rel(center,desktop, item_copied, Geom::Scale(scale,scale));

                sp_spray_rotate_rel(center,desktop,item_copied, Geom::Rotate(angle));
                // Move the cursor p
                Geom::Point move = (Geom::Point(cos(tilt)*cos(dp)*dr/(1-ratio)+sin(tilt)*sin(dp)*dr/(1+ratio), -sin(tilt)*cos(dp)*dr/(1-ratio)+cos(tilt)*sin(dp)*dr/(1+ratio)))+(p-a->midpoint());
                sp_item_move_rel(item_copied, Geom::Translate(move[Geom::X], -move[Geom::Y]));
                did = true;
            }
        }
#ifdef ENABLE_SPRAY_MODE_SINGLE_PATH
    } else if (mode == SPRAY_MODE_SINGLE_PATH) {

        SPItem *parent_item = NULL;    // Initial object
        SPItem *item_copied = NULL;    // Projected object
        SPItem *unionResult = NULL;    // Previous union

        int i=1;
        std::vector<SPItem*> items=selection->itemList();
        for(std::vector<SPItem*>::const_iterator it=items.begin();it!=items.end();it++){
            SPItem *item1 = *it;
            if (i == 1) {
                parent_item = item1;
            }
            if (i == 2) {
                unionResult = item1;
            }
            i++;
        }
        if (parent_item) {
            SPDocument *doc = parent_item->document;
            Inkscape::XML::Document* xml_doc = doc->getReprDoc();
            Inkscape::XML::Node *old_repr = parent_item->getRepr();
            Inkscape::XML::Node *parent = old_repr->parent();

            Geom::OptRect a = parent_item->documentVisualBounds();
            if (a) {
                if (_fid <= population) { // Rules the population of objects sprayed
                    // Duplicates the parent item
                    Inkscape::XML::Node *copy = old_repr->duplicate(xml_doc);
                    parent->appendChild(copy);
                    SPObject *new_obj = doc->getObjectByRepr(copy);
                    item_copied = dynamic_cast<SPItem *>(new_obj);

                    // Move around the cursor
                    Geom::Point move = (Geom::Point(cos(tilt)*cos(dp)*dr/(1-ratio)+sin(tilt)*sin(dp)*dr/(1+ratio), -sin(tilt)*cos(dp)*dr/(1-ratio)+cos(tilt)*sin(dp)*dr/(1+ratio)))+(p-a->midpoint()); 

                    Geom::Point center = parent_item->getCenter();
                    sp_spray_scale_rel(center, desktop, item_copied, Geom::Scale(_scale, _scale));
                    sp_spray_scale_rel(center, desktop, item_copied, Geom::Scale(scale, scale));
                    sp_spray_rotate_rel(center, desktop, item_copied, Geom::Rotate(angle));
                    sp_item_move_rel(item_copied, Geom::Translate(move[Geom::X], -move[Geom::Y]));

                    // Union and duplication
                    selection->clear();
                    selection->add(item_copied);
                    if (unionResult) { // No need to add the very first item (initialized with NULL).
                        selection->add(unionResult);
                    }
                    sp_selected_path_union_skip_undo(selection, selection->desktop());
                    selection->add(parent_item);
                    Inkscape::GC::release(copy);
                    did = true;
                }
            }
        }
#endif
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
                // Ad the clone to the list of the parent's children
                parent->appendChild(clone);
                // Generates the link between parent and child attributes
                gchar *href_str = g_strdup_printf("#%s", old_repr->attribute("id"));
                clone->setAttribute("xlink:href", href_str, false); 
                g_free(href_str);

                SPObject *clone_object = doc->getObjectByRepr(clone);
                // Conversion object->item
                item_copied = dynamic_cast<SPItem *>(clone_object);
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

static bool sp_spray_dilate(SprayTool *tc, Geom::Point /*event_p*/, Geom::Point p, Geom::Point vector, bool reverse)
{
    SPDesktop *desktop = tc->desktop;
    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty()) {
        return false;
    }

    bool did = false;
    double radius = get_dilate_radius(tc);
    double population = get_population(tc);
    if (radius == 0 || population == 0) {
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
    double move_mean = get_move_mean(tc);
    double move_standard_deviation = get_move_standard_deviation(tc);

    {
        std::vector<SPItem*> const items(selection->itemList());

        for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
            SPItem *item = *i;
            g_assert(item != NULL);
            sp_object_ref(item);
        }

        for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
            SPItem *item = *i;
            g_assert(item != NULL);

            if (sp_spray_recursive(desktop, selection, item, p, vector, tc->mode, radius, population, tc->scale, tc->scale_variation, reverse, move_mean, move_standard_deviation, tc->ratio, tc->tilt, tc->rotation_variation, tc->distrib)) {
                did = true;
            }
        }

        for(std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
            SPItem *item = *i;
            g_assert(item != NULL);
            sp_object_unref(item);
        }
    }

    return did;
}

static void sp_spray_update_area(SprayTool *tc)
{
    double radius = get_dilate_radius(tc);
    Geom::Affine const sm ( Geom::Scale(radius/(1-tc->ratio), radius/(1+tc->ratio)) );
    sp_canvas_item_affine_absolute(tc->dilate_area, (sm* Geom::Rotate(tc->tilt))* Geom::Translate(SP_EVENT_CONTEXT(tc)->desktop->point()));
    sp_canvas_item_show(tc->dilate_area);
}

static void sp_spray_switch_mode(SprayTool *tc, gint mode, bool with_shift)
{
    // Select the button mode
    SP_EVENT_CONTEXT(tc)->desktop->setToolboxSelectOneValue("spray_tool_mode", mode); 
    // Need to set explicitly, because the prefs may not have changed by the previous
    tc->mode = mode;
    tc->update_cursor(with_shift);
}

bool SprayTool::root_handler(GdkEvent* event) {
    gint ret = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            sp_canvas_item_show(this->dilate_area);
            break;
        case GDK_LEAVE_NOTIFY:
            sp_canvas_item_hide(this->dilate_area);
            break;
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !this->space_panning) {
                if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
                    return TRUE;
                }

                Geom::Point const motion_w(event->button.x, event->button.y);
                Geom::Point const motion_dt(desktop->w2d(motion_w));
                this->last_push = desktop->dt2doc(motion_dt);

                sp_spray_extinput(this, event);

                desktop->canvas->forceFullRedrawAfterInterruptions(3);
                this->is_drawing = true;
                this->is_dilating = true;
                this->has_dilated = false;

                if(this->is_dilating && event->button.button == 1 && !this->space_panning) {
                    sp_spray_dilate(this, motion_w, desktop->dt2doc(motion_dt), Geom::Point(0,0), MOD__SHIFT(event));
                }

                this->has_dilated = true;
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY: {
            Geom::Point const motion_w(event->motion.x,
                                     event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));
            Geom::Point motion_doc(desktop->dt2doc(motion_dt));
            sp_spray_extinput(this, event);

            // Draw the dilating cursor
            double radius = get_dilate_radius(this);
            Geom::Affine const sm (Geom::Scale(radius/(1-this->ratio), radius/(1+this->ratio)) );
            sp_canvas_item_affine_absolute(this->dilate_area, (sm*Geom::Rotate(this->tilt))*Geom::Translate(desktop->w2d(motion_w)));
            sp_canvas_item_show(this->dilate_area);

            guint num = 0;
            if (!desktop->selection->isEmpty()) {
                num = desktop->selection->itemList().size();
            }
            if (num == 0) {
                this->message_context->flash(Inkscape::ERROR_MESSAGE, _("<b>Nothing selected!</b> Select objects to spray."));
            }

            // Dilating:
            if (this->is_drawing && ( event->motion.state & GDK_BUTTON1_MASK )) {
                sp_spray_dilate(this, motion_w, motion_doc, motion_doc - this->last_push, event->button.state & GDK_SHIFT_MASK? true : false);
                //this->last_push = motion_doc;
                this->has_dilated = true;

                // It's slow, so prevent clogging up with events
                gobble_motion_events(GDK_BUTTON1_MASK);
                return TRUE;
            }
        }
        break;
        /* Spray with the scroll */
        case GDK_SCROLL: {
            if (event->scroll.state & GDK_BUTTON1_MASK) {
                double temp ;
                temp = this->population;
                this->population = 1.0;
                desktop->setToolboxAdjustmentValue("population", this->population * 100);
                Geom::Point const scroll_w(event->button.x, event->button.y);
                Geom::Point const scroll_dt = desktop->point();;
                
                switch (event->scroll.direction) {
                    case GDK_SCROLL_DOWN:
                    case GDK_SCROLL_UP: {
                        if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
                            return TRUE;
                        }
                        this->last_push = desktop->dt2doc(scroll_dt);
                        sp_spray_extinput(this, event);
                        desktop->canvas->forceFullRedrawAfterInterruptions(3);
                        this->is_drawing = true;
                        this->is_dilating = true;
                        this->has_dilated = false;
                        if(this->is_dilating && !this->space_panning) {
                            sp_spray_dilate(this, scroll_w, desktop->dt2doc(scroll_dt), Geom::Point(0,0), false);
                        }
                        this->has_dilated = true;
                        
                        this->population = temp;
                        desktop->setToolboxAdjustmentValue("population", this->population * 100);

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
            this->is_drawing = false;

            if (this->is_dilating && event->button.button == 1 && !this->space_panning) {
                if (!this->has_dilated) {
                    // If we did not rub, do a light tap
                    this->pressure = 0.03;
                    sp_spray_dilate(this, motion_w, desktop->dt2doc(motion_dt), Geom::Point(0,0), MOD__SHIFT(event));
                }
                this->is_dilating = false;
                this->has_dilated = false;
                switch (this->mode) {
                    case SPRAY_MODE_COPY:
                        DocumentUndo::done(this->desktop->getDocument(),
                                           SP_VERB_CONTEXT_SPRAY, _("Spray with copies"));
                        break;
                    case SPRAY_MODE_CLONE:
                        DocumentUndo::done(this->desktop->getDocument(),
                                           SP_VERB_CONTEXT_SPRAY, _("Spray with clones"));
                        break;
                    case SPRAY_MODE_SINGLE_PATH:
                        DocumentUndo::done(this->desktop->getDocument(),
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
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_spray_switch_mode(this, SPRAY_MODE_COPY, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_k:
                case GDK_KEY_K:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_spray_switch_mode(this, SPRAY_MODE_CLONE, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
#ifdef ENABLE_SPRAY_MODE_SINGLE_PATH
                case GDK_KEY_l:
                case GDK_KEY_L:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_spray_switch_mode(this, SPRAY_MODE_SINGLE_PATH, MOD__SHIFT(event));
                        ret = TRUE;
                    }
                    break;
#endif
                case GDK_KEY_Up:
                case GDK_KEY_KP_Up:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->population += 0.01;
                        if (this->population > 1.0) {
                            this->population = 1.0;
                        }
                        desktop->setToolboxAdjustmentValue("spray-population", this->population * 100);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Down:
                case GDK_KEY_KP_Down:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->population -= 0.01;
                        if (this->population < 0.0) {
                            this->population = 0.0;
                        }
                        desktop->setToolboxAdjustmentValue("spray-population", this->population * 100);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Right:
                case GDK_KEY_KP_Right:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->width += 0.01;
                        if (this->width > 1.0) {
                            this->width = 1.0;
                        }
                        // The same spinbutton is for alt+x
                        desktop->setToolboxAdjustmentValue("altx-spray", this->width * 100);
                        sp_spray_update_area(this);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Left:
                case GDK_KEY_KP_Left:
                    if (!MOD__CTRL_ONLY(event)) {
                        this->width -= 0.01;
                        if (this->width < 0.01) {
                            this->width = 0.01;
                        }
                        desktop->setToolboxAdjustmentValue("altx-spray", this->width * 100);
                        sp_spray_update_area(this);
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Home:
                case GDK_KEY_KP_Home:
                    this->width = 0.01;
                    desktop->setToolboxAdjustmentValue("altx-spray", this->width * 100);
                    sp_spray_update_area(this);
                    ret = TRUE;
                    break;
                case GDK_KEY_End:
                case GDK_KEY_KP_End:
                    this->width = 1.0;
                    desktop->setToolboxAdjustmentValue("altx-spray", this->width * 100);
                    sp_spray_update_area(this);
                    ret = TRUE;
                    break;
                case GDK_KEY_x:
                case GDK_KEY_X:
                    if (MOD__ALT_ONLY(event)) {
                        desktop->setToolboxFocusTo("altx-spray");
                        ret = TRUE;
                    }
                    break;
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    this->update_cursor(true);
                    break;
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
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

        case GDK_KEY_RELEASE: {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            switch (get_group0_keyval(&event->key)) {
                case GDK_KEY_Shift_L:
                case GDK_KEY_Shift_R:
                    this->update_cursor(false);
                    break;
                case GDK_KEY_Control_L:
                case GDK_KEY_Control_R:
                    sp_spray_switch_mode (this, prefs->getInt("/tools/spray/mode"), MOD__SHIFT(event));
                    this->message_context->clear();
                    break;
                default:
                    sp_spray_switch_mode (this, prefs->getInt("/tools/spray/mode"), MOD__SHIFT(event));
                    break;
            }
        }

        default:
            break;
    }

    if (!ret) {
//        if ((SP_EVENT_CONTEXT_CLASS(sp_spray_context_parent_class))->root_handler) {
//            ret = (SP_EVENT_CONTEXT_CLASS(sp_spray_context_parent_class))->root_handler(event_context, event);
//        }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

