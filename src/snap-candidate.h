#ifndef SEEN_SNAP_CANDIDATE_H
#define SEEN_SNAP_CANDIDATE_H

/**
 * \file snap-candidate.h
 * \brief some utility classes to store various kinds of snap candidates.
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

//#include "snapped-point.h"
#include "snap-enums.h"

struct SPItem; // forward declaration

namespace Inkscape {

/// Class to store data for points which are snap candidates, either as a source or as a target
class SnapCandidatePoint
{
public:
    SnapCandidatePoint(Geom::Point const &point, Inkscape::SnapSourceType const source, long const source_num, Inkscape::SnapTargetType const target, Geom::OptRect const &bbox)
        : _point(point),
        _source_type(source),
        _target_type(target),
        _source_num(source_num),
        _target_bbox(bbox)
    {
    };

    SnapCandidatePoint(Geom::Point const &point, Inkscape::SnapSourceType const source, Inkscape::SnapTargetType const target)
        : _point(point),
        _source_type(source),
        _target_type(target)
    {
        _source_num = 0;
        _target_bbox = Geom::OptRect();
    }

    SnapCandidatePoint(Geom::Point const &point, Inkscape::SnapSourceType const source, long const source_num = 0)
        : _point(point),
        _source_type(source),
        _target_type(Inkscape::SNAPTARGET_UNDEFINED),
        _source_num(source_num)
    {
        _target_bbox = Geom::OptRect();
    }

    inline Geom::Point const & getPoint() const {return _point;}
    inline Inkscape::SnapSourceType getSourceType() const {return _source_type;}
    inline Inkscape::SnapTargetType getTargetType() const {return _target_type;}
    inline long getSourceNum() const {return _source_num;}
    inline Geom::OptRect const getTargetBBox() const {return _target_bbox;}

private:
    // Coordinates of the point
    Geom::Point _point;

    // If this SnapCandidatePoint is a snap source, then _source_type must be defined. If it
    // is a snap target, then _target_type must be defined. If it's yet unknown whether it will
    // be a source or target, then both may be defined
    Inkscape::SnapSourceType _source_type;
    Inkscape::SnapTargetType _target_type;

    //Sequence number of the source point within the set of points that is to be snapped. Starting at zero
    long _source_num;

    // If this is a target and it belongs to a bounding box, e.g. when the target type is
    // SNAPTARGET_BBOX_EDGE_MIDPOINT, then _target_bbox stores the relevant bounding box
    Geom::OptRect _target_bbox;
};

class SnapCandidateItem
{
public:
    SnapCandidateItem(SPItem* item, bool clip_or_mask, Geom::Matrix additional_affine)
        : item(item), clip_or_mask(clip_or_mask), additional_affine(additional_affine) {}
    ~SnapCandidateItem() {};

    SPItem* item;        // An item that is to be considered for snapping to
    bool clip_or_mask;    // If true, then item refers to a clipping path or a mask

    /* To find out the absolute position of a clipping path or mask, we not only need to know
     * the transformation of the clipping path or mask itself, but also the transformation of
     * the object to which the clip or mask is being applied; that transformation is stored here
     */
    Geom::Matrix additional_affine;
}
;

class SnapCandidatePath
{

public:
    SnapCandidatePath(Geom::PathVector* path, SnapTargetType target, Geom::OptRect bbox, bool edited = false)
        : path_vector(path), target_type(target), target_bbox(bbox), currently_being_edited(edited) {};
    ~SnapCandidatePath() {};

    Geom::PathVector* path_vector;
    SnapTargetType target_type;
    Geom::OptRect target_bbox;
    bool currently_being_edited; // true for the path that's currently being edited in the node tool (if any)

};

} // end of namespace Inkscape

#endif /* !SEEN_SNAP_CANDIDATE_H */
