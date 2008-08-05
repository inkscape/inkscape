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
    
    void setup(SPDesktop const *desktop_for_snapindicator = NULL, SPItem const *item_to_ignore = NULL, std::vector<NR::Point> *unselected_nodes = NULL);
    void setup(SPDesktop const *desktop_for_snapindicator, std::vector<SPItem const *> &items_to_ignore, std::vector<NR::Point> *unselected_nodes = NULL);

    // freeSnapReturnByRef() is preferred over freeSnap(), because it only returns a 
    // point if snapping has occured (by overwriting p); otherwise p is untouched    
    void freeSnapReturnByRef(Inkscape::Snapper::PointType point_type,
                      NR::Point &p,
                      bool first_point = true,
                      boost::optional<NR::Rect> const &bbox_to_snap = boost::optional<NR::Rect>()) const;
    
    Inkscape::SnappedPoint freeSnap(Inkscape::Snapper::PointType point_type,
                                    NR::Point const &p,
                                    bool first_point = true,
                                    boost::optional<NR::Rect> const &bbox_to_snap = boost::optional<NR::Rect>() ) const;
    
    // constrainedSnapReturnByRef() is preferred over constrainedSnap(), because it only returns a 
    // point, by overwriting p, if snapping has occured; otherwise p is untouched
    void constrainedSnapReturnByRef(Inkscape::Snapper::PointType point_type,
                             NR::Point &p,
                             Inkscape::Snapper::ConstraintLine const &constraint,
                             bool first_point = true,
                             boost::optional<NR::Rect> const &bbox_to_snap = boost::optional<NR::Rect>()) const;
    
    Inkscape::SnappedPoint constrainedSnap(Inkscape::Snapper::PointType point_type,
                                           NR::Point const &p,
                                           Inkscape::Snapper::ConstraintLine const &constraint,
                                           bool first_point = true,
                                           boost::optional<NR::Rect> const &bbox_to_snap = boost::optional<NR::Rect>()) const;
                                           
    void guideSnap(NR::Point &p, NR::Point const &guide_normal) const;

    Inkscape::SnappedPoint freeSnapTranslation(Inkscape::Snapper::PointType point_type,
                                               std::vector<NR::Point> const &p,
                                               NR::Point const &tr) const;

    Inkscape::SnappedPoint constrainedSnapTranslation(Inkscape::Snapper::PointType point_type,
                                                      std::vector<NR::Point> const &p,
                                                      Inkscape::Snapper::ConstraintLine const &constraint,
                                                      NR::Point const &tr) const;

    Inkscape::SnappedPoint freeSnapScale(Inkscape::Snapper::PointType point_type,
                                         std::vector<NR::Point> const &p,
                                         NR::scale const &s,
                                         NR::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapScale(Inkscape::Snapper::PointType point_type,
                                                std::vector<NR::Point> const &p,
                                                NR::scale const &s,
                                                NR::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapStretch(Inkscape::Snapper::PointType point_type,
                                                  std::vector<NR::Point> const &p,
                                                  NR::Coord const &s,
                                                  NR::Point const &o,
                                                  NR::Dim2 d,
                                                  bool uniform) const;

    Inkscape::SnappedPoint constrainedSnapSkew(Inkscape::Snapper::PointType point_type,
                                               std::vector<NR::Point> const &p,
                                               Inkscape::Snapper::ConstraintLine const &constraint,
                                               NR::Point const &s, // s[0] = skew factor, s[1] = scale factor
                                               NR::Point const &o,
                                               NR::Dim2 d) const;
                                        
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
    bool getSnapIntersectionGG() {return _intersectionGG;}
    bool getSnapIntersectionLS() {return _intersectionLS;}    

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
    
    std::vector<SPItem const *> *_items_to_ignore;
    SPItem const *_item_to_ignore;
    SPDesktop const *_desktop_for_snapindicator;    
    std::vector<NR::Point> *_unselected_nodes;                                    
    
    Inkscape::SnappedPoint _snapTransformed(Inkscape::Snapper::PointType type,
                                            std::vector<NR::Point> const &points,
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
