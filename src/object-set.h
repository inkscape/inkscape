/*
 * Multiindex container for selection
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_PROTOTYPE_OBJECTSET_H
#define INKSCAPE_PROTOTYPE_OBJECTSET_H

#include <string>
#include <unordered_map>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/sub_range.hpp>
#include <boost/range/any_range.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <sigc++/connection.h>
#include <inkgc/gc-soft-ptr.h>
#include "sp-object.h"
#include "sp-item.h"
#include "sp-item-group.h"

class SPBox3D;
class Persp3D;
class SPDesktop;

namespace Inkscape {

namespace XML {
class Node;
}

struct hashed{};
struct random_access{};

struct is_item {
    bool operator()(SPObject* obj) {
        return SP_IS_ITEM(obj);
    }
};

struct is_group {
    bool operator()(SPObject* obj) {
        return SP_IS_GROUP(obj);
    }
};

struct object_to_item {
    typedef SPItem* result_type;
    SPItem* operator()(SPObject* obj) const {
        return SP_ITEM(obj);
    }
};

struct object_to_node {
    typedef XML::Node* result_type;
    XML::Node* operator()(SPObject* obj) const {
        return obj->getRepr();
    }
};

struct object_to_group {
    typedef SPGroup* result_type;
    SPGroup* operator()(SPObject* obj) const {
        return SP_GROUP(obj);
    }
};

typedef boost::multi_index_container<
        SPObject*,
        boost::multi_index::indexed_by<
                boost::multi_index::sequenced<>,
                boost::multi_index::random_access<
                        boost::multi_index::tag<random_access>>,
                boost::multi_index::hashed_unique<
                        boost::multi_index::tag<hashed>,
                        boost::multi_index::identity<SPObject*>>
        >> MultiIndexContainer;

typedef boost::any_range<
        SPObject*,
        boost::random_access_traversal_tag,
        SPObject* const&,
        std::ptrdiff_t> SPObjectRange;

class ObjectSet {
public:
    enum CompareSize {HORIZONTAL, VERTICAL, AREA};
    typedef decltype(MultiIndexContainer().get<random_access>() | boost::adaptors::filtered(is_item()) | boost::adaptors::transformed(object_to_item())) SPItemRange;
    typedef decltype(MultiIndexContainer().get<random_access>() | boost::adaptors::filtered(is_group()) | boost::adaptors::transformed(object_to_group())) SPGroupRange;
    typedef decltype(MultiIndexContainer().get<random_access>() | boost::adaptors::filtered(is_item()) | boost::adaptors::transformed(object_to_node())) XMLNodeRange;

    ObjectSet(SPDesktop* desktop): _desktop(desktop) {};
    ObjectSet(): _desktop(nullptr) {};
    virtual ~ObjectSet();

    /**
     * Add an SPObject to the set of selected objects.
     *
     * @param obj the SPObject to add
     */
    bool add(SPObject* object);

    /**  Add items from an STL iterator range to the selection.
     *  \param from the begin iterator
     *  \param to the end iterator
     */
    template <typename InputIterator>
    void add(InputIterator from, InputIterator to) {
        for(auto it = from; it != to; ++it) {
            _add(*it);
        }
        _emitSignals();
    }

    /**
     * Removes an item from the set of selected objects.
     *
     * It is ok to call this method for an unselected item.
     *
     * @param item the item to unselect
     *
     * @return is success
     */
    bool remove(SPObject* object);

    /**
     * Returns true if the given object is selected.
     */
    bool includes(SPObject *object);

    /**
     * Set the selection to a single specific object.
     *
     * @param obj the object to select
     */
    void set(SPObject *object);

    /**
     * Unselects all selected objects.
     */
    void clear();

    /**
     * Returns size of the selection.
     */
    int size();

    /**
     * Returns true if no items are selected.
     */
    bool isEmpty();

    /**
     * Removes an item if selected, adds otherwise.
     *
     * @param item the item to unselect
     */
    void toggle(SPObject *obj);

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

    /** Returns the list of selected objects. */
    SPObjectRange objects();

    /** Returns a range of selected SPItems. */
    SPItemRange items() {
        return SPItemRange(_container.get<random_access>()
           | boost::adaptors::filtered(is_item())
           | boost::adaptors::transformed(object_to_item()));
    };

    /** Returns a range of selected groups. */
    SPGroupRange groups() {
        return SPGroupRange (_container.get<random_access>()
            | boost::adaptors::filtered(is_group())
            | boost::adaptors::transformed(object_to_group()));
    }

    /** Returns a range of the xml nodes of all selected objects. */
    XMLNodeRange xmlNodes() {
        return XMLNodeRange(_container.get<random_access>()
                            | boost::adaptors::filtered(is_item())
                            | boost::adaptors::transformed(object_to_node()));
    }

    /**
     * Returns a single selected object's xml node.
     *
     * @return NULL unless exactly one object is selected
     */
    XML::Node *singleRepr();

    /**
     * Selects exactly the specified objects.
     *
     * @param objs the objects to select
     */
    template <class T>
    typename boost::enable_if<boost::is_base_of<SPObject, T>, void>::type
    setList(const std::vector<T*> &objs) {
        _clear();
        addList(objs);
    }

    /**
     * Adds the specified objects to selection, without deselecting first.
     *
     * @param objs the objects to select
     */
    template <class T>
    typename boost::enable_if<boost::is_base_of<SPObject, T>, void>::type
    addList(const std::vector<T*> &objs) {
        for (auto obj: objs) {
            if (!includes(obj)) {
                add(obj);
            }
        }
    }

    /** Returns the bounding rectangle of the selection. */
    Geom::OptRect bounds(SPItem::BBoxType type) const;
    Geom::OptRect visualBounds() const;
    Geom::OptRect geometricBounds() const;

    /**
     * Returns either the visual or geometric bounding rectangle of the selection, based on the
     * preferences specified for the selector tool
     */
    Geom::OptRect preferredBounds() const;

    /* Returns the bounding rectangle of the selectionin document coordinates.*/
    Geom::OptRect documentBounds(SPItem::BBoxType type) const;

    /**
     * Returns the rotation/skew center of the selection.
     */
    boost::optional<Geom::Point> center() const;

    /** Returns a list of all perspectives which have a 3D box in the current selection.
       (these may also be nested in groups) */
    std::list<Persp3D *> const perspList();

    /**
     * Returns a list of all 3D boxes in the current selection which are associated to @c
     * persp. If @c pers is @c NULL, return all selected boxes.
     */
    std::list<SPBox3D *> const box3DList(Persp3D *persp = NULL);

    /**
     * Returns the desktop the selection is bound to
     *
     * @return the desktop the selection is bound to, or NULL if in console mode
     */
    SPDesktop *desktop() { return _desktop; }

protected:
    virtual void _connectSignals(SPObject* object) {};
    virtual void _releaseSignals(SPObject* object) {};
    virtual void _emitSignals() {};
    void _add(SPObject* object);
    void _clear();
    void _remove(SPObject* object);
    bool _anyAncestorIsInSet(SPObject *object);
    void _removeDescendantsFromSet(SPObject *object);
    void _removeAncestorsFromSet(SPObject *object);
    SPItem *_sizeistItem(bool sml, CompareSize compare);
    SPObject *_getMutualAncestor(SPObject *object);
    virtual void _add3DBoxesRecursively(SPObject *obj);
    virtual void _remove3DBoxesRecursively(SPObject *obj);

    MultiIndexContainer _container;
    GC::soft_ptr<SPDesktop> _desktop;
    std::list<SPBox3D *> _3dboxes;
    std::unordered_map<SPObject*, sigc::connection> _releaseConnections;

};

typedef ObjectSet::SPItemRange SPItemRange;
typedef ObjectSet::SPGroupRange SPGroupRange;
typedef ObjectSet::XMLNodeRange XMLNodeRange;

} // namespace Inkscape

#endif //INKSCAPE_PROTOTYPE_OBJECTSET_H

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
