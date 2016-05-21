/*
 * Class for pure transformations, such as translating, scaling, stretching, skewing, and rotating
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2015 Diederik van Lierop
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "pure-transform.h"
#include "snap.h"

namespace Inkscape

{

void PureTransform::snap(::SnapManager *sm, std::vector<Inkscape::SnapCandidatePoint> const &points, Geom::Point const &pointer) {
    std::vector<Inkscape::SnapCandidatePoint> transformed_points;
    Geom::Rect bbox;

    long source_num = 0;
    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator i = points.begin(); i != points.end(); ++i) {

        /* Work out the transformed version of this point */
        Geom::Point transformed = getTransformedPoint(*i); //  _transformPoint(*i, transformation_type, transformation, origin, dim, uniform);

        // add the current transformed point to the box hulling all transformed points
        if (i == points.begin()) {
            bbox = Geom::Rect(transformed, transformed);
        } else {
            bbox.expandTo(transformed);
        }

        transformed_points.push_back(Inkscape::SnapCandidatePoint(transformed, (*i).getSourceType(), source_num, Inkscape::SNAPTARGET_UNDEFINED, Geom::OptRect()));
        source_num++;
    }

    /* The current best metric for the best transformation; lower is better, whereas Geom::infinity()
    ** means that we haven't snapped anything.
    */
    Inkscape::SnapCandidatePoint best_original_point;
    g_assert(best_snapped_point.getAlwaysSnap() == false); // Check initialization of snapped point
    g_assert(best_snapped_point.getAtIntersection() == false);
    g_assert(best_snapped_point.getSnapped() == false); // Check initialization to catch any regression

    std::vector<Inkscape::SnapCandidatePoint>::iterator j = transformed_points.begin();

    // std::cout << std::endl;
    bool first_free_snap = true;

    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator i = points.begin(); i != points.end(); ++i) {

        // If we have a collection of SnapCandidatePoints, with mixed constrained snapping and free snapping
        // requirements (this can happen when scaling, see PureScale::snap()), then freeSnap might never see the
        // SnapCandidatePoint with source_num == 0. The freeSnap() method in the object snapper depends on this,
        // because only for source-num == 0 the target nodes will be collected. Therefore we enforce that the first
        // SnapCandidatePoint that is to be freeSnapped always has source_num == 0;
        // TODO: This is a bit ugly so fix this; do we need sourcenum for anything else? if we don't then get rid
        // of it and explicitly communicate to the object snapper that this is a first point
        if (first_free_snap) {
            (*j).setSourceNum(0);
            first_free_snap = false;
        }

        Inkscape::SnappedPoint snapped_point = snap(sm, *j, (*i).getPoint(), bbox); // Calls the snap() method of the derived classes

        snapped_point.setPointerDistance(Geom::L2(pointer - (*i).getPoint()));

        /*Find the transformation that describes where the snapped point has
        ** ended up, and also the metric for this transformation.
        */

        bool store_best_snap = false;
        if (snapped_point.getSnapped()) {
            // We snapped; keep track of the best snap
            if (best_snapped_point.isOtherSnapBetter(snapped_point, true)) {
                store_best_snap = true;
            }
        } else {
            // So we didn't snap for this point
            if (!best_snapped_point.getSnapped()) {
                // ... and none of the points before snapped either
                // We might still need to apply a constraint though, if we tried a constrained snap. And
                // in case of a free snap we might have use for the transformed point, so let's return that
                // point, whether it's constrained or not

                if (best_snapped_point.isOtherSnapBetter(snapped_point, true) ) {
                    // .. so we must keep track of the best non-snapped constrained point.. but what
                    // is the best? There is no best, or is there? We cannot compare on snapped distance
                    // because neither has snapped, and both have their snapped distance set to infinity.
                    // There might be a difference in "constrainedness" though, 1D vs 2D snapping
                    store_best_snap = true;
                }
            }
        }

        if (store_best_snap || i == points.begin()) {
            best_original_point = (*i);
            best_snapped_point = snapped_point; // Can be a point that didn't snap, but then at least we
            // return something meaningful; we might have use for the transformation. The default
            // snapped_point, as initialized before this loop, is not very meaningful at all.
        }

        ++j;
    }

    /* The current best transformation */
    //Geom::Point best_transformation = getResult(best_original_point, best_snapped_point);
    storeTransform(best_original_point, best_snapped_point);

    Geom::Coord best_metric = best_snapped_point.getSnapDistance();

    // Using " < 1e6" instead of " < Geom::infinity()" for catching some rounding errors
    // These rounding errors might be caused by NRRects, see bug #1584301
    best_snapped_point.setSnapDistance(best_metric < 1e6 ? best_metric : Geom::infinity());
}





Geom::Point PureTranslate::getTransformedPoint(SnapCandidatePoint const &p) const {
    return p.getPoint() + _vector;
}

void PureTranslate::storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point) {
    /* Consider the case in which a box is almost aligned with a grid in both
     * horizontal and vertical directions. The distance to the intersection of
     * the grid lines will always be larger then the distance to a single grid
     * line. If we prefer snapping to an intersection over to a single
     * grid line, then we cannot use "metric = Geom::L2(result)". Therefore the
     * snapped distance will be used as a metric. Please note that the snapped
     * distance to an intersection is defined as the distance to the nearest line
     *  of the intersection, and not to the intersection itself!
     */
    // Only for translations, the relevant metric will be the real snapped distance,
    // so we don't have to do anything special here
    _vector_snapped = snapped_point.getPoint() - original_point.getPoint();
}

SnappedPoint PureTranslate::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point /*pt_orig*/, Geom::OptRect const &bbox_to_snap) const {
    return sm->freeSnap(p, bbox_to_snap);
}

SnappedPoint PureTranslateConstrained::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const {
    // Calculate a constraint dedicated for this specific point
    // When doing a constrained translation, all points will move in the same direction, i.e.
    // either horizontally or vertically. The lines along which they move are therefore all
    // parallel, but might not be co-linear. Therefore we will have to specify the point through
    // which the constraint-line runs here, for each point individually.
    Snapper::SnapConstraint dedicated_constraint = Snapper::SnapConstraint(pt_orig, _direction);
    return sm->constrainedSnap(p, dedicated_constraint, bbox_to_snap);
}





Geom::Point PureScale::getTransformedPoint(SnapCandidatePoint const &p) const {
    return (p.getPoint() - _origin) * _scale + _origin;
}

void PureScale::storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point) {
    _scale_snapped = Geom::Scale(Geom::infinity(), Geom::infinity());
    // If this point *i is horizontally or vertically aligned with
    // the origin of the scaling, then it will scale purely in X or Y
    // We can therefore only calculate the scaling in this direction
    // and the scaling factor for the other direction should remain
    // untouched (unless scaling is uniform of course)
    Geom::Point const a = snapped_point.getPoint() - _origin; // vector to snapped point
    Geom::Point const b = original_point.getPoint() - _origin; // vector to original point (not the transformed point!)
    for (int index = 0; index < 2; index++) {
        if (fabs(b[index]) > 1e-4) { // if SCALING CAN occur in this direction
            if (fabs(fabs(a[index]/b[index]) - fabs(_scale[index])) > 1e-7) { // if SNAPPING DID occur in this direction
                _scale_snapped[index] = a[index] / b[index]; // then calculate it!
                // _scale_snapped will be (1,1) if we haven't snapped, because the snapped point equals the original point
            }
            // we might have left result[1-index] = Geom::infinity() if scaling didn't occur in the other direction
        }
    }

    if (_scale_snapped == Geom::Scale(Geom::infinity(), Geom::infinity())) {
        // This point must have been at the origin, so we cannot possibly snap; it won't scale (i.e. won't move while dragging)
        snapped_point.setSnapDistance(Geom::infinity());
        snapped_point.setSecondSnapDistance(Geom::infinity());
        return;
    }

    if (_uniform) {
        // Lock the scaling the be uniform, but keep the sign such that we don't change which quadrant we have dragged into
        if (fabs(_scale_snapped[0]) < fabs(_scale_snapped[1])) {
            _scale_snapped[1] = fabs(_scale_snapped[0]) * Geom::sgn(_scale[1]);
        } else {
            _scale_snapped[0] = fabs(_scale_snapped[1]) * Geom::sgn(_scale[0]);
        }
    }

    // Don't ever exit with one of scaling components uninitialized
    for (int index = 0; index < 2; index++) {
        if (_scale_snapped[index] == Geom::infinity()) {
            _scale_snapped[index] = _scale[index];
        }
    }

    // Compare the resulting scaling with the desired scaling
    Geom::Point scale_metric = _scale_snapped.vector() - _scale.vector();
    snapped_point.setSnapDistance(Geom::L2(scale_metric));
    snapped_point.setSecondSnapDistance(Geom::infinity());
}

// When scaling, a point aligned either horizontally or vertically with the origin can only
// move in that specific direction; therefore it should only snap in that direction, so this
// then becomes a constrained snap; otherwise we can use a free snap;
SnappedPoint PureScale::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const {
    Geom::Point const b = (pt_orig - _origin); // vector to original point (not the transformed point!)
    bool const c1 = fabs(b[Geom::X]) < 1e-6;
    bool const c2 = fabs(b[Geom::Y]) < 1e-6;
    if ((c1 || c2) && !(c1 && c2)) {
        Geom::Point cvec; cvec[c1] = 1.;
        Snapper::SnapConstraint dedicated_constraint = Inkscape::Snapper::SnapConstraint(_origin, cvec);
        return sm->constrainedSnap(p, dedicated_constraint, bbox_to_snap);
    } else {
        return sm->freeSnap(p, bbox_to_snap);
    }
}

SnappedPoint PureScaleConstrained::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const {
    // When constrained scaling, only uniform scaling is supported.
    // When uniformly scaling, each point will have its own unique constraint line,
    // running from the scaling origin to the original untransformed point. We will
    // calculate that line here as a dedicated constraint
    Geom::Point b = pt_orig - _origin;
    Snapper::SnapConstraint dedicated_constraint = Inkscape::Snapper::SnapConstraint(_origin, b);
    return sm->constrainedSnap(p, dedicated_constraint, bbox_to_snap);
}





Geom::Point PureStretchConstrained::getTransformedPoint(SnapCandidatePoint const &p) const {
    Geom::Scale s(1, 1);
    if (_uniform)
        s[Geom::X] = s[Geom::Y] = _magnitude;
    else {
        s[_direction] = _magnitude;
        s[1 - _direction] = 1;
    }
    return ((p.getPoint() - _origin) * s) + _origin;
}

SnappedPoint PureStretchConstrained::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const {
    Snapper::SnapConstraint dedicated_constraint;
    if (_uniform) {
        // When uniformly stretching, each point will have its own unique constraint line,
        // running from the scaling origin to the original untransformed point. We will
        // calculate that line here
        Geom::Point b = pt_orig - _origin;
        dedicated_constraint = Inkscape::Snapper::SnapConstraint(_origin, b); // dedicated constraint
    } else {
        Geom::Point cvec; cvec[_direction] = 1.;
        dedicated_constraint = Inkscape::Snapper::SnapConstraint(pt_orig, cvec);
    }

    return sm->constrainedSnap(p, dedicated_constraint, bbox_to_snap);
}

void PureStretchConstrained::storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point) {
    Geom::Point const a = snapped_point.getPoint() - _origin; // vector to snapped point
    Geom::Point const b = original_point.getPoint() - _origin; // vector to original point (not the transformed point!)

    _stretch_snapped = Geom::Scale(Geom::infinity(), Geom::infinity());
    if (fabs(b[_direction]) > 1e-4) { // if STRETCHING will occur for this point
        _stretch_snapped[_direction] = a[_direction] / b[_direction];
        _stretch_snapped[1-_direction] = _uniform ? _stretch_snapped[_direction] : 1;
    } else { // STRETCHING might occur for this point, but only when the stretching is uniform
        if (_uniform && fabs(b[1-_direction]) > 1e-4) {
           _stretch_snapped[1-_direction] = a[1-_direction] / b[1-_direction];
           _stretch_snapped[_direction] = _stretch_snapped[1-_direction];
        }
    }

    // _stretch_snapped might have one or both components at infinity!

    // Store the metric for this transformation as a virtual distance
    snapped_point.setSnapDistance(std::abs(_stretch_snapped[_direction] - _magnitude));
    snapped_point.setSecondSnapDistance(Geom::infinity());
}





Geom::Point PureSkewConstrained::getTransformedPoint(SnapCandidatePoint const &p) const {
    Geom::Point transformed;
    // Apply the skew factor
    transformed[_direction] = (p.getPoint())[_direction] + _skew * ((p.getPoint())[1 - _direction] - _origin[1 - _direction]);
    // While skewing, mirroring and scaling (by integer multiples) in the opposite direction is also allowed.
    // Apply that scale factor here
    transformed[1-_direction] = (p.getPoint() - _origin)[1 - _direction] * _scale + _origin[1 - _direction];
    return transformed;
}

SnappedPoint PureSkewConstrained::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const {
    // Snapping the nodes of the bounding box of a selection that is being transformed, will only work if
    // the transformation of the bounding box is equal to the transformation of the individual nodes. This is
    // NOT the case for example when rotating or skewing. The bounding box itself cannot possibly rotate or skew,
    // so it's corners have a different transformation. The snappers cannot handle this, therefore snapping
    // of bounding boxes is not allowed here.
    g_assert(!(p.getSourceType() & Inkscape::SNAPSOURCE_BBOX_CATEGORY));

    Geom::Point constraint_vector;
    constraint_vector[1-_direction] = 0.0;
    constraint_vector[_direction] = 1.0;

    return sm->constrainedSnap(p, Inkscape::Snapper::SnapConstraint(constraint_vector), bbox_to_snap);
}

void PureSkewConstrained::storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point) {
    Geom::Point const b = original_point.getPoint() - _origin; // vector to original point (not the transformed point!)
    _skew_snapped = (snapped_point.getPoint()[_direction] - (original_point.getPoint())[_direction]) / b[1 - _direction]; // skew factor

    // Store the metric for this transformation as a virtual distance
    snapped_point.setSnapDistance(std::abs(_skew_snapped - _skew));
    snapped_point.setSecondSnapDistance(Geom::infinity());
}




Geom::Point PureRotateConstrained::getTransformedPoint(SnapCandidatePoint const &p) const {
    return (p.getPoint() - _origin) * Geom::Rotate(_angle) + _origin;
}

SnappedPoint PureRotateConstrained::snap(::SnapManager *sm, SnapCandidatePoint const &p, Geom::Point pt_orig, Geom::OptRect const &bbox_to_snap) const {
    // Snapping the nodes of the bounding box of a selection that is being transformed, will only work if
    // the transformation of the bounding box is equal to the transformation of the individual nodes. This is
    // NOT the case for example when rotating or skewing. The bounding box itself cannot possibly rotate or skew,
    // so it's corners have a different transformation. The snappers cannot handle this, therefore snapping
    // of bounding boxes is not allowed here.
    g_assert(!(p.getSourceType() & Inkscape::SNAPSOURCE_BBOX_CATEGORY));

    // Calculate a constraint dedicated for this specific point
    Geom::Point b = pt_orig - _origin;
    Geom::Coord r = Geom::L2(b); // the radius of the circular constraint
    Snapper::SnapConstraint dedicated_constraint = Inkscape::Snapper::SnapConstraint(_origin, b, r);
    return sm->constrainedSnap(p, dedicated_constraint, bbox_to_snap);
}

void PureRotateConstrained::storeTransform(SnapCandidatePoint const &original_point, SnappedPoint &snapped_point) {
    Geom::Point const a = snapped_point.getPoint() - _origin; // vector to snapped point
    Geom::Point const b = (original_point.getPoint() - _origin); // vector to original point (not the transformed point!)
    // a is vector to snapped point; b is vector to original point; now lets calculate angle between a and b
    _angle_snapped = atan2(Geom::dot(Geom::rot90(b), a), Geom::dot(b, a));
    if (Geom::L2(b) < 1e-9) { // points too close to the rotation center will not move. Don't try to snap these
        // as they will always yield a perfect snap result if they're already snapped beforehand (e.g.
        // when the transformation center has been snapped to a grid intersection in the selector tool)
        snapped_point.setSnapDistance(Geom::infinity());
        // PS1: Apparently we don't have to do this for skewing, but why?
        // PS2: We cannot easily filter these points upstream, e.g. in the grab() method (seltrans.cpp)
        // because the rotation center will change when pressing shift, and grab() won't be recalled.
        // Filtering could be done in handleRequest() (again in seltrans.cpp), by iterating through
        // the snap candidates. But hey, we're iterating here anyway.
    } else {
        snapped_point.setSnapDistance(fabs(_angle_snapped - _angle));
    }
    snapped_point.setSecondSnapDistance(Geom::infinity());

}

}
