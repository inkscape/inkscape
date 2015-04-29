#ifndef SEEN_INKSCAPE_SELECTION_H
#define SEEN_INKSCAPE_SELECTION_H
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include <map>
#include <list>
#include <set>
#include <stddef.h>
#include <sigc++/sigc++.h>

#include "inkgc/gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include "inkgc/gc-soft-ptr.h"
#include "sp-item.h"



class SPDesktop;
class SPItem;
class SPBox3D;
class Persp3D;

namespace Inkscape {
class LayerModel;
namespace XML {
class Node;
}
}


namespace Inkscape {

/**
 * The set of selected SPObjects for a given document and layer model.
 *
 * This class represents the set of selected SPItems for a given
 * document (referenced in LayerModel).
 *
 * An SPObject and its parent cannot be simultaneously selected;
 * selecting an SPObjects has the side-effect of unselecting any of
 * its children which might have been selected.
 *
 * This is a per-desktop object that keeps the list of selected objects
 * at the given desktop. Both SPItem and SPRepr lists can be retrieved
 * from the selection. Many actions operate on the selection, so it is
 * widely used throughout the code.
 * It also implements its own asynchronous notification signals that
 * UI elements can listen to.
 */
class Selection : public Inkscape::GC::Managed<>,
                  public Inkscape::GC::Finalized,
                  public Inkscape::GC::Anchored
{
public:
    enum CompareSize { HORIZONTAL, VERTICAL, AREA };
    /**
     * Constructs an selection object, bound to a particular
     * layer model
     *
     * @param layers the layer model (for the SPDesktop, if GUI)
     * @param desktop the desktop associated with the layer model, or NULL if in console mode
     */
    Selection(LayerModel *layers, SPDesktop *desktop);
    ~Selection();

    /**
     * Returns the layer model the selection is bound to (works in console or GUI mode)
     *
     * @return the layer model the selection is bound to, which is the same as the desktop
     * layer model for GUI mode
     */
    LayerModel *layers() { return _layers; }

    /**
     * Returns the desktop the selection is bound to
     *
     * @return the desktop the selection is bound to, or NULL if in console mode
     */
    SPDesktop *desktop() { return _desktop; }

    /**
     * Returns active layer for selection (currentLayer or its parent).
     *
     * @return layer item the selection is bound to
     */
    SPObject *activeContext();

    /**
     * Add an SPObject to the set of selected objects.
     *
     * @param obj the SPObject to add
     */
    void add(SPObject *obj, bool persist_selection_context = false);

    /**
     * Add an XML node's SPObject to the set of selected objects.
     *
     * @param the xml node of the item to add
     */
    void add(XML::Node *repr) { add(_objectForXMLNode(repr)); }

    /**
     * Set the selection to a single specific object.
     *
     * @param obj the object to select
     */
    void set(SPObject *obj, bool persist_selection_context = false);

    /**
     * Set the selection to an XML node's SPObject.
     *
     * @param repr the xml node of the item to select
     */
    void set(XML::Node *repr) { set(_objectForXMLNode(repr)); }

    /**
     * Removes an item from the set of selected objects.
     *
     * It is ok to call this method for an unselected item.
     *
     * @param item the item to unselect
     */
    void remove(SPObject *obj);

    /**
     * Removes an item if selected, adds otherwise.
     *
     * @param item the item to unselect
     */
    void toggle(SPObject *obj);

    /**
     * Removes an item from the set of selected objects.
     *
     * It is ok to call this method for an unselected item.
     *
     * @param repr the xml node of the item to remove
     */
    void remove(XML::Node *repr) { remove(_objectForXMLNode(repr)); }

    /**
     * Selects exactly the specified objects.
     *
     * @param objs the objects to select
     */
    void setList(std::vector<SPItem*> const &objs);

    /**
     * Adds the specified objects to selection, without deselecting first.
     *
     * @param objs the objects to select
     */
    void addList(std::vector<SPItem*> const &objs);

    /**
     * Clears the selection and selects the specified objects.
     *
     * @param repr a list of xml nodes for the items to select
     */
    void setReprList(std::vector<XML::Node*> const &reprs);

    /**  Add items from an STL iterator range to the selection.
     *  \param  from the begin iterator
     *  \param  to   the end iterator
     */
    template <typename InputIterator>
    void add(InputIterator from, InputIterator to) {
        _invalidateCachedLists();
        while ( from != to ) {
            _add(*from);
            ++from;
        }
        _emitChanged();
    }

    /**
     * Unselects all selected objects..
     */
    void clear();

    /**
     * Returns true if no items are selected.
     */
    bool isEmpty() const { return _objs.empty(); }

    /**
     * Returns true if the given object is selected.
     */
    bool includes(SPObject *obj) const;

    /**
     * Returns true if the given item is selected.
     */
    bool includes(XML::Node *repr) const {
        return includes(_objectForXMLNode(repr));
    }

    /**
     * Returns a single selected object.
     *
     * @return NULL unless exactly one object is selected
     */
    SPObject *single();

    /**
     * Returns a single selected item.
     *
     * @return NULL unless exactly one object is selected
     */
    SPItem *singleItem();

    /**
     * Returns the smallest item from this selection.
     */
    SPItem *smallestItem(CompareSize compare);

    /**
     * Returns the largest item from this selection.
     */
    SPItem *largestItem(CompareSize compare);

    /**
     * Returns a single selected object's xml node.
     *
     * @return NULL unless exactly one object is selected
     */
    XML::Node *singleRepr();

    /** Returns the list of selected objects. */
    std::vector<SPObject*> const &list();
    /** Returns the list of selected SPItems. */
    std::vector<SPItem*> const &itemList();
    /** Returns a list of the xml nodes of all selected objects. */
    /// \todo only returns reprs of SPItems currently; need a separate
    ///      method for that
    std::vector<XML::Node*> const &reprList();

    /** Returns a list of all perspectives which have a 3D box in the current selection.
       (these may also be nested in groups) */
    std::list<Persp3D *> const perspList();

    /**
     * Returns a list of all 3D boxes in the current selection which are associated to @c
     * persp. If @c pers is @c NULL, return all selected boxes.
     */
    std::list<SPBox3D *> const box3DList(Persp3D *persp = NULL);

    /** Returns the number of layers in which there are selected objects. */
    unsigned int numberOfLayers();

    /** Returns the number of parents to which the selected objects belong. */
    unsigned int numberOfParents();

    /** Returns the bounding rectangle of the selection. */
    Geom::OptRect bounds(SPItem::BBoxType type) const;
    Geom::OptRect visualBounds() const;
    Geom::OptRect geometricBounds() const;

    /**
     * Returns either the visual or geometric bounding rectangle of the selection, based on the
     * preferences specified for the selector tool
     */
    Geom::OptRect preferredBounds() const;

    /// Returns the bounding rectangle of the selectionin document coordinates.
    Geom::OptRect documentBounds(SPItem::BBoxType type) const;

    /**
     * Returns the rotation/skew center of the selection.
     */
    boost::optional<Geom::Point> center() const;

    /**
     * Compute the list of points in the selection that are to be considered for snapping from.
     *
     * @return Selection's snap points
     */
    std::vector<Inkscape::SnapCandidatePoint> getSnapPoints(SnapPreferences const *snapprefs) const;

    /**
     * Connects a slot to be notified of selection changes.
     *
     * This method connects the given slot such that it will
     * be called upon any change in the set of selected objects.
     *
     * @param slot the slot to connect
     *
     * @return the resulting connection
     */
    sigc::connection connectChanged(sigc::slot<void, Selection *> const &slot) {
        return _changed_signal.connect(slot);
    }
    sigc::connection connectChangedFirst(sigc::slot<void, Selection *> const &slot)
    {
        return _changed_signal.slots().insert(_changed_signal.slots().begin(), slot);
    }

    /**
     * Connects a slot to be notified of selected object modifications.
     *
     * This method connects the given slot such that it will
     * receive notifications whenever any selected item is
     * modified.
     *
     * @param slot the slot to connect
     *
     * @return the resulting connection
     *
     */
    sigc::connection connectModified(sigc::slot<void, Selection *, unsigned int> const &slot)
    {
        return _modified_signal.connect(slot);
    }
    sigc::connection connectModifiedFirst(sigc::slot<void, Selection *, unsigned int> const &slot)
    {
        return _modified_signal.slots().insert(_modified_signal.slots().begin(), slot);
    }

private:
    /** no copy. */
    Selection(Selection const &);
    /** no assign. */
    void operator=(Selection const &);

    /** Issues modification notification signals. */
    static int _emit_modified(Selection *selection);
    /** Schedules an item modification signal to be sent. */
    void _schedule_modified(SPObject *obj, unsigned int flags);

    /** Issues modified selection signal. */
    void _emitModified(unsigned int flags);
    /** Issues changed selection signal. */
    void _emitChanged(bool persist_selection_context = false);

    void _invalidateCachedLists();

    /** unselect all descendants of the given item. */
    void _removeObjectDescendants(SPObject *obj);
    /** unselect all ancestors of the given item. */
    void _removeObjectAncestors(SPObject *obj);
    /** clears the selection (without issuing a notification). */
    void _clear();
    /** adds an object (without issuing a notification). */
    void _add(SPObject *obj);
    /** removes an object (without issuing a notification). */
    void _remove(SPObject *obj);
    /** returns the SPObject corresponding to an xml node (if any). */
    SPObject *_objectForXMLNode(XML::Node *repr) const;
    /** Releases an active layer object that is being removed. */
    void _releaseContext(SPObject *obj);

    mutable std::list<SPObject*> _objs; //to more efficiently remove arbitrary elements
    mutable std::vector<SPObject*> _objs_vector; // to be returned by list();
    mutable std::set<SPObject*> _objs_set; //to efficiently test if object is selected
    mutable std::vector<XML::Node*> _reprs;
    mutable std::vector<SPItem*> _items;

    void add_box_perspective(SPBox3D *box);
    void add_3D_boxes_recursively(SPObject *obj);
    void remove_box_perspective(SPBox3D *box);
    void remove_3D_boxes_recursively(SPObject *obj);
    SPItem *_sizeistItem(bool sml, CompareSize compare);

    std::list<SPBox3D *> _3dboxes;

    LayerModel *_layers;
    GC::soft_ptr<SPDesktop> _desktop;
    SPObject* _selection_context;
    unsigned int _flags;
    unsigned int _idle;

    std::map<SPObject *, sigc::connection> _modified_connections;
    std::map<SPObject *, sigc::connection> _release_connections;
    sigc::connection _context_release_connection;

    sigc::signal<void, Selection *> _changed_signal;
    sigc::signal<void, Selection *, unsigned int> _modified_signal;
};

}

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
