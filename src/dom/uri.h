#ifndef __URI_H__
#define __URI_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "dom.h"


namespace org
{
namespace w3c
{
namespace dom
{



class URI
{
public:

    typedef enum
        {
        SCHEME_NONE,
        SCHEME_DATA,
        SCHEME_HTTP,
        SCHEME_FTP,
        SCHEME_FILE,
        SCHEME_LDAP,
        SCHEME_MAILTO,
        SCHEME_NEWS,
        SCHEME_TELNET
        } SchemeTypes;

    /**
     *
     */
    URI();

    /**
     *
     */
    URI(const DOMString &str);


    /**
     *
     */
    URI(const char *str);

    /**
     *
     */
    URI(const URI &other);

    /**
     *
     */
    virtual ~URI();

    /**
     *
     */
    virtual bool parse(const DOMString &str);

    /**
     *
     */
    virtual DOMString toString();

    /**
     *
     */
    virtual int getScheme();

    /**
     *
     */
    virtual DOMString getSchemeStr();

    /**
     *
     */
    virtual DOMString getAuthority();

    /**
     *
     */
    virtual DOMString getPath();

    /**
     *
     */
    virtual bool getIsAbsolute();

    /**
     *
     */
    virtual DOMString getQuery();

    /**
     *
     */
    virtual DOMString getFragment();

private:

    void init();

    int scheme;

    DOMString schemeStr;

    DOMString authority;

    DOMString path;

    bool absolute;

    DOMString query;

    DOMString fragment;

    void error(const char *fmt, ...);

    void trace(const char *fmt, ...);


    int peek(int p);

    int match(int p, char *key);

    int parseScheme(int p);

    int parseHierarchicalPart(int p0);

    int parseQuery(int p0);

    int parseFragment(int p0);

    int parse(int p);

    char *parsebuf;

    int parselen;

};



}  //namespace dom
}  //namespace w3c
}  //namespace org



#endif /* __URI_H__ */

