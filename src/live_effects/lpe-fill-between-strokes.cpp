/*
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-fill-between-strokes.h"

#include "display/curve.h"
#include "sp-item.h"
#include "2geom/path.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "2geom/bezier-curve.h"

namespace Inkscape {
namespace LivePathEffect {

LPEFillBetweenStrokes::LPEFillBetweenStrokes(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    linked_path(_("Linked path:"), _("Path from which to take the original path data"), "linkedpath", &wr, this),
    second_path(_("Second path:"), _("Second path from which to take the original path data"), "secondpath", &wr, this),
    reverse_second(_("Reverse Second"), _("Reverses the second path order"), "reversesecond", &wr, this)
{
    registerParameter( dynamic_cast<Parameter *>(&linked_path) );
    registerParameter( dynamic_cast<Parameter *>(&second_path) );
    registerParameter( dynamic_cast<Parameter *>(&reverse_second) );
    //perceived_path = true;
}

LPEFillBetweenStrokes::~LPEFillBetweenStrokes()
{

}

void LPEFillBetweenStrokes::doEffect (SPCurve * curve)
{
    if (curve) {
        if ( linked_path.linksToPath() && second_path.linksToPath() && linked_path.getObject() && second_path.getObject() ) {
            Geom::PathVector linked_pathv = linked_path.get_pathvector();
            Geom::PathVector second_pathv = second_path.get_pathvector();
            Geom::PathVector result_linked_pathv;
            Geom::PathVector result_second_pathv;
            Geom::Affine second_transform = second_path.getObject()->getRelativeTransform(linked_path.getObject());

            for (Geom::PathVector::iterator iter = linked_pathv.begin(); iter != linked_pathv.end(); ++iter)
            {
                result_linked_pathv.push_back((*iter));
            }
            for (Geom::PathVector::iterator iter = second_pathv.begin(); iter != second_pathv.end(); ++iter)
            {
                result_second_pathv.push_back((*iter) * second_transform);
            }

            if ( !result_linked_pathv.empty() && !result_second_pathv.empty() && !result_linked_pathv.front().closed() ) {
                if (reverse_second.get_value())
                {
                    result_linked_pathv.front().appendNew<Geom::LineSegment>(result_second_pathv.front().finalPoint());
                    result_linked_pathv.front().append(result_second_pathv.front().reversed());
                }
                else
                {
                    result_linked_pathv.front().appendNew<Geom::LineSegment>(result_second_pathv.front().initialPoint());
                    result_linked_pathv.front().append(result_second_pathv.front());
                }
                curve->set_pathvector(result_linked_pathv);
            }
            else if ( !result_linked_pathv.empty() ) {
                curve->set_pathvector(result_linked_pathv);
            }
            else if ( !result_second_pathv.empty() ) {
                curve->set_pathvector(result_second_pathv);
            }
        }
        else if ( linked_path.linksToPath() && linked_path.getObject() ) {
            Geom::PathVector linked_pathv = linked_path.get_pathvector();
            Geom::PathVector result_pathv;

            for (Geom::PathVector::iterator iter = linked_pathv.begin(); iter != linked_pathv.end(); ++iter)
            {
                result_pathv.push_back((*iter));
            }
            if ( !result_pathv.empty() ) {
                curve->set_pathvector(result_pathv);
            }
        }
        else if ( second_path.linksToPath() && second_path.getObject() ) {
            Geom::PathVector second_pathv = second_path.get_pathvector();
            Geom::PathVector result_pathv;

            for (Geom::PathVector::iterator iter = second_pathv.begin(); iter != second_pathv.end(); ++iter)
            {
                result_pathv.push_back((*iter));
            }
            if ( !result_pathv.empty() ) {
                curve->set_pathvector(result_pathv);
            }
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
