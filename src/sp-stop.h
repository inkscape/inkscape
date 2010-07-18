#ifndef SEEN_SP_STOP_H
#define SEEN_SP_STOP_H

/** \file
 * SPStop: SVG <stop> implementation.
 */
/*
 * Authors?
 */

#include <glib/gtypes.h>
#include <glibmm/ustring.h>
#include "sp-object.h"
#include "color.h"

class SPObjectClass;
class SPColor;

struct SPStop;
struct SPStopClass;

#define SP_TYPE_STOP (sp_stop_get_type())
#define SP_STOP(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_STOP, SPStop))
#define SP_STOP_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_STOP, SPStopClass))
#define SP_IS_STOP(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_STOP))
#define SP_IS_STOP_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_STOP))

GType sp_stop_get_type();

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


    static SPColor readStopColor( Glib::ustring const &styleStr, guint32 dfl = 0 );

    SPStop* getNextStop();
    SPStop* getPrevStop();

    SPColor getEffectiveColor() const;
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
