/*
 * Per-desktop object that handles snapping queries.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2000-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SNAP_H
#define SEEN_SNAP_H

#include <vector>
#include "guide-snapper.h"
#include "object-snapper.h"
#include "snap-preferences.h"
//#include "pure-transform.h"


// Guides
enum SPGuideDragType { // used both here and in desktop-events.cpp
    SP_DRAG_TRANSLATE,
    SP_DRAG_ROTATE,
    SP_DRAG_MOVE_ORIGIN,
    SP_DRAG_NONE
};

class SPGuide;
class SPNamedView;

namespace Inkscape {
    class PureTransform;
}


/**
 * Class to coordinate snapping operations.
 *
 * The SnapManager class handles most (if not all) of the interfacing of the snapping mechanisms
 * with the other parts of the code base. It stores the references to the various types of snappers
 * for grid, guides and objects, and it stores most of the snapping preferences. Besides that
 * it provides methods to setup the snapping environment (e.g. keeps a list of the items to ignore
 * when looking for snap target candidates, and toggling of the snap indicator), and it provides
 * many different methods for snapping queries (free snapping vs. constrained snapping,
 * returning the result by reference or through a return statement, etc.)
 * 
 * Each SPNamedView has one of these.  It offers methods to snap points to whatever
 * snappers are defined (e.g. grid, guides etc.).  It also allows callers to snap
 * points which have undergone some transformation (e.g. translation, scaling etc.)
 *
 * \par How snapping is implemented in Inkscape
 * \par
 * The snapping system consists of two key elements. The first one is the snap manager
 * (this class), which keeps some data about objects in the document and answers queries
 * of the type "given this point and type of transformation, what is the best place
 * to snap to?".
 * 
 * The second is in event-context.cpp and implements the snapping timeout. Whenever a motion
 * events happens over the canvas, it stores it for later use and initiates a timeout.
 * This timeout is discarded whenever a new motion event occurs. When the timeout expires,
 * a global flag in SnapManager, accessed via getSnapPostponedGlobally(), is set to true
 * and the stored event is replayed, but this time with snapping enabled. This way you can
 * write snapping code directly in your control point's dragged handler as if there was
 * no timeout.
 */
class SnapManager
{
public:
    enum Transformation {
        TRANSLATE,
        SCALE,
        STRETCH,
        SKEW,
        ROTATE
    };

    /**
     * Construct a SnapManager for a SPNamedView.
     *
     * @param v 'Owning' SPNamedView.
     */
    SnapManager(SPNamedView const *v);

    typedef std::list<const Inkscape::Snapper*> SnapperList;

    /**
     * Return true if any snapping might occur, whether its to grids, guides or objects.
     *
     * Each snapper instance handles its own snapping target, e.g. grids, guides or
     * objects. This method iterates through all these snapper instances and returns
     * true if any of the snappers might possible snap, considering only the relevant
     * snapping preferences.
     *
     * @return true if one of the snappers will try to snap to something.
     */
    bool someSnapperMightSnap(bool immediately = true) const;

    /**
     * @return true if one of the grids might be snapped to.
     */
    bool gridSnapperMightSnap() const;

    /**
     * Convenience shortcut when there is only one item to ignore.
     */
    void setup(SPDesktop const *desktop,
            bool snapindicator = true,
            SPItem const *item_to_ignore = NULL,
            std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes = NULL,
            SPGuide *guide_to_ignore = NULL);

    /**
     * Prepare the snap manager for the actual snapping, which includes building a list of snap targets
     * to ignore and toggling the snap indicator.
     *
     * There are two overloaded setup() methods, of which the other one only allows for a single item to be ignored
     * whereas this one will take a list of items to ignore
     *
     * @param desktop Reference to the desktop to which this snap manager is attached.
     * @param snapindicator If true then a snap indicator will be displayed automatically (when enabled in the preferences).
     * @param items_to_ignore These items will not be snapped to, e.g. the items that are currently being dragged. This avoids "self-snapping".
     * @param unselected_nodes Stationary nodes of the path that is currently being edited in the node tool and
     * that can be snapped too. Nodes not in this list will not be snapped to, to avoid "self-snapping". Of each
     * unselected node both the position (Geom::Point) and the type (Inkscape::SnapTargetType) will be stored.
     * @param guide_to_ignore Guide that is currently being dragged and should not be snapped to.
     */
    void setup(SPDesktop const *desktop,
               bool snapindicator,
               std::vector<SPItem const *> &items_to_ignore,
               std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes = NULL,
               SPGuide *guide_to_ignore = NULL);

    void setupIgnoreSelection(SPDesktop const *desktop,
                              bool snapindicator = true,
                              std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes = NULL,
                              SPGuide *guide_to_ignore = NULL);

    void unSetup() {_rotation_center_source_items.clear();
                    _guide_to_ignore = NULL;
                    _desktop = NULL;
                    _unselected_nodes = NULL;}

    // If we're dragging a rotation center, then setRotationCenterSource() stores the parent item
    // of this rotation center; this reference is used to make sure that we do not snap a rotation
    // center to itself
    // NOTE: Must be called after calling setup(), not before!
    void setRotationCenterSource(const std::vector<SPItem*> &items) {_rotation_center_source_items = items;}
    const std::vector<SPItem*> &getRotationCenterSource() {return _rotation_center_source_items;}

    // freeSnapReturnByRef() is preferred over freeSnap(), because it only returns a
    // point if snapping has occurred (by overwriting p); otherwise p is untouched

    /**
     * Try to snap a point to grids, guides or objects.
     *
     * Try to snap a point to grids, guides or objects, in two degrees-of-freedom,
     * i.e. snap in any direction on the two dimensional canvas to the nearest
     * snap target. freeSnapReturnByRef() is equal in snapping behavior to
     * freeSnap(), but the former returns the snapped point trough the referenced
     * parameter p. This parameter p initially contains the position of the snap
     * source and will we overwritten by the target position if snapping has occurred.
     * This makes snapping transparent to the calling code. If this is not desired
     * because either the calling code must know whether snapping has occurred, or
     * because the original position should not be touched, then freeSnap() should be
     * called instead.
     *
     * PS:
     * 1) SnapManager::setup() must have been called before calling this method,
     * although only once for each set of points
     * 2) Only to be used when a single source point is to be snapped; it assumes
     * that source_num = 0, which is inefficient when snapping sets our source points
     *
     * @param p Current position of the snap source; will be overwritten by the position of the snap target if snapping has occurred.
     * @param source_type Detailed description of the source type, will be used by the snap indicator.
     * @param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation.
     */
    void freeSnapReturnByRef(Geom::Point &p,
                             Inkscape::SnapSourceType const source_type,
                             Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    /**
     * Try to snap a point to grids, guides or objects.
     *
     * Try to snap a point to grids, guides or objects, in two degrees-of-freedom,
     * i.e. snap in any direction on the two dimensional canvas to the nearest
     * snap target. freeSnap() is equal in snapping behavior to
     * freeSnapReturnByRef(). Please read the comments of the latter for more details
     *
     * PS: SnapManager::setup() must have been called before calling this method,
     * although only once for each set of points
     *
     * @param p Source point to be snapped.
     * @param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation.
     * @param to_path_only Only snap to points on a path, such as path intersections with itself or with grids/guides. This is used for
     *        example when adding nodes to a path. We will not snap for example to grid intersections
     * @return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
     */
    Inkscape::SnappedPoint freeSnap(Inkscape::SnapCandidatePoint const &p,
                                    Geom::OptRect const &bbox_to_snap = Geom::OptRect(),
                                    bool to_path_only = false) const;

    void preSnap(Inkscape::SnapCandidatePoint const &p, bool to_path_only = false);

    /**
     * Snap to the closest multiple of a grid pitch.
     *
     * When pasting, we would like to snap to the grid. Problem is that we don't know which
     * nodes were aligned to the grid at the time of copying, so we don't know which nodes
     * to snap. If we'd snap an unaligned node to the grid, previously aligned nodes would
     * become unaligned. That's undesirable. Instead we will make sure that the offset
     * between the source and its pasted copy is a multiple of the grid pitch. If the source
     * was aligned, then the copy will therefore also be aligned.
     *
     * PS: Whether we really find a multiple also depends on the snapping range! Most users
     * will have "always snap" enabled though, in which case a multiple will always be found.
     * PS2: When multiple grids are present then the result will become ambiguous. There is no
     * way to control to which grid this method will snap.
     *
     * @param t Vector that represents the offset of the pasted copy with respect to the original.
     * @return Offset vector after snapping to the closest multiple of a grid pitch.
     */
    Geom::Point multipleOfGridPitch(Geom::Point const &t, Geom::Point const &origin);

    // constrainedSnapReturnByRef() is preferred over constrainedSnap(), because it only returns a
    // point, by overwriting p, if snapping has occurred; otherwise p is untouched

    /**
     * Try to snap a point along a constraint line to grids, guides or objects.
     *
     * Try to snap a point to grids, guides or objects, in only one degree-of-freedom,
     * i.e. snap in a specific direction on the two dimensional canvas to the nearest
     * snap target.
     *
     * constrainedSnapReturnByRef() is equal in snapping behavior to
     * constrainedSnap(), but the former returns the snapped point trough the referenced
     * parameter p. This parameter p initially contains the position of the snap
     * source and will be overwritten by the target position if snapping has occurred.
     * This makes snapping transparent to the calling code. If this is not desired
     * because either the calling code must know whether snapping has occurred, or
     * because the original position should not be touched, then constrainedSnap() should
     * be called instead. If there's nothing to snap to or if snapping has been disabled,
     * then this method will still apply the constraint (but without snapping)
     *
     * PS:
     * 1) SnapManager::setup() must have been called before calling this method,
     * although only once for each set of points
     * 2) Only to be used when a single source point is to be snapped; it assumes
     * that source_num = 0, which is inefficient when snapping sets our source points

     *
     * @param p Current position of the snap source; will be overwritten by the position of the snap target if snapping has occurred.
     * @param source_type Detailed description of the source type, will be used by the snap indicator.
     * @param constraint The direction or line along which snapping must occur.
     * @param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation.
     */
    void constrainedSnapReturnByRef(Geom::Point &p,
                                    Inkscape::SnapSourceType const source_type,
                                    Inkscape::Snapper::SnapConstraint const &constraint,
                                    Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    /**
     * Try to snap a point along a constraint line to grids, guides or objects.
     *
     * Try to snap a point to grids, guides or objects, in only one degree-of-freedom,
     * i.e. snap in a specific direction on the two dimensional canvas to the nearest
     * snap target. constrainedSnap is equal in snapping behavior to
     * constrainedSnapReturnByRef(). Please read the comments of the latter for more details.
     *
     * PS: SnapManager::setup() must have been called before calling this method,
     * although only once for each set of points
     * PS: If there's nothing to snap to or if snapping has been disabled, then this
     * method will still apply the constraint (but without snapping)
     *
     * @param p Source point to be snapped.
     * @param constraint The direction or line along which snapping must occur.
     * @param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation.
     */
    Inkscape::SnappedPoint constrainedSnap(Inkscape::SnapCandidatePoint const &p,
                                           Inkscape::Snapper::SnapConstraint const &constraint,
                                           Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    Inkscape::SnappedPoint multipleConstrainedSnaps(Inkscape::SnapCandidatePoint const &p,
                                                    std::vector<Inkscape::Snapper::SnapConstraint> const &constraints,
                                                    bool dont_snap = false,
                                                    Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    /**
     * Try to snap a point to something at a specific angle.
     *
     * When drawing a straight line or modifying a gradient, it will snap to specific angle increments
     * if CTRL is being pressed. This method will enforce this angular constraint (even if there is nothing
     * to snap to)
     *
     * @param p Source point to be snapped.
     * @param p_ref Optional original point, relative to which the angle should be calculated. If empty then
     * the angle will be calculated relative to the y-axis.
     * @param snaps Number of angular increments per PI radians; E.g. if snaps = 2 then we will snap every PI/2 = 90 degrees.
     */
    Inkscape::SnappedPoint constrainedAngularSnap(Inkscape::SnapCandidatePoint const &p,
                                                    boost::optional<Geom::Point> const &p_ref,
                                                    Geom::Point const &o,
                                                    unsigned const snaps) const;

    /**
     * Wrapper method to make snapping of the guide origin a bit easier (i.e. simplifies the calling code).
     *
     * PS: SnapManager::setup() must have been called before calling this method,
     *
     * @param p Current position of the point on the guide that is to be snapped; will be overwritten by the position of the snap target if snapping has occurred.
     * @param origin_or_vector Data used for tangential and perpendicular snapping. When rotating a guide the origin of the rotation is specified here, whereas when
     * dragging a guide its vector is specified here
     * @param origin If true then origin_or_vector contains an origin, other it contains a vector
     * @param freeze_angle If true (in which case origin is false), then the vector specified in origin_or_vector will not be touched, i.e. the guide will
     * in all circumstances keep its angle. Otherwise the vector in origin_or_vector can be updated, meaning that the guide might take on the angle of a curve that
     * has been snapped too tangentially or perpendicularly
     */
    void guideFreeSnap(Geom::Point &p, Geom::Point &origin_or_vector, bool origin, bool freeze_angle) const;

    /**
     * Wrapper method to make snapping of the guide origin a bit easier (i.e. simplifies the calling code).
     *
     * PS: SnapManager::setup() must have been called before calling this method,
     *
     * @param p Current position of the point on the guide that is to be snapped; will be overwritten by the position of the snap target if snapping has occurred.
     * @param guideline The guide that is currently being dragged
     */
    void guideConstrainedSnap(Geom::Point &p, SPGuide const &guideline) const;

    Inkscape::GuideSnapper guide;      ///< guide snapper
    Inkscape::ObjectSnapper object;    ///< snapper to other objects
    Inkscape::SnapPreferences snapprefs;

    /**
     * Return a list of snappers.
     *
     * Inkscape snaps to objects, grids, and guides. For each of these snap targets a
     * separate class is used, which has been derived from the base Snapper class. The
     * getSnappers() method returns a list of pointers to instances of this class. This
     * list contains exactly one instance of the guide snapper and of the object snapper
     * class, but any number of grid snappers (because each grid has its own snapper
     * instance)
     *
     * @return List of snappers that we use.
     */
    SnapperList getSnappers() const;

    /**
     * Return a list of gridsnappers.
     *
     * Each grid has its own instance of the snapper class. This way snapping can
     * be enabled per grid individually. A list will be returned containing the
     * pointers to these instances, but only for grids that are being displayed
     * and for which snapping is enabled.
     *
     * @return List of gridsnappers that we use.
     */
    SnapperList getGridSnappers() const;

    SPDesktop const *getDesktop() const {return _desktop;}
    SPNamedView const *getNamedView() const {return _named_view;}
    SPDocument *getDocument() const;
    SPGuide const *getGuideToIgnore() const {return _guide_to_ignore;}

    bool getSnapIndicator() const {return _snapindicator;}

    /**
     * Given a set of possible snap targets, find the best target (which is not necessarily
     * also the nearest target), and show the snap indicator if requested.
     *
     * @param p Source point to be snapped.
     * @param isr A structure holding all snap targets that have been found so far.
     * @param constrained True if the snap is constrained, e.g. for stretching or for purely horizontal translation.
     * @param allowOffScreen If true, then snapping to points which are off the screen is allowed (needed for example when pasting to the grid).
     * @param to_path_only Only snap to points on a path, such as path intersections with itself or with grids/guides. This is used for
     *        example when adding nodes to a path. We will not snap for example to grid intersections
     * @return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
     */
    Inkscape::SnappedPoint findBestSnap(Inkscape::SnapCandidatePoint const &p, IntermSnapResults const &isr, bool constrained, bool allowOffScreen = false, bool to_paths_only = false) const;

    /**
     * Mark the location of the snap source (not the snap target!) on the canvas by drawing a symbol.
     *
     * @param point_type Category of points to which the source point belongs: node, guide or bounding box.
     * @param p The transformed position of the source point, paired with an identifier of the type of the snap source.
     */
    void displaySnapsource(Inkscape::SnapCandidatePoint const &p) const;

    /**
     * Method for snapping sets of points while they are being transformed.
     *
     * Method for snapping sets of points while they are being transformed, when using
     * for example the selector tool. This method is for internal use only, and should
     * not have to be called directly. Use freeSnapTransalation(), constrainedSnapScale(),
     * etc. instead.
     *
     * This is what is being done in this method: transform each point, find out whether
     * a free snap or constrained snap is more appropriate, do the snapping, calculate
     * some metrics to quantify the snap "distance", and see if it's better than the
     * previous snap. Finally, the best ("nearest") snap from all these points is returned.
     * If no snap has occurred and we're asked for a constrained snap then the constraint
     * will be applied nevertheless
     *
     * @param points Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
     * @param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
     * @param transform Describes the type of transformation, it's parameters, and any additional constraints
     */
    void snapTransformed(std::vector<Inkscape::SnapCandidatePoint> const &points,
                                            Geom::Point const &pointer,
                                            Inkscape::PureTransform &transform);

protected:
    SPNamedView const *_named_view;

private:
    std::vector<SPItem const *> _items_to_ignore; ///< Items that should not be snapped to, for example the items that are currently being dragged. Set using the setup() method
    std::vector<SPItem*> _rotation_center_source_items; // to avoid snapping a rotation center to itself
    SPGuide *_guide_to_ignore; ///< A guide that should not be snapped to, e.g. the guide that is currently being dragged
    SPDesktop const *_desktop;
    bool _snapindicator; ///< When true, an indicator will be drawn at the position that was being snapped to
    std::vector<Inkscape::SnapCandidatePoint> *_unselected_nodes; ///< Nodes of the path that is currently being edited and which have not been selected and which will therefore be stationary. Only these nodes will be considered for snapping to. Of each unselected node both the position (Geom::Point) and the type (Inkscape::SnapTargetType) will be stored

};

#endif // !SEEN_SNAP_H

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
