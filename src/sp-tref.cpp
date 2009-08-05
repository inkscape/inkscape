#define __SP_TREF_CPP__

/** \file
 * SVG <tref> implementation - All character data within the referenced
 * element, including character data enclosed within additional markup,
 * will be rendered.
 *
 * This file was created based on skeleton.cpp
 */
/*
 * Authors:
 *   Gail Banaszkiewicz <Gail.Banaszkiewicz@gmail.com>
 *
 * Copyright (C) 2007 Gail Banaszkiewicz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include "attributes.h"
#include "document.h"
#include "sp-object-repr.h"
#include "sp-text.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "style.h"
#include "text-editing.h"
#include "uri.h"

#include "display/nr-arena-group.h"
#include "libnr/nr-matrix-fns.h"
#include "xml/node.h"
#include "xml/repr.h"


//#define DEBUG_TREF
#ifdef DEBUG_TREF
# define debug(f, a...) { g_message("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_message(f, ## a); \
                          g_message("\n"); \
                        }
#else
# define debug(f, a...) /**/
#endif


static void build_string_from_root(Inkscape::XML::Node *root, Glib::ustring *retString);

/* TRef base class */

static void sp_tref_class_init(SPTRefClass *tref_class);
static void sp_tref_init(SPTRef *tref);
static void sp_tref_finalize(GObject *obj);

static void sp_tref_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_tref_release(SPObject *object);
static void sp_tref_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_tref_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_tref_modified(SPObject *object, guint flags);
static Inkscape::XML::Node *sp_tref_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static void sp_tref_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static gchar *sp_tref_description(SPItem *item);

static void sp_tref_href_changed(SPObject *old_ref, SPObject *ref, SPTRef *tref);
static void sp_tref_delete_self(SPObject *deleted, SPTRef *self);

static SPObjectClass *tref_parent_class;

GType
sp_tref_get_type()
{
    static GType tref_type = 0;

    if (!tref_type) {
        GTypeInfo tref_info = {
            sizeof(SPTRefClass),
            NULL, NULL,
            (GClassInitFunc) sp_tref_class_init,
            NULL, NULL,
            sizeof(SPTRef),
            16,
            (GInstanceInitFunc) sp_tref_init,
            NULL,    /* value_table */
        };
        tref_type = g_type_register_static(SP_TYPE_ITEM, "SPTRef", &tref_info, (GTypeFlags)0);
    }
    return tref_type;
}

static void
sp_tref_class_init(SPTRefClass *tref_class)
{
    GObjectClass *gobject_class = (GObjectClass *) tref_class;
    SPObjectClass *sp_object_class = (SPObjectClass *)tref_class;

    tref_parent_class = (SPObjectClass*)g_type_class_peek_parent(tref_class);

    sp_object_class->build = sp_tref_build;
    sp_object_class->release = sp_tref_release;
    sp_object_class->write = sp_tref_write;
    sp_object_class->set = sp_tref_set;
    sp_object_class->update = sp_tref_update;
    sp_object_class->modified = sp_tref_modified;

    gobject_class->finalize = sp_tref_finalize;

    SPItemClass *item_class = (SPItemClass *) tref_class;

    item_class->bbox = sp_tref_bbox;
    item_class->description = sp_tref_description;
}

static void
sp_tref_init(SPTRef *tref)
{
    new (&tref->attributes) TextTagAttributes;

    tref->href = NULL;
    tref->uriOriginalRef = new SPTRefReference(SP_OBJECT(tref));
    new (&tref->_delete_connection) sigc::connection();
    new (&tref->_changed_connection) sigc::connection();

    tref->_changed_connection =
        tref->uriOriginalRef->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_tref_href_changed), tref));
}


static void
sp_tref_finalize(GObject *obj)
{
    SPTRef *tref = (SPTRef *) obj;

    delete tref->uriOriginalRef;

    tref->_delete_connection.~connection();
    tref->_changed_connection.~connection();
}


/**
 * Reads the Inkscape::XML::Node, and initializes SPTRef variables.
 */
static void
sp_tref_build(SPObject *object, Document *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) tref_parent_class)->build) {
        ((SPObjectClass *) tref_parent_class)->build(object, document, repr);
    }

    sp_object_read_attr(object, "xlink:href");
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "dx");
    sp_object_read_attr(object, "dy");
    sp_object_read_attr(object, "rotate");
}

/**
 * Drops any allocated memory.
 */
static void
sp_tref_release(SPObject *object)
{
    SPTRef *tref = SP_TREF(object);

    tref->attributes.~TextTagAttributes();

    tref->_delete_connection.disconnect();
    tref->_changed_connection.disconnect();

    g_free(tref->href);
    tref->href = NULL;

    tref->uriOriginalRef->detach();

    if (((SPObjectClass *) tref_parent_class)->release)
        ((SPObjectClass *) tref_parent_class)->release(object);
}

/**
 * Sets a specific value in the SPTRef.
 */
static void
sp_tref_set(SPObject *object, unsigned int key, gchar const *value)
{
    debug("0x%p %s(%u): '%s'",object,
            sp_attribute_name(key),key,value ? value : "<no value>");

    SPTRef *tref = SP_TREF(object);

    if (tref->attributes.readSingleAttribute(key, value)) { // x, y, dx, dy, rotate
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    } else if (key == SP_ATTR_XLINK_HREF) { // xlink:href
        if ( !value ) {
            // No value
            g_free(tref->href);
            tref->href = NULL;
            tref->uriOriginalRef->detach();
        } else if ((tref->href && strcmp(value, tref->href) != 0) || (!tref->href)) {

            // Value has changed

            if ( tref->href ) {
                g_free(tref->href);
                tref->href = NULL;
            }

            tref->href = g_strdup(value);

            try {
                tref->uriOriginalRef->attach(Inkscape::URI(value));
                tref->uriOriginalRef->updateObserver();
            } catch ( Inkscape::BadURIException &e ) {
                g_warning("%s", e.what());
                tref->uriOriginalRef->detach();
            }

            // No matter what happened, an update should be in order
            SP_OBJECT(tref)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }

    } else { // default
        if (((SPObjectClass *) tref_parent_class)->set) {
            ((SPObjectClass *) tref_parent_class)->set(object, key, value);
        }
    }


}

/**
 * Receives update notifications.  Code based on sp_use_update and sp_tspan_update.
 */
static void
sp_tref_update(SPObject *object, SPCtx *ctx, guint flags)
{
    debug("0x%p",object);

    SPTRef *tref = SP_TREF(object);

    if (((SPObjectClass *) tref_parent_class)->update) {
        ((SPObjectClass *) tref_parent_class)->update(object, ctx, flags);
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    SPObject *child = tref->stringChild;
    if (child) {
        if ( flags || ( child->uflags & SP_OBJECT_MODIFIED_FLAG )) {
            child->updateDisplay(ctx, flags);
        }
    }


}

static void
sp_tref_modified(SPObject *object, guint flags)
{
    SPTRef *tref_obj = SP_TREF(object);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    SPObject *child = tref_obj->stringChild;
    if (child) {
        g_object_ref(G_OBJECT(child));
        if (flags || (child->mflags & SP_OBJECT_MODIFIED_FLAG)) {
            child->emitModified(flags);
        }
        g_object_unref(G_OBJECT(child));
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_tref_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    debug("0x%p",object);

    SPTRef *tref = SP_TREF(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:tref");
    }

    tref->attributes.writeTo(repr);

    if (tref->uriOriginalRef->getURI()) {
        gchar *uri_string = tref->uriOriginalRef->getURI()->toString();
        debug("uri_string=%s", uri_string);
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    if (((SPObjectClass *) tref_parent_class)->write) {
        ((SPObjectClass *) tref_parent_class)->write(object, xml_doc, repr, flags);
    }

    return repr;
}

/**
 *  The code for this function is swiped from the tspan bbox code, since tref should work pretty much the same way
 */
static void
sp_tref_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const /*flags*/)
{
    // find out the ancestor text which holds our layout
    SPObject *parent_text = SP_OBJECT(item);
    for (; parent_text != NULL && !SP_IS_TEXT(parent_text); parent_text = SP_OBJECT_PARENT (parent_text)){};
    if (parent_text == NULL) return;

    // get the bbox of our portion of the layout
    SP_TEXT(parent_text)->layout.getBoundingBox(
        bbox, transform, sp_text_get_length_upto(parent_text, item), sp_text_get_length_upto(item, NULL) - 1);

    // Add stroke width
    SPStyle* style=SP_OBJECT_STYLE (item);
    if (!style->stroke.isNone()) {
        double const scale = transform.descrim();
        if ( fabs(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
            double const width = MAX(0.125, style->stroke_width.computed * scale);
            if ( fabs(bbox->x1 - bbox->x0) > -0.00001 && fabs(bbox->y1 - bbox->y0) > -0.00001 ) {
                bbox->x0-=0.5*width;
                bbox->x1+=0.5*width;
                bbox->y0-=0.5*width;
                bbox->y1+=0.5*width;
            }
        }
    }
}


static gchar *
sp_tref_description(SPItem *item)
{
    SPTRef *tref = SP_TREF(item);

    SPObject *referred = tref->getObjectReferredTo();

    if (tref && tref->getObjectReferredTo()) {
        char *child_desc;

        if (SP_IS_ITEM(referred)) {
            child_desc = sp_item_description(SP_ITEM(referred));
        } else {
            child_desc = g_strdup("");
        }

        char *ret = g_strdup_printf(
                _("<b>Cloned character data</b>%s%s"),
                (SP_IS_ITEM(referred) ? _(" from ") : ""),
                child_desc);
        g_free(child_desc);
        return ret;
    } else {
        return g_strdup(_("<b>Orphaned cloned character data</b>"));
    }
}


/* For the sigc::connection changes (i.e. when the object being refered to changes) */
static void
sp_tref_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, SPTRef *tref)
{
    if (tref)
    {
        // Save a pointer to the original object being referred to
        SPObject *refRoot = tref->getObjectReferredTo();

        tref->_delete_connection.disconnect();

        if (tref->stringChild) {
            sp_object_detach(SP_OBJECT(tref), tref->stringChild);
            tref->stringChild = NULL;
        }

        // Ensure that we are referring to a legitimate object
        if (tref->href && refRoot && sp_tref_reference_allowed(tref, refRoot)) {

            // Update the text being referred to (will create a new string child)
            sp_tref_update_text(tref);

            // Restore the delete connection now that we're done messing with stuff
            tref->_delete_connection = SP_OBJECT(refRoot)->connectDelete(sigc::bind(sigc::ptr_fun(&sp_tref_delete_self), tref));
        }

    }
}


/**
 * Delete the tref object
 */
static void
sp_tref_delete_self(SPObject */*deleted*/, SPTRef *self)
{
    SP_OBJECT(self)->deleteObject();
}

/**
 * Return the object referred to via the URI reference
 */
SPObject * SPTRef::getObjectReferredTo(void)
{
    SPObject *referredObject = NULL;

    if (uriOriginalRef) {
        referredObject = SP_OBJECT(uriOriginalRef->getObject());
    }

    return referredObject;
}


/**
 * Returns true when the given tref is allowed to refer to a particular object
 */
bool
sp_tref_reference_allowed(SPTRef *tref, SPObject *possible_ref)
{
    bool allowed = false;

    if (tref && possible_ref) {
        if (tref != possible_ref) {
            bool ancestor = false;
            for (SPObject *obj = tref; obj; obj = SP_OBJECT_PARENT(obj)) {
                if (possible_ref == obj) {
                    ancestor = true;
                    break;
                }
            }
            allowed = !ancestor;
        }
    }

    return allowed;
}


/**
 * Returns true if a tref is fully contained in the confines of the given
 * iterators and layout (or if there is no tref).
 */
bool
sp_tref_fully_contained(SPObject *start_item, Glib::ustring::iterator &start,
                             SPObject *end_item, Glib::ustring::iterator &end)
{
    bool fully_contained = false;

    if (start_item && end_item) {

        // If neither the beginning or the end is a tref then we return true (whether there
        // is a tref in the innards or not, because if there is one then it must be totally
        // contained)
        if (!(SP_IS_STRING(start_item) && SP_IS_TREF(SP_OBJECT_PARENT(start_item)))
                && !(SP_IS_STRING(end_item) && SP_IS_TREF(SP_OBJECT_PARENT(end_item)))) {
            fully_contained = true;
        }

        // Both the beginning and end are trefs; but in this case, the string iterators
        // must be at the right places
        else if ((SP_IS_STRING(start_item) && SP_IS_TREF(SP_OBJECT_PARENT(start_item)))
                && (SP_IS_STRING(end_item) && SP_IS_TREF(SP_OBJECT_PARENT(end_item)))) {
            if (start == SP_STRING(start_item)->string.begin()
                    && end == SP_STRING(start_item)->string.end()) {
                fully_contained = true;
            }
        }

        // If the beginning is a string that is a child of a tref, the iterator has to be
        // at the beginning of the item
        else if ((SP_IS_STRING(start_item) && SP_IS_TREF(SP_OBJECT_PARENT(start_item)))
                    && !(SP_IS_STRING(end_item) && SP_IS_TREF(SP_OBJECT_PARENT(end_item)))) {
            if (start == SP_STRING(start_item)->string.begin()) {
                fully_contained = true;
            }
        }

        // Same, but the for the end
        else if (!(SP_IS_STRING(start_item) && SP_IS_TREF(SP_OBJECT_PARENT(start_item)))
                    && (SP_IS_STRING(end_item) && SP_IS_TREF(SP_OBJECT_PARENT(end_item)))) {
            if (end == SP_STRING(start_item)->string.end()) {
                fully_contained = true;
            }
        }
    }

    return fully_contained;
}


void
sp_tref_update_text(SPTRef *tref)
{
    if (tref) {
        // Get the character data that will be used with this tref
        Glib::ustring charData = "";
        build_string_from_root(SP_OBJECT_REPR(tref->getObjectReferredTo()), &charData);

        if (tref->stringChild) {
            sp_object_detach(SP_OBJECT(tref), tref->stringChild);
            tref->stringChild = NULL;
        }

        // Create the node and SPString to be the tref's child
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(tref));

        Inkscape::XML::Node *newStringRepr = xml_doc->createTextNode(charData.c_str());
        tref->stringChild = SP_OBJECT(g_object_new(sp_repr_type_lookup(newStringRepr), NULL));

        // Add this SPString as a child of the tref
        sp_object_attach(SP_OBJECT(tref), tref->stringChild, tref->lastChild());
        sp_object_unref(tref->stringChild, NULL);
        sp_object_invoke_build(tref->stringChild, SP_OBJECT(tref)->document, newStringRepr, TRUE);

        Inkscape::GC::release(newStringRepr);
    }
}



/**
 * Using depth-first search, build up a string by concatenating all SPStrings
 * found in the tree starting at the root
 */
static void
build_string_from_root(Inkscape::XML::Node *root, Glib::ustring *retString)
{
    if (root && retString) {

        // Stop and concatenate when a SPString is found
        if (root->type() == Inkscape::XML::TEXT_NODE) {
            *retString += (root->content());

            debug("%s", retString->c_str());

        // Otherwise, continue searching down the tree (with the assumption that no children nodes
        // of a SPString are actually legal)
        } else {
            Inkscape::XML::Node *childNode;
            for (childNode = root->firstChild(); childNode; childNode = childNode->next()) {
                build_string_from_root(childNode, retString);
            }
        }
    }
}

/**
 * This function will create a new tspan element with the same attributes as
 * the tref had and add the same text as a child.  The tref is replaced in the
 * tree with the new tspan.
 * The code is based partially on sp_use_unlink
 */
SPObject *
sp_tref_convert_to_tspan(SPObject *obj)
{
    SPObject * new_tspan = NULL;

    ////////////////////
    // BASE CASE
    ////////////////////
    if (SP_IS_TREF(obj)) {

        SPTRef *tref = SP_TREF(obj);

        if (tref && tref->stringChild) {
            Inkscape::XML::Node *tref_repr = SP_OBJECT_REPR(tref);
            Inkscape::XML::Node *tref_parent = sp_repr_parent(tref_repr);

            Document *document = SP_OBJECT(tref)->document;
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

            Inkscape::XML::Node *new_tspan_repr = xml_doc->createElement("svg:tspan");

            // Add the new tspan element just after the current tref
            tref_parent->addChild(new_tspan_repr, tref_repr);
            Inkscape::GC::release(new_tspan_repr);

            new_tspan = document->getObjectByRepr(new_tspan_repr);

            // Create a new string child for the tspan
            Inkscape::XML::Node *new_string_repr = SP_OBJECT_REPR(tref->stringChild)->duplicate(xml_doc);
            new_tspan_repr->addChild(new_string_repr, NULL);

            //SPObject * new_string_child = document->getObjectByRepr(new_string_repr);

            // Merge style from the tref
            SPStyle *new_tspan_sty = SP_OBJECT_STYLE(new_tspan);
            SPStyle const *tref_sty = SP_OBJECT_STYLE(tref);
            sp_style_merge_from_dying_parent(new_tspan_sty, tref_sty);
            sp_style_merge_from_parent(new_tspan_sty, new_tspan->parent->style);


            SP_OBJECT(new_tspan)->updateRepr();

            // Hold onto our SPObject and repr for now.
            sp_object_ref(SP_OBJECT(tref), NULL);
            Inkscape::GC::anchor(tref_repr);

            // Remove ourselves, not propagating delete events to avoid a
            // chain-reaction with other elements that might reference us.
            SP_OBJECT(tref)->deleteObject(false);

            // Give the copy our old id and let go of our old repr.
            new_tspan_repr->setAttribute("id", tref_repr->attribute("id"));
            Inkscape::GC::release(tref_repr);

            // Establish the succession and let go of our object.
            SP_OBJECT(tref)->setSuccessor(new_tspan);
            sp_object_unref(SP_OBJECT(tref), NULL);
        }
    }
    ////////////////////
    // RECURSIVE CASE
    ////////////////////
    else {
        GSList *l = NULL;
        for (SPObject *child = sp_object_first_child(obj) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            sp_object_ref (SP_OBJECT (child), obj);
            l = g_slist_prepend (l, child);
        }
        l = g_slist_reverse (l);
        while (l) {
            SPObject *child = SP_OBJECT (l->data);
            l = g_slist_remove (l, child);

            // Note that there may be more than one conversion happening here, so if it's not a
            // tref being passed into this function, the returned value can't be specifically known
            new_tspan = sp_tref_convert_to_tspan(child);

            sp_object_unref (SP_OBJECT (child), obj);
        }
    }

    return new_tspan;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
