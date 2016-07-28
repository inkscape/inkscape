/*
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-bounding-box.h"

#include "display/curve.h"
#include "sp-item.h"
#include "2geom/path.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "2geom/bezier-curve.h"
#include "lpe-bounding-box.h"

namespace Inkscape {
namespace LivePathEffect {

LPEBoundingBox::LPEBoundingBox(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    linked_path(_("Linked path:"), _("Path from which to take the original path data"), "linkedpath", &wr, this),
    visual_bounds(_("Visual Bounds"), _("Uses the visual bounding box"), "visualbounds", &wr, this)
{
    registerParameter( dynamic_cast<Parameter *>(&linked_path) );
    registerParameter( dynamic_cast<Parameter *>(&visual_bounds) );
    //perceived_path = true;
}

LPEBoundingBox::~LPEBoundingBox()
{

}

void LPEBoundingBox::doEffect (SPCurve * curve)
{
    if (curve) {
        if ( linked_path.linksToPath() && linked_path.getObject() ) {
            SPItem * item = linked_path.getObject();
            Geom::OptRect bbox = visual_bounds.get_value() ? item->visualBounds() : item->geometricBounds();
            Geom::Path p(Geom::Point(bbox->left(), bbox->top()));
            p.appendNew<Geom::LineSegment>(Geom::Point(bbox->right(), bbox->top()));
            p.appendNew<Geom::LineSegment>(Geom::Point(bbox->right(), bbox->bottom()));
            p.appendNew<Geom::LineSegment>(Geom::Point(bbox->left(), bbox->bottom()));
            p.appendNew<Geom::LineSegment>(Geom::Point(bbox->left(), bbox->top()));
            Geom::PathVector out;
            out.push_back(p);
            curve->set_pathvector(out);
        }
    }
}

} // namespace LivePathEffect
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
