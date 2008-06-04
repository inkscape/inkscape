#define __KNOT_HOLDER_C__

/*
 * Container for SPKnot visual handles
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2001-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "document.h"
#include "sp-shape.h"
#include "knot.h"
#include "knotholder.h"
#include "rect-context.h"
#include "sp-rect.h"
#include "arc-context.h"
#include "sp-ellipse.h"
#include "star-context.h"
#include "sp-star.h"
#include "spiral-context.h"
#include "sp-spiral.h"
#include "sp-offset.h"
#include "box3d.h"
#include "sp-pattern.h"
#include "style.h"

#include "xml/repr.h" // for debugging only

#include <libnr/nr-matrix-div.h>
#include <glibmm/i18n.h>

class SPDesktop;

//static void knot_clicked_handler (SPKnot *knot, guint state, gpointer data);
//static void knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state, gpointer data);
//static void knot_ungrabbed_handler (SPKnot *knot, unsigned int state, KnotHolder *kh);

KnotHolder::KnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler)
{
    Inkscape::XML::Node *repr = SP_OBJECT(item)->repr;

    if (!desktop || !item || !SP_IS_ITEM(item)) {
        g_print ("Error! Throw an exception, please!\n");
    }

    this->desktop = desktop;
    this->item = item;
    g_object_ref(G_OBJECT(item)); // TODO: is this still needed after C++-ification?

    this->released = relhandler;

    this->repr = repr;
    this->local_change = FALSE;
}

KnotHolder::~KnotHolder() {
    g_object_unref(G_OBJECT(item));
    for(std::list<KnotHolderEntity *>::iterator i = entity.begin(); i != entity.end(); ++i) {
        delete *i;
    }
    entity.clear(); // this shouldn't be necessary, though
}

/** TODO: is this still needed?
void sp_knot_holder_destroy(SPKnotHolder *kh) {
    g_object_unref(kh);
    }
**/

/**
 * \param p In desktop coordinates.
 */

void
KnotHolder::update_knots()
{
    NR::Matrix const i2d(sp_item_i2d_affine(item));

    for(std::list<KnotHolderEntity *>::iterator i = entity.begin(); i != entity.end(); ++i) {
        KnotHolderEntity *e = *i;
        e->update_knot();
    }
}

void
KnotHolder::knot_clicked_handler(SPKnot *knot, guint state)
{
    KnotHolder *knot_holder = this;

    for(std::list<KnotHolderEntity *>::iterator i = knot_holder->entity.begin(); i != knot_holder->entity.end(); ++i) {
        KnotHolderEntity *e = *i;
        if (e->knot == knot) {
            // no need to test whether knot_click exists since it's virtual now
            e->knot_click_func(state);
            break;
        }
    }

    if (SP_IS_SHAPE(item)) {
        sp_shape_set_shape(SP_SHAPE(item));
    }

    knot_holder->update_knots();

    unsigned int object_verb = SP_VERB_NONE;

    if (SP_IS_RECT(item))
        object_verb = SP_VERB_CONTEXT_RECT;
    else if (SP_IS_BOX3D(item))
        object_verb = SP_VERB_CONTEXT_3DBOX;
    else if (SP_IS_GENERICELLIPSE(item))
        object_verb = SP_VERB_CONTEXT_ARC;
    else if (SP_IS_STAR(item))
        object_verb = SP_VERB_CONTEXT_STAR;
    else if (SP_IS_SPIRAL(item))
        object_verb = SP_VERB_CONTEXT_SPIRAL;
    else if (SP_IS_OFFSET(item)) {
        if (SP_OFFSET(item)->sourceHref)
            object_verb = SP_VERB_SELECTION_LINKED_OFFSET;
        else
            object_verb = SP_VERB_SELECTION_DYNAMIC_OFFSET;
    }

    // for drag, this is done by ungrabbed_handler, but for click we must do it here
    sp_document_done(SP_OBJECT_DOCUMENT(item), object_verb, 
                     _("Change handle"));
}

void
KnotHolder::knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state)
{
    // this was a local change and the knotholder does not need to be recreated:
    this->local_change = TRUE;

    for(std::list<KnotHolderEntity *>::iterator i = this->entity.begin(); i != this->entity.end(); ++i) {
        KnotHolderEntity *e = *i;
        if (e->knot == knot) {
            NR::Point const q = *p / sp_item_i2d_affine(item);
            e->knot_set_func(q, e->knot->drag_origin / sp_item_i2d_affine(item), state);
            break;
        }
    }

    if (SP_IS_SHAPE (item)) {
        sp_shape_set_shape(SP_SHAPE (item));
    }

    this->update_knots();
}

void
KnotHolder::knot_ungrabbed_handler()
{
    if (this->released) {
        this->released(this->item);
    } else {
        SPObject *object = (SPObject *) this->item;
        object->updateRepr(object->repr, SP_OBJECT_WRITE_EXT);

        unsigned int object_verb = SP_VERB_NONE;

        if (SP_IS_RECT(object))
            object_verb = SP_VERB_CONTEXT_RECT;
        else if (SP_IS_BOX3D(object))
            object_verb = SP_VERB_CONTEXT_3DBOX;
        else if (SP_IS_GENERICELLIPSE(object))
            object_verb = SP_VERB_CONTEXT_ARC;
        else if (SP_IS_STAR(object))
            object_verb = SP_VERB_CONTEXT_STAR;
        else if (SP_IS_SPIRAL(object))
            object_verb = SP_VERB_CONTEXT_SPIRAL;
        else if (SP_IS_OFFSET(object)) {
            if (SP_OFFSET(object)->sourceHref)
                object_verb = SP_VERB_SELECTION_LINKED_OFFSET;
            else
                object_verb = SP_VERB_SELECTION_DYNAMIC_OFFSET;
        }
        
        sp_document_done(SP_OBJECT_DOCUMENT (object), object_verb,
                         _("Move handle"));
    }
}

void
KnotHolder::add_pattern_knotholder()
{
    if ((SP_OBJECT(item)->style->fill.isPaintserver())
        && SP_IS_PATTERN(SP_STYLE_FILL_SERVER(SP_OBJECT(item)->style)))
    {
        PatternKnotHolderEntityXY *entity_xy = new PatternKnotHolderEntityXY();
        PatternKnotHolderEntityAngle *entity_angle = new PatternKnotHolderEntityAngle();
        PatternKnotHolderEntityScale *entity_scale = new PatternKnotHolderEntityScale();
        entity_xy->create(desktop, item, this,
                          // TRANSLATORS: This refers to the pattern that's inside the object
                          _("<b>Move</b> the pattern fill inside the object"),
                          SP_KNOT_SHAPE_CROSS);
        entity_angle->create(desktop, item, this,
                             _("<b>Scale</b> the pattern fill uniformly"),
                             SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR);
        entity_scale->create(desktop, item, this,
                             _("<b>Rotate</b> the pattern fill; with <b>Ctrl</b> to snap angle"),
                             SP_KNOT_SHAPE_CIRCLE, SP_KNOT_MODE_XOR);
        entity.push_back(entity_xy);
        entity.push_back(entity_angle);
        entity.push_back(entity_scale);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
