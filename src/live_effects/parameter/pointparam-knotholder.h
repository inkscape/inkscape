#ifndef INKSCAPE_LPE_POINTPARAM_KNOTHOLDER_H
#define INKSCAPE_LPE_POINTPARAM_KNOTHOLDER_H

/*
 * PointParamKnotHolder - Hold SPKnot list and manage signals for LPE PointParam
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 *
 */

#include "knotholder.h"
#include <glib/gtypes.h>
#include "knot-enums.h"
#include "forward.h"
#include "libnr/nr-forward.h"
#include <2geom/point.h>
#include "live_effects/lpeobject.h"

namespace Inkscape {
namespace XML {
class Node;
}



typedef void (* PointParamKnotHolderSetFunc) (SPItem *item, Geom::Point const &p, Geom::Point const &origin, guint state);
typedef Geom::Point (* PointParamKnotHolderGetFunc) (SPItem *item);
typedef void (* PointParamKnotHolderClickedFunc) (SPItem *item, guint state);

class PointParamKnotHolder : public KnotHolder {
public:
    PointParamKnotHolder(SPDesktop *desktop, SPObject *lpeobject, const gchar * key, SPItem *item);
    ~PointParamKnotHolder();

    LivePathEffectObject * lpeobject;
    Inkscape::XML::Node  * repr;
    const gchar          * repr_key;

    void add_knot ( Geom::Point         & p,
                    PointParamKnotHolderClickedFunc knot_click,
                    SPKnotShapeType     shape,
                    SPKnotModeType      mode,
                    guint32             color,
                    const gchar *tip );
};

} // namespace Inkscape


#endif /* INKSCAPE_LPE_POINTPARAM_KNOTHOLDER_H */

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
