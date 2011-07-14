#ifndef SEEN_INKSCAPE_LIVAROT_PATH_DESCRIPTION_H
#define SEEN_INKSCAPE_LIVAROT_PATH_DESCRIPTION_H

#include <2geom/point.h>
#include "svg/stringstream.h"

// path description commands
/* FIXME: these should be unnecessary once the refactoring of the path
** description stuff is finished.
*/
enum
{
  descr_moveto = 0,         // a moveto
  descr_lineto = 1,         // a (guess what) lineto
  descr_cubicto = 2,
  descr_bezierto = 3,       // "beginning" of a quadratic bezier spline, will contain its endpoint (i know, it's bad...)
  descr_arcto = 4,
  descr_close = 5,
  descr_interm_bezier = 6,  // control point of the bezier spline
  descr_forced = 7,

  descr_type_mask = 15      // the command no will be stored in a "flags" field, potentially with other info, so we need
                            // a mask to AND the field and extract the command
};

struct PathDescr
{
  PathDescr() : flags(0), associated(-1), tSt(0), tEn(1) {}
  PathDescr(int f) : flags(f), associated(-1), tSt(0), tEn(1) {}
  virtual ~PathDescr() {}

  int getType() const { return flags & descr_type_mask; }
  void setType(int t) {
    flags &= ~descr_type_mask;
    flags |= t;
  }

    virtual void dumpSVG(Inkscape::SVGOStringStream &/*s*/, Geom::Point const &/*last*/) const {}
    virtual PathDescr *clone() const = 0;
    virtual void transform(Geom::Affine const &/*t*/) {}
    virtual void dump(std::ostream &/*s*/) const {}

  int    flags;         // most notably contains the path command no
  int    associated;    // index in the polyline of the point that ends the path portion of this command
  double tSt;
  double tEn;
};

struct PathDescrMoveTo : public PathDescr
{
  PathDescrMoveTo(Geom::Point const &pp)
      : PathDescr(descr_moveto), p(pp) {}

  void dumpSVG(Inkscape::SVGOStringStream &s, Geom::Point const &last) const;
  PathDescr *clone() const;
  void transform(Geom::Affine const &t);
  void dump(std::ostream &s) const;

  Geom::Point p;
};

struct PathDescrLineTo : public PathDescr
{
  PathDescrLineTo(Geom::Point const &pp)
    : PathDescr(descr_lineto), p(pp) {}

  void dumpSVG(Inkscape::SVGOStringStream &s, Geom::Point const &last) const;
  PathDescr *clone() const;
  void transform(Geom::Affine const &t);
  void dump(std::ostream &s) const;

  Geom::Point p;
};

// quadratic bezier curves: a set of control points, and an endpoint
struct PathDescrBezierTo : public PathDescr
{
  PathDescrBezierTo(Geom::Point const &pp, int n)
    : PathDescr(descr_bezierto), p(pp), nb(n) {}

  PathDescr *clone() const;
  void transform(Geom::Affine const &t);
  void dump(std::ostream &s) const;

  Geom::Point p;        // the endpoint's coordinates
  int nb;             // number of control points, stored in the next path description commands
};

/* FIXME: I don't think this should be necessary */
struct PathDescrIntermBezierTo : public PathDescr
{
  PathDescrIntermBezierTo()
    : PathDescr(descr_interm_bezier) , p(0, 0) {}
  PathDescrIntermBezierTo(Geom::Point const &pp)
    : PathDescr(descr_interm_bezier), p(pp) {}

  PathDescr *clone() const;
  void transform(Geom::Affine const &t);
  void dump(std::ostream &s) const;

  Geom::Point p;                  // control point coordinates
};

// cubic spline curve: 2 tangents and one endpoint
struct PathDescrCubicTo : public PathDescr
{
  PathDescrCubicTo(Geom::Point const &pp, Geom::Point const &s, Geom::Point const& e)
    : PathDescr(descr_cubicto), p(pp), start(s), end(e) {}

  void dumpSVG(Inkscape::SVGOStringStream &s, Geom::Point const &last) const;
  PathDescr *clone() const;
  void transform(Geom::Affine const &t);
  void dump(std::ostream &s) const;

  Geom::Point p;
  Geom::Point start;
  Geom::Point end;
};

// arc: endpoint, 2 radii and one angle, plus 2 booleans to choose the arc (svg style)
struct PathDescrArcTo : public PathDescr
{
  PathDescrArcTo(Geom::Point const &pp, double x, double y, double a, bool l, bool c)
    : PathDescr(descr_arcto), p(pp), rx(x), ry(y), angle(a), large(l), clockwise(c) {}

  void dumpSVG(Inkscape::SVGOStringStream &s, Geom::Point const &last) const;
  PathDescr *clone() const;
  void transform(Geom::Affine const &t);
  void dump(std::ostream &s) const;

  Geom::Point p;
  double rx;
  double ry;
  double angle;
  bool large;
  bool clockwise;
};

struct PathDescrForced : public PathDescr
{
  PathDescrForced() : PathDescr(descr_forced), p(0, 0) {}

  PathDescr *clone() const;

  /* FIXME: not sure whether _forced should have a point associated with it;
  ** Path::ConvertForcedToMoveTo suggests that maybe it should.
  */
  Geom::Point p;
};

struct PathDescrClose : public PathDescr
{
  PathDescrClose() : PathDescr(descr_close) {}

  void dumpSVG(Inkscape::SVGOStringStream &s, Geom::Point const &last) const;
  PathDescr *clone() const;

  /* FIXME: not sure whether _forced should have a point associated with it;
  ** Path::ConvertForcedToMoveTo suggests that maybe it should.
  */
  Geom::Point p;
};

#endif


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
