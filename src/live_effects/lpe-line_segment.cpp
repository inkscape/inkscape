#define INKSCAPE_LPE_LINE_SEGMENT_CPP

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

#include <2geom/pathvector.h>
#include <2geom/geom.h>
#include <2geom/bezier-curve.h>

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<EndType> EndTypeData[] = {
    {END_CLOSED       , N_("Closed"), "closed"},
    {END_OPEN_LEFT    , N_("Open left"), "open_left"},
    {END_OPEN_RIGHT   , N_("Open right"), "open_right"},
    {END_OPEN_BOTH    , N_("Open both"), "open_both"},
};
static const Util::EnumDataConverter<EndType> EndTypeConverter(EndTypeData, sizeof(EndTypeData)/sizeof(*EndTypeData));

LPELineSegment::LPELineSegment(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    end_type(_("End type"), _("Determines on which side the line or line segment is infinite."), "end_type", EndTypeConverter, &wr, this, END_OPEN_BOTH)
{
    /* register all your parameters here, so Inkscape knows which parameters this effect has: */
    registerParameter( dynamic_cast<Parameter *>(&end_type) );
}

LPELineSegment::~LPELineSegment()
{

}

void
LPELineSegment::doBeforeEffect (SPLPEItem *lpeitem)
{
    SPDocument *document = SP_OBJECT_DOCUMENT(lpeitem);
    w = sp_document_width(document);
    h = sp_document_height(document);
    
}

std::vector<Geom::Path>
LPELineSegment::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    std::vector<Geom::Path> output;

    A = Geom::initialPoint(path_in);
    B = Geom::finalPoint(path_in);

    Geom::Point E(0,0);
    Geom::Point F(0,h);
    Geom::Point G(w,h);
    Geom::Point H(w,0);

    std::vector<Geom::Point> intersections = Geom::rect_line_intersect(E, G, A, B);

    if (intersections.size() < 2) {
        g_print ("Possible error - no intersection with limiting bounding box.\n");
        return path_in;
    }

    if (end_type == END_OPEN_RIGHT || end_type == END_OPEN_BOTH) {
        A = intersections[0];
    }

    if (end_type == END_OPEN_LEFT || end_type == END_OPEN_BOTH) {
        B = intersections[1];
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
