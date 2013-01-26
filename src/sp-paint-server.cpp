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
#include "sp-paint-server-reference.h"
#include "sp-paint-server.h"

#include "sp-gradient.h"
#include "xml/node.h"

static cairo_pattern_t *sp_paint_server_create_dummy_pattern(SPPaintServer *ps, cairo_t *ct, Geom::OptRect const &bbox, double opacity);

SPPaintServer *SPPaintServerReference::getObject() const
{
    return static_cast<SPPaintServer *>(URIReference::getObject());
}

bool SPPaintServerReference::_acceptObject(SPObject *obj) const
{
    return SP_IS_PAINT_SERVER(obj);
}

G_DEFINE_TYPE(SPPaintServer, sp_paint_server, SP_TYPE_OBJECT);

static void sp_paint_server_class_init(SPPaintServerClass *psc)
{
    psc->pattern_new = sp_paint_server_create_dummy_pattern;
}

static void
sp_paint_server_init(SPPaintServer * /*ps*/)
{
}

cairo_pattern_t *sp_paint_server_create_pattern(SPPaintServer *ps,
                                                cairo_t *ct,
                                                Geom::OptRect const &bbox,
                                                double opacity)
{
    g_return_val_if_fail(ps != NULL, NULL);
    g_return_val_if_fail(SP_IS_PAINT_SERVER(ps), NULL);

    cairo_pattern_t *cp = NULL;
    SPPaintServerClass *psc = (SPPaintServerClass *) G_OBJECT_GET_CLASS(ps);
    if ( psc->pattern_new ) {
        cp = (*psc->pattern_new)(ps, ct, bbox, opacity);
    }

    return cp;
}

static cairo_pattern_t *
sp_paint_server_create_dummy_pattern(SPPaintServer */*ps*/,
                                     cairo_t */* ct */,
                                     Geom::OptRect const &/*bbox*/,
                                     double /* opacity */)
{
    cairo_pattern_t *cp = cairo_pattern_create_rgb(1.0, 0.0, 1.0);
    return cp;
}

bool SPPaintServer::isSwatch() const
{
    return swatch;
}

bool SPPaintServer::isSolid() const
{
    bool solid = false;
    if (swatch && SP_IS_GRADIENT(this)) {
        SPGradient *grad = SP_GRADIENT(this);
        if ( grad->hasStops() && (grad->getStopCount() == 0) ) {
            solid = true;
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
