#ifndef SEEN_SP_STOP_H
#define SEEN_SP_STOP_H

/** \file
 * SPStop: SVG <stop> implementation.
 */
/*
 * Authors:
 */

#include "sp-object.h"
#include "color.h"

typedef unsigned int guint32;

namespace Glib {
class ustring;
}

#define SP_STOP(obj) (dynamic_cast<SPStop*>((SPObject*)obj))
#define SP_IS_STOP(obj) (dynamic_cast<const SPStop*>((SPObject*)obj) != NULL)

/** Gradient stop. */
class SPStop : public SPObject {
public:
	SPStop();
	virtual ~SPStop();

    /// \todo fixme: Should be SPSVGPercentage
    float offset;

    bool currentColor;

    /** \note
     * N.B.\ Meaningless if currentColor is true.  Use sp_stop_get_rgba32 or sp_stop_get_color
     * (currently static in sp-gradient.cpp) if you want the effective color.
     */
    SPColor specified_color;

    /// \todo fixme: Implement SPSVGNumber or something similar.
    float opacity;

    Glib::ustring * path_string;
    //SPCurve path;

    static SPColor readStopColor( Glib::ustring const &styleStr, guint32 dfl = 0 );

    SPStop* getNextStop();
    SPStop* getPrevStop();

    SPColor getEffectiveColor() const;

    guint32 get_rgba32() const;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void set(unsigned int key, const char* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
