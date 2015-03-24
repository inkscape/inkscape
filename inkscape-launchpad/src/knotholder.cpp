/*
 * Container for SPKnot visual handles.
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "document.h"
#include "document-undo.h"
#include "sp-shape.h"
#include "knot.h"
#include "knotholder.h"
#include "knot-holder-entity.h"
#include "ui/tools/rect-tool.h"
#include "sp-rect.h"
#include "ui/tools/arc-tool.h"
#include "sp-ellipse.h"
#include "ui/tools/tweak-tool.h"
#include "sp-star.h"
#include "ui/tools/spiral-tool.h"
#include "sp-spiral.h"
#include "sp-offset.h"
#include "box3d.h"
#include "sp-pattern.h"
#include "style.h"
#include "live_effects/lpeobject.h"
#include "live_effects/effect.h"
#include "desktop.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-item.h"
#include "verbs.h"
#include "ui/control-manager.h"

#include "xml/repr.h" // for debugging only

using Inkscape::ControlManager;
using Inkscape::DocumentUndo;

class SPDesktop;

KnotHolder::KnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler) :
    desktop(desktop),
    item(item),
    //XML Tree being used directly for item->getRepr() while it shouldn't be...
    repr(item ? item->getRepr() : 0),
    entity(),
    sizeUpdatedConn(),
    released(relhandler),
    local_change(FALSE),
    dragging(false)
{
    if (!desktop || !item) {
        g_print ("Error! Throw an exception, please!\n");
    }

    sp_object_ref(item);

    sizeUpdatedConn = ControlManager::getManager().connectCtrlSizeChanged(sigc::mem_fun(*this, &KnotHolder::updateControlSizes));
}

KnotHolder::~KnotHolder() {
	sp_object_unref(item);

    for (std::list<KnotHolderEntity *>::iterator i = entity.begin(); i != entity.end(); ++i)
    {
        delete (*i);
        (*i) = NULL;
    }
    entity.clear(); // is this necessary?
    sizeUpdatedConn.disconnect();
}

void KnotHolder::updateControlSizes()
{
    ControlManager &mgr = ControlManager::getManager();

    for (std::list<KnotHolderEntity *>::iterator it = entity.begin(); it != entity.end(); ++it) {
        KnotHolderEntity *e = *it;
        mgr.updateItem(e->knot->item);
    }
}

void KnotHolder::update_knots()
{
    for (std::list<KnotHolderEntity *>::iterator i = entity.begin(); i != entity.end(); ++i) {
        KnotHolderEntity *e = *i;
        e->update_knot();
    }
}

/**
 * Returns true if at least one of the KnotHolderEntities has the mouse hovering above it.
 */
bool KnotHolder::knot_mouseover() const {
    for (std::list<KnotHolderEntity *>::const_iterator i = entity.begin(); i != entity.end(); ++i) {
        const SPKnot *knot = (*i)->knot;

        if (knot && (knot->flags & SP_KNOT_MOUSEOVER)) {
            return true;
        }
    }

    return false;
}

void
KnotHolder::knot_clicked_handler(SPKnot *knot, guint state)
{
    KnotHolder *knot_holder = this;
    SPItem *saved_item = this->item;

    for(std::list<KnotHolderEntity *>::iterator i = knot_holder->entity.begin(); i != knot_holder->entity.end(); ++i) {
        KnotHolderEntity *e = *i;
        if (e->knot == knot) {
            // no need to test whether knot_click exists since it's virtual now
            e->knot_click(state);
            break;
        }
    }

    {
        SPShape *savedShape = dynamic_cast<SPShape *>(saved_item);
        if (savedShape) {
            savedShape->set_shape();
        }
    }

    knot_holder->update_knots();

    unsigned int object_verb = SP_VERB_NONE;

    // TODO extract duplicated blocks;
    if (dynamic_cast<SPRect *>(saved_item)) {
        object_verb = SP_VERB_CONTEXT_RECT;
    } else if (dynamic_cast<SPBox3D *>(saved_item)) {
        object_verb = SP_VERB_CONTEXT_3DBOX;
    } else if (dynamic_cast<SPGenericEllipse *>(saved_item)) {
        object_verb = SP_VERB_CONTEXT_ARC;
    } else if (dynamic_cast<SPStar *>(saved_item)) {
        object_verb = SP_VERB_CONTEXT_STAR;
    } else if (dynamic_cast<SPSpiral *>(saved_item)) {
        object_verb = SP_VERB_CONTEXT_SPIRAL;
    } else {
        SPOffset *offset = dynamic_cast<SPOffset *>(saved_item);
        if (offset) {
            if (offset->sourceHref) {
                object_verb = SP_VERB_SELECTION_LINKED_OFFSET;
            } else {
                object_verb = SP_VERB_SELECTION_DYNAMIC_OFFSET;
            }
        }
    }

    // for drag, this is done by ungrabbed_handler, but for click we must do it here
    
    if (saved_item) { //increasingly aggressive sanity checks
       if (saved_item->document) {
            // enum is unsigned so can't be less than SP_VERB_INVALID
            if (object_verb <= SP_VERB_LAST) {
                DocumentUndo::done(saved_item->document, object_verb,
                                   _("Change handle"));
            }
       }
    } // else { abort(); }
}

void
KnotHolder::knot_moved_handler(SPKnot *knot, Geom::Point const &p, guint state)
{
    if (this->dragging == false) {
    	this->dragging = true;
    }

	// this was a local change and the knotholder does not need to be recreated:
    this->local_change = TRUE;

    for(std::list<KnotHolderEntity *>::iterator i = this->entity.begin(); i != this->entity.end(); ++i) {
        KnotHolderEntity *e = *i;
        if (e->knot == knot) {
            Geom::Point const q = p * item->i2dt_affine().inverse();
            e->knot_set(q, e->knot->drag_origin * item->i2dt_affine().inverse(), state);
            break;
        }
    }

    SPShape *shape = dynamic_cast<SPShape *>(item);
    if (shape) {
        shape->set_shape();
    }

    this->update_knots();
}

void
KnotHolder::knot_ungrabbed_handler(SPKnot */*knot*/, guint)
{
	this->dragging = false;

	if (this->released) {
        this->released(this->item);
    } else {
        SPObject *object = (SPObject *) this->item;

        // Caution: this call involves a screen update, which may process events, and as a
        // result the knotholder may be destructed. So, after the updateRepr, we cannot use any
        // fields of this knotholder (such as this->item), but only values we have saved beforehand
        // (such as object).
        object->updateRepr();

        /* do cleanup tasks (e.g., for LPE items write the parameter values
         * that were changed by dragging the handle to SVG)
         */
        SPLPEItem *lpeItem = dynamic_cast<SPLPEItem *>(object);
        if (lpeItem) {
            // This writes all parameters to SVG. Is this sufficiently efficient or should we only
            // write the ones that were changed?
            Inkscape::LivePathEffect::Effect *lpe = lpeItem->getCurrentLPE();
            if (lpe) {
                LivePathEffectObject *lpeobj = lpe->getLPEObj();
                lpeobj->updateRepr();
            }
        }

        unsigned int object_verb = SP_VERB_NONE;

        // TODO extract duplicated blocks:
        if (dynamic_cast<SPRect *>(object)) {
            object_verb = SP_VERB_CONTEXT_RECT;
        } else if (dynamic_cast<SPBox3D *>(object)) {
            object_verb = SP_VERB_CONTEXT_3DBOX;
        } else if (dynamic_cast<SPGenericEllipse *>(object)) {
            object_verb = SP_VERB_CONTEXT_ARC;
        } else if (dynamic_cast<SPStar *>(object)) {
            object_verb = SP_VERB_CONTEXT_STAR;
        } else if (dynamic_cast<SPSpiral *>(object)) {
            object_verb = SP_VERB_CONTEXT_SPIRAL;
        } else {
            SPOffset *offset = dynamic_cast<SPOffset *>(object);
            if (offset) {
                if (offset->sourceHref) {
                    object_verb = SP_VERB_SELECTION_LINKED_OFFSET;
                } else {
                    object_verb = SP_VERB_SELECTION_DYNAMIC_OFFSET;
                }
            }
        }

        DocumentUndo::done(object->document, object_verb, _("Move handle"));
    }
}

void KnotHolder::add(KnotHolderEntity *e)
{
    // g_message("Adding a knot at %p", e);
    entity.push_back(e);
    updateControlSizes();
}

void KnotHolder::add_pattern_knotholder()
{
    if ((item->style->fill.isPaintserver()) && dynamic_cast<SPPattern *>(item->style->getFillPaintServer())) {
        PatternKnotHolderEntityXY *entity_xy = new PatternKnotHolderEntityXY(true);
        PatternKnotHolderEntityAngle *entity_angle = new PatternKnotHolderEntityAngle(true);
        PatternKnotHolderEntityScale *entity_scale = new PatternKnotHolderEntityScale(true);
        entity_xy->create(desktop, item, this, Inkscape::CTRL_TYPE_POINT,
                          // TRANSLATORS: This refers to the pattern that's inside the object
                          _("<b>Move</b> the pattern fill inside the object"),
                          SP_KNOT_SHAPE_CROSS);

        entity_scale->create(desktop, item, this, Inkscape::CTRL_TYPE_SIZER,
                             _("<b>Scale</b> the pattern fill; uniformly if with <b>Ctrl</b>"),
                             SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);

        entity_angle->create(desktop, item, this, Inkscape::CTRL_TYPE_ROTATE,
                             _("<b>Rotate</b> the pattern fill; with <b>Ctrl</b> to snap angle"),
                             SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);

        entity.push_back(entity_xy);
        entity.push_back(entity_angle);
        entity.push_back(entity_scale);
    }

    if ((item->style->stroke.isPaintserver()) && dynamic_cast<SPPattern *>(item->style->getStrokePaintServer())) {
        PatternKnotHolderEntityXY *entity_xy = new PatternKnotHolderEntityXY(false);
        PatternKnotHolderEntityAngle *entity_angle = new PatternKnotHolderEntityAngle(false);
        PatternKnotHolderEntityScale *entity_scale = new PatternKnotHolderEntityScale(false);
        entity_xy->create(desktop, item, this, Inkscape::CTRL_TYPE_POINT,
                          // TRANSLATORS: This refers to the pattern that's inside the object
                          _("<b>Move</b> the pattern fill inside the object"),
                          SP_KNOT_SHAPE_CROSS);

        entity_scale->create(desktop, item, this, Inkscape::CTRL_TYPE_SIZER,
                             _("<b>Scale</b> the pattern fill; uniformly if with <b>Ctrl</b>"),
                             SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);

        entity_angle->create(desktop, item, this, Inkscape::CTRL_TYPE_ROTATE,
                             _("<b>Rotate</b> the pattern fill; with <b>Ctrl</b> to snap angle"),
                             SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);

        entity.push_back(entity_xy);
        entity.push_back(entity_angle);
        entity.push_back(entity_scale);
    }
    updateControlSizes();
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
