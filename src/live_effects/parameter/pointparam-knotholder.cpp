#define INKSCAPE_LPE_POINTPARAM_KNOTHOLDER_C

/*
 * Container for PointParamKnotHolder visual handles
 *
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/pointparam-knotholder.h"
#include "live_effects/lpeobject.h"
#include "document.h"
#include "sp-shape.h"
#include "knot.h"
#include "knotholder.h"
#include "knot-holder-entity.h"

#include <libnr/nr-matrix-div.h>
#include <glibmm/i18n.h>
#include <2geom/point.h>
#include <2geom/matrix.h>
#include "svg/stringstream.h"
#include "xml/repr.h"

class SPDesktop;

static void pointparam_knot_clicked_handler (SPKnot *knot, guint state, gpointer data);
static void pointparam_knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state, gpointer data);
static void pointparam_knot_ungrabbed_handler (SPKnot *knot, unsigned int state, PointParamKnotHolder *kh);
static void pointparam_knot_holder_class_init(PointParamKnotHolderClass *klass);

void pointparam_knot_holder_dispose(GObject *object);

static SPKnotHolderClass *parent_class;

/**
 * Registers SPKnotHolder class and returns its type number.
 */
GType pointparam_knot_holder_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(PointParamKnotHolderClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) pointparam_knot_holder_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof (PointParamKnotHolder),
            16,	/* n_preallocs */
            NULL,
            NULL
        };
        type = g_type_register_static (G_TYPE_OBJECT, "InkscapePointParamKnotHolder", &info, (GTypeFlags) 0);
    }
    return type;
}

/**
 * PointParamKnotHolder vtable initialization.
 */
static void pointparam_knot_holder_class_init(PointParamKnotHolderClass *klass)
{
    GObjectClass *gobject_class;
    gobject_class = (GObjectClass *) klass;

    parent_class = (SPKnotHolderClass*) g_type_class_peek_parent(klass);
    gobject_class->dispose = pointparam_knot_holder_dispose;
}

PointParamKnotHolder *pointparam_knot_holder_new(SPDesktop *desktop, SPObject *lpeobject, const gchar * key, SPItem *item)
{
    g_return_val_if_fail(desktop != NULL, NULL);
    g_return_val_if_fail(item != NULL, NULL);
    g_return_val_if_fail(SP_IS_ITEM(item), NULL);

    PointParamKnotHolder *knot_holder = (PointParamKnotHolder*)g_object_new (INKSCAPE_TYPE_POINTPARAM_KNOT_HOLDER, 0);
    knot_holder->desktop = desktop;
    knot_holder->item = item;
    knot_holder->lpeobject = LIVEPATHEFFECT(lpeobject);
    g_object_ref(G_OBJECT(item));
    g_object_ref(G_OBJECT(lpeobject));
    knot_holder->entity = NULL;

    knot_holder->released = NULL;

    knot_holder->repr = lpeobject->repr;
    knot_holder->repr_key = key;

    knot_holder->local_change = FALSE;

    return knot_holder;
}

void pointparam_knot_holder_dispose(GObject *object) {
    PointParamKnotHolder *kh = G_TYPE_CHECK_INSTANCE_CAST((object), INKSCAPE_TYPE_POINTPARAM_KNOT_HOLDER, PointParamKnotHolder);

    g_object_unref(G_OBJECT(kh->item));
    g_object_unref(G_OBJECT(kh->lpeobject));
    while (kh->entity) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) kh->entity->data;
        g_signal_handler_disconnect(e->knot, e->_click_handler_id);
        g_signal_handler_disconnect(e->knot, e->_ungrab_handler_id);
        /* unref should call destroy */
        g_object_unref(e->knot);
        g_free(e);
        kh->entity = g_slist_remove(kh->entity, e);
    }
}

void pointparam_knot_holder_destroy(PointParamKnotHolder *kh) {
    g_object_unref(kh);
}

void pointparam_knot_holder_add_full(
    PointParamKnotHolder *knot_holder,
    Geom::Point         & p,
    void (* knot_click) (SPItem *item, guint state),
    SPKnotShapeType     shape,
    SPKnotModeType      mode,
    guint32             color,
    const gchar *tip )
{
    g_return_if_fail(knot_holder != NULL);

    SPItem *item = SP_ITEM(knot_holder->item);

    /* create new SPKnotHolderEntry */
    SPKnotHolderEntity *e = g_new(SPKnotHolderEntity, 1);
    e->knot = sp_knot_new(knot_holder->desktop, tip);
    e->knot_set = NULL;
    e->knot_get = NULL;
    if (knot_click) {
        e->knot_click = knot_click;
    } else {
        e->knot_click = NULL;
    }

    g_object_set(G_OBJECT (e->knot->item), "shape", shape, NULL);
    g_object_set(G_OBJECT (e->knot->item), "mode", mode, NULL);

    e->knot->fill [SP_KNOT_STATE_NORMAL] = color;
    g_object_set (G_OBJECT (e->knot->item), "fill_color", color, NULL);

    knot_holder->entity = g_slist_append(knot_holder->entity, e);

    /* Move to current point. */
    NR::Point dp = p * sp_item_i2d_affine(item);
    sp_knot_set_position(e->knot, &dp, SP_KNOT_STATE_NORMAL);

    e->handler_id = g_signal_connect(e->knot, "moved", G_CALLBACK(pointparam_knot_moved_handler), knot_holder);
    e->_click_handler_id = g_signal_connect(e->knot, "clicked", G_CALLBACK(pointparam_knot_clicked_handler), knot_holder);
    e->_ungrab_handler_id = g_signal_connect(e->knot, "ungrabbed", G_CALLBACK(pointparam_knot_ungrabbed_handler), knot_holder);

    sp_knot_show(e->knot);
}

/**
 * \param p In desktop coordinates.
 */

 // don't write to XML yet...
//static
void pointparam_knotholder_update_knots(PointParamKnotHolder *knot_holder, SPItem *item)
{

    NR::Matrix const i2d(sp_item_i2d_affine(item));
/*
    for (GSList *el = knot_holder->entity; el; el = el->next) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) el->data;
        GObject *kob = e->knot;

        NR::Point dp( e->knot_get(item) * i2d );
        g_signal_handler_block(kob, e->handler_id);
        sp_knot_set_position(e->knot, &dp, SP_KNOT_STATE_NORMAL);
        g_signal_handler_unblock(kob, e->handler_id);
    }
    
    knot_holder->lpeobject->lpe->setParameter(np->repr_key, svgpath);

    knot_holder->lpeobject->requestModified(SP_OBJECT_MODIFIED_FLAG);
*/
}

static void pointparam_knot_clicked_handler(SPKnot *knot, guint state, gpointer data)
{

    SPKnotHolder *knot_holder = (SPKnotHolder *) data;
    SPItem *item  = SP_ITEM (knot_holder->item);
/*
    g_object_ref(knot_holder);
    for (GSList *el = knot_holder->entity; el; el = el->next) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) el->data;
        if (e->knot == knot) {
            if (e->knot_click) {
                e->knot_click(item, state);
            }
            break;
        }
    }

    if (SP_IS_SHAPE(item)) {
        sp_shape_set_shape(SP_SHAPE(item));
    }

    knotholder_update_knots(knot_holder, item);
    g_object_unref(knot_holder);

    // for drag, this is done by ungrabbed_handler, but for click we must do it here
    sp_document_done(SP_OBJECT_DOCUMENT(knot_holder->item), SP_VERB_CONTEXT_LPE, 
                     _("Change LPE point parameter"));
*/
}

static void pointparam_knot_moved_handler(SPKnot *knot, NR::Point const *p, guint state, gpointer data)
{

    SPKnotHolder *knot_holder = (SPKnotHolder *) data;
    SPItem *item  = SP_ITEM (knot_holder->item);
/*    // this was a local change and the knotholder does not need to be recreated:
    knot_holder->local_change = TRUE;

    for (GSList *el = knot_holder->entity; el; el = el->next) {
        SPKnotHolderEntity *e = (SPKnotHolderEntity *) el->data;
        if (e->knot == knot) {
            NR::Point const q = *p / sp_item_i2d_affine(item);
            e->knot_set(item, q, e->knot->drag_origin / sp_item_i2d_affine(item), state);
            break;
        }
    }

    if (SP_IS_SHAPE (item)) {
        sp_shape_set_shape(SP_SHAPE (item));
    }

    knotholder_update_knots(knot_holder, item);
*/
}

static void pointparam_knot_ungrabbed_handler(SPKnot *knot, unsigned int /*state*/, PointParamKnotHolder *kh)
{
    NR::Matrix const i2d(sp_item_i2d_affine(kh->item));
    NR::Point pos = sp_knot_position(knot) / i2d;

    Inkscape::SVGOStringStream os;
    os << pos[0] << "," << pos[1];

    kh->repr->setAttribute(kh->repr_key , os.str().c_str());

    sp_document_done(SP_OBJECT_DOCUMENT (kh->lpeobject), SP_VERB_CONTEXT_LPE, _("Change LPE point parameter"));
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
