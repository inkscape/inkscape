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

#ifndef INKSCAPE_URI_H
#define INKSCAPE_URI_H

#include <glib/gtypes.h>
#include <exception>
#include <libxml/uri.h>
#include "bad-uri-exception.h"

namespace Inkscape {

/** \brief Represents an URI as per RFC 2396. */
class URI {
public:
    URI(URI const &uri);
    explicit URI(gchar const *preformed) throw(BadURIException);
    ~URI();

    bool isOpaque() const { return _impl->isOpaque(); }
    bool isRelative() const { return _impl->isRelative(); }
    bool isNetPath() const { return _impl->isNetPath(); }
    bool isRelativePath() const { return _impl->isRelativePath(); }
    bool isAbsolutePath() const { return _impl->isAbsolutePath(); }
    const gchar *getScheme() const { return _impl->getScheme(); }
    const gchar *getPath() const { return _impl->getPath(); }
    const gchar *getQuery() const { return _impl->getQuery(); }
    const gchar *getFragment() const { return _impl->getFragment(); }
    const gchar *getOpaque() const { return _impl->getOpaque(); }

    static URI fromUtf8( gchar const* path ) throw (BadURIException);
    static URI from_native_filename(gchar const *path) throw(BadURIException);
    static gchar *to_native_filename(gchar const* uri) throw(BadURIException);

    gchar *toNativeFilename() const throw(BadURIException);
    gchar *toString() const { return _impl->toString(); }

    URI &operator=(URI const &uri);

private:
    class Impl {
    public:
        static Impl *create(xmlURIPtr uri);
        void reference();
        void unreference();

        bool isOpaque() const;
        bool isRelative() const;
        bool isNetPath() const;
        bool isRelativePath() const;
        bool isAbsolutePath() const;
        const gchar *getScheme() const;
        const gchar *getPath() const;
        const gchar *getQuery() const;
        const gchar *getFragment() const;
        const gchar *getOpaque() const;
        gchar *toString() const;
    private:
        Impl(xmlURIPtr uri);
        ~Impl();
        int _refcount;
        xmlURIPtr _uri;
    };
    Impl *_impl;
};

}  /* namespace Inkscape */

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
