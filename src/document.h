#ifndef SEEN_SP_DOCUMENT_H
#define SEEN_SP_DOCUMENT_H

/** \file
 * SPDocument: Typed SVG document implementation
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stddef.h>
#include <sigc++/sigc++.h>
#include "libcroco/cr-cascade.h"
#include <2geom/forward.h>
#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include <glibmm/ustring.h>
#include <boost/ptr_container/ptr_list.hpp>
#include <vector>

namespace Avoid {
class Router;
}

class  SPItem;
class  SPObject;
class SPGroup;
class SPRoot;

namespace Inkscape {
    class Selection; 
    class UndoStackObserver;
    class EventLog;
    class ProfileManager;
    namespace XML {
        struct Document;
        class Node;
    }
    namespace Util {
        class Unit;
        class Quantity;
    }
}

class SPDefs;
class SP3DBox;
class Persp3D;
class Persp3DImpl;
class SPItemCtx;

namespace Proj {
    class TransfMat3x4;
}

struct SPDocumentPrivate;

/// Typed SVG document implementation.
class SPDocument : public Inkscape::GC::Managed<>,
                    public Inkscape::GC::Finalized,
                    public Inkscape::GC::Anchored
{
// Note: multiple public and private sections is not a good practice, but happens
// in this class as transitional to fixing encapsulation:
public:
    typedef sigc::signal<void, SPObject *> IDChangedSignal;
    typedef sigc::signal<void> ResourcesChangedSignal;
    typedef sigc::signal<void, unsigned> ModifiedSignal;
    typedef sigc::signal<void, char const *> URISetSignal;
    typedef sigc::signal<void, double, double> ResizedSignal;
    typedef sigc::signal<void> ReconstructionStart;
    typedef sigc::signal<void> ReconstructionFinish;
    typedef sigc::signal<void> CommitSignal;

    SPDocument();
    virtual ~SPDocument();

    sigc::connection connectDestroy(sigc::signal<void>::slot_type slot);


    unsigned int keepalive : 1;
    unsigned int virgin    : 1; ///< Has the document never been touched?
    unsigned int modified_since_save : 1;

    Inkscape::XML::Document *rdoc; ///< Our Inkscape::XML::Document
    Inkscape::XML::Node *rroot; ///< Root element of Inkscape::XML::Document
private:
    SPRoot *root;             ///< Our SPRoot
public:
    CRCascade *style_cascade;

protected:
    char *uri;   ///< A filename (not a URI yet), or NULL
    char *base;  ///< To be used for resolving relative hrefs.
    char *name;  ///< basename(uri) or other human-readable label for the document.

public:

    SPDocumentPrivate *priv;

    /// Last action key
    Glib::ustring actionkey;

    /// Handler ID
    unsigned modified_id;
    
    /// Connector rerouting handler ID
    unsigned rerouting_handler_id;

    Inkscape::ProfileManager* profileManager;

    // Instance of the connector router
    Avoid::Router *router;

    GSList *_collection_queue;

    bool oldSignalsConnected;

    /** Returns our SPRoot */
    SPRoot *getRoot() { return root; }
    SPRoot const *getRoot() const { return root; }

    Inkscape::XML::Node *getReprRoot() { return rroot; }

    /** Our Inkscape::XML::Document. */
    Inkscape::XML::Document *getReprDoc() { return rdoc; }
    Inkscape::XML::Document const *getReprDoc() const { return rdoc; }

    /** A filename (not a URI yet), or NULL */
    char const *getURI() const { return uri; }
    void setUri(char const *uri);

    /** To be used for resolving relative hrefs. */
    char const *getBase() const { return base; };
    void setBase( char const* base );

    /** basename(uri) or other human-readable label for the document. */
    char const* getName() const { return name; }

    /** Return the main defs object for the document. */
    SPDefs *getDefs();


    void setCurrentPersp3D(Persp3D * const persp);
    inline void setCurrentPersp3DImpl(Persp3DImpl * const persp_impl) { current_persp3d_impl = persp_impl; }
    /*
     * getCurrentPersp3D returns current_persp3d (if non-NULL) or the first
     * perspective in the defs. If no perspective exists, returns NULL.
     */
    Persp3D * getCurrentPersp3D();
    Persp3DImpl * getCurrentPersp3DImpl();

    void getPerspectivesInDefs(std::vector<Persp3D*> &list) const;

    unsigned int numPerspectivesInDefs() const {
        std::vector<Persp3D*> list;
        getPerspectivesInDefs(list);
        return list.size();
    }

    sigc::connection connectModified(ModifiedSignal::slot_type slot);
    sigc::connection connectURISet(URISetSignal::slot_type slot);
    sigc::connection connectResized(ResizedSignal::slot_type slot);
    sigc::connection connectCommit(CommitSignal::slot_type slot);

    void bindObjectToId(char const *id, SPObject *object);
    SPObject *getObjectById(Glib::ustring const &id) const;
    SPObject *getObjectById(char const *id) const;
    sigc::connection connectIdChanged(const char *id, IDChangedSignal::slot_type slot);

    void bindObjectToRepr(Inkscape::XML::Node *repr, SPObject *object);
    SPObject *getObjectByRepr(Inkscape::XML::Node *repr) const;

    Glib::ustring getLanguage() const;

    void queueForOrphanCollection(SPObject *object);
    void collectOrphans();

    void _emitModified();

    void addUndoObserver(Inkscape::UndoStackObserver& observer);
    void removeUndoObserver(Inkscape::UndoStackObserver& observer);

    bool _updateDocument();

    /// Are we currently in a transition between two "known good" states of the document?
    bool isSeeking() const;

    bool isModifiedSinceSave() const { return modified_since_save; }
    void setModifiedSinceSave(bool modified = true);

private:
    SPDocument(SPDocument const &); // no copy
    void operator=(SPDocument const &); // no assign

    Persp3D *current_persp3d; /**< Currently 'active' perspective (to which, e.g., newly created boxes are attached) */
    Persp3DImpl *current_persp3d_impl;

    // A list of svg documents being used or shown within this document
    boost::ptr_list<SPDocument> _child_documents;
    // Conversely this is a parent document because this is a child.
    SPDocument *_parent_document;

public:
    sigc::connection connectReconstructionStart(ReconstructionStart::slot_type slot);
    sigc::connection connectReconstructionFinish(ReconstructionFinish::slot_type slot);
    void emitReconstructionStart(void);
    void emitReconstructionFinish(void);

    unsigned long serial() const;
    void reset_key(void *dummy);
    sigc::connection _selection_changed_connection;
    sigc::connection _desktop_activated_connection;

    sigc::connection connectResourcesChanged(char const *key, SPDocument::ResourcesChangedSignal::slot_type slot);

    void fitToRect(Geom::Rect const &rect, bool with_margins = false);
    static SPDocument *createNewDoc(char const*uri, unsigned int keepalive,
            bool make_new = false, SPDocument *parent=NULL );
    static SPDocument *createNewDocFromMem(char const*buffer, int length, unsigned int keepalive);
           SPDocument *createChildDoc(std::string const &uri);

    /**
     * Returns the bottommost item from the list which is at the point, or NULL if none.
     */
    static SPItem *getItemFromListAtPointBottom(unsigned int dkey, SPGroup *group, const GSList *list, Geom::Point const &p, bool take_insensitive = false);

    static SPDocument *createDoc(Inkscape::XML::Document *rdoc, char const *uri,
            char const *base, char const *name, unsigned int keepalive,
            SPDocument *parent);

    SPDocument *doRef();
    SPDocument *doUnref();
    Inkscape::Util::Unit const* getDisplayUnit() const;
    Inkscape::Util::Unit const& getSVGUnit() const;
    Geom::Scale getDocumentScale() const;
    Inkscape::Util::Quantity getWidth() const;
    Inkscape::Util::Quantity getHeight() const;
    Geom::Point getDimensions() const;
    Geom::OptRect preferredBounds() const;
    void setWidthAndHeight(const Inkscape::Util::Quantity &width, const Inkscape::Util::Quantity &height, bool changeSize=true);
    void setWidth(const Inkscape::Util::Quantity &width, bool changeSize=true);
    void setHeight(const Inkscape::Util::Quantity &height, bool changeSize=true);
    void setViewBox(const Geom::Rect &viewBox);
    void requestModified();
    int ensureUpToDate();
    bool addResource(char const *key, SPObject *object);
    bool removeResource(char const *key, SPObject *object);
    const GSList *getResourceList(char const *key) const;
    GSList *getItemsInBox(unsigned int dkey, Geom::Rect const &box) const;
    GSList *getItemsPartiallyInBox(unsigned int dkey, Geom::Rect const &box) const;
    SPItem *getItemAtPoint(unsigned int key, Geom::Point const &p, bool into_groups, SPItem *upto = NULL) const;
    GSList *getItemsAtPoints(unsigned const key, std::vector<Geom::Point> points) const;
    SPItem *getGroupAtPoint(unsigned int key,  Geom::Point const &p) const;

    void changeUriAndHrefs(char const *uri);
    void emitResizedSignal(double width, double height);
	
    unsigned int vacuumDocument();

    void importDefs(SPDocument *source);

private:
    void do_change_uri(char const *const filename, bool const rebase);
    void setupViewport(SPItemCtx *ctx);
};

/*
 * Ideas: How to overcome style invalidation nightmare
 *
 * 1. There is reference request dictionary, that contains
 * objects (styles) needing certain id. Object::build checks
 * final id against it, and invokes necesary methods
 *
 * 2. Removing referenced object is simply prohibited -
 * needs analyse, how we can deal with situations, where
 * we simply want to ungroup etc. - probably we need
 * Repr::reparent method :( [Or was it ;)]
 *
 */

#endif // SEEN_SP_DOCUMENT_H

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
