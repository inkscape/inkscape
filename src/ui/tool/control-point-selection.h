/** @file
 * Node selection - stores a set of nodes and applies transformations
 * to them
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_NODE_SELECTION_H
#define SEEN_UI_TOOL_NODE_SELECTION_H

#include <memory>
#include <boost/optional.hpp>
#include "util/set-types.h"
#include <sigc++/sigc++.h>
#include <2geom/forward.h>
#include <2geom/point.h>
#include <2geom/rect.h>
#include "display/display-forward.h"
#include "util/accumulators.h"
#include "ui/tool/commit-events.h"
#include "ui/tool/manipulator.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
class TransformHandleSet;
class SelectableControlPoint;
}
}

#ifdef USE_GNU_HASHES
namespace __gnu_cxx {
template<>
struct hash<Inkscape::UI::SelectableControlPoint*> {
    size_t operator()(Inkscape::UI::SelectableControlPoint *p) const {
        return reinterpret_cast<size_t>(p);
    }
};
} // namespace __gnu_cxx
#endif // USE_GNU_HASHES

namespace Inkscape {
namespace UI {

class ControlPointSelection : public Manipulator, public sigc::trackable {
public:
    ControlPointSelection(SPDesktop *d, SPCanvasGroup *th_group);
    ~ControlPointSelection();
    typedef optim_set< SelectableControlPoint * > set_type;
    typedef set_type Set; // convenience alias

    typedef set_type::iterator iterator;
    typedef set_type::const_iterator const_iterator;
    typedef set_type::size_type size_type;
    typedef SelectableControlPoint *value_type;
    typedef SelectableControlPoint *key_type;

    // size
    bool empty() { return _points.empty(); }
    size_type size() { return _points.size(); }

    // iterators
    iterator begin() { return _points.begin(); }
    const_iterator begin() const { return _points.begin(); }
    iterator end() { return _points.end(); }
    const_iterator end() const { return _points.end(); }

    // insert
    std::pair<iterator, bool> insert(const value_type& x);
    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    // erase
    void clear();
    void erase(iterator pos);
    size_type erase(const key_type& k);
    void erase(iterator first, iterator last);

    // find
    iterator find(const key_type &k) {
        return _points.find(k);
    }

    // Sometimes it is very useful to keep a list of all selectable points.
    set_type const &allPoints() const { return _all_points; }
    set_type &allPoints() { return _all_points; }
    // ...for example in these methods. Another useful case is snapping.
    void selectAll();
    void selectArea(Geom::Rect const &);
    void invertSelection();
    void spatialGrow(SelectableControlPoint *origin, int dir);

    virtual bool event(GdkEvent *);

    void transform(Geom::Matrix const &m);
    void align(Geom::Dim2 d);
    void distribute(Geom::Dim2 d);

    Geom::OptRect pointwiseBounds();
    Geom::OptRect bounds();

    bool transformHandlesEnabled() { return _handles_visible; }
    void showTransformHandles(bool v, bool one_node);
    // the two methods below do not modify the state; they are for use in manipulators
    // that need to temporarily hide the handles, for example when moving a node
    void hideTransformHandles();
    void restoreTransformHandles();
    void toggleTransformHandlesMode();

    sigc::signal<void> signal_update;
    sigc::signal<void, SelectableControlPoint *, bool> signal_point_changed;
    sigc::signal<void, CommitEvent> signal_commit;
private:
    // The functions below are invoked from SelectableControlPoint.
    // Previously they were connected to handlers when selecting, but this
    // creates problems when dragging a point that was not selected.
    void _pointGrabbed(SelectableControlPoint *);
    void _pointDragged(Geom::Point &, GdkEventMotion *);
    void _pointUngrabbed();
    bool _pointClicked(SelectableControlPoint *, GdkEventButton *);
    void _pointChanged(SelectableControlPoint *, bool);
    void _mouseoverChanged();

    void _updateTransformHandles(bool preserve_center);
    void _updateBounds();
    bool _keyboardMove(GdkEventKey const &, Geom::Point const &);
    bool _keyboardRotate(GdkEventKey const &, int);
    bool _keyboardScale(GdkEventKey const &, int);
    bool _keyboardFlip(Geom::Dim2);
    void _keyboardTransform(Geom::Matrix const &);
    void _commitHandlesTransform(CommitEvent ce);
    double _rotationRadius(Geom::Point const &);

    set_type _points;
    set_type _all_points;
    optim_map<SelectableControlPoint *, Geom::Point> _original_positions;
    boost::optional<double> _rot_radius;
    boost::optional<double> _mouseover_rot_radius;
    Geom::OptRect _bounds;
    TransformHandleSet *_handles;
    SelectableControlPoint *_grabbed_point, *_farthest_point;
    unsigned _dragging         : 1;
    unsigned _handles_visible  : 1;
    unsigned _one_node_handles : 1;

    friend class SelectableControlPoint;
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
