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
    void unset_item (SubType type, bool keep_knotholder = false);

    bool has_nodepath (); //((deprecated))
    void update_knotholder (); //((deprecated))

    bool has_local_change (SubType type);
    void decrement_local_change (SubType type);

    GList *save_nodepath_selection ();
    void restore_nodepath_selection (GList *saved);

    void nodepath_destroyed ();

    bool has_selection ();

    Inkscape::NodePath::Path *get_nodepath() {return NULL;} //((deprecated))
    ShapeEditorsCollective *get_container() {return NULL;}

    // this one is only public because it's called from non-C++ repr changed callback
    void shapeeditor_event_attr_changed(gchar const *name);

    bool knot_mouseover();

private:
    bool has_knotholder ();
    void reset_item (SubType type, bool keep_knotholder = true);
    const SPItem *get_item (SubType type);

    SPDesktop *desktop;
    KnotHolder *knotholder;
    Inkscape::XML::Node *knotholder_listener_attached_for;
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

