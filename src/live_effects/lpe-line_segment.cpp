/** \file
 * LPE <line_segment> implementation
 */

/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-line_segment.h"
#include "ui/tools/lpe-tool.h"

#include <2geom/pathvector.h>
#include <2geom/geom.h>
#include <2geom/bezier-curve.h>

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<EndType> EndTypeData[] = {
    {END_CLOSED       , N_("Closed"), "closed"},
    {END_OPEN_INITIAL , N_("Open start"), "open_start"},
    {END_OPEN_FINAL   , N_("Open end"), "open_end"},
    {END_OPEN_BOTH    , N_("Open both"), "open_both"},
};
static const Util::EnumDataConverter<EndType> EndTypeConverter(EndTypeData, sizeof(EndTypeData)/sizeof(*EndTypeData));

LPELineSegment::LPELineSegment(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    end_type(_("End type:"), _("Determines on which side the line or line segment is infinite."), "end_type", EndTypeConverter, &wr, this, END_OPEN_BOTH)
{
    /* register all your parameters here, so Inkscape knows which parameters this effect has: */
    registerParameter( dynamic_cast<Parameter *>(&end_type) );
}

LPELineSegment::~LPELineSegment()
{

}

void
LPELineSegment::doBeforeEffect (SPLPEItem const* lpeitem)
{
    Inkscape::UI::Tools::lpetool_get_limiting_bbox_corners(lpeitem->document, bboxA, bboxB);
}

Geom::PathVector
LPELineSegment::doEffect_path (Geom::PathVector const & path_in)
{
    Geom::PathVector output;

    A = path_in.initialPoint();
    B = path_in.finalPoint();

    Geom::Rect dummyRect(bboxA, bboxB);
    boost::optional<Geom::LineSegment> intersection_segment = Geom::Line(A, B).clip(dummyRect);

    if (!intersection_segment) {
        g_print ("Possible error - no intersection with limiting bounding box.\n");
        return path_in;
    }

    if (end_type == END_OPEN_INITIAL || end_type == END_OPEN_BOTH) {
        A = intersection_segment->initialPoint();
    }

    if (end_type == END_OPEN_FINAL || end_type == END_OPEN_BOTH) {
        B = intersection_segment->finalPoint();
    }

    Geom::Path path(A);
    path.appendNew<Geom::LineSegment>(B);

    output.push_back(path);

    return output;
}

} //namespace LivePathEffect
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
