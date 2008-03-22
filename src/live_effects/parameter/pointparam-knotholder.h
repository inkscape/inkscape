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



typedef void (* PointParamKnotHolderSetFunc) (SPItem *item, NR::Point const &p, NR::Point const &origin, guint state);
typedef NR::Point (* PointParamKnotHolderGetFunc) (SPItem *item);
typedef void (* PointParamKnotHolderClickedFunc) (SPItem *item, guint state);

class PointParamKnotHolder : public SPKnotHolder {
public:
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

struct PointParamKnotHolderClass : SPKnotHolderClass {
};

PointParamKnotHolder *pointparam_knot_holder_new(SPDesktop *desktop, SPObject *lpeobject, const gchar * key, SPItem *item);

GType pointparam_knot_holder_get_type();


#define INKSCAPE_TYPE_POINTPARAM_KNOT_HOLDER      (Inkscape::pointparam_knot_holder_get_type())


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
