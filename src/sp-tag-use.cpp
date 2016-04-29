/*
 * SVG <inkscape:tagref> implementation
 *
 * Authors:
 *   Theodore Janeczko
 *   Liam P White
 *
 * Copyright (C) Theodore Janeczko 2012-2014 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>

#include <glibmm/i18n.h>
#include "display/drawing-group.h"
#include "attributes.h"
#include "document.h"
#include "uri.h"
#include "xml/repr.h"
#include "preferences.h"
#include "style.h"
#include "sp-factory.h"
#include "sp-symbol.h"
#include "sp-tag-use.h"
#include "sp-tag-use-reference.h"

SPTagUse::SPTagUse()
{
    href = NULL;
    //new (_changed_connection) sigc::connection;
    ref = new SPTagUseReference(this);
    
    _changed_connection = ref->changedSignal().connect(sigc::mem_fun(*this, &SPTagUse::href_changed));
}

SPTagUse::~SPTagUse()
{

    if (child) {
        detach(child);
        child = NULL;
    }

    ref->detach();
    delete ref;
    ref = 0;

    _changed_connection.~connection(); //FIXME why?
}

void
SPTagUse::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    SPObject::build(document, repr);
    readAttr( "xlink:href" );

    // We don't need to create child here:
    // reading xlink:href will attach ref, and that will cause the changed signal to be emitted,
    // which will call sp_tag_use_href_changed, and that will take care of the child
}

void
SPTagUse::release()
{

    if (child) {
        detach(child);
        child = NULL;
    }

    _changed_connection.disconnect();

    g_free(href);
    href = NULL;

    ref->detach();

    SPObject::release();
}

void
SPTagUse::set(unsigned key, gchar const *value)
{

    switch (key) {
        case SP_ATTR_XLINK_HREF: {
            if ( value && href && ( strcmp(value, href) == 0 ) ) {
                /* No change, do nothing. */
            } else {
                g_free(href);
                href = NULL;
                if (value) {
                    // First, set the href field, because sp_tag_use_href_changed will need it.
                    href = g_strdup(value);

                    // Now do the attaching, which emits the changed signal.
                    try {
                        ref->attach(Inkscape::URI(value));
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        ref->detach();
                    }
                } else {
                    ref->detach();
                }
            }
            break;
        }

        default:
                SPObject::set(key, value);
            break;
    }
}

Inkscape::XML::Node *
SPTagUse::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("inkscape:tagref");
    }

    SPObject::write(xml_doc, repr, flags);
    
    if (ref->getURI()) {
        gchar *uri_string = ref->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    return repr;
}

/**
 * Returns the ultimate original of a SPTagUse (i.e. the first object in the chain of its originals
 * which is not an SPTagUse). If no original is found, NULL is returned (it is the responsibility
 * of the caller to make sure that this is handled correctly).
 *
 * Note that the returned is the clone object, i.e. the child of an SPTagUse (of the argument one for
 * the trivial case) and not the "true original".
 */
 
SPItem * SPTagUse::root()
{
    SPObject *orig = child;
    while (orig && SP_IS_TAG_USE(orig)) {
        orig = SP_TAG_USE(orig)->child;
    }
    if (!orig || !SP_IS_ITEM(orig))
        return NULL;
    return SP_ITEM(orig);
}

void
SPTagUse::href_changed(SPObject */*old_ref*/, SPObject */*ref*/)
{
    if (href) {
        SPItem *refobj = ref->getObject();
        if (refobj) {
            Inkscape::XML::Node *childrepr = refobj->getRepr();
            const std::string typeString = NodeTraits::get_type_string(*childrepr);
            
            SPObject* child_ = SPFactory::createObject(typeString);
            if (child_) {
                child = child_;
                attach(child_, lastChild());
                sp_object_unref(child_, 0);
                child_->invoke_build(this->document, childrepr, TRUE);

            }
        }
    }
}

SPItem * SPTagUse::get_original()
{
    SPItem *ref_ = NULL;
    if (ref) {
        ref_ = ref->getObject();
    }
    return ref_;
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
