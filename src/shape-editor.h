#ifndef __SHAPE_EDITOR_H__
#define __SHAPE_EDITOR_H__

/*
 * Inkscape::ShapeEditor
 *
 * This is a container class which contains either knotholder (for shapes) or nodepath (for
 * paths). It is attached to a single item so only one of these is active at a time.
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *
 */

#include <forward.h>
#include <2geom/forward.h>

namespace Inkscape { namespace NodePath { class Path; } }
namespace Inkscape { namespace XML { class Node; } }

class KnotHolder;
class SPDesktop;
class SPNodeContext;
class ShapeEditorsCollective;
class LivePathEffectObject;

#include "libnr/nr-path-code.h"
#include <2geom/point.h>
#include <boost/optional.hpp>
#include <vector>

enum SubType{
    SH_NODEPATH,
    SH_KNOTHOLDER
};

class ShapeEditor {
public:

    ShapeEditor(SPDesktop *desktop);
    ~ShapeEditor();

    void set_item (SPItem *item, SubType type, bool keep_knotholder = false);
    void set_item_lpe_path_parameter(SPItem *item, LivePathEffectObject *lpeobject, const char * key);
    void unset_item (SubType type, bool keep_knotholder = false);

    bool has_nodepath (); //((deprecated))
    void update_knotholder (); //((deprecated))

    bool has_local_change (SubType type);
    void decrement_local_change (SubType type);

    GList *save_nodepath_selection ();
    void restore_nodepath_selection (GList *saved);

    void nodepath_destroyed ();

    void update_statusbar ();

    bool is_over_stroke (Geom::Point event_p, bool remember);

    void add_node_near_point(); // uses the shapeeditor's remembered point, if any

    void select_segment_near_point(bool toggle); // uses the shapeeditor's remembered point, if any

    void cancel_hit ();

    bool hits_curve ();

    void curve_drag (gdouble eventx, gdouble eventy);

    void finish_drag ();

    void select_rect (Geom::Rect  const &rect, bool add);

    bool has_selection ();
    void deselect ();

    Inkscape::NodePath::Path *get_nodepath() {return nodepath;} //((deprecated))
    ShapeEditorsCollective *get_container() {return container;}

    void add_node();

    void delete_nodes();
    void delete_nodes_preserving_shape();
    void delete_segment();

    void set_node_type(int type);

    void break_at_nodes();
    void join_nodes();
    void join_segments();

    void duplicate_nodes();

    void set_type_of_segments(NRPathcode code);

    void move_nodes(gdouble dx, gdouble dy);
    void move_nodes_screen(SPDesktop *desktop, gdouble dx, gdouble dy);

    void rotate_nodes(gdouble angle, int which, bool screen);

    void scale_nodes(gdouble const grow, int const which);
    void scale_nodes_screen(gdouble const grow, int const which);

    void select_all (bool invert);
    void select_all_from_subpath (bool invert);
    void select_next ();
    void select_prev ();

    void show_handles (bool show);
    void show_helperpath (bool show);

    void flip (Geom::Dim2 axis, boost::optional<Geom::Point> center = boost::optional<Geom::Point>());

    void distribute (Geom::Dim2 axis);
    void align (Geom::Dim2 axis);

    bool nodepath_edits_repr_key(gchar const *name);

    // this one is only public because it's called from non-C++ repr changed callback
    void shapeeditor_event_attr_changed(gchar const *name);

private:
    bool has_knotholder ();
    void reset_item (SubType type, bool keep_knotholder = true);
    const SPItem *get_item (SubType type);

    SPDesktop *desktop;

    Inkscape::NodePath::Path *nodepath;

    // TODO: std::list<KnotHolder *> knotholders;
    KnotHolder *knotholder;

    ShapeEditorsCollective *container;

    //Inkscape::XML::Node *lidtened_repr;

    double grab_t;
    int grab_node; // number of node grabbed by sp_node_context_is_over_stroke
    bool hit;
    Geom::Point curvepoint_event; // int coords from event
    Geom::Point curvepoint_doc; // same, in doc coords

    Inkscape::XML::Node *listener_attached_for;
};


/* As the next stage, this will be a collection of multiple ShapeEditors,
with the same interface as the single ShapeEditor, passing the actions to all its
contained ShapeEditors. Thus it should be easy to switch node context from 
using a single ShapeEditor to using a ShapeEditorsCollective. */

class ShapeEditorsCollective {
public:

    ShapeEditorsCollective(SPDesktop *desktop);
    ~ShapeEditorsCollective();

    void update_statusbar();

private:
    std::vector<ShapeEditor> editors;

    SPNodeContext *nc; // who holds us
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

