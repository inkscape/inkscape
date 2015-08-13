/** @file
 * Path manipulator - a component that edits a single path on-canvas
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_PATH_MANIPULATOR_H
#define SEEN_UI_TOOL_PATH_MANIPULATOR_H

#include <string>
#include <memory>
#include <2geom/pathvector.h>
#include <2geom/affine.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "ui/tool/node.h"
#include "ui/tool/manipulator.h"
#include "live_effects/lpe-bspline.h"

struct SPCanvasItem;
class SPCurve;
class SPPath;

namespace Inkscape {
namespace XML { class Node; }

namespace UI {

class PathManipulator;
class ControlPointSelection;
class PathManipulatorObserver;
class CurveDragPoint;
class PathCanvasGroups;
class MultiPathManipulator;
class Node;
class Handle;

struct PathSharedData {
    NodeSharedData node_data;
    SPCanvasGroup *outline_group;
    SPCanvasGroup *dragpoint_group;
};

/**
 * Manipulator that edits a single path using nodes with handles.
 * Currently only cubic bezier and linear segments are supported, but this might change
 * some time in the future.
 */
class PathManipulator : public PointManipulator {
public:
    typedef SPPath *ItemType;

    PathManipulator(MultiPathManipulator &mpm, SPPath *path, Geom::Affine const &edit_trans,
        guint32 outline_color, Glib::ustring lpe_key);
    ~PathManipulator();
    virtual bool event(Inkscape::UI::Tools::ToolBase *, GdkEvent *);

    bool empty();
    void writeXML();
    void update(bool alert_LPE = false); // update display, but don't commit
    void clear(); // remove all nodes from manipulator
    SPPath *item() { return _path; }

    void selectSubpaths();
    void invertSelectionInSubpaths();

    void insertNodeAtExtremum(ExtremumType extremum);
    void insertNodes();
    void insertNode(Geom::Point);
    void insertNode(NodeList::iterator first, double t, bool take_selection);
    void duplicateNodes();
    void weldNodes(NodeList::iterator preserve_pos = NodeList::iterator());
    void weldSegments();
    void breakNodes();
    void deleteNodes(bool keep_shape = true);
    void deleteSegments();
    void reverseSubpaths(bool selected_only);
    void setSegmentType(SegmentType);

    void scaleHandle(Node *n, int which, int dir, bool pixel);
    void rotateHandle(Node *n, int which, int dir, bool pixel);

    void showOutline(bool show);
    void showHandles(bool show);
    void showPathDirection(bool show);
    void setLiveOutline(bool set);
    void setLiveObjects(bool set);
    void updateHandles();
    void setControlsTransform(Geom::Affine const &);
    void hideDragPoint();
    MultiPathManipulator &mpm() { return _multi_path_manipulator; }

    NodeList::iterator subdivideSegment(NodeList::iterator after, double t);
    NodeList::iterator extremeNode(NodeList::iterator origin, bool search_selected,
        bool search_unselected, bool closest);

    int _bsplineGetSteps() const;
    // this is necessary for Tab-selection in MultiPathManipulator
    SubpathList &subpathList() { return _subpaths; }

    static bool is_item_type(void *item);
private:
    typedef NodeList Subpath;
    typedef boost::shared_ptr<NodeList> SubpathPtr;

    void _createControlPointsFromGeometry();

    void _recalculateIsBSpline();
    bool _isBSpline() const;
    double _bsplineHandlePosition(Handle *h, bool check_other = true);
    Geom::Point _bsplineHandleReposition(Handle *h, bool check_other = true);
    Geom::Point _bsplineHandleReposition(Handle *h, double pos);
    void _createGeometryFromControlPoints(bool alert_LPE = false);
    unsigned _deleteStretch(NodeList::iterator first, NodeList::iterator last, bool keep_shape);
    std::string _createTypeString();
    void _updateOutline();
    //void _setOutline(Geom::PathVector const &);
    void _getGeometry();
    void _setGeometry();
    Glib::ustring _nodetypesKey();
    Inkscape::XML::Node *_getXMLNode();

    void _selectionChangedM(std::vector<SelectableControlPoint *> pvec, bool selected);
    void _selectionChanged(SelectableControlPoint * p, bool selected);
    bool _nodeClicked(Node *, GdkEventButton *);
    void _handleGrabbed();
    bool _handleClicked(Handle *, GdkEventButton *);
    void _handleUngrabbed();

    void _externalChange(unsigned type);
    void _removeNodesFromSelection();
    void _commit(Glib::ustring const &annotation);
    void _commit(Glib::ustring const &annotation, gchar const *key);
    Geom::Coord _updateDragPoint(Geom::Point const &);
    void _updateOutlineOnZoomChange();
    double _getStrokeTolerance();
    Handle *_chooseHandle(Node *n, int which);

    SubpathList _subpaths;
    MultiPathManipulator &_multi_path_manipulator;
    SPPath *_path; ///< can be an SPPath or an Inkscape::LivePathEffect::Effect  !!!
    SPCurve *_spcurve; // in item coordinates
    SPCanvasItem *_outline;
    CurveDragPoint *_dragpoint; // an invisible control point hovering over curve
    PathManipulatorObserver *_observer;
    Geom::Affine _d2i_transform; ///< desktop-to-item transform
    Geom::Affine _i2d_transform; ///< item-to-desktop transform, inverse of _d2i_transform
    Geom::Affine _edit_transform; ///< additional transform to apply to editing controls
    unsigned _num_selected; ///< number of selected nodes
    bool _show_handles;
    bool _show_outline;
    bool _show_path_direction;
    bool _live_outline;
    bool _live_objects;
    bool _is_bspline;
    Glib::ustring _lpe_key;

    friend class PathManipulatorObserver;
    friend class CurveDragPoint;
    friend class Node;
    friend class Handle;
};

} // namespace UI
} // namespace Inkscape

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
