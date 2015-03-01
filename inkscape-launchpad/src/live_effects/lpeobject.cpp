/*
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *   Abhishek Sharma
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpeobject.h"

#include "live_effects/effect.h"

#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "sp-object.h"
#include "attributes.h"
#include "document.h"
#include "document-private.h"

#include <glibmm/i18n.h>

//#define LIVEPATHEFFECT_VERBOSE

static void livepatheffect_on_repr_attr_changed (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);

static Inkscape::XML::NodeEventVector const livepatheffect_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    livepatheffect_on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


LivePathEffectObject::LivePathEffectObject()
    : SPObject(), effecttype(Inkscape::LivePathEffect::INVALID_LPE), effecttype_set(false),
      lpe(NULL)
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_message("Init livepatheffectobject");
#endif
}

LivePathEffectObject::~LivePathEffectObject() {
}

/**
 * Virtual build: set livepatheffect attributes from its associated XML node.
 */
void LivePathEffectObject::build(SPDocument *document, Inkscape::XML::Node *repr) {
    g_assert(this != NULL);
    g_assert(SP_IS_OBJECT(this));

    SPObject::build(document, repr);

    this->readAttr( "effect" );

    if (repr) {
        repr->addListener (&livepatheffect_repr_events, this);
    }

    /* Register ourselves, is this necessary? */
//    document->addResource("path-effect", object);
}

/**
 * Virtual release of livepatheffect members before destruction.
 */
void LivePathEffectObject::release() {
    this->getRepr()->removeListenerByData(this);

/*
    if (object->document) {
        // Unregister ourselves
        sp_document_removeResource(object->document, "livepatheffect", object);
    }

    if (gradient->ref) {
        gradient->modified_connection.disconnect();
        gradient->ref->detach();
        delete gradient->ref;
        gradient->ref = NULL;
    }

    gradient->modified_connection.~connection();
*/

    if (this->lpe) {
        delete this->lpe;
        this->lpe = NULL;
    }

    this->effecttype = Inkscape::LivePathEffect::INVALID_LPE;

    SPObject::release();
}

/**
 * Virtual set: set attribute to value.
 */
void LivePathEffectObject::set(unsigned key, gchar const *value) {
#ifdef LIVEPATHEFFECT_VERBOSE
    g_print("Set livepatheffect");
#endif

    switch (key) {
        case SP_PROP_PATH_EFFECT:
            if (this->lpe) {
                delete this->lpe;
                this->lpe = NULL;
            }

            if ( value && Inkscape::LivePathEffect::LPETypeConverter.is_valid_key(value) ) {
                this->effecttype = Inkscape::LivePathEffect::LPETypeConverter.get_id_from_key(value);
                this->lpe = Inkscape::LivePathEffect::Effect::New(this->effecttype, this);
                this->effecttype_set = true;
            } else {
                this->effecttype = Inkscape::LivePathEffect::INVALID_LPE;
                this->effecttype_set = false;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    }

    SPObject::set(key, value);
}

/**
 * Virtual write: write object attributes to repr.
 */
Inkscape::XML::Node* LivePathEffectObject::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("inkscape:path-effect");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->lpe) {
        repr->setAttribute("effect", Inkscape::LivePathEffect::LPETypeConverter.get_key(this->effecttype).c_str());

        this->lpe->writeParamsToSVG();
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}

static void
livepatheffect_on_repr_attr_changed ( Inkscape::XML::Node * /*repr*/,
                                      const gchar *key,
                                      const gchar */*oldval*/,
                                      const gchar *newval,
                                      bool /*is_interactive*/,
                                      void * data )
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_print("livepatheffect_on_repr_attr_changed");
#endif

    if (!data)
        return;

    LivePathEffectObject *lpeobj = (LivePathEffectObject*) data;
    if (!lpeobj->get_lpe())
        return;

    lpeobj->get_lpe()->setParameter(key, newval);

    lpeobj->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * If this has other users, create a new private duplicate and return it
 * returns 'this' when no forking was necessary (and therefore no duplicate was made)
 * Check out SPLPEItem::forkPathEffectsIfNecessary !
 */
LivePathEffectObject *LivePathEffectObject::fork_private_if_necessary(unsigned int nr_of_allowed_users)
{
    if (hrefcount > nr_of_allowed_users) {
        SPDocument *doc = this->document;
        Inkscape::XML::Document *xml_doc = doc->getReprDoc();
        Inkscape::XML::Node *dup_repr = this->getRepr()->duplicate(xml_doc);

        doc->getDefs()->getRepr()->addChild(dup_repr, NULL);
        LivePathEffectObject *lpeobj_new = LIVEPATHEFFECT( doc->getObjectByRepr(dup_repr) );

        Inkscape::GC::release(dup_repr);
        return lpeobj_new;
    }
    return this;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
