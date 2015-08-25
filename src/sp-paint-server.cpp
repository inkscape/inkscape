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

SPPaintServer *SPPaintServerReference::getObject() const
{
    return static_cast<SPPaintServer *>(URIReference::getObject());
}

bool SPPaintServerReference::_acceptObject(SPObject *obj) const
{
    return SP_IS_PAINT_SERVER(obj) && URIReference::_acceptObject(obj);
}

SPPaintServer::SPPaintServer() : SPObject() {
	this->swatch = 0;
}

SPPaintServer::~SPPaintServer() {
}

bool SPPaintServer::isSwatch() const
{
    if( this ) // Protect against assumption that "vector" always exists.
        return swatch;
    return( false );
}


// TODO: So a solid brush is a gradient with a swatch and zero stops?
// Should we derive a new class for that? Or at least make this method
// virtual and move it out of the way?
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

bool SPPaintServer::isValid() const
{
    return true;
}

Inkscape::DrawingPattern *SPPaintServer::show(Inkscape::Drawing &/*drawing*/, unsigned int /*key*/, Geom::OptRect /*bbox*/)
{
    return NULL;
}

void SPPaintServer::hide(unsigned int /*key*/)
{
}

void SPPaintServer::setBBox(unsigned int /*key*/, Geom::OptRect const &/*bbox*/)
{
}

cairo_pattern_t* SPPaintServer::pattern_new(cairo_t * /*ct*/, Geom::OptRect const &/*bbox*/, double /*opacity*/)
{
    return NULL;
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
