#ifndef SEEN_SP_STOP_H
#define SEEN_SP_STOP_H

/** \file
 * SPStop: SVG <stop> implementation.
 */
/*
 * Authors?
 */

#include <glib/gtypes.h>
//#include <glib-object.h>
#include "sp-object.h"
#include "color.h"
#include "sp-stop-fns.h"

class SPObjectClass;
class SPColor;

/** Gradient stop. */
struct SPStop : public SPObject {
    /// \todo fixme: Should be SPSVGPercentage
    gfloat offset;

    bool currentColor;

    /** \note
     * N.B.\ Meaningless if currentColor is true.  Use sp_stop_get_rgba32 or sp_stop_get_color
     * (currently static in sp-gradient.cpp) if you want the effective color.
     */
    SPColor specified_color;

    /// \todo fixme: Implement SPSVGNumber or something similar.
    gfloat opacity;
};

/// The SPStop vtable.
struct SPStopClass {
    SPObjectClass parent_class;
};

guint32 sp_stop_get_rgba32(SPStop const *);


#endif /* !SEEN_SP_STOP_H */

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
