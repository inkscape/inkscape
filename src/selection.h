#ifndef SEEN_INKSCAPE_SELECTION_H
#define SEEN_INKSCAPE_SELECTION_H

/** \file
 * Inkscape::Selection: per-desktop selection container
 *
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
#include <sigc++/sigc++.h>

//#include "libnr/nr-rect.h"
#include "libnr/nr-convex-hull.h"
#include "forward.h"
#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include "gc-soft-ptr.h"
#include "util/list.h"
#include "sp-item.h"

class SPItem;
class SPBox3D;
class Persp3D;

namespace Inkscape {
namespace XML {
class Node;
}
}

namespace Inkscape {

/**
 * @brief The set of selected SPObjects for a given desktop.
 *
 * This class represents the set of selected SPItems for a given
 * SPDesktop.
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
    /**
     * Constructs an selection object, bound to a particular
     * SPDesktop
     *
     * @param desktop the desktop in question
     */
    Selection(SPDesktop *desktop);
    ~Selection();

    /**
     * @brief Returns the desktop the selection is bound to
     *
     * @return the desktop the selection is bound to
     */
    SPDesktop *desktop() { return _desktop; }

    /**
     * @brief Returns active layer for selection (currentLayer or its parent)
     *
     * @return layer item the selection is bound to
     */
    SPObject *activeContext();

    /**
     * @brief Add an SPObject to the set of selected objects
     *
     * @param obj the SPObject to add
     */
    void add(SPObject *obj, bool persist_selection_context = false);

    /**
     * @brief Add an XML node's SPObject to the set of selected objects
     *
     * @param the xml node of the item to add
     */
    void add(XML::Node *repr) { add(_objectForXMLNode(repr)); }

    /**
     * @brief Set the selection to a single specific object
     *
     * @param obj the object to select
     */
    void set(SPObject *obj, bool persist_selection_context = false);

    /**
     * @brief Set the selection to an XML node's SPObject
     *
     * @param repr the xml node of the item to select
     */
    void set(XML::Node *repr) { set(_objectForXMLNode(repr)); }

    /**
     * @brief Removes an item from the set of selected objects
     *
     * It is ok to call this method for an unselected item.
     *
     * @param item the item to unselect
     */
    void remove(SPObject *obj);

    /**
     * @brief Removes an item if selected, adds otherwise
     *
     * @param item the item to unselect
     */
    void toggle(SPObject *obj);

    /**
     * @brief Removes an item from the set of selected objects
     *
     * It is ok to call this method for an unselected item.
     *
     * @param repr the xml node of the item to remove
     */
    void remove(XML::Node *repr) { remove(_objectForXMLNode(repr)); }

    /**
     * @brief Selects exactly the specified objects
     *
     * @param objs the objects to select
     */
    void setList(GSList const *objs);

    /**
     * @brief Adds the specified objects to selection, without deselecting first
     *
     * @param objs the objects to select
     */
    void addList(GSList const *objs);

    /**
     * @brief Clears the selection and selects the specified objects
     *
     * @param repr a list of xml nodes for the items to select
     */
    void setReprList(GSList const *reprs);

    /** \brief  Add items from an STL iterator range to the selection
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
     * @brief Unselects all selected objects.
     */
    void clear();

    /**
     * @brief Returns true if no items are selected
     */
    bool isEmpty() const { return _objs == NULL; }

    /**
     * @brief Returns true if the given object is selected
     */
    bool includes(SPObject *obj) const;

    /**
     * @brief Returns true if the given item is selected
     */
    bool includes(XML::Node *repr) const {
        return includes(_objectForXMLNode(repr));
    }

    /**
     * @brief Returns a single selected object
     *
     * @return NULL unless exactly one object is selected
     */
    SPObject *single();

    /**
     * @brief Returns a single selected item
     *
     * @return NULL unless exactly one object is selected
     */
    SPItem *singleItem();

    /**
     * @brief Returns a single selected object's xml node
     *
     * @return NULL unless exactly one object is selected
     */
    XML::Node *singleRepr();

    /** @brief Returns the list of selected objects */
    GSList const *list();
    /** @brief Returns the list of selected SPItems */
    GSList const *itemList();
    /** @brief Returns a list of the xml nodes of all selected objects */
    /// \todo only returns reprs of SPItems currently; need a separate
    ///      method for that
    GSList const *reprList();

    /* list of all perspectives which have a 3D box in the current selection
       (these may also be nested in groups) */
    std::list<Persp3D *> const perspList();

    std::list<SPBox3D *> const box3DList();

    /** @brief Returns the number of layers in which there are selected objects */
    guint numberOfLayers();

    /** @brief Returns the number of parents to which the selected objects belong */
    guint numberOfParents();

    /** @brief Returns the bounding rectangle of the selection */
    NRRect *bounds(NRRect *dest, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) const;
    /** @brief Returns the bounding rectangle of the selection */
    Geom::OptRect bounds(SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) const;

    /**
     * @brief Returns the bounding rectangle of the selection
     *
     * \todo how is this different from bounds()?
     */ 
    NRRect *boundsInDocument(NRRect *dest, SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) const;

    /**
     * @brief Returns the bounding rectangle of the selection
     *
     * \todo how is this different from bounds()?
     */
    Geom::OptRect boundsInDocument(SPItem::BBoxType type = SPItem::APPROXIMATE_BBOX) const;

    /**
     * @brief Returns the rotation/skew center of the selection
     */
    boost::optional<Geom::Point> center() const;

    /**
     * @brief Gets the selection's snap points.
     * @return Selection's snap points
     */
    std::vector<Geom::Point> getSnapPoints(SnapPreferences const *snapprefs) const;

    /**
     * @brief Gets the snap points of a selection that form a convex hull.
     * @return Selection's convex hull points
     */
    std::vector<Geom::Point> getSnapPointsConvexHull(SnapPreferences const *snapprefs) const;

    /**
     * @brief Connects a slot to be notified of selection changes
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

    /**
     * @brief Connects a slot to be notified of selected 
     *        object modifications 
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
    sigc::connection connectModified(sigc::slot<void, Selection *, guint> const &slot)
    {
        return _modified_signal.connect(slot);
    }

private:
    /** @brief no copy */
    Selection(Selection const &);
    /** @brief no assign */
    void operator=(Selection const &);

    /** @brief Issues modification notification signals */
    static gboolean _emit_modified(Selection *selection);
    /** @brief Schedules an item modification signal to be sent */
    void _schedule_modified(SPObject *obj, guint flags);

    /** @brief Issues modified selection signal */
    void _emitModified(guint flags);
    /** @brief Issues changed selection signal */
    void _emitChanged(bool persist_selection_context = false);

    void _invalidateCachedLists();

    /** @brief unselect all descendants of the given item */
    void _removeObjectDescendants(SPObject *obj);
    /** @brief unselect all ancestors of the given item */
    void _removeObjectAncestors(SPObject *obj);
    /** @brief clears the selection (without issuing a notification) */
    void _clear();
    /** @brief adds an object (without issuing a notification) */
    void _add(SPObject *obj);
    /** @brief removes an object (without issuing a notification) */
    void _remove(SPObject *obj);
    /** @brief returns the SPObject corresponding to an xml node (if any) */
    SPObject *_objectForXMLNode(XML::Node *repr) const;
    /** @brief Releases an active layer object that is being removed */
    void _releaseContext(SPObject *obj);

    mutable GSList *_objs;
    mutable GSList *_reprs;
    mutable GSList *_items;

    void add_box_perspective(SPBox3D *box);
    void add_3D_boxes_recursively(SPObject *obj);
    void remove_box_perspective(SPBox3D *box);
    void remove_3D_boxes_recursively(SPObject *obj);

    std::map<Persp3D *, unsigned int> _persps;
    std::list<SPBox3D *> _3dboxes;

    GC::soft_ptr<SPDesktop> _desktop;
    SPObject* _selection_context;
    guint _flags;
    guint _idle;

    std::map<SPObject *, sigc::connection> _modified_connections;
    std::map<SPObject *, sigc::connection> _release_connections;
    sigc::connection _context_release_connection;

    sigc::signal<void, Selection *> _changed_signal;
    sigc::signal<void, Selection *, guint> _modified_signal;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
