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
#include <2geom/matrix.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "display/display-forward.h"
#include "forward.h"
#include "ui/tool/node.h"
#include "ui/tool/manipulator.h"

struct SPCanvasItem;

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

    PathManipulator(MultiPathManipulator &mpm, SPPath *path, Geom::Matrix const &edit_trans,
        guint32 outline_color, Glib::ustring lpe_key);
    ~PathManipulator();
    virtual bool event(GdkEvent *);

    bool empty();
    void writeXML();
    void update(); // update display, but don't commit
    void clear(); // remove all nodes from manipulator
    SPPath *item() { return _path; }

    void selectSubpaths();
    void shiftSelection(int dir);
    void invertSelectionInSubpaths();

    void insertNodes();
    void weldNodes(NodeList::iterator preserve_pos = NodeList::iterator());
    void weldSegments();
    void breakNodes();
    void deleteNodes(bool keep_shape = true);
    void deleteSegments();
    void reverseSubpaths(bool selected_only);
    void setSegmentType(SegmentType);

    void showOutline(bool show);
    void showHandles(bool show);
    void showPathDirection(bool show);
    void setLiveOutline(bool set);
    void setLiveObjects(bool set);
    void setControlsTransform(Geom::Matrix const &);
    void hideDragPoint();

    NodeList::iterator subdivideSegment(NodeList::iterator after, double t);
    NodeList::iterator extremeNode(NodeList::iterator origin, bool search_selected,
        bool search_unselected, bool closest);

    static bool is_item_type(void *item);
private:
    typedef NodeList Subpath;
    typedef boost::shared_ptr<NodeList> SubpathPtr;

    void _createControlPointsFromGeometry();
    void _createGeometryFromControlPoints();
    unsigned _deleteStretch(NodeList::iterator first, NodeList::iterator last, bool keep_shape);
    std::string _createTypeString();
    void _updateOutline();
    //void _setOutline(Geom::PathVector const &);
    void _getGeometry();
    void _setGeometry();
    Glib::ustring _nodetypesKey();
    Inkscape::XML::Node *_getXMLNode();

    void _selectionChanged(SelectableControlPoint *p, bool selected);
    bool _nodeClicked(Node *, GdkEventButton *);
    void _handleGrabbed();
    bool _handleClicked(Handle *, GdkEventButton *);
    void _handleUngrabbed();

    void _externalChange(unsigned type);
    void _removeNodesFromSelection();
    void _commit(Glib::ustring const &annotation);
    void _updateDragPoint(Geom::Point const &);
    void _updateOutlineOnZoomChange();
    double _getStrokeTolerance();

    SubpathList _subpaths;
    MultiPathManipulator &_multi_path_manipulator;
    SPPath *_path;
    SPCurve *_spcurve; // in item coordinates
    SPCanvasItem *_outline;
    CurveDragPoint *_dragpoint; // an invisible control point hoverng over curve
    PathManipulatorObserver *_observer;
    Geom::Matrix _d2i_transform; ///< desktop-to-item transform
    Geom::Matrix _i2d_transform; ///< item-to-desktop transform, inverse of _d2i_transform
    Geom::Matrix _edit_transform; ///< additional transform to apply to editing controls
    unsigned _num_selected; ///< number of selected nodes
    bool _show_handles;
    bool _show_outline;
    bool _show_path_direction;
    bool _live_outline;
    bool _live_objects;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
