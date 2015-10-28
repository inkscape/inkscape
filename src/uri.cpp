/*
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
#include <string>
#include <glibmm/ustring.h>
#include <glibmm/miscutils.h>

namespace Inkscape {

URI::URI() {
    const gchar *in = "";
    _impl = Impl::create(xmlParseURI(in));
}

URI::URI(const URI &uri) {
    uri._impl->reference();
    _impl = uri._impl;
}

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

URI::~URI() {
    _impl->unreference();
}

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

bool URI::Impl::isOpaque() const {
    bool opq = !isRelative() && (getOpaque() != NULL);
    return opq;
}

bool URI::Impl::isRelative() const {
    return !_uri->scheme;
}

bool URI::Impl::isNetPath() const {
    bool isNet = false;
    if ( isRelative() )
    {
        const gchar *path = getPath();
        isNet = path && path[0] == '\\' && path[1] == '\\';
    }
    return isNet;
}

bool URI::Impl::isRelativePath() const {
    bool isRel = false;
    if ( isRelative() )
    {
        const gchar *path = getPath();
        isRel = !path || path[0] != '\\';
    }
    return isRel;
}

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
/*
 * Returns the absolute path to an existing file referenced in this URI,
 * if the uri is data, the path is empty or the file doesn't exist, then
 * an empty string is returned.
 *
 * Does not check if the returned path is the local document's path (local)
 * and thus redundent. Caller is expected to check against the document's path.
 */
const std::string URI::getFullPath(std::string const &base) const {
    if (!_impl->getPath()) {
        return "";
    }
    std::string path = std::string(_impl->getPath());
    // Calculate the absolute path from an available base
    if(!base.empty() && !path.empty() && path[0] != '/') {
        path = Glib::build_filename(base, path);
    }
    // Check the existance of the file
    if(! g_file_test(path.c_str(), G_FILE_TEST_EXISTS) 
      || g_file_test(path.c_str(), G_FILE_TEST_IS_DIR) ) {
        path.clear();
    }
    return path;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
