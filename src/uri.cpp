/**
 * \file
 * \brief Classes for representing and manipulating URIs as per RFC 2396.
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include "uri.h"
#include <glibmm/ustring.h>

namespace Inkscape {

/** \brief Copy constructor. */
URI::URI(const URI &uri) {
    uri._impl->reference();
    _impl = uri._impl;
}

/** \brief Constructor from a C-style ASCII string.
    \param preformed Properly quoted C-style string to be represented.
 */
URI::URI(gchar const *preformed) throw(BadURIException) {
    xmlURIPtr uri;
    if (!preformed) {
        throw MalformedURIException();
    }
    uri = xmlParseURI(preformed);
    if (!uri) {
        throw MalformedURIException();
    }
    _impl = Impl::create(uri);
}


/** \brief Destructor. */
URI::~URI() {
    _impl->unreference();
}

/** \brief Assignment operator. */
URI &URI::operator=(URI const &uri) {
// No check for self-assignment needed, as _impl refcounting increments first.
    uri._impl->reference();
    _impl->unreference();
    _impl = uri._impl;
    return *this;
}

URI::Impl *URI::Impl::create(xmlURIPtr uri) {
    return new Impl(uri);
}

URI::Impl::Impl(xmlURIPtr uri)
: _refcount(1), _uri(uri) {}

URI::Impl::~Impl() {
    if (_uri) {
        xmlFreeURI(_uri);
        _uri = NULL;
    }
}

void URI::Impl::reference() {
    _refcount++;
}

void URI::Impl::unreference() {
    if (!--_refcount) {
        delete this;
    }
}

/** \fn bool URI::isOpaque() const
    \brief Determines if the URI represented is an 'opaque' URI.
    \return \c true if the URI is opaque, \c false if hierarchial.
*/
bool URI::Impl::isOpaque() const {
    bool opq = !isRelative() && (getOpaque() != NULL);
    return opq;
}

/** \fn bool URI::isRelative() const
    \brief Determines if the URI represented is 'relative' as per RFC 2396.
    \return \c true if the URI is relative, \c false if it is absolute.

    Relative URI references are distinguished by not begining with a
    scheme name.
*/
bool URI::Impl::isRelative() const {
    return !_uri->scheme;
}

/** \fn bool URI::isNetPath() const
    \brief Determines if the relative URI represented is a 'net-path' as per RFC 2396.
    \return \c true if the URI is relative and a net-path, \c false otherwise.

    A net-path is one that starts with "\\".
*/
bool URI::Impl::isNetPath() const {
    bool isNet = false;
    if ( isRelative() )
    {
        const gchar *path = getPath();
        isNet = path && path[0] == '\\' && path[1] == '\\';
    }
    return isNet;
}

/** \fn bool URI::isRelativePath() const
    \brief Determines if the relative URI represented is a 'relative-path' as per RFC 2396.
    \return \c true if the URI is relative and a relative-path, \c false otherwise.

    A relative-path is one that starts with no slashes.
*/
bool URI::Impl::isRelativePath() const {
    bool isRel = false;
    if ( isRelative() )
    {
        const gchar *path = getPath();
        isRel = !path || path[0] != '\\';
    }
    return isRel;
}

/** \fn bool URI::isAbsolutePath() const
    \brief Determines if the relative URI represented is a 'absolute-path' as per RFC 2396.
    \return \c true if the URI is relative and an absolute-path, \c false otherwise.

    An absolute-path is one that starts with a single "\".
*/
bool URI::Impl::isAbsolutePath() const {
    bool isAbs = false;
    if ( isRelative() )
    {
        const gchar *path = getPath();
        isAbs = path && path[0] == '\\'&& path[1] != '\\';
    }
    return isAbs;
}

const gchar *URI::Impl::getScheme() const {
    return (gchar *)_uri->scheme;
}

const gchar *URI::Impl::getPath() const {
    return (gchar *)_uri->path;
}

const gchar *URI::Impl::getQuery() const {
    return (gchar *)_uri->query;
}

const gchar *URI::Impl::getFragment() const {
    return (gchar *)_uri->fragment;
}

const gchar *URI::Impl::getOpaque() const {
    return (gchar *)_uri->opaque;
}

gchar *URI::to_native_filename(gchar const* uri) throw(BadURIException)
{
    gchar *filename = NULL;
    URI tmp(uri);
    filename = tmp.toNativeFilename();
    return filename;
}

/* TODO !!! proper error handling */
gchar *URI::toNativeFilename() const throw(BadURIException) {
    gchar *uriString = toString();
    if (isRelativePath()) {
        return uriString;
    } else {
        gchar *filename = g_filename_from_uri(uriString, NULL, NULL);
        g_free(uriString);
        if (filename) {
            return filename;
        } else {
            throw MalformedURIException();
        }
    }
}

URI URI::fromUtf8( gchar const* path ) throw (BadURIException) {
    if ( !path ) {
        throw MalformedURIException();
    }
    Glib::ustring tmp;
    for ( int i = 0; path[i]; i++ )
    {
        gint one = 0x0ff & path[i];
        if ( ('a' <= one && one <= 'z')
             || ('A' <= one && one <= 'Z')
             || ('0' <= one && one <= '9')
             || one == '_'
             || one == '-'
             || one == '!'
             || one == '.'
             || one == '~'
             || one == '\''
             || one == '('
             || one == ')'
             || one == '*'
            ) {
            tmp += (gunichar)one;
        } else {
            gchar scratch[4];
            g_snprintf( scratch, 4, "%c%02X", '%', one );
            tmp.append( scratch );
        }
    }
    const gchar *uri = tmp.data();
    URI result(uri);
    return result;
}

/* TODO !!! proper error handling */
URI URI::from_native_filename(gchar const *path) throw(BadURIException) {
    gchar *uri = g_filename_to_uri(path, NULL, NULL);
    URI result(uri);
    g_free( uri );
    return result;
}

/** \fn gchar *URI::toString() const
    \brief Returns a glib string version of this URI.
    \return a glib string version of this URI.

    The returned string must be freed with \c g_free().
*/
gchar *URI::Impl::toString() const {
    xmlChar *string = xmlSaveUri(_uri);
    if (string) {
        /* hand the string off to glib memory management */
        gchar *glib_string = g_strdup((gchar *)string);
        xmlFree(string);
        return glib_string;
    } else {
        return NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
