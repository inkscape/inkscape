#ifndef INKSCAPE_SPIRO_CONVERTERS_H
#define INKSCAPE_SPIRO_CONVERTERS_H

#include <2geom/forward.h>
class SPCurve;

namespace Spiro {

class ConverterBase {
public:
    ConverterBase() {};
    virtual ~ConverterBase() {};

    virtual void moveto(double x, double y) = 0;
    virtual void lineto(double x, double y, bool close_last) = 0;
    virtual void quadto(double x1, double y1, double x2, double y2, bool close_last) = 0;
    virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3, bool close_last) = 0;
};


/**
 * Converts Spiro to Inkscape's SPCurve
 */
class ConverterSPCurve : public ConverterBase {
public:
    ConverterSPCurve(SPCurve &curve)
        : _curve(curve)
    {}

    virtual void moveto(double x, double y);
    virtual void lineto(double x, double y, bool close_last);
    virtual void quadto(double x1, double y1, double x2, double y2, bool close_last);
    virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3, bool close_last);

private:
    SPCurve &_curve;

    ConverterSPCurve(const ConverterSPCurve&);
    ConverterSPCurve& operator=(const ConverterSPCurve&);
};


/**
 * Converts Spiro to 2Geom's Path
 */
class ConverterPath : public ConverterBase {
public:
    ConverterPath(Geom::Path &path);

    virtual void moveto(double x, double y);
    virtual void lineto(double x, double y, bool close_last);
    virtual void quadto(double x1, double y1, double x2, double y2, bool close_last);
    virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3, bool close_last);

private:
    Geom::Path &_path;

    ConverterPath(const ConverterPath&);
    ConverterPath& operator=(const ConverterPath&);
};


} // namespace Spiro

#endif
