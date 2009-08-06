#ifndef __SP_DOCUMENT_H__
#define __SP_DOCUMENT_H__

/** \file
 * SPDocument: Typed SVG document implementation
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib-object.h>
#include <gtk/gtksignal.h>
#include <sigc++/sigc++.h>
#include <sigc++/class_slot.h>

#include "libcroco/cr-cascade.h"
#include <2geom/forward.h>

#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"
#include <glibmm/ustring.h>
#include "verbs.h"
#include <vector>
#include <set>

namespace Avoid {
class Router;
}

struct NRRect;
struct SPDesktop;
struct SPItem;
struct SPObject;
struct SPGroup;

namespace Inkscape {
    struct Application;
    class Selection; 
    class UndoStackObserver;
    class EventLog;
    class ProfileManager;
    namespace XML {
        class Document;
        class Node;
    }
}

class SP3DBox;
class Persp3D;

namespace Proj {
    class TransfMat3x4;
}

class SPDocumentPrivate;

/// Typed SVG document implementation.
struct SPDocument : public Inkscape::GC::Managed<>,
                    public Inkscape::GC::Finalized,
                    public Inkscape::GC::Anchored
{
    typedef sigc::signal<void, SPObject *> IDChangedSignal;
    typedef sigc::signal<void> ResourcesChangedSignal;
    typedef sigc::signal<void, guint> ModifiedSignal;
    typedef sigc::signal<void, gchar const *> URISetSignal;
    typedef sigc::signal<void, double, double> ResizedSignal;
    typedef sigc::signal<void> ReconstructionStart;
    typedef sigc::signal<void> ReconstructionFinish;
    typedef sigc::signal<void> CommitSignal;

    SPDocument();
    virtual ~SPDocument();

    unsigned int keepalive : 1;
    unsigned int virgin    : 1; ///< Has the document never been touched?
    unsigned int modified_since_save : 1;

    Inkscape::XML::Document *rdoc; ///< Our Inkscape::XML::Document
    Inkscape::XML::Node *rroot; ///< Root element of Inkscape::XML::Document
    SPObject *root;             ///< Our SPRoot
    CRCascade *style_cascade;

    gchar *uri;   ///< A filename (not a URI yet), or NULL
    gchar *base;  ///< To be used for resolving relative hrefs.
    gchar *name;  ///< basename(uri) or other human-readable label for the document.

    SPDocumentPrivate *priv;

    /// Last action key
    const gchar *actionkey;
    /// Handler ID
    guint modified_id;

    Inkscape::ProfileManager* profileManager;

    // Instance of the connector router
    Avoid::Router *router;

    GSList *perspectives;

    Persp3D *current_persp3d; // "currently active" perspective (e.g., newly created boxes are attached to this one)

    GSList *_collection_queue;

    bool oldSignalsConnected;

    void add_persp3d(Persp3D * const persp);
    void remove_persp3d(Persp3D * const persp);

    sigc::connection connectModified(ModifiedSignal::slot_type slot);
    sigc::connection connectURISet(URISetSignal::slot_type slot);
    sigc::connection connectResized(ResizedSignal::slot_type slot);
sigc::connection connectCommit(CommitSignal::slot_type slot);

    void bindObjectToId(gchar const *id, SPObject *object);
    SPObject *getObjectById(gchar const *id);
    sigc::connection connectIdChanged(const gchar *id, IDChangedSignal::slot_type slot);

    void bindObjectToRepr(Inkscape::XML::Node *repr, SPObject *object);
    SPObject *getObjectByRepr(Inkscape::XML::Node *repr);

    Glib::ustring getLanguage();

    void queueForOrphanCollection(SPObject *object);
    void collectOrphans();

    void _emitModified();

    void addUndoObserver(Inkscape::UndoStackObserver& observer);
    void removeUndoObserver(Inkscape::UndoStackObserver& observer);

    bool _updateDocument();

    /// Are we currently in a transition between two "known good" states of the document?
    bool isSeeking() const;

    bool isModifiedSinceSave() const { return modified_since_save; }
    void setModifiedSinceSave(bool modified = true) {
        modified_since_save = modified;
    }

private:
    SPDocument(SPDocument const &); // no copy
    void operator=(SPDocument const &); // no assign

public:
    sigc::connection connectReconstructionStart(ReconstructionStart::slot_type slot);
    sigc::connection connectReconstructionFinish(ReconstructionFinish::slot_type slot);
    void emitReconstructionStart(void);
    void emitReconstructionFinish(void);

    unsigned long serial() const;
    void reset_key(void *dummy);
    sigc::connection _selection_changed_connection;
    sigc::connection _desktop_activated_connection;

    void fitToRect(Geom::Rect const &rect);
};

SPDocument *sp_document_new(const gchar *uri, unsigned int keepalive, bool make_new = false);
SPDocument *sp_document_new_from_mem(const gchar *buffer, gint length, unsigned int keepalive);

SPDocument *sp_document_ref(SPDocument *doc);
SPDocument *sp_document_unref(SPDocument *doc);


SPDocument *sp_document_create(Inkscape::XML::Document *rdoc, gchar const *uri, gchar const *base, gchar const *name, unsigned int keepalive);

/*
 * Access methods
 */

#define sp_document_repr_doc(d) (d->rdoc)
#define sp_document_repr_root(d) (d->rroot)
#define sp_document_root(d) (d->root)
#define SP_DOCUMENT_ROOT(d) (d->root)

gdouble sp_document_width(SPDocument *document);
gdouble sp_document_height(SPDocument *document);
Geom::Point sp_document_dimensions(SPDocument *document);

struct SPUnit;

void sp_document_set_width(SPDocument *document, gdouble width, const SPUnit *unit);
void sp_document_set_height(SPDocument *document, gdouble height, const SPUnit *unit);

#define SP_DOCUMENT_URI(d)  (d->uri)
#define SP_DOCUMENT_NAME(d) (d->name)
#define SP_DOCUMENT_BASE(d) (d->base)

/*
 * Dictionary
 */

/*
 * Undo & redo
 */

void sp_document_set_undo_sensitive(SPDocument *document, bool sensitive);
bool sp_document_get_undo_sensitive(SPDocument const *document);

void sp_document_clear_undo(SPDocument *document);
void sp_document_clear_redo(SPDocument *document);

void sp_document_child_added(SPDocument *doc, SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
void sp_document_child_removed(SPDocument *doc, SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
void sp_document_attr_changed(SPDocument *doc, SPObject *object, const gchar *key, const gchar *oldval, const gchar *newval);
void sp_document_content_changed(SPDocument *doc, SPObject *object, const gchar *oldcontent, const gchar *newcontent);
void sp_document_order_changed(SPDocument *doc, SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *oldref, Inkscape::XML::Node *newref);

/* Object modification root handler */
void sp_document_request_modified(SPDocument *doc);
gint sp_document_ensure_up_to_date(SPDocument *doc);

/* Save all previous actions to stack, as one undo step */
void sp_document_done(SPDocument *document, unsigned int event_type, Glib::ustring event_description);
void sp_document_maybe_done(SPDocument *document, const gchar *keyconst, unsigned int event_type, Glib::ustring event_description);
void sp_document_reset_key(Inkscape::Application *inkscape, SPDesktop *desktop, GtkObject *base);

/* Cancel (and revert) current unsaved actions */
void sp_document_cancel(SPDocument *document);

/* Undo and redo */
gboolean sp_document_undo(SPDocument *document);
gboolean sp_document_redo(SPDocument *document);

/* Resource management */
gboolean sp_document_add_resource(SPDocument *document, const gchar *key, SPObject *object);
gboolean sp_document_remove_resource(SPDocument *document, const gchar *key, SPObject *object);
const GSList *sp_document_get_resource_list(SPDocument *document, const gchar *key);
sigc::connection sp_document_resources_changed_connect(SPDocument *document, const gchar *key, SPDocument::ResourcesChangedSignal::slot_type slot);


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

/*
 * Misc
 */

GSList *sp_document_items_in_box(SPDocument *document, unsigned int dkey, Geom::Rect const &box);
GSList *sp_document_partial_items_in_box(SPDocument *document, unsigned int dkey, Geom::Rect const &box);
SPItem *sp_document_item_from_list_at_point_bottom(unsigned int dkey, SPGroup *group, const GSList *list, Geom::Point const p, bool take_insensitive = false);
SPItem *sp_document_item_at_point  (SPDocument *document, unsigned int key, Geom::Point const p, gboolean into_groups, SPItem *upto = NULL);
GSList *sp_document_items_at_points(SPDocument *document, unsigned const key, std::vector<Geom::Point> points);
SPItem *sp_document_group_at_point (SPDocument *document, unsigned int key,  Geom::Point const p);

void sp_document_set_uri(SPDocument *document, gchar const *uri);
void sp_document_change_uri_and_hrefs(SPDocument *document, gchar const *uri);

void sp_document_resized_signal_emit(SPDocument *doc, gdouble width, gdouble height);

unsigned int vacuum_document(SPDocument *document);


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
