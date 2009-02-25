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
#include "snap-preferences.h"

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
	enum Transformation {
        TRANSLATION,
        SCALE,
        STRETCH,
        SKEW
    };

	SnapManager(SPNamedView const *v);

    typedef std::list<const Inkscape::Snapper*> SnapperList;

    bool someSnapperMightSnap() const;
    bool gridSnapperMightSnap() const;

    void setup(SPDesktop const *desktop,
			bool snapindicator = true,
			SPItem const *item_to_ignore = NULL,
			std::vector<std::pair<Geom::Point, int> > *unselected_nodes = NULL,
			SPGuide *guide_to_ignore = NULL);

    void setup(SPDesktop const *desktop,
    		bool snapindicator,
    		std::vector<SPItem const *> &items_to_ignore,
    		std::vector<std::pair<Geom::Point, int> > *unselected_nodes = NULL,
    		SPGuide *guide_to_ignore = NULL);

    // freeSnapReturnByRef() is preferred over freeSnap(), because it only returns a
    // point if snapping has occurred (by overwriting p); otherwise p is untouched
    void freeSnapReturnByRef(Inkscape::SnapPreferences::PointType point_type,
    							Geom::Point &p,
    							Inkscape::SnapSourceType const source_type,
    							bool first_point = true,
    							Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;


    Inkscape::SnappedPoint freeSnap(Inkscape::SnapPreferences::PointType point_type,
									Geom::Point const &p,
    							    Inkscape::SnapSourceType const &source_type,
    		                        bool first_point = true,
                                    Geom::OptRect const &bbox_to_snap = Geom::OptRect() ) const;

    Geom::Point multipleOfGridPitch(Geom::Point const &t) const;

    // constrainedSnapReturnByRef() is preferred over constrainedSnap(), because it only returns a
    // point, by overwriting p, if snapping has occurred; otherwise p is untouched
    void constrainedSnapReturnByRef(Inkscape::SnapPreferences::PointType point_type,
									Geom::Point &p,
									Inkscape::SnapSourceType const source_type,
									Inkscape::Snapper::ConstraintLine const &constraint,
									bool first_point = true,
									Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    Inkscape::SnappedPoint constrainedSnap(Inkscape::SnapPreferences::PointType point_type,
										   Geom::Point const &p,
										   Inkscape::SnapSourceType const &source_type,
										   Inkscape::Snapper::ConstraintLine const &constraint,
                                           bool first_point = true,
                                           Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    void guideSnap(Geom::Point &p, Geom::Point const &guide_normal) const;

    Inkscape::SnappedPoint freeSnapTranslation(Inkscape::SnapPreferences::PointType point_type,
                                               std::vector<std::pair<Geom::Point, int> > const &p,
                                               Geom::Point const &pointer,
                                               Geom::Point const &tr) const;

    Inkscape::SnappedPoint constrainedSnapTranslation(Inkscape::SnapPreferences::PointType point_type,
                                                      std::vector<std::pair<Geom::Point, int> > const &p,
                                                      Geom::Point const &pointer,
                                                      Inkscape::Snapper::ConstraintLine const &constraint,
                                                      Geom::Point const &tr) const;

    Inkscape::SnappedPoint freeSnapScale(Inkscape::SnapPreferences::PointType point_type,
                                         std::vector<std::pair<Geom::Point, int> > const &p,
                                         Geom::Point const &pointer,
                                         Geom::Scale const &s,
                                         Geom::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapScale(Inkscape::SnapPreferences::PointType point_type,
                                                std::vector<std::pair<Geom::Point, int> > const &p,
                                                Geom::Point const &pointer,
                                                Geom::Scale const &s,
                                                Geom::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapStretch(Inkscape::SnapPreferences::PointType point_type,
                                                  std::vector<std::pair<Geom::Point, int> > const &p,
                                                  Geom::Point const &pointer,
                                                  Geom::Coord const &s,
                                                  Geom::Point const &o,
                                                  Geom::Dim2 d,
                                                  bool uniform) const;

    Inkscape::SnappedPoint constrainedSnapSkew(Inkscape::SnapPreferences::PointType point_type,
                                               std::vector<std::pair<Geom::Point, int> > const &p,
                                               Geom::Point const &pointer,
                                               Inkscape::Snapper::ConstraintLine const &constraint,
                                               Geom::Point const &s, // s[0] = skew factor, s[1] = scale factor
                                               Geom::Point const &o,
                                               Geom::Dim2 d) const;

    Inkscape::GuideSnapper guide;      ///< guide snapper
    Inkscape::ObjectSnapper object;    ///< snapper to other objects
    Inkscape::SnapPreferences snapprefs;

    SnapperList getSnappers() const;
    SnapperList getGridSnappers() const;

    SPDesktop const *getDesktop() const {return _desktop;}
    SPNamedView const *getNamedView() const {return _named_view;}
    SPDocument *getDocument() const;
    SPGuide const *getGuideToIgnore() const {return _guide_to_ignore;}

    bool getSnapIndicator() const {return _snapindicator;}

protected:
    SPNamedView const *_named_view;

private:
    std::vector<SPItem const *> *_items_to_ignore;
    SPItem const *_item_to_ignore;
    SPGuide *_guide_to_ignore;
    SPDesktop const *_desktop;
    bool _snapindicator;
    std::vector<std::pair<Geom::Point, int> > *_unselected_nodes;

    Inkscape::SnappedPoint _snapTransformed(Inkscape::SnapPreferences::PointType type,
                                            std::vector<std::pair<Geom::Point, int> > const &points,
                                            Geom::Point const &pointer,
                                            bool constrained,
                                            Inkscape::Snapper::ConstraintLine const &constraint,
                                            Transformation transformation_type,
                                            Geom::Point const &transformation,
                                            Geom::Point const &origin,
                                            Geom::Dim2 dim,
                                            bool uniform) const;

    Geom::Point _transformPoint(std::pair<Geom::Point, int> const &p,
                                            Transformation const transformation_type,
                                            Geom::Point const &transformation,
                                            Geom::Point const &origin,
                                            Geom::Dim2 const dim,
                                            bool const uniform) const;

    void _displaySnapsource(Inkscape::SnapPreferences::PointType point_type, std::pair<Geom::Point, int> const &p) const;

    Inkscape::SnappedPoint findBestSnap(Geom::Point const &p, Inkscape::SnapSourceType const source_type, SnappedConstraints &sc, bool constrained) const;
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
