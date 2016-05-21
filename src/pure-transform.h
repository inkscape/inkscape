/*
 * Class for pure transformations, such as translating, scaling, stretching, skewing, and rotating. Pure means that they cannot
 * be combined. This is what makes them different from affine transformations. Pure transformations are being used in the selector
 * tool and node tool
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2015 Diederik van Lierop
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_PURE_TRANSFORM_H
#define SEEN_PURE_TRANSFORM_H

#include <glib.h> // for g_warning
#include "snapper.h" // for SnapConstraint

class SnapManager;

namespace Inkscape {

class PureTransform {

protected:
    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const = 0;
    virtual Geom::Point getTransformedPoint(SnapCandidatePoint const &p) const = 0;
    virtual void storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point) = 0;

public:
    //PureTransform();
    virtual ~PureTransform() {};
//    virtual PureTransform * clone () const = 0;  // https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Virtual_Constructor

    // Snap a group of points
    SnappedPoint best_snapped_point;
    void snap(::SnapManager *sm, std::vector<Inkscape::SnapCandidatePoint> const &points, Geom::Point const &pointer);
};

// **************************************************************************************************************

class PureTranslate: public PureTransform {

protected:
    Geom::Point _vector;
    Geom::Point _vector_snapped;

    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;
    virtual Geom::Point getTransformedPoint(SnapCandidatePoint const &p) const;
    virtual void storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point);

public:
//    PureTranslate();                        // Default constructor
//    PureTranslate(PureTranslate const &);   // Copy constructor
    virtual ~PureTranslate() {};
    PureTranslate(Geom::Point vector = Geom::Point()) : _vector(vector), _vector_snapped(vector) {}

    Geom::Point getTranslationSnapped() {return _vector_snapped;}
//    PureTranslate * clone () const {return new PureTranslate(*this);}
};


class PureTranslateConstrained: public PureTranslate {

protected:
    Geom::Dim2 _direction;
    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;

public:
    virtual ~PureTranslateConstrained() {};
    PureTranslateConstrained(Geom::Coord displacement, Geom::Dim2 direction):
        PureTranslate() {
            _vector[direction] = displacement;
            _vector[1-direction] = 0.0;
            _direction = direction;
        }
    // PureTranslateConstrained * clone () const {return new PureTranslateConstrained(*this);}
};

// **************************************************************************************************************

class PureScale: public PureTransform {

protected:
    Geom::Scale _scale;
    Geom::Scale _scale_snapped;
    Geom::Point _origin;
    bool        _uniform;

    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;
    virtual Geom::Point getTransformedPoint(SnapCandidatePoint const &p) const;
    virtual void storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point);

public:
//    PureScale();                    // Default constructor
//    PureScale(PureScale const &);   // Copy constructor
    virtual ~PureScale() {};

    PureScale(Geom::Scale scale, Geom::Point origin, bool uniform) : 
        _scale (scale),
        _scale_snapped (scale),
        _origin (origin),
        _uniform (uniform)
    {}

    Geom::Scale getScaleSnapped() {return _scale_snapped;}
//    PureScale * clone () const {return new PureScale (*this);}
};

class PureScaleConstrained: public PureScale {
//Magnitude of the scale components will be the same, but the sign could still be different ()
protected:
    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;

public:
    virtual ~PureScaleConstrained() {};
    PureScaleConstrained(Geom::Scale scale, Geom::Point origin):
        PureScale(scale, origin, true) {}; // Non-uniform constrained scaling is not supported

//    PureScaleConstrained * clone () const {return new PureScaleConstrained(*this);}
};

// **************************************************************************************************************

class PureStretchConstrained: public PureTransform {
// A stretch is always implicitly constrained

protected:
    Geom::Coord _magnitude;
    Geom::Scale _stretch_snapped;
    Geom::Point _origin;
    Geom::Dim2  _direction;
    bool        _uniform;

    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;
    virtual Geom::Point getTransformedPoint(SnapCandidatePoint const &p) const;
    virtual void storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point);

public:
    virtual ~PureStretchConstrained() {};
    PureStretchConstrained(Geom::Coord magnitude, Geom::Point origin, Geom::Dim2 direction, bool uniform) :
        _magnitude (magnitude),
        _stretch_snapped (Geom::Scale(magnitude, magnitude)),
        _origin (origin),
        _direction (direction),
        _uniform (uniform)
    {
        if (not uniform) {
            _stretch_snapped[1-direction] = 1.0;
        }
    }

    Geom::Scale getStretchSnapped() {return _stretch_snapped;}
    Geom::Coord getMagnitude() {return _magnitude;}
    Geom::Coord getMagnitudeSnapped() {return _stretch_snapped[_direction];}

//    PureStretchConstrained * clone () const {return new PureStretchConstrained(*this);}
};

// **************************************************************************************************************

class PureSkewConstrained: public PureTransform {
// A skew is always implicitly constrained

protected:
    Geom::Coord _skew;
    Geom::Coord _skew_snapped;
    Geom::Coord _scale;
    Geom::Point _origin;
    Geom::Dim2  _direction;

    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;
    Geom::Point getTransformedPoint(SnapCandidatePoint const &p) const;
    virtual void storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point);

public:
    virtual ~PureSkewConstrained() {};
    PureSkewConstrained(Geom::Coord skew, Geom::Coord scale, Geom::Point origin, Geom::Dim2 direction) :
        _skew (skew),
        _skew_snapped (skew),
        _scale (scale),
        _origin (origin),
        _direction (direction)
    {};

    Geom::Coord getSkewSnapped() {return _skew_snapped;}

//    PureSkewConstrained * clone () const {return new PureSkewConstrained(*this);}
};

// **************************************************************************************************************

class PureRotateConstrained: public PureTransform {
// A rotation is always implicitly constrained, so we will hide the constructor by making it protected; devs should use PureRotateConstrained instead
// It's _constraint member variable though will be empty

protected:
    double      _angle; // in radians
    double      _angle_snapped;
    Geom::Point _origin;
    bool        _uniform;

    virtual SnappedPoint snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const;
    virtual Geom::Point getTransformedPoint(SnapCandidatePoint const &p) const;
    virtual void storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point);

public:
//    PureRotate();                        // Default constructor
//    PureRotate(PureRotate const &);   // Copy constructor
    virtual ~PureRotateConstrained() {};

    PureRotateConstrained(double angle, Geom::Point origin) :
        _angle (angle), // in radians!
        _angle_snapped (angle),
        _origin (origin),
        _uniform (true) // We do not yet allow for simultaneous rotation and scaling
    {}

    double getAngleSnapped() {return _angle_snapped;}

//    PureRotate * clone () const {return new PureRotate(*this);}
};

}

#endif // !SEEN_PURE_TRANSFORM_H

