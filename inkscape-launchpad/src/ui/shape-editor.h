#ifndef SEEN_SHAPE_EDITOR_H
#define SEEN_SHAPE_EDITOR_H

/*
 * Inkscape::ShapeEditor
 *
 * This is a container class which contains a knotholder for shapes.
 * It is attached to a single item.
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *
 */

class KnotHolder;
class LivePathEffectObject;
class SPDesktop;
class SPItem;

namespace Inkscape { namespace XML { class Node; }
namespace UI {

class ShapeEditor {
public:

    ShapeEditor(SPDesktop *desktop);
    ~ShapeEditor();

    void set_item(SPItem *item, bool keep_knotholder = false);
    void unset_item(bool keep_knotholder = false);

    void update_knotholder(); //((deprecated))

    bool has_local_change();
    void decrement_local_change();

    bool knot_mouseover() const;
    
    static void blockSetItem(bool b) { _blockSetItem = b; } // kludge

    static void event_attr_changed(Inkscape::XML::Node * /*repr*/, char const *name, char const * /*old_value*/,
                                   char const * /*new_value*/, bool /*is_interactive*/, void *data);
private:
    bool has_knotholder();
    void reset_item (bool keep_knotholder = true);
    const SPItem *get_item();

    static bool _blockSetItem;

    SPDesktop *desktop;
    KnotHolder *knotholder;
    Inkscape::XML::Node *knotholder_listener_attached_for;
};

} // namespace UI
} // namespace Inkscape

#endif // SEEN_SHAPE_EDITOR_H


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

