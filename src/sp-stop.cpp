/** @file
 * @gradient stop class.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999,2005 authors
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "sp-stop.h"


// A stop might have some non-stop siblings
SPStop* SPStop::getNextStop()
{
    SPStop *result = 0;

    for (SPObject* obj = getNext(); obj && !result; obj = obj->getNext()) {
        if (SP_IS_STOP(obj)) {
            result = SP_STOP(obj);
        }
    }

    return result;
}

SPStop* SPStop::getPrevStop()
{
    SPStop *result = 0;

    for (SPObject* obj = getPrev(); obj; obj = obj->getPrev()) {
        // The closest previous SPObject that is an SPStop *should* be ourself.
        if (SP_IS_STOP(obj)) {
            SPStop* stop = SP_STOP(obj);
            // Sanity check to ensure we have a proper sibling structure.
            if (stop->getNextStop() == this) {
                result = stop;
            } else {
                g_warning("SPStop previous/next relationship broken");
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
