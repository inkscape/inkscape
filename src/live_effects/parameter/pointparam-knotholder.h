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
}


typedef void (* SPKnotHolderSetFunc) (SPItem *item, NR::Point const &p, NR::Point const &origin, guint state);
typedef NR::Point (* SPKnotHolderGetFunc) (SPItem *item);
/* fixme: Think how to make callbacks most sensitive (Lauris) */
typedef void (* SPKnotHolderReleasedFunc) (SPItem *item);

struct PointParamKnotHolder : public SPKnotHolder {
    LivePathEffectObject * lpeobject;
    Inkscape::XML::Node  * repr;
    const gchar          * repr_key;
};

struct PointParamKnotHolderClass : SPKnotHolderClass {
};

/* fixme: As a temporary solution, if released is NULL knotholder flushes undo itself (Lauris) */
PointParamKnotHolder *pointparam_knot_holder_new(SPDesktop *desktop, SPObject *lpeobject, const gchar * key, SPItem *item);

void pointparam_knot_holder_destroy(PointParamKnotHolder *knots);

void pointparam_knot_holder_add_full(PointParamKnotHolder *knot_holder,
                             Geom::Point &p,
                             void (* knot_click) (SPItem *item, guint state),
                             SPKnotShapeType shape,
                             SPKnotModeType mode,
                             guint32 color,
                             gchar const *tip);

GType pointparam_knot_holder_get_type();

// FIXME: see knotholder.h
void pointparam_knotholder_update_knots(SPKnotHolder *knot_holder, SPItem *item);

#define INKSCAPE_TYPE_POINTPARAM_KNOT_HOLDER      (pointparam_knot_holder_get_type())

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
