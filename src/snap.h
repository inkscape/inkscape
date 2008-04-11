#ifndef SEEN_SNAP_H
#define SEEN_SNAP_H

/**
 * \file snap.h
 * \brief SnapManager class.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2000-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-forward.h>
#include <libnr/nr-scale.h>

#include "guide-snapper.h"
#include "object-snapper.h"

class SPNamedView;

/// Class to coordinate snapping operations

/**
 *  Each SPNamedView has one of these.  It offers methods to snap points to whatever
 *  snappers are defined (e.g. grid, guides etc.).  It also allows callers to snap
 *  points which have undergone some transformation (e.g. translation, scaling etc.)
 */

class SnapManager
{
public:
    SnapManager(SPNamedView const *v);

    typedef std::list<const Inkscape::Snapper*> SnapperList;

    bool SomeSnapperMightSnap() const;

    Inkscape::SnappedPoint freeSnap(Inkscape::Snapper::PointType t,
                                    NR::Point const &p,
                                    SPItem const *it,
                                    NR::Maybe<NR::Point> point_not_to_snap_to = NR::Nothing()) const;
                                    
    Inkscape::SnappedPoint freeSnap(Inkscape::Snapper::PointType t,
                                    NR::Point const &p,
                                    SPItem const *it,
                                    std::vector<NR::Point> *unselected_nodes) const;
    
    Inkscape::SnappedPoint freeSnap(Inkscape::Snapper::PointType t,
                                    NR::Point const &p,
                                    bool const &first_point,
                                    std::vector<NR::Point> &points_to_snap,
                                    std::vector<SPItem const *> const &it,
                                    std::vector<NR::Point> *unselected_nodes) const;

    Inkscape::SnappedPoint constrainedSnap(Inkscape::Snapper::PointType t,
                                           NR::Point const &p,
                                           Inkscape::Snapper::ConstraintLine const &c,
                                           SPItem const *it) const;
    
    Inkscape::SnappedPoint constrainedSnap(Inkscape::Snapper::PointType t,
                                           NR::Point const &p,
                                           bool const &first_point,
                                           std::vector<NR::Point> &points_to_snap,
                                           Inkscape::Snapper::ConstraintLine const &c,
                                           std::vector<SPItem const *> const &it) const;
                                           
    Inkscape::SnappedPoint guideSnap(NR::Point const &p,
                                     NR::Point const &guide_normal) const;

    Inkscape::SnappedPoint freeSnapTranslation(Inkscape::Snapper::PointType t,
                                               std::vector<NR::Point> const &p,
                                               std::vector<SPItem const *> const &it,
                                               NR::Point const &tr) const;

    Inkscape::SnappedPoint constrainedSnapTranslation(Inkscape::Snapper::PointType t,
                                                      std::vector<NR::Point> const &p,
                                                      std::vector<SPItem const *> const &it,
                                                      Inkscape::Snapper::ConstraintLine const &c,
                                                      NR::Point const &tr) const;

    Inkscape::SnappedPoint freeSnapScale(Inkscape::Snapper::PointType t,
                                         std::vector<NR::Point> const &p,
                                         std::vector<SPItem const *> const &it,
                                         NR::scale const &s,
                                         NR::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapScale(Inkscape::Snapper::PointType t,
                                                std::vector<NR::Point> const &p,
                                                std::vector<SPItem const *> const &it,
                                                NR::scale const &s,
                                                NR::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapStretch(Inkscape::Snapper::PointType t,
                                                   std::vector<NR::Point> const &p,
                                                   std::vector<SPItem const *> const &it,
                                                   NR::Coord const &s,
                                                   NR::Point const &o,
                                                   NR::Dim2 d,
                                                   bool uniform) const;

    Inkscape::SnappedPoint freeSnapSkew(Inkscape::Snapper::PointType t,
                                        std::vector<NR::Point> const &p,
                                        std::vector<SPItem const *> const &it,
                                        NR::Coord const &s,
                                        NR::Point const &o,
                                        NR::Dim2 d) const;
                                            
  	Inkscape::SnappedPoint guideSnap(NR::Point const &p,
                           Inkscape::ObjectSnapper::DimensionToSnap const snap_dim) const;
  

    Inkscape::GuideSnapper guide;      ///< guide snapper
    Inkscape::ObjectSnapper object;    ///< snapper to other objects

    SnapperList getSnappers() const;
    SnapperList getGridSnappers() const;
    
    void setSnapModeBBox(bool enabled);
    void setSnapModeNode(bool enabled);
    void setSnapModeGuide(bool enabled);
    bool getSnapModeBBox() const;
    bool getSnapModeNode() const;
    bool getSnapModeGuide() const;
    
    void setSnapIntersectionGG(bool enabled) {_intersectionGG = enabled;}
    void setSnapIntersectionLS(bool enabled) {_intersectionLS = enabled;}
    bool getSnapIntersectionGG() { return _intersectionGG;}
    bool getSnapIntersectionLS() { return _intersectionLS;}    

    void setIncludeItemCenter(bool enabled)    {
        _include_item_center = enabled;
        // also store a local copy in the object-snapper instead of passing it through many functions
        object.setIncludeItemCenter(enabled);
	}
    
    bool getIncludeItemCenter() const {
        return _include_item_center;
    }
    
    void setSnapEnabledGlobally(bool enabled) {
        _snap_enabled_globally = enabled;   
    }
        
    bool getSnapEnabledGlobally() const {
        return _snap_enabled_globally;   
    }
    
    void toggleSnapEnabledGlobally() {
        _snap_enabled_globally = !_snap_enabled_globally;   
    }
        
protected:
    SPNamedView const *_named_view;

private:

    enum Transformation {
        TRANSLATION,
        SCALE,
        STRETCH,
        SKEW
    };
    
    bool _include_item_center; //If true, snapping nodes will also snap the item's center
    bool _intersectionGG;
    bool _intersectionLS;
    bool _snap_enabled_globally; //Toggles ALL snapping
    
    Inkscape::SnappedPoint _snapTransformed(Inkscape::Snapper::PointType type,
                                            std::vector<NR::Point> const &points,
                                            std::vector<SPItem const *> const &ignore,
                                            bool constrained,
                                            Inkscape::Snapper::ConstraintLine const &constraint,
                                            Transformation transformation_type,
                                            NR::Point const &transformation,
                                            NR::Point const &origin,
                                            NR::Dim2 dim,
                                            bool uniform) const;
                                                
    Inkscape::SnappedPoint findBestSnap(NR::Point const &p, SnappedConstraints &sc, bool constrained) const;
};

#endif /* !SEEN_SNAP_H */

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
