/*
 * Vanishing point for 3D perspectives
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Maximilian Albert <Anhalter42@gmx.de>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2005-2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "vanishing-point.h"

#include "desktop.h"
#include "display/sp-canvas-item.h"
#include "display/sp-ctrlline.h"
#include "ui/tools/tool-base.h"
#include "xml/repr.h"
#include "perspective-line.h"
#include "ui/shape-editor.h"
#include "snap.h"
#include "sp-namedview.h"
#include "ui/control-manager.h"
#include "document-undo.h"
#include "verbs.h"

using Inkscape::CTLINE_PRIMARY;
using Inkscape::CTLINE_SECONDARY;
using Inkscape::CTLINE_TERTIARY;
using Inkscape::CTRL_TYPE_ANCHOR;
using Inkscape::ControlManager;
using Inkscape::CtrlLineType;
using Inkscape::DocumentUndo;

namespace Box3D {

#define VP_KNOT_COLOR_NORMAL 0xffffff00
#define VP_KNOT_COLOR_SELECTED 0x0000ff00

// screen pixels between knots when they snap:
#define SNAP_DIST 5

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

// knot shapes corresponding to GrPointType enum
SPKnotShapeType vp_knot_shapes [] = {
        SP_KNOT_SHAPE_SQUARE, // VP_FINITE
        SP_KNOT_SHAPE_CIRCLE  //VP_INFINITE
};

static void
vp_drag_sel_changed(Inkscape::Selection */*selection*/, gpointer data)
{
    VPDrag *drag = (VPDrag *) data;
    drag->updateDraggers();
    drag->updateLines();
    drag->updateBoxReprs();
}

static void
vp_drag_sel_modified (Inkscape::Selection */*selection*/, guint /*flags*/, gpointer data)
{
    VPDrag *drag = (VPDrag *) data;
    drag->updateLines ();
    //drag->updateBoxReprs();
    drag->updateBoxHandles (); // FIXME: Only update the handles of boxes on this dragger (not on all)
    drag->updateDraggers ();
}

static bool
have_VPs_of_same_perspective (VPDragger *dr1, VPDragger *dr2)
{
    for (std::list<VanishingPoint>::iterator i = dr1->vps.begin(); i != dr1->vps.end(); ++i) {
        if (dr2->hasPerspective ((*i).get_perspective())) {
            return true;
        }
    }
    return false;
}

static void
vp_knot_moved_handler (SPKnot *knot, Geom::Point const &ppointer, guint state, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;
    VPDrag *drag = dragger->parent;

    Geom::Point p = ppointer;

    // FIXME: take from prefs
    double snap_dist = SNAP_DIST / SP_ACTIVE_DESKTOP->current_zoom();

    /*
     * We use dragging_started to indicate if we have already checked for the need to split Draggers up.
     * This only has the purpose of avoiding costly checks in the routine below.
     */
    if (!dragger->dragging_started && (state & GDK_SHIFT_MASK)) {
        /* with Shift; if there is more than one box linked to this VP
           we need to split it and create a new perspective */
        if (dragger->numberOfBoxes() > 1) { // FIXME: Don't do anything if *all* boxes of a VP are selected
            std::set<VanishingPoint*, less_ptr> sel_vps = dragger->VPsOfSelectedBoxes();

            std::list<SPBox3D *> sel_boxes;
            for (std::set<VanishingPoint*, less_ptr>::iterator vp = sel_vps.begin(); vp != sel_vps.end(); ++vp) {
                // for each VP that has selected boxes:
                Persp3D *old_persp = (*vp)->get_perspective();
                sel_boxes = (*vp)->selectedBoxes(SP_ACTIVE_DESKTOP->getSelection());

                // we create a new perspective ...
                Persp3D *new_persp = persp3d_create_xml_element (dragger->parent->document, old_persp->perspective_impl);

                /* ... unlink the boxes from the old one and
                   FIXME: We need to unlink the _un_selected boxes of each VP so that
                          the correct boxes are kept with the VP being moved */
                std::list<SPBox3D *> bx_lst = persp3d_list_of_boxes(old_persp);
                for (std::list<SPBox3D *>::iterator i = bx_lst.begin(); i != bx_lst.end(); ++i) {
                    if (std::find(sel_boxes.begin(), sel_boxes.end(), *i) == sel_boxes.end()) {
                        /* if a box in the VP is unselected, move it to the
                           newly created perspective so that it doesn't get dragged **/
                        box3d_switch_perspectives(*i, old_persp, new_persp);
                    }
                }
            }
            // FIXME: Do we need to create a new dragger as well?
            dragger->updateZOrders ();
            DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_CONTEXT_3DBOX,
                   _("Split vanishing points"));
            return;
        }
    }

    if (!(state & GDK_SHIFT_MASK)) {
        // without Shift; see if we need to snap to another dragger
        for (std::vector<VPDragger *>::const_iterator di = dragger->parent->draggers.begin(); di != dragger->parent->draggers.end(); ++di) {
            VPDragger *d_new = *di; 
            if ((d_new != dragger) && (Geom::L2 (d_new->point - p) < snap_dist)) {
                if (have_VPs_of_same_perspective (dragger, d_new)) {
                    // this would result in degenerate boxes, which we disallow for the time being
                    continue;
                }

                // update positions ... (this is needed so that the perspectives are detected as identical)
                // FIXME: This is called a bit too often, isn't it?
                for (std::list<VanishingPoint>::iterator j = dragger->vps.begin(); j != dragger->vps.end(); ++j) {
                    (*j).set_pos(d_new->point);
                }

                // ... join lists of VPs ...
                d_new->vps.merge(dragger->vps);

                // ... delete old dragger ...
                drag->draggers.erase(std::remove(drag->draggers.begin(), drag->draggers.end(), dragger),drag->draggers.end());
                delete dragger;
                dragger = NULL;

                // ... and merge any duplicate perspectives
                d_new->mergePerspectives();

                // TODO: Update the new merged dragger
                d_new->updateTip();

                d_new->parent->updateBoxDisplays (); // FIXME: Only update boxes in current dragger!
                d_new->updateZOrders ();

                drag->updateLines ();

                // TODO: Undo machinery; this doesn't work yet because perspectives must be created and
                //       deleted according to changes in the svg representation, not based on any user input
                //       as is currently the case.

                DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_CONTEXT_3DBOX,
                   _("Merge vanishing points"));

                return;
            }
        }
    }

    // We didn't hit the return statement above, so we didn't snap to another dragger. Therefore we'll now try a regular snap
    // Regardless of the status of the SHIFT key, we will try to snap; Here SHIFT does not disable snapping, as the shift key
    // has a different purpose in this context (see above)
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop);
    Inkscape::SnappedPoint s = m.freeSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_OTHER_HANDLE));
    m.unSetup();
    if (s.getSnapped()) {
        p = s.getPoint();
        knot->moveto(p);
    }

    dragger->point = p; // FIXME: Is dragger->point being used at all?

    dragger->updateVPs(p);
    dragger->updateBoxDisplays();
    dragger->parent->updateBoxHandles (); // FIXME: Only update the handles of boxes on this dragger (not on all)
    dragger->updateZOrders();

    drag->updateLines();

    dragger->dragging_started = true;
}

static void
vp_knot_grabbed_handler (SPKnot */*knot*/, unsigned int /*state*/, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;
    VPDrag *drag = dragger->parent;

    drag->dragging = true;
}

static void
vp_knot_ungrabbed_handler (SPKnot *knot, guint /*state*/, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;

    dragger->point_original = dragger->point = knot->pos;

    dragger->dragging_started = false;

    for (std::list<VanishingPoint>::iterator i = dragger->vps.begin(); i != dragger->vps.end(); ++i) {
        (*i).set_pos (knot->pos);
        (*i).updateBoxReprs();
        (*i).updatePerspRepr();
    }

    dragger->parent->updateDraggers ();
    dragger->parent->updateLines ();
    dragger->parent->updateBoxHandles ();

    // TODO: Update box's paths and svg representation

    dragger->parent->dragging = false;

    // TODO: Undo machinery!!
    g_return_if_fail (dragger->parent);
    g_return_if_fail (dragger->parent->document);
    DocumentUndo::done(dragger->parent->document, SP_VERB_CONTEXT_3DBOX,
               _("3D box: Move vanishing point"));
}

unsigned int VanishingPoint::global_counter = 0;

// FIXME: Rename to something more meaningful!
void
VanishingPoint::set_pos(Proj::Pt2 const &pt) {
    g_return_if_fail (_persp);
    _persp->perspective_impl->tmat.set_image_pt (_axis, pt);
}

std::list<SPBox3D *>
VanishingPoint::selectedBoxes(Inkscape::Selection *sel) {
    std::list<SPBox3D *> sel_boxes;
    std::vector<SPItem*> itemlist=sel->itemList();
    for (std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i) {
        SPItem *item = *i;
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box && this->hasBox(box)) {
            sel_boxes.push_back(box);
        }
    }
    return sel_boxes;
}

VPDragger::VPDragger(VPDrag *parent, Geom::Point p, VanishingPoint &vp) :
    parent(parent),
    knot(NULL),
    point(p),
    point_original(p),
    dragging_started(false),
    vps()
{
    if (vp.is_finite()) {
        // create the knot
        this->knot = new SPKnot(SP_ACTIVE_DESKTOP, NULL);
        this->knot->setMode(SP_KNOT_MODE_XOR);
        this->knot->setFill(VP_KNOT_COLOR_NORMAL, VP_KNOT_COLOR_NORMAL, VP_KNOT_COLOR_NORMAL);
        this->knot->setStroke(0x000000ff, 0x000000ff, 0x000000ff);
        this->knot->updateCtrl();
        knot->item->ctrlType = CTRL_TYPE_ANCHOR;
        ControlManager::getManager().track(knot->item);

        // move knot to the given point
        this->knot->setPosition(this->point, SP_KNOT_STATE_NORMAL);
        this->knot->show();

        // connect knot's signals
        this->_moved_connection = this->knot->moved_signal.connect(sigc::bind(sigc::ptr_fun(vp_knot_moved_handler), this));
        this->_grabbed_connection = this->knot->grabbed_signal.connect(sigc::bind(sigc::ptr_fun(vp_knot_grabbed_handler), this));
        this->_ungrabbed_connection = this->knot->ungrabbed_signal.connect(sigc::bind(sigc::ptr_fun(vp_knot_ungrabbed_handler), this));

        // add the initial VP (which may be NULL!)
        this->addVP (vp);
    }
}

VPDragger::~VPDragger()
{
    // disconnect signals
    this->_moved_connection.disconnect();
    this->_grabbed_connection.disconnect();
    this->_ungrabbed_connection.disconnect();

    /* unref should call destroy */
    knot_unref(this->knot);
}

/**
Updates the statusbar tip of the dragger knot, based on its draggables
 */
void
VPDragger::updateTip ()
{
    if (this->knot && this->knot->tip) {
        g_free (this->knot->tip);
        this->knot->tip = NULL;
    }

    guint num = this->numberOfBoxes();
    if (this->vps.size() == 1) {
        if (this->vps.front().is_finite()) {
            this->knot->tip = g_strdup_printf (ngettext("<b>Finite</b> vanishing point shared by <b>%d</b> box",
                                                        "<b>Finite</b> vanishing point shared by <b>%d</b> boxes; drag with <b>Shift</b> to separate selected box(es)",
                                                        num),
                                               num);
        } else {
            // This won't make sense any more when infinite VPs are not shown on the canvas,
            // but currently we update the status message anyway
            this->knot->tip = g_strdup_printf (ngettext("<b>Infinite</b> vanishing point shared by <b>%d</b> box",
                                                        "<b>Infinite</b> vanishing point shared by <b>%d</b> boxes; drag with <b>Shift</b> to separate selected box(es)",
                                                        num),
                                               num);
        }
    } else {
        int length = this->vps.size();
        char *desc1 = g_strdup_printf ("Collection of <b>%d</b> vanishing points ", length);
        char *desc2 = g_strdup_printf (ngettext("shared by <b>%d</b> box; drag with <b>Shift</b> to separate selected box(es)",
                                                "shared by <b>%d</b> boxes; drag with <b>Shift</b> to separate selected box(es)",
                                                num),
                                       num);
        this->knot->tip = g_strconcat(desc1, desc2, NULL);
        g_free (desc1);
        g_free (desc2);
    }
}

/**
 * Adds a vanishing point to the dragger (also updates the position if necessary);
 * the perspective is stored separately, too, for efficiency in updating boxes.
 */
void
VPDragger::addVP (VanishingPoint &vp, bool update_pos)
{
    if (!vp.is_finite() || std::find (vps.begin(), vps.end(), vp) != vps.end()) {
        // don't add infinite VPs; don't add the same VP twice
        return;
    }

    if (update_pos) {
        vp.set_pos (this->point);
    }
    this->vps.push_front (vp);

    this->updateTip();
}

void
VPDragger::removeVP (VanishingPoint const &vp)
{
    std::list<VanishingPoint>::iterator i = std::find (this->vps.begin(), this->vps.end(), vp);
    if (i != this->vps.end()) {
        this->vps.erase (i);
    }
    this->updateTip();
}

VanishingPoint *
VPDragger::findVPWithBox (SPBox3D *box) {
    for (std::list<VanishingPoint>::iterator vp = vps.begin(); vp != vps.end(); ++vp) {
        if ((*vp).hasBox(box)) {
            return &(*vp);
        }
    }
    return NULL;
}

std::set<VanishingPoint*, less_ptr>
VPDragger::VPsOfSelectedBoxes() {
    std::set<VanishingPoint*, less_ptr> sel_vps;
    VanishingPoint *vp;
    // FIXME: Should we take the selection from the parent VPDrag? I guess it shouldn't make a difference.
    Inkscape::Selection *sel = SP_ACTIVE_DESKTOP->getSelection();
    std::vector<SPItem*> itemlist=sel->itemList();
    for (std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i) {
        SPItem *item = *i;
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box) {
            vp = this->findVPWithBox(box);
            if (vp) {
                sel_vps.insert (vp);
            }
        }
    }
    return sel_vps;
}

guint
VPDragger::numberOfBoxes ()
{
    guint num = 0;
    for (std::list<VanishingPoint>::iterator vp = vps.begin(); vp != vps.end(); ++vp) {
        num += (*vp).numberOfBoxes();
    }
    return num;
}

bool
VPDragger::hasPerspective (const Persp3D *persp)
{
    for (std::list<VanishingPoint>::iterator i = vps.begin(); i != vps.end(); ++i) {
        if (persp3d_perspectives_coincide(persp, (*i).get_perspective())) {
            return true;
        }
    }
    return false;
}

void
VPDragger::mergePerspectives ()
{
    Persp3D *persp1, *persp2;
    for (std::list<VanishingPoint>::iterator i = vps.begin(); i != vps.end(); ++i) {
        persp1 = (*i).get_perspective();
        for (std::list<VanishingPoint>::iterator j = i; j != vps.end(); ++j) {
            persp2 = (*j).get_perspective();
            if (persp1 == persp2) {
                /* don't merge a perspective with itself */
                continue;
            }
            if (persp3d_perspectives_coincide(persp1,persp2)) {
                /* if perspectives coincide but are not the same, merge them */
                persp3d_absorb(persp1, persp2);

                this->parent->swap_perspectives_of_VPs(persp2, persp1);

                SP_OBJECT(persp2)->deleteObject(false);
            }
        }
    }
}

void
VPDragger::updateBoxDisplays ()
{
    for (std::list<VanishingPoint>::iterator i = this->vps.begin(); i != this->vps.end(); ++i) {
        (*i).updateBoxDisplays();
    }
}

void
VPDragger::updateVPs (Geom::Point const &pt)
{
    for (std::list<VanishingPoint>::iterator i = this->vps.begin(); i != this->vps.end(); ++i) {
        (*i).set_pos (pt);
    }
}

void
VPDragger::updateZOrders ()
{
    for (std::list<VanishingPoint>::iterator i = this->vps.begin(); i != this->vps.end(); ++i) {
        persp3d_update_z_orders((*i).get_perspective());
    }
}

void
VPDragger::printVPs() {
    g_print ("VPDragger at position (%f, %f):\n", point[Geom::X], point[Geom::Y]);
    for (std::list<VanishingPoint>::iterator i = this->vps.begin(); i != this->vps.end(); ++i) {
        g_print ("    VP %s\n", (*i).axisString());
    }
}

VPDrag::VPDrag (SPDocument *document)
{
    this->document = document;
    this->selection = SP_ACTIVE_DESKTOP->getSelection();

    this->show_lines = true;
    this->front_or_rear_lines = 0x1;

    this->dragging = false;

    this->sel_changed_connection = this->selection->connectChanged(
        sigc::bind (
            sigc::ptr_fun(&vp_drag_sel_changed),
            (gpointer)this )

        );
    this->sel_modified_connection = this->selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&vp_drag_sel_modified),
            (gpointer)this )
        );

    this->updateDraggers ();
    this->updateLines ();
}

VPDrag::~VPDrag()
{
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();

    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        delete (*i);
    }
    this->draggers.clear();

    for (std::vector<SPCtrlLine *>::const_iterator i = this->lines.begin(); i != this->lines.end(); ++i) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(*i));
    }
    this->lines.clear(); 
}

/**
 * Select the dragger that has the given VP.
 */
VPDragger *
VPDrag::getDraggerFor (VanishingPoint const &vp)
{
    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        VPDragger *dragger = *i;
        for (std::list<VanishingPoint>::iterator j = dragger->vps.begin(); j != dragger->vps.end(); ++j) {
            // TODO: Should we compare the pointers or the VPs themselves!?!?!?!
            if (*j == vp) {
                return (dragger);
            }
        }
    }
    return NULL;
}

void
VPDrag::printDraggers ()
{
    g_print ("=== VPDrag info: =================================\n");
    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        (*i)->printVPs();
        g_print ("========\n");
    }
    g_print ("=================================================\n");
}

/**
 * Regenerates the draggers list from the current selection; is called when selection is changed or modified
 */
void
VPDrag::updateDraggers ()
{
    if (this->dragging)
        return;
    // delete old draggers
    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        delete (*i);
    }
    this->draggers.clear();

    g_return_if_fail (this->selection != NULL);

    std::vector<SPItem*> itemlist=this->selection->itemList();
    for (std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i) {
        SPItem *item = *i;
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box) {
            VanishingPoint vp;
            for (int i = 0; i < 3; ++i) {
                vp.set(box3d_get_perspective(box), Proj::axes[i]);
                addDragger (vp);
            }
        }
    }
}

/**
Regenerates the lines list from the current selection; is called on each move
of a dragger, so that lines are always in sync with the actual perspective
*/
void
VPDrag::updateLines ()
{
    // delete old lines
    for (std::vector<SPCtrlLine *>::const_iterator i = this->lines.begin(); i != this->lines.end(); ++i) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(*i));
    }
    this->lines.clear();

    // do nothing if perspective lines are currently disabled
    if (this->show_lines == 0) return;

    g_return_if_fail (this->selection != NULL);

    std::vector<SPItem*> itemlist=this->selection->itemList();
    for (std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();++i) {
        SPItem *item = *i;
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box) {
            this->drawLinesForFace (box, Proj::X);
            this->drawLinesForFace (box, Proj::Y);
            this->drawLinesForFace (box, Proj::Z);
        }
    }
}

void
VPDrag::updateBoxHandles ()
{
    // FIXME: Is there a way to update the knots without accessing the
    //        (previously) statically linked function KnotHolder::update_knots?

    std::vector<SPItem*> sel = selection->itemList();
    if (sel.empty())
        return; // no selection

    if (sel.size() > 1) {
        // Currently we only show handles if a single box is selected
        return;
    }

    Inkscape::UI::Tools::ToolBase *ec = INKSCAPE.active_event_context();
    g_assert (ec != NULL);
    if (ec->shape_editor != NULL) {
        ec->shape_editor->update_knotholder();
    }
}

void
VPDrag::updateBoxReprs ()
{
    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        VPDragger *dragger = *i;
        for (std::list<VanishingPoint>::iterator i = dragger->vps.begin(); i != dragger->vps.end(); ++i) {
            (*i).updateBoxReprs();
        }
    }
}

void
VPDrag::updateBoxDisplays ()
{
    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        VPDragger *dragger = *i;
        for (std::list<VanishingPoint>::iterator i = dragger->vps.begin(); i != dragger->vps.end(); ++i) {
            (*i).updateBoxDisplays();
        }
    }
}


/**
 * Depending on the value of all_lines, draw the front and/or rear perspective lines starting from the given corners.
 */
void VPDrag::drawLinesForFace(const SPBox3D *box, Proj::Axis axis) //, guint corner1, guint corner2, guint corner3, guint corner4)
{
    CtrlLineType type = CTLINE_PRIMARY;
    switch (axis) {
        // TODO: Make color selectable by user
        case Proj::X:
            type = CTLINE_SECONDARY;
            break;
        case Proj::Y:
            type = CTLINE_PRIMARY;
            break;
        case Proj::Z:
            type = CTLINE_TERTIARY;
            break;
        default:
            g_assert_not_reached();
    }

    Geom::Point corner1, corner2, corner3, corner4;
    box3d_corners_for_PLs (box, axis, corner1, corner2, corner3, corner4);

    g_return_if_fail (box3d_get_perspective(box));
    Proj::Pt2 vp = persp3d_get_VP (box3d_get_perspective(box), axis);
    if (vp.is_finite()) {
        // draw perspective lines for finite VPs
        Geom::Point pt = vp.affine();
        if (this->front_or_rear_lines & 0x1) {
            // draw 'front' perspective lines
            this->addLine(corner1, pt, type);
            this->addLine(corner2, pt, type);
        }
        if (this->front_or_rear_lines & 0x2) {
            // draw 'rear' perspective lines
            this->addLine(corner3, pt, type);
            this->addLine(corner4, pt, type);
        }
    } else {
        // draw perspective lines for infinite VPs
        boost::optional<Geom::Point> pt1, pt2, pt3, pt4;
        Persp3D *persp = box3d_get_perspective(box);
        SPDesktop *desktop = SP_ACTIVE_DESKTOP; // FIXME: Store the desktop in VPDrag
        Box3D::PerspectiveLine pl (corner1, axis, persp);
        pt1 = pl.intersection_with_viewbox(desktop);

        pl = Box3D::PerspectiveLine (corner2, axis, persp);
        pt2 = pl.intersection_with_viewbox(desktop);

        pl = Box3D::PerspectiveLine (corner3, axis, persp);
        pt3 = pl.intersection_with_viewbox(desktop);

        pl = Box3D::PerspectiveLine (corner4, axis, persp);
        pt4 = pl.intersection_with_viewbox(desktop);

        if (!pt1 || !pt2 || !pt3 || !pt4) {
            // some perspective lines s are outside the canvas; currently we don't draw any of them
            return;
        }
        if (this->front_or_rear_lines & 0x1) {
            // draw 'front' perspective lines
            this->addLine(corner1, *pt1, type);
            this->addLine(corner2, *pt2, type);
        }
        if (this->front_or_rear_lines & 0x2) {
            // draw 'rear' perspective lines
            this->addLine(corner3, *pt3, type);
            this->addLine(corner4, *pt4, type);
        }
    }
}

/**
 * If there already exists a dragger within MERGE_DIST of p, add the VP to it;
 * otherwise create new dragger and add it to draggers list
 * We also store the corresponding perspective in case it is not already present.
 */
void
VPDrag::addDragger (VanishingPoint &vp)
{
    if (!vp.is_finite()) {
        // don't create draggers for infinite vanishing points
        return;
    }
    Geom::Point p = vp.get_pos();

    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        VPDragger *dragger = *i;
        if (Geom::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addVP (vp);
            return;
        }
    }

    VPDragger *new_dragger = new VPDragger(this, p, vp);
    // fixme: draggers should be added AFTER the last one: this way tabbing through them will be from begin to end.
    this->draggers.push_back(new_dragger);
}

void
VPDrag::swap_perspectives_of_VPs(Persp3D *persp2, Persp3D *persp1)
{
    // iterate over all VP in all draggers and replace persp2 with persp1
    for (std::vector<VPDragger *>::const_iterator i = this->draggers.begin(); i != this->draggers.end(); ++i) {
        for (std::list<VanishingPoint>::iterator j = (*i)->vps.begin();
             j != (*i)->vps.end(); ++j) {
            if ((*j).get_perspective() == persp2) {
                (*j).set_perspective(persp1);
            }
        }
    }
}

void VPDrag::addLine(Geom::Point const &p1, Geom::Point const &p2, Inkscape::CtrlLineType type)
{
    SPCtrlLine *line = ControlManager::getManager().createControlLine(SP_ACTIVE_DESKTOP->getControls(), p1, p2, type);
    sp_canvas_item_show(line);
    this->lines.push_back(line);
}

} // namespace Box3D

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
