/*
 * LPEToolContext: a context for a generic tool composed of subtools that are given by LPEs
 *
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <2geom/sbasis-geometric.h>
#include <gdk/gdkkeysyms.h>

#include <glibmm/i18n.h>
#include "macros.h"
#include "pixmaps/cursor-crosshairs.xpm"
#include <gtk/gtk.h>
#include "desktop.h"
#include "message-context.h"
#include "preferences.h"
#include "ui/shape-editor.h"
#include "selection.h"
#include "desktop-handles.h"
#include "document.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "display/canvas-text.h"
#include "message-stack.h"
#include "sp-path.h"
#include "util/units.h"

#include "ui/tools/lpe-tool.h"

using Inkscape::Util::unit_table;
using Inkscape::UI::Tools::PenTool;

const int num_subtools = 8;

SubtoolEntry lpesubtools[] = {
    // this must be here to account for the "all inactive" action
    {Inkscape::LivePathEffect::INVALID_LPE, "draw-geometry-inactive"},
    {Inkscape::LivePathEffect::LINE_SEGMENT, "draw-geometry-line-segment"},
    {Inkscape::LivePathEffect::CIRCLE_3PTS, "draw-geometry-circle-from-three-points"},
    {Inkscape::LivePathEffect::CIRCLE_WITH_RADIUS, "draw-geometry-circle-from-radius"},
    {Inkscape::LivePathEffect::PARALLEL, "draw-geometry-line-parallel"},
    {Inkscape::LivePathEffect::PERP_BISECTOR, "draw-geometry-line-perpendicular"},
    {Inkscape::LivePathEffect::ANGLE_BISECTOR, "draw-geometry-angle-bisector"},
    {Inkscape::LivePathEffect::MIRROR_SYMMETRY, "draw-geometry-mirror"}
};


#include "ui/tool-factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

void sp_lpetool_context_selection_changed(Inkscape::Selection *selection, gpointer data);

namespace {
	ToolBase* createLPEToolContext() {
		return new LpeTool();
	}

	bool lpetoolContextRegistered = ToolFactory::instance().registerObject("/tools/lpetool", createLPEToolContext);
}

const std::string& LpeTool::getPrefsPath() {
	return LpeTool::prefsPath;
}

const std::string LpeTool::prefsPath = "/tools/lpetool";

LpeTool::LpeTool()
    : PenTool(cursor_crosshairs_xpm, 7, 7)
    , shape_editor(NULL)
    , canvas_bbox(NULL)
    , mode(Inkscape::LivePathEffect::BEND_PATH)
// TODO: pointer?
    , measuring_items(new std::map<SPPath *, SPCanvasItem*>)
{
}

LpeTool::~LpeTool() {
    delete this->shape_editor;
    this->shape_editor = NULL;

    if (this->canvas_bbox) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(this->canvas_bbox));
        this->canvas_bbox = NULL;
    }

    lpetool_delete_measuring_items(this);
    delete this->measuring_items;
    this->measuring_items = NULL;

    this->sel_changed_connection.disconnect();
}

void LpeTool::setup() {
    PenTool::setup();

    Inkscape::Selection *selection = this->desktop->getSelection();
    SPItem *item = selection->singleItem();

    this->sel_changed_connection.disconnect();
    this->sel_changed_connection =
        selection->connectChanged(sigc::bind(sigc::ptr_fun(&sp_lpetool_context_selection_changed), (gpointer)this));

    this->shape_editor = new ShapeEditor(this->desktop);

    lpetool_context_switch_mode(this, Inkscape::LivePathEffect::INVALID_LPE);
    lpetool_context_reset_limiting_bbox(this);
    lpetool_create_measuring_items(this);

// TODO temp force:
    this->enableSelectionCue();
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (item) {
        this->shape_editor->set_item(item);
    }

    if (prefs->getBool("/tools/lpetool/selcue")) {
        this->enableSelectionCue();
    }
}

/**
 * Callback that processes the "changed" signal on the selection;
 * destroys old and creates new nodepath and reassigns listeners to the new selected item's repr.
 */
void sp_lpetool_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    LpeTool *lc = SP_LPETOOL_CONTEXT(data);

    lc->shape_editor->unset_item();
    SPItem *item = selection->singleItem();
    lc->shape_editor->set_item(item);
}

void LpeTool::set(const Inkscape::Preferences::Entry& val) {
    if (val.getEntryName() == "mode") {
        Inkscape::Preferences::get()->setString("/tools/geometric/mode", "drag");
        SP_PEN_CONTEXT(this)->mode = PenTool::MODE_DRAG;
    }
}

bool LpeTool::item_handler(SPItem* item, GdkEvent* event) {
    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
        {
            // select the clicked item but do nothing else
            Inkscape::Selection * const selection = this->desktop->getSelection();
            selection->clear();
            selection->add(item);
            ret = TRUE;
            break;
        }
        case GDK_BUTTON_RELEASE:
            // TODO: do we need to catch this or can we pass it on to the parent handler?
            ret = TRUE;
            break;
        default:
            break;
    }

    if (!ret) {
    	ret = PenTool::item_handler(item, event);
    }

    return ret;
}

bool LpeTool::root_handler(GdkEvent* event) {
    Inkscape::Selection *selection = desktop->getSelection();

    bool ret = false;

    if (this->hasWaitingLPE()) {
        // quit when we are waiting for a LPE to be applied
        //ret = ((ToolBaseClass *) sp_lpetool_context_parent_class)->root_handler(event_context, event);
	return PenTool::root_handler(event);
    }

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !this->space_panning) {
                if (this->mode == Inkscape::LivePathEffect::INVALID_LPE) {
                    // don't do anything for now if we are inactive (except clearing the selection
                    // since this was a click into empty space)
                    selection->clear();
                    desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Choose a construction tool from the toolbar."));
                    ret = true;
                    break;
                }

                // save drag origin
                this->xp = (gint) event->button.x;
                this->yp = (gint) event->button.y;
                this->within_tolerance = true;

                using namespace Inkscape::LivePathEffect;

                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                int mode = prefs->getInt("/tools/lpetool/mode");
                EffectType type = lpesubtools[mode].type;

                //bool over_stroke = lc->shape_editor->is_over_stroke(Geom::Point(event->button.x, event->button.y), true);

                this->waitForLPEMouseClicks(type, Inkscape::LivePathEffect::Effect::acceptsNumClicks(type));

                // we pass the mouse click on to pen tool as the first click which it should collect
                //ret = ((ToolBaseClass *) sp_lpetool_context_parent_class)->root_handler(event_context, event);
		ret = PenTool::root_handler(event);
            }
            break;


    case GDK_BUTTON_RELEASE:
    {
        /**
        break;
        **/
    }

    case GDK_KEY_PRESS:
        /**
        switch (get_group0_keyval (&event->key)) {
        }
        break;
        **/

    case GDK_KEY_RELEASE:
        /**
        switch (get_group0_keyval(&event->key)) {
            case GDK_Control_L:
            case GDK_Control_R:
                dc->_message_context->clear();
                break;
            default:
                break;
        }
        **/

    default:
        break;
    }

    if (!ret) {
    	ret = PenTool::root_handler(event);
    }

    return ret;
}

/*
 * Finds the index in the list of geometric subtools corresponding to the given LPE type.
 * Returns -1 if no subtool is found.
 */
int
lpetool_mode_to_index(Inkscape::LivePathEffect::EffectType const type) {
    for (int i = 0; i < num_subtools; ++i) {
        if (lpesubtools[i].type == type) {
            return i;
        }
    }
    return -1;
}

/*
 * Checks whether an item has a construction applied as LPE and if so returns the index in
 * lpesubtools of this construction
 */
int lpetool_item_has_construction(LpeTool */*lc*/, SPItem *item)
{
    if (!SP_IS_LPE_ITEM(item)) {
        return -1;
    }

    Inkscape::LivePathEffect::Effect* lpe = SP_LPE_ITEM(item)->getCurrentLPE();
    if (!lpe) {
        return -1;
    }
    return lpetool_mode_to_index(lpe->effectType());
}

/*
 * Attempts to perform the construction of the given type (i.e., to apply the corresponding LPE) to
 * a single selected item. Returns whether we succeeded.
 */
bool
lpetool_try_construction(LpeTool *lc, Inkscape::LivePathEffect::EffectType const type)
{
    Inkscape::Selection *selection = lc->desktop->getSelection();
    SPItem *item = selection->singleItem();

    // TODO: should we check whether type represents a valid geometric construction?
    if (item && SP_IS_LPE_ITEM(item) && Inkscape::LivePathEffect::Effect::acceptsNumClicks(type) == 0) {
        Inkscape::LivePathEffect::Effect::createAndApply(type, sp_desktop_document(lc->desktop), item);
        return true;
    }
    return false;
}

void
lpetool_context_switch_mode(LpeTool *lc, Inkscape::LivePathEffect::EffectType const type)
{
    int index = lpetool_mode_to_index(type);
    if (index != -1) {
        lc->mode = type;
        lc->desktop->setToolboxSelectOneValue ("lpetool_mode_action", index);
    } else {
        g_warning ("Invalid mode selected: %d", type);
        return;
    }
}

void
lpetool_get_limiting_bbox_corners(SPDocument *document, Geom::Point &A, Geom::Point &B) {
    Geom::Coord w = document->getWidth().value("px");
    Geom::Coord h = document->getHeight().value("px");
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    double ulx = prefs->getDouble("/tools/lpetool/bbox_upperleftx", 0);
    double uly = prefs->getDouble("/tools/lpetool/bbox_upperlefty", 0);
    double lrx = prefs->getDouble("/tools/lpetool/bbox_lowerrightx", w);
    double lry = prefs->getDouble("/tools/lpetool/bbox_lowerrighty", h);

    A = Geom::Point(ulx, uly);
    B = Geom::Point(lrx, lry);
}

/*
 * Reads the limiting bounding box from preferences and draws it on the screen
 */
// TODO: Note that currently the bbox is not user-settable; we simply use the page borders
void
lpetool_context_reset_limiting_bbox(LpeTool *lc)
{
    if (lc->canvas_bbox) {
        sp_canvas_item_destroy(lc->canvas_bbox);
        lc->canvas_bbox = NULL;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/tools/lpetool/show_bbox", true))
        return;

    SPDocument *document = sp_desktop_document(lc->desktop);

    Geom::Point A, B;
    lpetool_get_limiting_bbox_corners(document, A, B);
    Geom::Affine doc2dt(lc->desktop->doc2dt());
    A *= doc2dt;
    B *= doc2dt;

    Geom::Rect rect(A, B);
    SPCurve *curve = SPCurve::new_from_rect(rect);

    lc->canvas_bbox = sp_canvas_bpath_new (lc->desktop->getControls(), curve);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(lc->canvas_bbox), 0x0000ffff, 0.8, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT, 5, 5);
}

static void
set_pos_and_anchor(SPCanvasText *canvas_text, const Geom::Piecewise<Geom::D2<Geom::SBasis> > &pwd2,
                   const double t, const double length, bool /*use_curvature*/ = false)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > pwd2_reparam = arc_length_parametrization(pwd2, 2 , 0.1);
    double t_reparam = pwd2_reparam.cuts.back() * t;
    Point pos = pwd2_reparam.valueAt(t_reparam);
    Point dir = unit_vector(derivative(pwd2_reparam).valueAt(t_reparam));
    Point n = -rot90(dir);
    double angle = Geom::angle_between(dir, Point(1,0));

    sp_canvastext_set_coords(canvas_text, pos + n * length);
    sp_canvastext_set_anchor_manually(canvas_text, std::sin(angle), -std::cos(angle));
}

void
lpetool_create_measuring_items(LpeTool *lc, Inkscape::Selection *selection)
{
    if (!selection) {
        selection = lc->desktop->getSelection();
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool show = prefs->getBool("/tools/lpetool/show_measuring_info",  true);

    SPPath *path;
    SPCurve *curve;
    SPCanvasText *canvas_text;
    SPCanvasGroup *tmpgrp = lc->desktop->getTempGroup();
    gchar *arc_length;
    double lengthval;

    for (GSList const *i = selection->itemList(); i != NULL; i = i->next) {
        if (SP_IS_PATH(i->data)) {
            path = SP_PATH(i->data);
            curve = path->getCurve();
            Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2 = paths_to_pw(curve->get_pathvector());
            canvas_text = (SPCanvasText *) sp_canvastext_new(tmpgrp, lc->desktop, Geom::Point(0,0), "");
            if (!show)
                sp_canvas_item_hide(SP_CANVAS_ITEM(canvas_text));

            Inkscape::Util::Unit const * unit = NULL;
            if (prefs->getString("/tools/lpetool/unit").compare("")) {
                unit = unit_table.getUnit(prefs->getString("/tools/lpetool/unit"));
            } else {
                unit = unit_table.getUnit("px");
            }

            lengthval = Geom::length(pwd2);
            lengthval = Inkscape::Util::Quantity::convert(lengthval, "px", unit);
            arc_length = g_strdup_printf("%.2f %s", lengthval, unit->abbr.c_str());
            sp_canvastext_set_text (canvas_text, arc_length);
            set_pos_and_anchor(canvas_text, pwd2, 0.5, 10);
            // TODO: must we free arc_length?
            (*lc->measuring_items)[path] = SP_CANVAS_ITEM(canvas_text);
        }
    }
}

void
lpetool_delete_measuring_items(LpeTool *lc)
{
    std::map<SPPath *, SPCanvasItem*>::iterator i;
    for (i = lc->measuring_items->begin(); i != lc->measuring_items->end(); ++i) {
        sp_canvas_item_destroy(i->second);
    }
    lc->measuring_items->clear();
}

void
lpetool_update_measuring_items(LpeTool *lc)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    for ( std::map<SPPath *, SPCanvasItem*>::iterator i = lc->measuring_items->begin();
          i != lc->measuring_items->end();
          ++i )
    {
        SPPath *path = i->first;
        SPCurve *curve = path->getCurve();
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2 = Geom::paths_to_pw(curve->get_pathvector());
        Inkscape::Util::Unit const * unit = NULL;
        if (prefs->getString("/tools/lpetool/unit").compare("")) {
            unit = unit_table.getUnit(prefs->getString("/tools/lpetool/unit"));
        } else {
            unit = unit_table.getUnit("px");
        }
        double lengthval = Geom::length(pwd2);
        lengthval = Inkscape::Util::Quantity::convert(lengthval, "px", unit);
        gchar *arc_length = g_strdup_printf("%.2f %s", lengthval, unit->abbr.c_str());
        sp_canvastext_set_text (SP_CANVASTEXT(i->second), arc_length);
        set_pos_and_anchor(SP_CANVASTEXT(i->second), pwd2, 0.5, 10);
        // TODO: must we free arc_length?
    }
}

void
lpetool_show_measuring_info(LpeTool *lc, bool show)
{
    std::map<SPPath *, SPCanvasItem*>::iterator i;
    for (i = lc->measuring_items->begin(); i != lc->measuring_items->end(); ++i) {
        if (show) {
            sp_canvas_item_show(i->second);
        } else {
            sp_canvas_item_hide(i->second);
        }
    }
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
