#define INKSCAPE_LPE_BOOLOPS_CPP
/** \file
 * LPE boolops implementation
 */
/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-boolops.h"

#include <2geom/path.h>
#include <2geom/shape.h>

namespace Inkscape {
namespace LivePathEffect {

static const Util::EnumData<unsigned> BoolopTypeData[] = {
    {Geom::BOOLOP_NULL          , N_("Null"), "null"},
    {Geom::BOOLOP_INTERSECT     , N_("Intersect"), "intersect"},
    {Geom::BOOLOP_SUBTRACT_A_B  , N_("Subtract A-B"), "subtract_a_b"},
    {Geom::BOOLOP_IDENTITY_A    , N_("Identity A"), "identity_a"},
    {Geom::BOOLOP_SUBTRACT_B_A  , N_("Subtract B-A"), "subtract_b_a"},
    {Geom::BOOLOP_IDENTITY_B    , N_("Identity B"), "identity_b"},
    {Geom::BOOLOP_EXCLUSION     , N_("Exclusion"), "Exclusion"},
    {Geom::BOOLOP_UNION         , N_("Union"), "Union"}
};
static const Util::EnumDataConverter<unsigned> BoolopTypeConverter(BoolopTypeData, sizeof(BoolopTypeData)/sizeof(*BoolopTypeData));

LPEBoolops::LPEBoolops(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    bool_path(_("2nd path"), _("Path to which the original path will be boolop'ed."), "path_2nd", &wr, this, "M0,0 L1,0"),
    boolop_type(_("Boolop type"), _("Determines which kind of boolop will be performed."), "boolop_type", BoolopTypeConverter, &wr, this, Geom::BOOLOP_UNION)
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&boolop_type) );
    registerParameter( dynamic_cast<Parameter *>(&bool_path) );
}

LPEBoolops::~LPEBoolops()
{

}


Geom::PathVector
LPEBoolops::doEffect_path (Geom::PathVector const & path_in)
{
    std::vector<Geom::Path> path_out;

    Geom::Shape shape_in = Geom::sanitize(path_in);

    Geom::Shape shape_param = Geom::sanitize(bool_path.get_pathvector());

    Geom::Shape shape_out = Geom::boolop(shape_in, shape_param, boolop_type.get_value());

    path_out = Geom::desanitize(shape_out);

    return path_out;
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
