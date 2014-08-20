#include "livarot/path-description.h"
#include <2geom/affine.h>

PathDescr *PathDescrMoveTo::clone() const
{
    return new PathDescrMoveTo(*this);
}

void PathDescrMoveTo::dumpSVG(Inkscape::SVGOStringStream& s, Geom::Point const &/*last*/) const
{
    s << "M " << p[Geom::X] << " " << p[Geom::Y] << " ";
}

void PathDescrMoveTo::transform(Geom::Affine const& t)
{
    p = p * t;
}

void PathDescrMoveTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  m " << p[Geom::X] << " " << p[Geom::Y];
}

void PathDescrLineTo::dumpSVG(Inkscape::SVGOStringStream& s, Geom::Point const &/*last*/) const
{
    s << "L " << p[Geom::X] << " " << p[Geom::Y] << " ";
}

PathDescr *PathDescrLineTo::clone() const
{
    return new PathDescrLineTo(*this);
}

void PathDescrLineTo::transform(Geom::Affine const& t)
{
    p = p * t;
}

void PathDescrLineTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  l " << p[Geom::X] << " " << p[Geom::Y];
}

PathDescr *PathDescrBezierTo::clone() const
{
    return new PathDescrBezierTo(*this);
}

void PathDescrBezierTo::transform(Geom::Affine const& t)
{
    p = p * t;
}

void PathDescrBezierTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  b " << p[Geom::X] << " " << p[Geom::Y] << " " << nb;
}

PathDescr *PathDescrIntermBezierTo::clone() const
{
    return new PathDescrIntermBezierTo(*this);
}

void PathDescrIntermBezierTo::transform(Geom::Affine const& t)
{
    p = p * t;
}

void PathDescrIntermBezierTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  i " << p[Geom::X] << " " << p[Geom::Y];
}

void PathDescrCubicTo::dumpSVG(Inkscape::SVGOStringStream& s, Geom::Point const &last) const
{
    s << "C "
      << last[Geom::X] + start[0] / 3 << " "
      << last[Geom::Y] + start[1] / 3 << " "
      << p[Geom::X] - end[0] / 3 << " "
      << p[Geom::Y] - end[1] / 3 << " "
      << p[Geom::X] << " "
      << p[Geom::Y] << " ";
}

PathDescr *PathDescrCubicTo::clone() const
{
    return new PathDescrCubicTo(*this);
}

void PathDescrCubicTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  c "
      << p[Geom::X] << " " << p[Geom::Y] << " "
      << start[Geom::X] << " " << start[Geom::Y] << " "
      << end[Geom::X] << " " << end[Geom::Y] << " ";
}

void PathDescrCubicTo::transform(Geom::Affine const& t)
{
    Geom::Affine tr = t;
    tr[4] = tr[5] = 0;
    start = start * tr;
    end = end * tr;
    
    p = p * t;
}

void PathDescrArcTo::dumpSVG(Inkscape::SVGOStringStream& s, Geom::Point const &/*last*/) const
{
    s << "A "
      << rx << " "
      << ry << " "
      << angle << " "
      << (large ? "1" : "0") << " "
      << (clockwise ? "0" : "1") << " "
      << p[Geom::X] << " "
      << p[Geom::Y] << " ";
}

PathDescr *PathDescrArcTo::clone() const
{
    return new PathDescrArcTo(*this);
}

void PathDescrArcTo::transform(Geom::Affine const& t)
{
    p = p * t;
}

void PathDescrArcTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  a "
      << p[Geom::X] << " " << p[Geom::Y] << " "
      << rx << " " << ry << " "
      << angle << " "
      << (clockwise ? 1 : 0) << " "
      << (large ? 1 : 0);
}

PathDescr *PathDescrForced::clone() const
{
    return new PathDescrForced(*this);
}

void PathDescrClose::dumpSVG(Inkscape::SVGOStringStream& s, Geom::Point const &/*last*/) const
{
    s << "z ";
}

PathDescr *PathDescrClose::clone() const
{
    return new PathDescrClose(*this);
}


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
