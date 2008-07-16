#include "libnr/nr-point-matrix-ops.h"
#include "livarot/path-description.h"

PathDescr *PathDescrMoveTo::clone() const
{
    return new PathDescrMoveTo(*this);
}

void PathDescrMoveTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &/*last*/) const
{
    s << "M " << p[NR::X] << " " << p[NR::Y] << " ";
}

void PathDescrMoveTo::transform(NR::Matrix const& t)
{
    p = p * t;
}

void PathDescrMoveTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  m " << p[NR::X] << " " << p[NR::Y];
}

void PathDescrLineTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &/*last*/) const
{
    s << "L " << p[NR::X] << " " << p[NR::Y] << " ";
}

PathDescr *PathDescrLineTo::clone() const
{
    return new PathDescrLineTo(*this);
}

void PathDescrLineTo::transform(NR::Matrix const& t)
{
    p = p * t;
}

void PathDescrLineTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  l " << p[NR::X] << " " << p[NR::Y];
}

PathDescr *PathDescrBezierTo::clone() const
{
    return new PathDescrBezierTo(*this);
}

void PathDescrBezierTo::transform(NR::Matrix const& t)
{
    p = p * t;
}

void PathDescrBezierTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  b " << p[NR::X] << " " << p[NR::Y] << " " << nb;
}

PathDescr *PathDescrIntermBezierTo::clone() const
{
    return new PathDescrIntermBezierTo(*this);
}

void PathDescrIntermBezierTo::transform(NR::Matrix const& t)
{
    p = p * t;
}

void PathDescrIntermBezierTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  i " << p[NR::X] << " " << p[NR::Y];
}

void PathDescrCubicTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &last) const
{
    s << "C "
      << last[NR::X] + start[0] / 3 << " "
      << last[NR::Y] + start[1] / 3 << " "
      << p[NR::X] - end[0] / 3 << " "
      << p[NR::Y] - end[1] / 3 << " "
      << p[NR::X] << " "
      << p[NR::Y] << " ";
}

PathDescr *PathDescrCubicTo::clone() const
{
    return new PathDescrCubicTo(*this);
}

void PathDescrCubicTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  c "
      << p[NR::X] << " " << p[NR::Y] << " "
      << start[NR::X] << " " << start[NR::Y] << " "
      << end[NR::X] << " " << end[NR::Y] << " ";
}

void PathDescrCubicTo::transform(NR::Matrix const& t)
{
    NR::Matrix tr = t;
    tr[4] = tr[5] = 0;
    start = start * tr;
    end = end * tr;
    
    p = p * t;
}

void PathDescrArcTo::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &/*last*/) const
{
    s << "A "
      << rx << " "
      << ry << " "
      << angle << " "
      << (large ? "1" : "0") << " "
      << (clockwise ? "0" : "1") << " "
      << p[NR::X] << " "
      << p[NR::Y] << " ";
}

PathDescr *PathDescrArcTo::clone() const
{
    return new PathDescrArcTo(*this);
}

void PathDescrArcTo::transform(NR::Matrix const& t)
{
    p = p * t;
}

void PathDescrArcTo::dump(std::ostream &s) const
{
    /* localizing ok */
    s << "  a "
      << p[NR::X] << " " << p[NR::Y] << " "
      << rx << " " << ry << " "
      << angle << " "
      << (clockwise ? 1 : 0) << " "
      << (large ? 1 : 0);
}

PathDescr *PathDescrForced::clone() const
{
    return new PathDescrForced(*this);
}

void PathDescrClose::dumpSVG(Inkscape::SVGOStringStream& s, NR::Point const &/*last*/) const
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
