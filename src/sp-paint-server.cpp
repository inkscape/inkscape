/*
 * Base class for gradients and patterns
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>
#include "libnr/nr-pixblock-pattern.h"
#include "sp-paint-server.h"

#include "sp-gradient.h"
#include "xml/node.h"

static void sp_paint_server_class_init(SPPaintServerClass *psc);
static void sp_paint_server_init(SPPaintServer *ps);

static void sp_paint_server_release(SPObject *object);

static void sp_painter_stale_fill(SPPainter *painter, NRPixBlock *pb);

static SPObjectClass *parent_class;
static GSList *stale_painters = NULL;

GType sp_paint_server_get_type (void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPPaintServerClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) sp_paint_server_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(SPPaintServer),
            16, /* n_preallocs */
            (GInstanceInitFunc) sp_paint_server_init,
            NULL,       /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPPaintServer", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_paint_server_class_init(SPPaintServerClass *psc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) psc;
    sp_object_class->release = sp_paint_server_release;
    parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);
}

static void sp_paint_server_init(SPPaintServer *ps)
{
    ps->painters = NULL;
}

static void sp_paint_server_release(SPObject *object)
{
    SPPaintServer *ps = SP_PAINT_SERVER(object);

    while (ps->painters) {
        SPPainter *painter = ps->painters;
        ps->painters = painter->next;
        stale_painters = g_slist_prepend(stale_painters, painter);
        painter->next = NULL;
        painter->server = NULL;
        painter->fill = sp_painter_stale_fill;
    }

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

SPPainter *sp_paint_server_painter_new(SPPaintServer *ps,
                                       Geom::Matrix const &full_transform,
                                       Geom::Matrix const &parent_transform,
                                       const NRRect *bbox)
{
    g_return_val_if_fail(ps != NULL, NULL);
    g_return_val_if_fail(SP_IS_PAINT_SERVER(ps), NULL);
    g_return_val_if_fail(bbox != NULL, NULL);

    SPPainter *painter = NULL;
    SPPaintServerClass *psc = (SPPaintServerClass *) G_OBJECT_GET_CLASS(ps);
    if ( psc->painter_new ) {
        painter = (*psc->painter_new)(ps, full_transform, parent_transform, bbox);
    }

    if (painter) {
        painter->next = ps->painters;
        painter->server = ps;
        painter->type = (SPPainterType) G_OBJECT_TYPE(ps);
        ps->painters = painter;
    }

    return painter;
}

static void sp_paint_server_painter_free(SPPaintServer *ps, SPPainter *painter)
{
    g_return_if_fail(ps != NULL);
    g_return_if_fail(SP_IS_PAINT_SERVER(ps));
    g_return_if_fail(painter != NULL);

    SPPaintServerClass *psc = (SPPaintServerClass *) G_OBJECT_GET_CLASS(ps);

    SPPainter *r = NULL;
    for (SPPainter *p = ps->painters; p != NULL; p = p->next) {
        if (p == painter) {
            if (r) {
                r->next = p->next;
            } else {
                ps->painters = p->next;
            }
            p->next = NULL;
            if (psc->painter_free) {
                (*psc->painter_free) (ps, painter);
            }
            return;
        }
        r = p;
    }

    g_assert_not_reached();
}

SPPainter *sp_painter_free(SPPainter *painter)
{
    g_return_val_if_fail(painter != NULL, NULL);

    if (painter->server) {
        sp_paint_server_painter_free(painter->server, painter);
    } else {
        SPPaintServerClass *psc = (SPPaintServerClass *) g_type_class_ref(painter->type);
        if (psc->painter_free)
            (*psc->painter_free)(NULL, painter);
        stale_painters = g_slist_remove(stale_painters, painter);
    }

    return NULL;
}

static void sp_painter_stale_fill(SPPainter */*painter*/, NRPixBlock *pb)
{
    nr_pixblock_render_gray_noise(pb, NULL);
}

bool SPPaintServer::isSwatch() const
{
    bool swatch = false;
    if (SP_IS_GRADIENT(this)) {
        SPGradient *grad = SP_GRADIENT(this);
        if ( SP_GRADIENT_HAS_STOPS(grad) ) {
            gchar const * attr = repr->attribute("osb:paint");
            if (attr && !strcmp(attr, "solid")) {
                swatch = true;
            }
        }
    }
    return swatch;
}

bool SPPaintServer::isSolid() const
{
    bool solid = false;
    if (SP_IS_GRADIENT(this)) {
        SPGradient *grad = SP_GRADIENT(this);
        if ( SP_GRADIENT_HAS_STOPS(grad) && (grad->getStopCount() == 0) ) {
            gchar const * attr = repr->attribute("osb:paint");
            if (attr && !strcmp(attr, "solid")) {
                solid = true;
            }
        }
    }
    return solid;
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
