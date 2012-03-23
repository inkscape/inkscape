#ifndef INKSCAPE_SPIRO_CONVERTERS_H
#define INKSCAPE_SPIRO_CONVERTERS_H

#include <2geom/forward.h>
class SPCurve;

namespace Spiro {

class ConverterBase {
public:
    ConverterBase() {};
    virtual ~ConverterBase() {};

    virtual void moveto(double x, double y, bool is_open) = 0;
    virtual void lineto(double x, double y) = 0;
    virtual void quadto(double x1, double y1, double x2, double y2) = 0;
    virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3) = 0;
};


/**
 * Converts Spiro to Inkscape's SPCurve
 */
class ConverterSPCurve : public ConverterBase {
public:
    ConverterSPCurve(SPCurve &curve)
        : _curve(curve), _is_open(false)
    {} ;

    virtual void moveto(double x, double y, bool is_open);
    virtual void lineto(double x, double y);
    virtual void quadto(double x1, double y1, double x2, double y2);
    virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3);

    SPCurve &_curve;
    bool _is_open;

private:
    ConverterSPCurve(const ConverterSPCurve&);
    ConverterSPCurve& operator=(const ConverterSPCurve&);
};


/**
 * Converts Spiro to 2Geom's Path
 */
class ConverterPath : public ConverterBase {
public:
    ConverterPath(Geom::Path &path)
        : _path(path), _is_open(false)
    {} ;

    virtual void moveto(double x, double y, bool is_open);
    virtual void lineto(double x, double y);
    virtual void quadto(double x1, double y1, double x2, double y2);
    virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3);

    Geom::Path &_path;
    bool _is_open;

private:
    ConverterPath(const ConverterPath&);
    ConverterPath& operator=(const ConverterPath&);
};


} // namespace Spiro

#endif
