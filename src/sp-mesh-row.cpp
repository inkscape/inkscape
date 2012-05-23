/** @file
 * @gradient meshpatch class.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Tavmjong Bah <tavjong@free.fr>
 *
 * Copyright (C) 1999,2005 authors
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "sp-mesh-row.h"
#include "style.h"

SPMeshRow* SPMeshRow::getNextMeshRow()
{
    SPMeshRow *result = 0;

    for (SPObject* obj = getNext(); obj && !result; obj = obj->getNext()) {
        if (SP_IS_MESHROW(obj)) {
            result = SP_MESHROW(obj);
        }
    }

    return result;
}

SPMeshRow* SPMeshRow::getPrevMeshRow()
{
    SPMeshRow *result = 0;

    for (SPObject* obj = getPrev(); obj; obj = obj->getPrev()) {
        // The closest previous SPObject that is an SPMeshRow *should* be ourself.
        if (SP_IS_MESHROW(obj)) {
            SPMeshRow* meshrow = SP_MESHROW(obj);
            // Sanity check to ensure we have a proper sibling structure.
            if (meshrow->getNextMeshRow() == this) {
                result = meshrow;
            } else {
                g_warning("SPMeshRow previous/next relationship broken");
            }
            break;
        }
    }

    return result;
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
