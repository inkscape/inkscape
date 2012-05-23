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
#include "sp-mesh-patch.h"
#include "style.h"

SPMeshPatch* SPMeshPatch::getNextMeshPatch()
{
    SPMeshPatch *result = 0;

    for (SPObject* obj = getNext(); obj && !result; obj = obj->getNext()) {
        if (SP_IS_MESHPATCH(obj)) {
            result = SP_MESHPATCH(obj);
        }
    }

    return result;
}

SPMeshPatch* SPMeshPatch::getPrevMeshPatch()
{
    SPMeshPatch *result = 0;

    for (SPObject* obj = getPrev(); obj; obj = obj->getPrev()) {
        // The closest previous SPObject that is an SPMeshPatch *should* be ourself.
        if (SP_IS_MESHPATCH(obj)) {
            SPMeshPatch* meshpatch = SP_MESHPATCH(obj);
            // Sanity check to ensure we have a proper sibling structure.
            if (meshpatch->getNextMeshPatch() == this) {
                result = meshpatch;
            } else {
                g_warning("SPMeshPatch previous/next relationship broken");
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
