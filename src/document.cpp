/*
 * SPDocument manipulation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \class SPDocument
 * SPDocument serves as the container of both model trees (agnostic XML
 * and typed object tree), and implements all of the document-level
 * functionality used by the program. Many document level operations, like
 * load, save, print, export and so on, use SPDocument as their basic datatype.
 *
 * SPDocument implements undo and redo stacks and an id-based object
 * dictionary.  Thanks to unique id attributes, the latter can be used to
 * map from the XML tree back to the object tree.
 *
 * SPDocument performs the basic operations needed for asynchronous
 * update notification (SPObject ::modified virtual method), and implements
 * the 'modified' signal, as well.
 */


#define noSP_DOCUMENT_DEBUG_IDLE
#define noSP_DOCUMENT_DEBUG_UNDO

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <string>
#include <cstring>
#include <2geom/transforms.h>

#include "widgets/desktop-widget.h"
#include "desktop.h"
#include "dir-util.h"
#include "display/drawing-item.h"
#include "document-private.h"
#include "document-undo.h"
#include "id-clash.h"
#include "inkscape.h"
#include "inkscape-version.h"
#include "libavoid/router.h"
#include "persp3d.h"
#include "preferences.h"
#include "profile-manager.h"
#include "rdf.h"
#include "sp-factory.h"
#include "sp-item-group.h"
#include "sp-namedview.h"
#include "sp-symbol.h"
#include "transf_mat_3x4.h"
#include "util/units.h"
#include "xml/repr.h"
#include "xml/rebase-hrefs.h"
#include "libcroco/cr-cascade.h"

using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

// Higher number means lower priority.
#define SP_DOCUMENT_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE - 2)

// Should have a lower priority than SP_DOCUMENT_UPDATE_PRIORITY,
// since we want it to happen when there are no more updates.
#define SP_DOCUMENT_REROUTING_PRIORITY (G_PRIORITY_HIGH_IDLE - 1)


static gint sp_document_idle_handler(gpointer data);
static gint sp_document_rerouting_handler(gpointer data);

gboolean sp_document_resource_list_free(gpointer key, gpointer value, gpointer data);

static gint doc_count = 0;
static gint doc_mem_count = 0;

static unsigned long next_serial = 0;

SPDocument::SPDocument() :
    keepalive(FALSE),
    virgin(TRUE),
    modified_since_save(FALSE),
    rdoc(NULL),
    rroot(NULL),
    root(NULL),
    style_cascade(cr_cascade_new(NULL, NULL, NULL)),
    uri(NULL),
    base(NULL),
    name(NULL),
    priv(NULL), // reset in ctor
    actionkey(),
    modified_id(0),
    rerouting_handler_id(0),
    profileManager(NULL), // deferred until after other initialization
    router(new Avoid::Router(Avoid::PolyLineRouting|Avoid::OrthogonalRouting)),
    _collection_queue(NULL),
    oldSignalsConnected(false),
    current_persp3d(NULL),
    current_persp3d_impl(NULL),
    _parent_document(NULL)
{
    // Penalise libavoid for choosing paths with needless extra segments.
    // This results in much better looking orthogonal connector paths.
    router->setRoutingPenalty(Avoid::segmentPenalty);

    SPDocumentPrivate *p = new SPDocumentPrivate();

    p->serial = next_serial++;

    p->iddef = g_hash_table_new(g_direct_hash, g_direct_equal);
    p->reprdef = g_hash_table_new(g_direct_hash, g_direct_equal);

    p->resources = g_hash_table_new(g_str_hash, g_str_equal);

    p->sensitive = false;
    p->partial = NULL;
    p->history_size = 0;
    p->undo = NULL;
    p->redo = NULL;
    p->seeking = false;

    priv = p;

    // Once things are set, hook in the manager
    profileManager = new Inkscape::ProfileManager(this);

    // XXX only for testing!
    priv->undoStackObservers.add(p->console_output_undo_observer);
}

SPDocument::~SPDocument() {
    priv->destroySignal.emit();

    // kill/unhook this first
    if ( profileManager ) {
        delete profileManager;
        profileManager = 0;
    }

    if (router) {
        delete router;
        router = NULL;
    }

    if (oldSignalsConnected) {
        priv->selChangeConnection.disconnect();
        priv->desktopActivatedConnection.disconnect();
    } else {
        _selection_changed_connection.disconnect();
        _desktop_activated_connection.disconnect();
    }

    if (priv) {
        if (priv->partial) {
            sp_repr_free_log(priv->partial);
            priv->partial = NULL;
        }

        DocumentUndo::clearRedo(this);
        DocumentUndo::clearUndo(this);

        if (root) {
            root->releaseReferences();
            sp_object_unref(root);
            root = NULL;
        }

        if (priv->iddef) g_hash_table_destroy(priv->iddef);
        if (priv->reprdef) g_hash_table_destroy(priv->reprdef);

        if (rdoc) Inkscape::GC::release(rdoc);

        /* Free resources */
        g_hash_table_foreach_remove(priv->resources, sp_document_resource_list_free, this);
        g_hash_table_destroy(priv->resources);

        delete priv;
        priv = NULL;
    }

    cr_cascade_unref(style_cascade);
    style_cascade = NULL;

    if (name) {
        g_free(name);
        name = NULL;
    }
    if (base) {
        g_free(base);
        base = NULL;
    }
    if (uri) {
        g_free(uri);
        uri = NULL;
    }

    if (modified_id) {
        g_source_remove(modified_id);
        modified_id = 0;
    }

    if (rerouting_handler_id) {
        g_source_remove(rerouting_handler_id);
        rerouting_handler_id = 0;
    }

    if (keepalive) {
        inkscape_unref(INKSCAPE);
        keepalive = FALSE;
    }

    if (this->current_persp3d_impl) 
        delete this->current_persp3d_impl;
    this->current_persp3d_impl = NULL;

    // This is at the end of the destructor, because preceding code adds new orphans to the queue
    collectOrphans();

}

sigc::connection SPDocument::connectDestroy(sigc::signal<void>::slot_type slot)
{
    return priv->destroySignal.connect(slot);
}

SPDefs *SPDocument::getDefs()
{
    if (!root) {
        return NULL;
    }
    return root->defs;
}

Persp3D *SPDocument::getCurrentPersp3D() {
    // Check if current_persp3d is still valid
    std::vector<Persp3D*> plist;
    getPerspectivesInDefs(plist);
    for (unsigned int i = 0; i < plist.size(); ++i) {
        if (current_persp3d == plist[i])
            return current_persp3d;
    }

    // If not, return the first perspective in defs (which may be NULL of none exists)
    current_persp3d = persp3d_document_first_persp (this);

    return current_persp3d;
}

Persp3DImpl *SPDocument::getCurrentPersp3DImpl() {
    return current_persp3d_impl;
}

void SPDocument::setCurrentPersp3D(Persp3D * const persp) {
    current_persp3d = persp;
    //current_persp3d_impl = persp->perspective_impl;
}

void SPDocument::getPerspectivesInDefs(std::vector<Persp3D*> &list) const
{
    for (SPObject *i = root->defs->firstChild(); i; i = i->getNext() ) {
        if (SP_IS_PERSP3D(i)) {
            list.push_back(SP_PERSP3D(i));
        }
    }
}

/**
void SPDocument::initialize_current_persp3d()
{
    this->current_persp3d = persp3d_document_first_persp(this);
    if (!this->current_persp3d) {
        this->current_persp3d = persp3d_create_xml_element(this);
    }
}
**/

unsigned long SPDocument::serial() const {
    return priv->serial;
}

void SPDocument::queueForOrphanCollection(SPObject *object) {
    g_return_if_fail(object != NULL);
    g_return_if_fail(object->document == this);

    sp_object_ref(object, NULL);
    _collection_queue = g_slist_prepend(_collection_queue, object);
}

void SPDocument::collectOrphans() {
    while (_collection_queue) {
        GSList *objects=_collection_queue;
        _collection_queue = NULL;
        for ( GSList *iter=objects ; iter ; iter = iter->next ) {
            SPObject *object=reinterpret_cast<SPObject *>(iter->data);
            object->collectOrphan();
            sp_object_unref(object, NULL);
        }
        g_slist_free(objects);
    }
}

void SPDocument::reset_key (void */*dummy*/)
{
    actionkey.clear();
}

SPDocument *SPDocument::createDoc(Inkscape::XML::Document *rdoc,
                                  gchar const *uri,
                                  gchar const *base,
                                  gchar const *name,
                                  unsigned int keepalive,
                                  SPDocument *parent)
{
    SPDocument *document = new SPDocument();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::XML::Node *rroot = rdoc->root();

    document->keepalive = keepalive;

    document->rdoc = rdoc;
    document->rroot = rroot;
    if (parent) {
        document->_parent_document = parent;
        parent->_child_documents.push_back(document);
    }

    if (document->uri){
        g_free(document->uri);
        document->uri = 0;
    }
    if (document->base){
        g_free(document->base);
        document->base = 0;
    }
    if (document->name){
        g_free(document->name);
        document->name = 0;
    }
#ifndef WIN32
    document->uri = prepend_current_dir_if_relative(uri);
#else
    // FIXME: it may be that prepend_current_dir_if_relative works OK on windows too, test!
    document->uri = uri? g_strdup(uri) : NULL;
#endif

    // base is simply the part of the path before filename; e.g. when running "inkscape ../file.svg" the base is "../"
    // which is why we use g_get_current_dir() in calculating the abs path above
    //This is NULL for a new document
    if (base) {
        document->base = g_strdup(base);
    } else {
        document->base = NULL;
    }
    document->name = g_strdup(name);

    // Create SPRoot element
    const std::string typeString = NodeTraits::get_type_string(*rroot);
    SPObject* rootObj = SPFactory::createObject(typeString);
    document->root = dynamic_cast<SPRoot*>(rootObj);

    if (document->root == 0) {
    	// Node is not a valid root element
    	delete rootObj;

    	// fixme: what to do here?
    	throw;
    }

    // Recursively build object tree
    document->root->invoke_build(document, rroot, false);

    /* fixme: Not sure about this, but lets assume ::build updates */
    rroot->setAttribute("inkscape:version", Inkscape::version_string);
    /* fixme: Again, I moved these here to allow version determining in ::build (Lauris) */


    /* Eliminate obsolete sodipodi:docbase, for privacy reasons */
    rroot->setAttribute("sodipodi:docbase", NULL);

    /* Eliminate any claim to adhere to a profile, as we don't try to */
    rroot->setAttribute("baseProfile", NULL);

    // creating namedview
    if (!sp_item_group_get_child_by_name(document->root, NULL, "sodipodi:namedview")) {
        // if there's none in the document already,
        Inkscape::XML::Node *rnew = NULL;

        rnew = rdoc->createElement("sodipodi:namedview");
        //rnew->setAttribute("id", "base");

        // Add namedview data from the preferences
        // we can't use getAllEntries because this could produce non-SVG doubles
        Glib::ustring pagecolor = prefs->getString("/template/base/pagecolor");
        if (!pagecolor.empty()) {
            rnew->setAttribute("pagecolor", pagecolor.data());
        }
        Glib::ustring bordercolor = prefs->getString("/template/base/bordercolor");
        if (!bordercolor.empty()) {
            rnew->setAttribute("bordercolor", bordercolor.data());
        }
        sp_repr_set_svg_double(rnew, "borderopacity",
            prefs->getDouble("/template/base/borderopacity", 1.0));
        sp_repr_set_svg_double(rnew, "objecttolerance",
            prefs->getDouble("/template/base/objecttolerance", 10.0));
        sp_repr_set_svg_double(rnew, "gridtolerance",
            prefs->getDouble("/template/base/gridtolerance", 10.0));
        sp_repr_set_svg_double(rnew, "guidetolerance",
            prefs->getDouble("/template/base/guidetolerance", 10.0));
        sp_repr_set_svg_double(rnew, "inkscape:pageopacity",
            prefs->getDouble("/template/base/inkscape:pageopacity", 0.0));
        sp_repr_set_int(rnew, "inkscape:pageshadow",
            prefs->getInt("/template/base/inkscape:pageshadow", 2));
        sp_repr_set_int(rnew, "inkscape:window-width",
            prefs->getInt("/template/base/inkscape:window-width", 640));
        sp_repr_set_int(rnew, "inkscape:window-height",
            prefs->getInt("/template/base/inkscape:window-height", 480));

        // insert into the document
        rroot->addChild(rnew, NULL);
        // clean up
        Inkscape::GC::release(rnew);
    }

    // Defs
    if (!document->root->defs) {
        Inkscape::XML::Node *r = rdoc->createElement("svg:defs");
        rroot->addChild(r, NULL);
        Inkscape::GC::release(r);
        g_assert(document->root->defs);
    }

    /* Default RDF */
    rdf_set_defaults( document );

    if (keepalive) {
        inkscape_ref(INKSCAPE);
    }

    // Check if the document already has a perspective (e.g., when opening an existing
    // document). If not, create a new one and set it as the current perspective.
    document->setCurrentPersp3D(persp3d_document_first_persp(document));
    if (!document->getCurrentPersp3D()) {
        //document->setCurrentPersp3D(persp3d_create_xml_element (document));
        Persp3DImpl *persp_impl = new Persp3DImpl();
        document->setCurrentPersp3DImpl(persp_impl);
    }

    DocumentUndo::setUndoSensitive(document, true);

    // reset undo key when selection changes, so that same-key actions on different objects are not coalesced
    document->priv->selChangeConnection = INKSCAPE.signal_selection_changed.connect(
                sigc::hide(sigc::bind(
                sigc::ptr_fun(&DocumentUndo::resetKey), document)
    ));
    document->priv->desktopActivatedConnection = INKSCAPE.signal_activate_desktop.connect(
                sigc::hide(sigc::bind(
                sigc::ptr_fun(&DocumentUndo::resetKey), document)
    ));
    document->oldSignalsConnected = true;

    return document;
}

/**
 * Fetches a document and attaches it to the current document as a child href
 */
SPDocument *SPDocument::createChildDoc(std::string const &uri)
{
    SPDocument *parent = this;
    SPDocument *document = NULL;

    while(parent != NULL && parent->getURI() != NULL && document == NULL) {
        // Check myself and any parents int he chain
        if(uri == parent->getURI()) {
            document = parent;
            break;
        }
        // Then check children of those.
        boost::ptr_list<SPDocument>::iterator iter;
        for (iter = parent->_child_documents.begin();
          iter != parent->_child_documents.end(); ++iter) {
            if(uri == iter->getURI()) {
                document = &*iter;
                break;
            }
        }
        parent = parent->_parent_document;
    }

    // Load a fresh document from the svg source.
    if(!document) {
        const char *path = uri.c_str();
        document = createNewDoc(path, false, false, this);
    }
    return document;
}
/**
 * Fetches document from URI, or creates new, if NULL; public document
 * appears in document list.
 */
SPDocument *SPDocument::createNewDoc(gchar const *uri, unsigned int keepalive, bool make_new, SPDocument *parent)
{
    Inkscape::XML::Document *rdoc = NULL;
    gchar *base = NULL;
    gchar *name = NULL;

    if (uri) {
        Inkscape::XML::Node *rroot;
        gchar *s, *p;
        /* Try to fetch repr from file */
        rdoc = sp_repr_read_file(uri, SP_SVG_NS_URI);
        /* If file cannot be loaded, return NULL without warning */
        if (rdoc == NULL) return NULL;
        rroot = rdoc->root();
        /* If xml file is not svg, return NULL without warning */
        /* fixme: destroy document */
        if (strcmp(rroot->name(), "svg:svg") != 0) return NULL;
        s = g_strdup(uri);
        p = strrchr(s, '/');
        if (p) {
            name = g_strdup(p + 1);
            p[1] = '\0';
            base = g_strdup(s);
        } else {
            base = NULL;
            name = g_strdup(uri);
        }
        if (make_new) {
            base = NULL;
            uri = NULL;
            name = g_strdup_printf(_("New document %d"), ++doc_count);
        }
        g_free(s);
    } else {
        if (make_new) {
            name = g_strdup_printf(_("Memory document %d"), ++doc_mem_count);
        }

        rdoc = sp_repr_document_new("svg:svg");
    }

    //# These should be set by now
    g_assert(name);

    SPDocument *doc = createDoc(rdoc, uri, base, name, keepalive, parent);

    g_free(base);
    g_free(name);

    return doc;
}

SPDocument *SPDocument::createNewDocFromMem(gchar const *buffer, gint length, unsigned int keepalive)
{
    SPDocument *doc = NULL;

    Inkscape::XML::Document *rdoc = sp_repr_read_mem(buffer, length, SP_SVG_NS_URI);
    if ( rdoc ) {
        // Only continue to create a non-null doc if it could be loaded
        Inkscape::XML::Node *rroot = rdoc->root();
        if ( strcmp(rroot->name(), "svg:svg") != 0 ) {
            // If xml file is not svg, return NULL without warning
            // TODO fixme: destroy document
        } else {
            Glib::ustring name = Glib::ustring::compose( _("Memory document %1"), ++doc_mem_count );
            doc = createDoc(rdoc, NULL, NULL, name.c_str(), keepalive, NULL);
        }
    }

    return doc;
}

SPDocument *SPDocument::doRef()
{
    Inkscape::GC::anchor(this);
    return this;
}

SPDocument *SPDocument::doUnref()
{
    Inkscape::GC::release(this);
    return NULL;
}

/// guaranteed not to return nullptr
Inkscape::Util::Unit const* SPDocument::getDisplayUnit() const
{
    SPNamedView const* nv = sp_document_namedview(this, NULL);
    return nv ? nv->getDisplayUnit() : unit_table.getUnit("px");
}

/// guaranteed not to return nullptr
// returns 'px' units as default, like legacy Inkscape
// THIS SHOULD NOT BE USED... INSTEAD USE DOCUMENT SCALE
Inkscape::Util::Unit const& SPDocument::getSVGUnit() const
{
    SPNamedView const* nv = sp_document_namedview(this, NULL);
    return nv ? nv->getSVGUnit() : *unit_table.getUnit("px");
}

/// Sets document scale (by changing viewBox)
void SPDocument::setDocumentScale( double scaleX, double scaleY ) {

    root->viewBox = Geom::Rect::from_xywh(
        root->viewBox.left(),
        root->viewBox.top(),
        root->width.computed  * scaleX,
        root->height.computed * scaleY );
    root->viewBox_set = true;
    root->updateRepr();
}

/// Sets document scale (by changing viewBox, x and y scaling equal) 
void SPDocument::setDocumentScale( double scale ) {
    setDocumentScale( scale, scale );
}

/// Returns document scale as defined by width/height (in pixels) and viewBox (real world to
/// user-units).
Geom::Scale SPDocument::getDocumentScale() const
{
    Geom::Scale scale;
    if( root->viewBox_set ) {
        double scale_x = 1.0;
        double scale_y = 1.0;
        if( root->viewBox.width() > 0.0 ) {
            scale_x = root->width.computed / root->viewBox.width();
        }
        if( root->viewBox.height() > 0.0 ) {
            scale_y = root->height.computed / root->viewBox.height();
        }
        scale = Geom::Scale(scale_x, scale_y);
    }
    // std::cout << "SPDocument::getDocumentScale():\n" << scale << std::endl;
    return scale;
}

// Avoid calling root->updateRepr() twice by combining setting width and height.
// (As done on every delete as clipboard calls this via fitToRect(). Also called in page-sizer.cpp)
void SPDocument::setWidthAndHeight(const Inkscape::Util::Quantity &width, const Inkscape::Util::Quantity &height, bool changeSize)
{
    Inkscape::Util::Unit const *old_width_units = unit_table.getUnit("px");
    if (root->width.unit)
        old_width_units = unit_table.getUnit(root->width.unit);
    gdouble old_width_converted;  // old width converted to new units
    if (root->width.unit == SVGLength::PERCENT)
        old_width_converted = Inkscape::Util::Quantity::convert(root->width.computed, "px", width.unit);
    else
        old_width_converted = Inkscape::Util::Quantity::convert(root->width.value, old_width_units, width.unit);

    root->width.computed = width.value("px");
    root->width.value = width.quantity;
    root->width.unit = (SVGLength::Unit) width.unit->svgUnit();

    Inkscape::Util::Unit const *old_height_units = unit_table.getUnit("px");
    if (root->height.unit)
        old_height_units = unit_table.getUnit(root->height.unit);
    gdouble old_height_converted;  // old height converted to new units
    if (root->height.unit == SVGLength::PERCENT)
        old_height_converted = Inkscape::Util::Quantity::convert(root->height.computed, "px", height.unit);
    else
        old_height_converted = Inkscape::Util::Quantity::convert(root->height.value, old_height_units, height.unit);

    root->height.computed = height.value("px");
    root->height.value = height.quantity;
    root->height.unit = (SVGLength::Unit) height.unit->svgUnit();

    // viewBox scaled by relative change in page size (maintains document scale).
    if (root->viewBox_set && changeSize) {
        root->viewBox.setMax(Geom::Point(
        root->viewBox.left() + (root->width.value /  old_width_converted ) * root->viewBox.width(),
        root->viewBox.top()  + (root->height.value / old_height_converted) * root->viewBox.height()));
    }
    root->updateRepr();
}

Inkscape::Util::Quantity SPDocument::getWidth() const
{
    g_return_val_if_fail(this->priv != NULL, Inkscape::Util::Quantity(0.0, unit_table.getUnit("")));
    g_return_val_if_fail(this->root != NULL, Inkscape::Util::Quantity(0.0, unit_table.getUnit("")));

    gdouble result = root->width.value;
    SVGLength::Unit u = root->width.unit;
    if (root->width.unit == SVGLength::PERCENT && root->viewBox_set) {
        result = root->viewBox.width();
        u = SVGLength::PX;
    }
    if (u == SVGLength::NONE) {
        u = SVGLength::PX;
    }
    return Inkscape::Util::Quantity(result, unit_table.getUnit(u));
}

void SPDocument::setWidth(const Inkscape::Util::Quantity &width, bool changeSize)
{
    Inkscape::Util::Unit const *old_width_units = unit_table.getUnit("px");
    if (root->width.unit)
        old_width_units = unit_table.getUnit(root->width.unit);
    gdouble old_width_converted;  // old width converted to new units
    if (root->width.unit == SVGLength::PERCENT)
        old_width_converted = Inkscape::Util::Quantity::convert(root->width.computed, "px", width.unit);
    else
        old_width_converted = Inkscape::Util::Quantity::convert(root->width.value, old_width_units, width.unit);

    root->width.computed = width.value("px");
    root->width.value = width.quantity;
    root->width.unit = (SVGLength::Unit) width.unit->svgUnit();

    if (root->viewBox_set && changeSize)
        root->viewBox.setMax(Geom::Point(root->viewBox.left() + (root->width.value / old_width_converted) * root->viewBox.width(), root->viewBox.bottom()));

    root->updateRepr();
}


Inkscape::Util::Quantity SPDocument::getHeight() const
{
    g_return_val_if_fail(this->priv != NULL, Inkscape::Util::Quantity(0.0, unit_table.getUnit("")));
    g_return_val_if_fail(this->root != NULL, Inkscape::Util::Quantity(0.0, unit_table.getUnit("")));

    gdouble result = root->height.value;
    SVGLength::Unit u = root->height.unit;
    if (root->height.unit == SVGLength::PERCENT && root->viewBox_set) {
        result = root->viewBox.height();
        u = SVGLength::PX;
    }
    if (u == SVGLength::NONE) {
        u = SVGLength::PX;
    }
    return Inkscape::Util::Quantity(result, unit_table.getUnit(u));
}

void SPDocument::setHeight(const Inkscape::Util::Quantity &height, bool changeSize)
{
    Inkscape::Util::Unit const *old_height_units = unit_table.getUnit("px");
    if (root->height.unit)
        old_height_units = unit_table.getUnit(root->height.unit);
    gdouble old_height_converted;  // old height converted to new units
    if (root->height.unit == SVGLength::PERCENT)
        old_height_converted = Inkscape::Util::Quantity::convert(root->height.computed, "px", height.unit);
    else
        old_height_converted = Inkscape::Util::Quantity::convert(root->height.value, old_height_units, height.unit);

    root->height.computed = height.value("px");
    root->height.value = height.quantity;
    root->height.unit = (SVGLength::Unit) height.unit->svgUnit();

    if (root->viewBox_set && changeSize)
        root->viewBox.setMax(Geom::Point(root->viewBox.right(), root->viewBox.top() + (root->height.value / old_height_converted) * root->viewBox.height()));

    root->updateRepr();
}

Geom::Rect SPDocument::getViewBox() const
{
    Geom::Rect viewBox;
    if (root->viewBox_set) {
        viewBox = root->viewBox;
    } else {
        viewBox = Geom::Rect::from_xywh( 0, 0, getWidth().value("px"), getHeight().value("px"));
    }
    return viewBox;
}

void SPDocument::setViewBox(const Geom::Rect &viewBox)
{
    root->viewBox_set = true;
    root->viewBox = viewBox;
    root->updateRepr();
}

Geom::Point SPDocument::getDimensions() const
{
    return Geom::Point(getWidth().value("px"), getHeight().value("px"));
}

Geom::OptRect SPDocument::preferredBounds() const
{
    return Geom::OptRect( Geom::Point(0, 0), getDimensions() );
}

/**
 * Given a Geom::Rect that may, for example, correspond to the bbox of an object,
 * this function fits the canvas to that rect by resizing the canvas
 * and translating the document root into position.
 * \param rect fit document size to this
 * \param with_margins add margins to rect, by taking margins from this
 *        document's namedview (<sodipodi:namedview> "fit-margin-..."
 *        attributes, and "units")
 */
void SPDocument::fitToRect(Geom::Rect const &rect, bool with_margins)
{
    double const w = rect.width();
    double const h = rect.height();

    double const old_height = getHeight().value("px");
    Inkscape::Util::Unit const *nv_units = unit_table.getUnit("px");
    if (root->height.unit && (root->height.unit != SVGLength::PERCENT))
        nv_units = unit_table.getUnit(root->height.unit);
    SPNamedView *nv = sp_document_namedview(this, NULL);
    
    /* in px */
    double margin_top = 0.0;
    double margin_left = 0.0;
    double margin_right = 0.0;
    double margin_bottom = 0.0;
    
    if (with_margins && nv) {
        if (nv != NULL) {
            margin_top = nv->getMarginLength("fit-margin-top", nv_units, unit_table.getUnit("px"), w, h, false);
            margin_left = nv->getMarginLength("fit-margin-left", nv_units, unit_table.getUnit("px"), w, h, true);
            margin_right = nv->getMarginLength("fit-margin-right", nv_units, unit_table.getUnit("px"), w, h, true);
            margin_bottom = nv->getMarginLength("fit-margin-bottom", nv_units, unit_table.getUnit("px"), w, h, false);
            margin_top = Inkscape::Util::Quantity::convert(margin_top, nv_units, "px");
            margin_left = Inkscape::Util::Quantity::convert(margin_left, nv_units, "px");
            margin_right = Inkscape::Util::Quantity::convert(margin_right, nv_units, "px");
            margin_bottom = Inkscape::Util::Quantity::convert(margin_bottom, nv_units, "px");
        }
    }
    
    Geom::Rect const rect_with_margins(
            rect.min() - Geom::Point(margin_left, margin_bottom),
            rect.max() + Geom::Point(margin_right, margin_top));
    
    setWidthAndHeight(
        Inkscape::Util::Quantity(Inkscape::Util::Quantity::convert(rect_with_margins.width(),  "px", nv_units), nv_units),
        Inkscape::Util::Quantity(Inkscape::Util::Quantity::convert(rect_with_margins.height(), "px", nv_units), nv_units)
        );

    Geom::Translate const tr(
            Geom::Point(0, old_height - rect_with_margins.height())
            - rect_with_margins.min());
    root->translateChildItems(tr);

    if(nv) {
        Geom::Translate tr2(-rect_with_margins.min());
        nv->translateGuides(tr2);
        nv->translateGrids(tr2);

        // update the viewport so the drawing appears to stay where it was
        nv->scrollAllDesktops(-tr2[0], tr2[1], false);
    }
}

void SPDocument::setBase( gchar const* base )
{
    if (this->base) {
        g_free(this->base);
        this->base = NULL;
    }
    if (base) {
        this->base = g_strdup(base);
    }
}

void SPDocument::do_change_uri(gchar const *const filename, bool const rebase)
{
    gchar *new_base = NULL;
    gchar *new_name = NULL;
    gchar *new_uri = NULL;
    if (filename) {

#ifndef WIN32
        new_uri = prepend_current_dir_if_relative(filename);
#else
        // FIXME: it may be that prepend_current_dir_if_relative works OK on windows too, test!
        new_uri = g_strdup(filename);
#endif

        new_base = g_path_get_dirname(new_uri);
        new_name = g_path_get_basename(new_uri);
    } else {
        new_uri = g_strdup_printf(_("Unnamed document %d"), ++doc_count);
        new_base = NULL;
        new_name = g_strdup(this->uri);
    }

    // Update saveable repr attributes.
    Inkscape::XML::Node *repr = getReprRoot();

    // Changing uri in the document repr must not be not undoable.
    bool const saved = DocumentUndo::getUndoSensitive(this);
    DocumentUndo::setUndoSensitive(this, false);

    if (rebase) {
        Inkscape::XML::rebase_hrefs(this, new_base, true);
    }

    if (strncmp(new_name, "ink_ext_XXXXXX", 14))	// do not use temporary filenames
        repr->setAttribute("sodipodi:docname", new_name);
    DocumentUndo::setUndoSensitive(this, saved);


    g_free(this->name);
    g_free(this->base);
    g_free(this->uri);
    this->name = new_name;
    this->base = new_base;
    this->uri = new_uri;

    this->priv->uri_set_signal.emit(this->uri);
}

/**
 * Sets base, name and uri members of \a document.  Doesn't update
 * any relative hrefs in the document: thus, this is primarily for
 * newly-created documents.
 *
 * \see sp_document_change_uri_and_hrefs
 */
void SPDocument::setUri(gchar const *filename)
{
    do_change_uri(filename, false);
}

/**
 * Changes the base, name and uri members of \a document, and updates any
 * relative hrefs in the document to be relative to the new base.
 *
 * \see sp_document_set_uri
 */
void SPDocument::changeUriAndHrefs(gchar const *filename)
{
    do_change_uri(filename, true);
}

void SPDocument::emitResizedSignal(gdouble width, gdouble height)
{
    this->priv->resized_signal.emit(width, height);
}

sigc::connection SPDocument::connectModified(SPDocument::ModifiedSignal::slot_type slot)
{
    return priv->modified_signal.connect(slot);
}

sigc::connection SPDocument::connectURISet(SPDocument::URISetSignal::slot_type slot)
{
    return priv->uri_set_signal.connect(slot);
}

sigc::connection SPDocument::connectResized(SPDocument::ResizedSignal::slot_type slot)
{
    return priv->resized_signal.connect(slot);
}

sigc::connection
SPDocument::connectReconstructionStart(SPDocument::ReconstructionStart::slot_type slot)
{
    return priv->_reconstruction_start_signal.connect(slot);
}

void
SPDocument::emitReconstructionStart(void)
{
    // printf("Starting Reconstruction\n");
    priv->_reconstruction_start_signal.emit();
    return;
}

sigc::connection
SPDocument::connectReconstructionFinish(SPDocument::ReconstructionFinish::slot_type  slot)
{
    return priv->_reconstruction_finish_signal.connect(slot);
}

void
SPDocument::emitReconstructionFinish(void)
{
    // printf("Finishing Reconstruction\n");
    priv->_reconstruction_finish_signal.emit();

/**    
    // Reference to the old persp3d object is invalid after reconstruction.
    initialize_current_persp3d();
    
    return;
**/
}

sigc::connection SPDocument::connectCommit(SPDocument::CommitSignal::slot_type slot)
{
    return priv->commit_signal.connect(slot);
}



void SPDocument::_emitModified() {
    static guint const flags = SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG;
    root->emitModified(0);
    priv->modified_signal.emit(flags);
}

void SPDocument::bindObjectToId(gchar const *id, SPObject *object) {
    GQuark idq = g_quark_from_string(id);

    if (object) {
        g_assert(g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq)) == NULL);
        g_hash_table_insert(priv->iddef, GINT_TO_POINTER(idq), object);
    } else {
        g_assert(g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq)) != NULL);
        g_hash_table_remove(priv->iddef, GINT_TO_POINTER(idq));
    }

    SPDocumentPrivate::IDChangedSignalMap::iterator pos;

    pos = priv->id_changed_signals.find(idq);
    if ( pos != priv->id_changed_signals.end() ) {
        if (!(*pos).second.empty()) {
            (*pos).second.emit(object);
        } else { // discard unused signal
            priv->id_changed_signals.erase(pos);
        }
    }
}

void
SPDocument::addUndoObserver(Inkscape::UndoStackObserver& observer)
{
    this->priv->undoStackObservers.add(observer);
}

void
SPDocument::removeUndoObserver(Inkscape::UndoStackObserver& observer)
{
    this->priv->undoStackObservers.remove(observer);
}

SPObject *SPDocument::getObjectById(Glib::ustring const &id) const
{
    return getObjectById( id.c_str() );
}

SPObject *SPDocument::getObjectById(gchar const *id) const
{
    g_return_val_if_fail(id != NULL, NULL);
    if (!priv || !priv->iddef) {
    	return NULL;
    }

    GQuark idq = g_quark_from_string(id);
    gpointer rv = g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq));
    if(rv != NULL)
    {
        return static_cast<SPObject*>(rv);
    }
    else
    {
        return NULL;
    }
}

sigc::connection SPDocument::connectIdChanged(gchar const *id,
                                              SPDocument::IDChangedSignal::slot_type slot)
{
    return priv->id_changed_signals[g_quark_from_string(id)].connect(slot);
}

void SPDocument::bindObjectToRepr(Inkscape::XML::Node *repr, SPObject *object)
{
    if (object) {
        g_assert(g_hash_table_lookup(priv->reprdef, repr) == NULL);
        g_hash_table_insert(priv->reprdef, repr, object);
    } else {
        g_assert(g_hash_table_lookup(priv->reprdef, repr) != NULL);
        g_hash_table_remove(priv->reprdef, repr);
    }
}

SPObject *SPDocument::getObjectByRepr(Inkscape::XML::Node *repr) const
{
    g_return_val_if_fail(repr != NULL, NULL);
    return static_cast<SPObject*>(g_hash_table_lookup(priv->reprdef, repr));
}

Glib::ustring SPDocument::getLanguage() const
{
    gchar const *document_language = rdf_get_work_entity(this, rdf_find_entity("language"));
    if (document_language) {
        while (isspace(*document_language))
            document_language++;
    }
    if ( !document_language || 0 == *document_language) {
        // retrieve system language
        document_language = getenv("LC_ALL");
        if ( NULL == document_language || *document_language == 0 ) {
            document_language = getenv ("LC_MESSAGES");
        }
        if ( NULL == document_language || *document_language == 0 ) {
            document_language = getenv ("LANG");
        }
        if ( NULL == document_language || *document_language == 0 ) {
            document_language = getenv ("LANGUAGE");
        }
        
        if ( NULL != document_language ) {
            const char *pos = strchr(document_language, '_');
            if ( NULL != pos ) {
                return Glib::ustring(document_language, pos - document_language);
            }
        }
    }

    if ( NULL == document_language )
        return Glib::ustring();
    return document_language;
}

/* Object modification root handler */

void SPDocument::requestModified()
{
    if (!modified_id) {
        modified_id = g_idle_add_full(SP_DOCUMENT_UPDATE_PRIORITY, 
                sp_document_idle_handler, this, NULL);
    }
    if (!rerouting_handler_id) {
        rerouting_handler_id = g_idle_add_full(SP_DOCUMENT_REROUTING_PRIORITY, 
                sp_document_rerouting_handler, this, NULL);
    }
}

void SPDocument::setupViewport(SPItemCtx *ctx)
{
    ctx->flags = 0;
    ctx->i2doc = Geom::identity();
    // Set up viewport in case svg has it defined as percentages
    if (root->viewBox_set) { // if set, take from viewBox
        ctx->viewport = root->viewBox;
    } else { // as a last resort, set size to A4
        ctx->viewport = Geom::Rect::from_xywh(0, 0, Inkscape::Util::Quantity::convert(210, "mm", "px"), Inkscape::Util::Quantity::convert(297, "mm", "px"));
    }
    ctx->i2vp = Geom::identity();
}

/**
 * Tries to update the document state based on the modified and
 * "update required" flags, and return true if the document has
 * been brought fully up to date.
 */
bool
SPDocument::_updateDocument()
{
    /* Process updates */
    if (this->root->uflags || this->root->mflags) {
        if (this->root->uflags) {
            SPItemCtx ctx;
            setupViewport(&ctx);

            bool saved = DocumentUndo::getUndoSensitive(this);
            DocumentUndo::setUndoSensitive(this, false);

            this->root->updateDisplay((SPCtx *)&ctx, 0);

            DocumentUndo::setUndoSensitive(this, saved);
        }
        this->_emitModified();
    }

    return !(this->root->uflags || this->root->mflags);
}


/**
 * Repeatedly works on getting the document updated, since sometimes
 * it takes more than one pass to get the document updated.  But it
 * usually should not take more than a few loops, and certainly never
 * more than 32 iterations.  So we bail out if we hit 32 iterations,
 * since this typically indicates we're stuck in an update loop.
 */
gint SPDocument::ensureUpToDate()
{
    // Bring the document up-to-date, specifically via the following:
    //   1a) Process all document updates.
    //   1b) When completed, process connector routing changes.
    //   2a) Process any updates resulting from connector reroutings.
    int counter = 32;
    for (unsigned int pass = 1; pass <= 2; ++pass) {
        // Process document updates.
        while (!_updateDocument()) {
            if (counter == 0) {
                g_warning("More than 32 iteration while updating document '%s'", uri);
                break;
            }
            counter--;
        }
        if (counter == 0)
        {
            break;
        }

        // After updates on the first pass we get libavoid to process all the 
        // changed objects and provide new routings.  This may cause some objects
            // to be modified, hence the second update pass.
        if (pass == 1) {
            router->processTransaction();
        }
    }
    
    if (modified_id) {
        // Remove handler
        g_source_remove(modified_id);
        modified_id = 0;
    }
    if (rerouting_handler_id) {
        // Remove handler
        g_source_remove(rerouting_handler_id);
        rerouting_handler_id = 0;
    }
    return counter>0;
}

/**
 * An idle handler to update the document.  Returns true if
 * the document needs further updates.
 */
static gint
sp_document_idle_handler(gpointer data)
{
    SPDocument *doc = static_cast<SPDocument *>(data);
    bool status = !doc->_updateDocument(); // method TRUE if it does NOT need further modification, so invert
    if (!status) {
        doc->modified_id = 0;
    }
    return status;
}

/**
 * An idle handler to reroute connectors in the document.  
 */
static gint
sp_document_rerouting_handler(gpointer data)
{
    // Process any queued movement actions and determine new routings for 
    // object-avoiding connectors.  Callbacks will be used to update and 
    // redraw affected connectors.
    SPDocument *doc = static_cast<SPDocument *>(data);
    doc->router->processTransaction();
    
    // We don't need to handle rerouting again until there are further 
    // diagram updates.
    doc->rerouting_handler_id = 0;
    return false;
}

static bool is_within(Geom::Rect const &area, Geom::Rect const &box)
{
    return area.contains(box);
}

static bool overlaps(Geom::Rect const &area, Geom::Rect const &box)
{
    return area.intersects(box);
}

static std::vector<SPItem*> &find_items_in_area(std::vector<SPItem*> &s, SPGroup *group, unsigned int dkey, Geom::Rect const &area,
                                  bool (*test)(Geom::Rect const &, Geom::Rect const &), bool take_insensitive = false)
{
    g_return_val_if_fail(SP_IS_GROUP(group), s);

    for ( SPObject *o = group->firstChild() ; o ; o = o->getNext() ) {
        if ( SP_IS_ITEM(o) ) {
            if (SP_IS_GROUP(o) && SP_GROUP(o)->effectiveLayerMode(dkey) == SPGroup::LAYER ) {
                s = find_items_in_area(s, SP_GROUP(o), dkey, area, test);
            } else {
                SPItem *child = SP_ITEM(o);
                Geom::OptRect box = child->desktopVisualBounds();
                if ( box && test(area, *box) && (take_insensitive || child->isVisibleAndUnlocked(dkey))) {
                    s.push_back(child);
                }
            }
        }
    }

    return s;
}

/**
Returns true if an item is among the descendants of group (recursively).
 */
static bool item_is_in_group(SPItem *item, SPGroup *group)
{
    bool inGroup = false;
    for ( SPObject *o = group->firstChild() ; o && !inGroup; o = o->getNext() ) {
        if ( SP_IS_ITEM(o) ) {
            if (SP_ITEM(o) == item) {
                inGroup = true;
            } else if ( SP_IS_GROUP(o) ) {
                inGroup = item_is_in_group(item, SP_GROUP(o));
            }
        }
    }
    return inGroup;
}

SPItem *SPDocument::getItemFromListAtPointBottom(unsigned int dkey, SPGroup *group, std::vector<SPItem*> const &list,Geom::Point const &p, bool take_insensitive)
{
    g_return_val_if_fail(group, NULL);
    SPItem *bottomMost = 0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble delta = prefs->getDouble("/options/cursortolerance/value", 1.0);

    for ( SPObject *o = group->firstChild() ; o && !bottomMost; o = o->getNext() ) {
        if ( SP_IS_ITEM(o) ) {
            SPItem *item = SP_ITEM(o);
            Inkscape::DrawingItem *arenaitem = item->get_arenaitem(dkey);
            if (arenaitem && arenaitem->pick(p, delta, 1) != NULL
                && (take_insensitive || item->isVisibleAndUnlocked(dkey))) {
                if (find(list.begin(),list.end(),item)!=list.end() ) {
                    bottomMost = item;
                }
            }

            if ( !bottomMost && SP_IS_GROUP(o) ) {
                // return null if not found:
                bottomMost = getItemFromListAtPointBottom(dkey, SP_GROUP(o), list, p, take_insensitive);
            }
        }
    }
    return bottomMost;
}

/**
Turn the SVG DOM into a flat list of nodes that can be searched from top-down.
The list can be persisted, which improves "find at multiple points" speed.
Returns true if upto is reached.
*/
static bool build_flat_item_list(std::deque<SPItem*> *nodes, unsigned int dkey, SPGroup *group, gboolean into_groups, bool take_insensitive = false, SPItem *upto = NULL)
{
    bool found_upto = false;
    for ( SPObject *o = group->firstChild() ; o ; o = o->getNext() ) {
        if (!SP_IS_ITEM(o)) {
            continue;
        }

        if (upto && SP_ITEM(o) == upto) {
            found_upto = true;
            break;
        }

        if (SP_IS_GROUP(o) && (SP_GROUP(o)->effectiveLayerMode(dkey) == SPGroup::LAYER || into_groups)) {
            found_upto = build_flat_item_list(nodes, dkey, SP_GROUP(o), into_groups, take_insensitive, upto);
            if (found_upto)
                break;
        } else {
            SPItem *child = SP_ITEM(o);

            if (take_insensitive || child->isVisibleAndUnlocked(dkey)) {
                nodes->push_front(child);
            }
        }
    }
    return found_upto;
}

/**
Returns the topmost (in z-order) item from the descendants of group (recursively) which
is at the point p, or NULL if none. Honors into_groups on whether to recurse into
non-layer groups or not. Honors take_insensitive on whether to return insensitive
items. If upto != NULL, then if item upto is encountered (at any level), stops searching
upwards in z-order and returns what it has found so far (i.e. the found item is
guaranteed to be lower than upto). Requires a list of nodes built by
build_flat_item_list.
 */
static SPItem *find_item_at_point(std::deque<SPItem*> *nodes, unsigned int dkey, Geom::Point const &p)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble delta = prefs->getDouble("/options/cursortolerance/value", 1.0);

    SPItem *seen = NULL;
    SPItem *child;
    for (unsigned long i = 0; i < nodes->size(); ++i) {
        child = nodes->at(i);
        Inkscape::DrawingItem *arenaitem = child->get_arenaitem(dkey);

        if (arenaitem && arenaitem->pick(p, delta, 1) != NULL) {
            seen = child;
            break;
        }
    }

    return seen;
}

/**
Returns the topmost non-layer group from the descendants of group which is at point
p, or NULL if none. Recurses into layers but not into groups.
 */
static SPItem *find_group_at_point(unsigned int dkey, SPGroup *group, Geom::Point const &p)
{
    SPItem *seen = NULL;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble delta = prefs->getDouble("/options/cursortolerance/value", 1.0);

    for ( SPObject *o = group->firstChild() ; o ; o = o->getNext() ) {
        if (!SP_IS_ITEM(o)) {
            continue;
        }
        if (SP_IS_GROUP(o) && SP_GROUP(o)->effectiveLayerMode(dkey) == SPGroup::LAYER) {
            SPItem *newseen = find_group_at_point(dkey, SP_GROUP(o), p);
            if (newseen) {
                seen = newseen;
            }
        }
        if (SP_IS_GROUP(o) && SP_GROUP(o)->effectiveLayerMode(dkey) != SPGroup::LAYER ) {
            SPItem *child = SP_ITEM(o);
            Inkscape::DrawingItem *arenaitem = child->get_arenaitem(dkey);

            // seen remembers the last (topmost) of groups pickable at this point
            if (arenaitem && arenaitem->pick(p, delta, 1) != NULL) {
                seen = child;
            }
        }
    }
    return seen;
}

/*
 * Return list of items, contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */
std::vector<SPItem*> SPDocument::getItemsInBox(unsigned int dkey, Geom::Rect const &box) const
{
    std::vector<SPItem*> x;
    g_return_val_if_fail(this->priv != NULL, x);
    return find_items_in_area(x, SP_GROUP(this->root), dkey, box, is_within);
}

/*
 * Return list of items, that the parts of the item contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

std::vector<SPItem*> SPDocument::getItemsPartiallyInBox(unsigned int dkey, Geom::Rect const &box) const
{
    std::vector<SPItem*> x;
    g_return_val_if_fail(this->priv != NULL, x);
    return find_items_in_area(x, SP_GROUP(this->root), dkey, box, overlaps);
}

std::vector<SPItem*> SPDocument::getItemsAtPoints(unsigned const key, std::vector<Geom::Point> points, bool all_layers, size_t limit) const
{
    std::vector<SPItem*> items;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // When picking along the path, we don't want small objects close together
    // (such as hatching strokes) to obscure each other by their deltas,
    // so we temporarily set delta to a small value
    gdouble saved_delta = prefs->getDouble("/options/cursortolerance/value", 1.0);
    prefs->setDouble("/options/cursortolerance/value", 0.25);

    // Cache a flattened SVG DOM to speed up selection.
    std::deque<SPItem*> nodes;
    build_flat_item_list(&nodes, key, SP_GROUP(this->root), true, false, NULL);
    SPObject *current_layer = SP_ACTIVE_DESKTOP->currentLayer();
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::LayerModel *layer_model = NULL;
    if(desktop){
        layer_model = desktop->layers;
    }
    size_t item_counter = 0;
    for(int i = points.size()-1;i>=0; i--) {
        SPItem *item = find_item_at_point(&nodes, key, points[i]);
        if (item && items.end()==find(items.begin(),items.end(), item))
            if(all_layers || (layer_model && layer_model->layerForObject(item) == current_layer)){
                items.push_back(item);
                item_counter++;
                //limit 0 = no limit
                if(item_counter == limit){
                    prefs->setDouble("/options/cursortolerance/value", saved_delta);
                    return items;
                }
            }
    }

    // and now we restore it back
    prefs->setDouble("/options/cursortolerance/value", saved_delta);

    return items;
}

SPItem *SPDocument::getItemAtPoint( unsigned const key, Geom::Point const &p,
                                    bool const into_groups, SPItem *upto) const
{
    g_return_val_if_fail(this->priv != NULL, NULL);

    // Build a flattened SVG DOM for find_item_at_point.
    std::deque<SPItem*> nodes;
    build_flat_item_list(&nodes, key, SP_GROUP(this->root), into_groups, false, upto);

    return find_item_at_point(&nodes, key, p);
}

SPItem *SPDocument::getGroupAtPoint(unsigned int key, Geom::Point const &p) const
{
    g_return_val_if_fail(this->priv != NULL, NULL);

    return find_group_at_point(key, SP_GROUP(this->root), p);
}

// Resource management

bool SPDocument::addResource(gchar const *key, SPObject *object)
{
    g_return_val_if_fail(key != NULL, false);
    g_return_val_if_fail(*key != '\0', false);
    g_return_val_if_fail(object != NULL, false);
    g_return_val_if_fail(SP_IS_OBJECT(object), false);

    bool result = false;

    if ( !object->cloned ) {
        GSList *rlist = (GSList*)g_hash_table_lookup(priv->resources, key);
        g_return_val_if_fail(!g_slist_find(rlist, object), false);
        rlist = g_slist_prepend(rlist, object);
        g_hash_table_insert(priv->resources, (gpointer) key, rlist);

        GQuark q = g_quark_from_string(key);

        /*in general, do not send signal if the object has no id (yet),
        it means the object is not completely built.
        (happens when pasting swatches across documents, cf bug 1495106)
        [this check should be more generally presend on emit() calls since 
        the backtrace is unusable with crashed from this cause]
        */
        if(object->getId() || dynamic_cast<SPGroup*>(object) )
            priv->resources_changed_signals[q].emit();

        result = true;
    }

    return result;
}

bool SPDocument::removeResource(gchar const *key, SPObject *object)
{
    g_return_val_if_fail(key != NULL, false);
    g_return_val_if_fail(*key != '\0', false);
    g_return_val_if_fail(object != NULL, false);
    g_return_val_if_fail(SP_IS_OBJECT(object), false);

    bool result = false;

    if ( !object->cloned ) {
        GSList *rlist = (GSList*)g_hash_table_lookup(priv->resources, key);
        g_return_val_if_fail(rlist != NULL, false);
        g_return_val_if_fail(g_slist_find(rlist, object), false);
        rlist = g_slist_remove(rlist, object);
        g_hash_table_insert(priv->resources, (gpointer) key, rlist);

        GQuark q = g_quark_from_string(key);
        priv->resources_changed_signals[q].emit();

        result = true;
    }

    return result;
}

GSList const *SPDocument::getResourceList(gchar const *key) const
{
    g_return_val_if_fail(key != NULL, NULL);
    g_return_val_if_fail(*key != '\0', NULL);

    return (GSList*)g_hash_table_lookup(this->priv->resources, key);
}

sigc::connection SPDocument::connectResourcesChanged(gchar const *key,
                                                     SPDocument::ResourcesChangedSignal::slot_type slot)
{
    GQuark q = g_quark_from_string(key);
    return this->priv->resources_changed_signals[q].connect(slot);
}

/* Helpers */

gboolean
sp_document_resource_list_free(gpointer /*key*/, gpointer value, gpointer /*data*/)
{
    g_slist_free((GSList *) value);
    return TRUE;
}

static unsigned int count_objects_recursive(SPObject *obj, unsigned int count)
{
    count++; // obj itself

    for ( SPObject *i = obj->firstChild(); i; i = i->getNext() ) {
        count = count_objects_recursive(i, count);
    }

    return count;
}

static unsigned int objects_in_document(SPDocument *document)
{
    return count_objects_recursive(document->getRoot(), 0);
}

static void vacuum_document_recursive(SPObject *obj)
{
    if (SP_IS_DEFS(obj)) {
        for ( SPObject *def = obj->firstChild(); def; def = def->getNext()) {
            // fixme: some inkscape-internal nodes in the future might not be collectable
            def->requestOrphanCollection();
        }
    } else {
        for ( SPObject *i = obj->firstChild(); i; i = i->getNext() ) {
            vacuum_document_recursive(i);
        }
    }
}

unsigned int SPDocument::vacuumDocument()
{
    unsigned int start = objects_in_document(this);
    unsigned int end;
    unsigned int newend = start;

    unsigned int iterations = 0;

    do {
        end = newend;

        vacuum_document_recursive(root);
        this->collectOrphans();
        iterations++;

        newend = objects_in_document(this);

    } while (iterations < 100 && newend < end);

    return start - newend;
}

bool SPDocument::isSeeking() const {
    return priv->seeking;
}

void SPDocument::setModifiedSinceSave(bool modified) {
    this->modified_since_save = modified;
    if (SP_ACTIVE_DESKTOP) {
        Gtk::Window *parent = SP_ACTIVE_DESKTOP->getToplevel();
        if (parent) { // during load, SP_ACTIVE_DESKTOP may be !nullptr, but parent might still be nullptr
            SPDesktopWidget *dtw = static_cast<SPDesktopWidget *>(parent->get_data("desktopwidget"));
            dtw->updateTitle( this->getName() );
        }
    }
}


/**
 * Paste SVG defs from the document retrieved from the clipboard or imported document into the active document.
 * @param clipdoc The document to paste.
 * @pre @c clipdoc != NULL and pasting into the active document is possible.
 */
void SPDocument::importDefs(SPDocument *source)
{
    Inkscape::XML::Node *root = source->getReprRoot();
    Inkscape::XML::Node *target_defs = this->getDefs()->getRepr();
    std::vector<Inkscape::XML::Node const *> defsNodes = sp_repr_lookup_name_many(root, "svg:defs");

    prevent_id_clashes(source, this);
    
    for (std::vector<Inkscape::XML::Node const *>::iterator defs = defsNodes.begin(); defs != defsNodes.end(); ++defs) {
       importDefsNode(source, const_cast<Inkscape::XML::Node *>(*defs), target_defs);
    }
}

void SPDocument::importDefsNode(SPDocument *source, Inkscape::XML::Node *defs, Inkscape::XML::Node *target_defs)
{    
    int stagger=0;

    /*  Note, "clipboard" throughout the comments means "the document that is either the clipboard
        or an imported document", as importDefs is called in both contexts.
        
        The order of the records in the clipboard is unpredictable and there may be both
        forward and backwards references to other records within it.  There may be definitions in
        the clipboard that duplicate definitions in the present document OR that duplicate other
        definitions in the clipboard.  (Inkscape will not have created these, but they may be read
        in from other SVG sources.)
         
        There are 3 passes to clean this up:

        In the first find and mark definitions in the clipboard that are duplicates of those in the
        present document.  Change the ID to "RESERVED_FOR_INKSCAPE_DUPLICATE_DEF_XXXXXXXXX".
        (Inkscape will not reuse an ID, and the XXXXXXXXX keeps it from automatically creating new ones.)
        References in the clipboard to the old clipboard name are converted to the name used
        in the current document. 

        In the second find and mark definitions in the clipboard that are duplicates of earlier 
        definitions in the clipbard.  Unfortunately this is O(n^2) and could be very slow for a large
        SVG with thousands of definitions.  As before, references are adjusted to reflect the name
        going forward.

        In the final cycle copy over those records not marked with that ID.

        If an SVG file uses the special ID it will cause problems!
        
        If this function is called because of the paste of a true clipboard the caller will have passed in a
        COPY of the clipboard items.  That is good, because this routine modifies that document.  If the calling
        behavior ever changes, so that the same document is passed in on multiple pastes, this routine will break
        as in the following example:
        1.  Paste clipboard containing B same as A into document containing A.  Result, B is dropped
        and all references to it will point to A.
        2.  Paste same clipboard into a new document.  It will not contain A, so there will be unsatisfied
        references in that window.
    */

    std::string DuplicateDefString = "RESERVED_FOR_INKSCAPE_DUPLICATE_DEF";
    
    /* First pass: remove duplicates in clipboard of definitions in document */
    for (Inkscape::XML::Node *def = defs->firstChild() ; def ; def = def->next()) {
        if(def->type() != Inkscape::XML::ELEMENT_NODE)continue;
        /* If this  clipboard has been pasted into one document, and is now being pasted into another,
        or pasted again into the same, it will already have been processed.  If we detect that then 
        skip the rest of this pass. */
        Glib::ustring defid = def->attribute("id");
        if( defid.find( DuplicateDefString ) != Glib::ustring::npos )break;

        SPObject *src = source->getObjectByRepr(def);

        // Prevent duplicates of solid swatches by checking if equivalent swatch already exists
        if (src && SP_IS_GRADIENT(src)) {
            SPGradient *s_gr = SP_GRADIENT(src);
            for (SPObject *trg = this->getDefs()->firstChild() ; trg ; trg = trg->getNext()) {
                if (trg && (src != trg) && SP_IS_GRADIENT(trg)) {
                    SPGradient *t_gr = SP_GRADIENT(trg);
                    if (t_gr && s_gr->isEquivalent(t_gr)) {
                         // Change object references to the existing equivalent gradient
                         Glib::ustring newid = trg->getId();
                         if(newid != defid){  // id could be the same if it is a second paste into the same document
                             change_def_references(src, trg);
                         }
                         gchar *longid = g_strdup_printf("%s_%9.9d", DuplicateDefString.c_str(), stagger++);
                         def->setAttribute("id", longid );
                         g_free(longid);
                         // do NOT break here, there could be more than 1 duplicate!
                    }
                }
            }
        }
    }

    /* Second pass: remove duplicates in clipboard of earlier definitions in clipboard */
    for (Inkscape::XML::Node *def = defs->firstChild() ; def ; def = def->next()) {
        if(def->type() != Inkscape::XML::ELEMENT_NODE)continue;
        Glib::ustring defid = def->attribute("id");
        if( defid.find( DuplicateDefString ) != Glib::ustring::npos )continue; // this one already handled
        SPObject *src = source->getObjectByRepr(def);
        if (src && SP_IS_GRADIENT(src)) {
            SPGradient *s_gr = SP_GRADIENT(src);
            for (Inkscape::XML::Node *laterDef = def->next() ; laterDef ; laterDef = laterDef->next()) {
                SPObject *trg = source->getObjectByRepr(laterDef);
                if(trg && (src != trg) && SP_IS_GRADIENT(trg)) {
                     Glib::ustring newid = trg->getId();
                     if( newid.find( DuplicateDefString ) != Glib::ustring::npos )continue; // this one already handled
                     SPGradient *t_gr = SP_GRADIENT(trg);
                     if (t_gr && s_gr->isEquivalent(t_gr)) {
                         // Change object references to the existing equivalent gradient
                         // two id's in the clipboard should never be the same, so always change references
                         change_def_references(trg, src);
                         gchar *longid = g_strdup_printf("%s_%9.9d", DuplicateDefString.c_str(), stagger++);
                         laterDef->setAttribute("id", longid );
                         g_free(longid);
                         // do NOT break here, there could be more than 1 duplicate!
                     }
                }
            }
        }
    }

    /* Final pass: copy over those parts which are not duplicates  */
    for (Inkscape::XML::Node *def = defs->firstChild() ; def ; def = def->next()) {
        if(def->type() != Inkscape::XML::ELEMENT_NODE)continue;

        /* Ignore duplicate defs marked in the first pass */
        Glib::ustring defid = def->attribute("id");
        if( defid.find( DuplicateDefString ) != Glib::ustring::npos )continue;

        bool duplicate = false;
        SPObject *src = source->getObjectByRepr(def);

        // Prevent duplication of symbols... could be more clever.
        // The tag "_inkscape_duplicate" is added to "id" by ClipboardManagerImpl::copySymbol(). 
        // We assume that symbols are in defs section (not required by SVG spec).
        if (src && SP_IS_SYMBOL(src)) {

            Glib::ustring id = src->getRepr()->attribute("id");
            size_t pos = id.find( "_inkscape_duplicate" );
            if( pos != Glib::ustring::npos ) {

                // This is our symbol, now get rid of tag
                id.erase( pos ); 

                // Check that it really is a duplicate
                for (SPObject *trg = this->getDefs()->firstChild() ; trg ; trg = trg->getNext()) {
                    if( trg && SP_IS_SYMBOL(trg) && src != trg ) { 
                        std::string id2 = trg->getRepr()->attribute("id");

                        if( !id.compare( id2 ) ) {
                            duplicate = true;
                            break;
                        }
                    }
                }
                if ( !duplicate ) {
                    src->getRepr()->setAttribute("id", id.c_str() );
                }
            }
        }

        if (!duplicate) {
            Inkscape::XML::Node * dup = def->duplicate(this->getReprDoc());
            target_defs->appendChild(dup);
            Inkscape::GC::release(dup);
        }
    }
}

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
