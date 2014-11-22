#ifndef SEEN_SP_PAINT_SERVER_REFERENCE_H
#define SEEN_SP_PAINT_SERVER_REFERENCE_H

/*
 * Reference class for gradients and patterns.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "uri-references.h"

class SPDocument;
class SPObject;
class SPPaintServer;

class SPPaintServerReference : public Inkscape::URIReference {
public:
    SPPaintServerReference (SPObject *obj) : URIReference(obj) {}
    SPPaintServerReference (SPDocument *doc) : URIReference(doc) {}
    SPPaintServer *getObject() const;

protected:
    virtual bool _acceptObject(SPObject *obj) const;
};

#endif // SEEN_SP_PAINT_SERVER_REFERENCE_H
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
