#ifndef SEEN_SP_URI_REFERENCES_H
#define SEEN_SP_URI_REFERENCES_H

/*
 * Helper methods for resolving URI References
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstddef>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "bad-uri-exception.h"
#include "sp-object.h"

namespace Inkscape {

class URI;

/**
 * A class encapsulating a reference to a particular URI; observers can
 * be notified when the URI comes to reference a different SPObject.
 *
 * The URIReference increments and decrements the SPObject's hrefcount
 * automatically.
 *
 * @see SPObject
 * @see sp_object_href
 * @see sp_object_hunref
 */
class URIReference : public sigc::trackable {
public:
    /**
     * Constructor.
     *
     * @param owner The object on whose behalf this URIReference
     *              is holding a reference to the target object.
     */
    URIReference(SPObject *owner);
    URIReference(SPDocument *owner_document);

    /**
     * Destructor.  Calls shutdown() if the reference has not been
     * shut down yet.
     */
    virtual ~URIReference();

    /**
     * Attaches to a URI, relative to the specified document.
     *
     * Throws a BadURIException if the URI is unsupported,
     * or the fragment identifier is xpointer and malformed.
     *
     * @param rel_document document for relative URIs
     * @param uri the URI to watch
     */
    void attach(URI const& uri) throw(BadURIException);

    /**
     * Detaches from the currently attached URI target, if any;
     * the current referrent is signaled as NULL.
     */
    void detach();

    /**
     * @brief Returns a pointer to the current referrent of the
     * attached URI, or NULL.
     *
     * @return a pointer to the referenced SPObject or NULL
     */
    SPObject *getObject() const { return _obj; }

    /**
     * @brief Returns a pointer to the URIReference's owner
     *
     * @return a pointer to the URIReference's owner
     */
    SPObject *getOwner() const { return _owner; }

    /**
     * Accessor for the referrent change notification signal;
     * this signal is emitted whenever the URIReference's
     * referrent changes.
     *
     * Signal handlers take two parameters: the old and new
     * referrents.
     *
     * @returns a signal
     */
    sigc::signal<void, SPObject *, SPObject *> changedSignal() {
        return _changed_signal;
    }

    /**
     * Returns a pointer to a URI containing the currently attached
     * URI, or NULL if no URI is currently attached.
     *
     * @returns the currently attached URI, or NULL
     */
    URI const* getURI() const {
        return _uri;
    }

    /**
     * Returns true if there is currently an attached URI
     *
     * @returns true if there is an attached URI
     */
    bool isAttached() const {
        return (bool)_uri;
    }

    SPDocument *getOwnerDocument() { return _owner_document; }
    SPObject   *getOwnerObject()   { return _owner; }

protected:
    virtual bool _acceptObject(SPObject *obj) const {
        (void)obj;
        return true;
    }

private:
    SPObject *_owner;
    SPDocument *_owner_document;
    sigc::connection _connection;
    sigc::connection _release_connection;
    SPObject *_obj;
    URI *_uri;

    sigc::signal<void, SPObject *, SPObject *> _changed_signal;

    void _setObject(SPObject *object);
    void _release(SPObject *object);

    void operator=(URIReference const& ref);
    /* Private and definition-less to prevent accidental use. */
};

}

/**
 * Resolves an item referenced by a URI in CSS form contained in "url(...)"
 */
SPObject* sp_css_uri_reference_resolve( SPDocument *document, const char *uri );

SPObject *sp_uri_reference_resolve (SPDocument *document, const char *uri);

#endif // SEEN_SP_URI_REFERENCES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
