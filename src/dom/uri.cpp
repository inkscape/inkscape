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




#include "uri.h"
#include "charclass.h"

#include <stdio.h>
#include <stdarg.h>



namespace org
{
namespace w3c
{
namespace dom
{


typedef struct
{
    int  ival;
    char *sval;
    int  port;
} LookupEntry;

LookupEntry schemes[] =
{
    { URI::SCHEME_DATA,   "data:",    0 },
    { URI::SCHEME_HTTP,   "http:",   80 },
    { URI::SCHEME_HTTPS,  "https:", 443 },
    { URI::SCHEME_FTP,    "ftp",     12 },
    { URI::SCHEME_FILE,   "file:",    0 },
    { URI::SCHEME_LDAP,   "ldap:",  123 },
    { URI::SCHEME_MAILTO, "mailto:", 25 },
    { URI::SCHEME_NEWS,   "news:",  117 },
    { URI::SCHEME_TELNET, "telnet:", 23 },
    { 0,                  NULL,       0 }
};



//#########################################################################
//# C O N S T R U C T O R
//#########################################################################

/**
 *
 */
URI::URI()
{
    init();
}

/**
 *
 */
URI::URI(const DOMString &str)
{
    init();
    parse(str);
}


/**
 *
 */
URI::URI(const char *str)
{
    init();
    DOMString domStr = str;
    parse(domStr);
}


/**
 *
 */
URI::URI(const URI &other)
{
    init();
    assign(other);
}


/**
 *
 */
URI &URI::operator=(const URI &other)
{
    init();
    assign(other);
    return *this;
}


/**
 *
 */
URI::~URI()
{
}





/**
 *
 */
void URI::init()
{
    parsebuf  = NULL;
    parselen  = 0;
    scheme    = SCHEME_NONE;
    schemeStr = "";
    port      = 0;
    authority = "";
    path      = "";
    absolute  = false;
    opaque    = false;
    query     = "";
    fragment  = "";
}


/**
 *
 */
void URI::assign(const URI &other)
{
    scheme    = other.scheme;
    schemeStr = other.schemeStr;
    authority = other.authority;
    port      = other.port;
    path      = other.path;
    absolute  = other.absolute;
    opaque    = other.opaque;
    query     = other.query;
    fragment  = other.fragment;
}


//#########################################################################
//#A T T R I B U T E S
//#########################################################################

DOMString URI::toString() const
{
    DOMString str = schemeStr;
    if (authority.size() > 0)
        {
        str.append("//");
        str.append(authority);
        }
    str.append(path);
    if (query.size() > 0)
        {
        str.append("?");
        str.append(query);
        }
    if (fragment.size() > 0)
        {
        str.append("#");
        str.append(fragment);
        }
    return str;
}


int URI::getScheme() const
{
    return scheme;
}

DOMString URI::getSchemeStr() const
{
    return schemeStr;
}


DOMString URI::getAuthority() const
{
    DOMString ret = authority;
    if (portSpecified && port>=0)
        {
        char buf[7];
        snprintf(buf, 6, ":%6d", port);
        ret.append(buf);
        }
    return ret;
}

DOMString URI::getHost() const
{
    return authority;
}

int URI::getPort() const
{
    return port;
}


DOMString URI::getPath() const
{
    return path;
}


bool URI::isAbsolute() const
{
    return absolute;
}

bool URI::isOpaque() const
{
    return opaque;
}


DOMString URI::getQuery() const
{
    return query;
}


DOMString URI::getFragment() const
{
    return fragment;
}


URI URI::resolve(const URI &other) const
{
    //### According to w3c, this is handled in 3 cases

    //## 1
    if (opaque || other.isAbsolute())
        return other;

    //## 2
    if (other.fragment.size()  >  0 &&
        other.path.size()      == 0 &&
        other.scheme           == SCHEME_NONE &&
        other.authority.size() == 0 &&
        other.query.size()     == 0 )
        {
        URI fragUri = *this;
        fragUri.fragment = other.fragment;
        return fragUri;
        }

    //## 3 http://www.ietf.org/rfc/rfc2396.txt, section 5.2
    URI newUri;
    //# 3.1
    newUri.scheme    = scheme;
    newUri.schemeStr = schemeStr;
    newUri.query     = other.query;
    newUri.fragment  = other.fragment;
    if (other.authority.size() > 0)
        {
        //# 3.2
        if (absolute || other.absolute)
            newUri.absolute = true;
        newUri.authority = other.authority;
        newUri.port      = other.port;//part of authority
        newUri.path      = other.path;
        }
    else
        {
        //# 3.3
        if (other.absolute)
            {
            newUri.absolute = true;
            newUri.path     = other.path;
            }
        else
            {
            unsigned int pos = path.rfind('/');
            if (pos != path.npos)
                {
                DOMString tpath = path.substr(pos);
                tpath.append(other.path);
                newUri.path = tpath;
                newUri.normalize();
                }
            }
        }
    return newUri;
}


/**
 *
 */
void URI::normalize() const
{






}



//#########################################################################
//# M E S S A G E S
//#########################################################################

void URI::error(const char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "URI error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void URI::trace(const char *fmt, ...)
{
    va_list args;
    fprintf(stdout, "URI: ");
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}



//#########################################################################
//# P A R S I N G
//#########################################################################



int URI::peek(int p)
{
    if (p<0 || p>=parselen)
        return -1;
    return parsebuf[p];
}



int URI::match(int p0, char *key)
{
    int p = p0;
    while (p < parselen)
        {
        if (*key == '\0')
            return p;
        else if (*key != parsebuf[p])
            break;
        p++; key++;
        }
    return p0;
}

//#########################################################################
//#  Parsing is performed according to:
//#  http://www.gbiv.com/protocols/uri/rfc/rfc3986.html#components
//#########################################################################

int URI::parseScheme(int p0)
{
    int p = p0;
    for (LookupEntry *entry = schemes; entry->sval ; entry++)
        {
        int p2 = match(p, entry->sval);
        if (p2 > p)
            {
            schemeStr = entry->sval;
            scheme    = entry->ival;
            port      = entry->port;
            p = p2;
            return p;
            }
        }

    return p;
}


int URI::parseHierarchicalPart(int p0)
{
    int p = p0;
    int ch;

    //# Authority field (host and port, for example)
    int p2 = match(p, "//");
    if (p2 > p)
        {
        p = p2;
        portSpecified = false;
        DOMString portStr;
        while (p < parselen)
            {
            ch = peek(p);
            if (ch == '/')
                break;
            else if (ch == ':')
                portSpecified = true;
            else if (portSpecified)
                portStr.push_back((XMLCh)ch);
            else
                authority.push_back((XMLCh)ch);
            p++;
            }
        if (portStr.size() > 0)
            {
            char *pstr = (char *)portStr.c_str();
            char *endStr;
            long val = strtol(pstr, &endStr, 10);
            if (endStr > pstr) //successful parse?
                port = val;
            }
        }

    //# Are we absolute?
    ch = peek(p);
    if (ch == '/')
        {
        absolute = true;
        if (p>p0) //in other words, if '/' is not the first char
            opaque = true;
        path.push_back((XMLCh)ch);
        p++;
        }

    while (p < parselen)
        {
        ch = peek(p);
        if (ch == '?' || ch == '#')
            break;
        path.push_back((XMLCh)ch);
        p++;
        }

    return p;
}

int URI::parseQuery(int p0)
{
    int p = p0;
    int ch = peek(p);
    if (ch != '?')
        return p0;

    p++;
    while (p < parselen)
        {
        ch = peek(p);
        if (ch == '#')
            break;
        query.push_back((XMLCh)ch);
        p++;
        }


    return p;
}

int URI::parseFragment(int p0)
{

    int p = p0;
    int ch = peek(p);
    if (ch != '#')
        return p0;

    p++;
    while (p < parselen)
        {
        ch = peek(p);
        if (ch == '?')
            break;
        fragment.push_back((XMLCh)ch);
        p++;
        }


    return p;
}


int URI::parse(int p0)
{

    int p = p0;

    int p2 = parseScheme(p);
    if (p2 < 0)
        {
        error("Scheme");
        return -1;
        }
    p = p2;


    p2 = parseHierarchicalPart(p);
    if (p2 < 0)
        {
        error("Hierarchical part");
        return -1;
        }
    p = p2;

    p2 = parseQuery(p);
    if (p2 < 0)
        {
        error("Query");
        return -1;
        }
    p = p2;


    p2 = parseFragment(p);
    if (p2 < 0)
        {
        error("Fragment");
        return -1;
        }
    p = p2;

    return p;

}



bool URI::parse(const DOMString &str)
{

    parselen = str.size();
    DOMString tmp = str;
    parsebuf = (char *) tmp.c_str();


    int p = parse(0);

    if (p < 0)
        {
        error("Syntax error");
        return false;
        }

    //printf("uri:%s\n", toString().c_str());
    //printf("path:%s\n", path.c_str());

    return true;

}





}  //namespace dom
}  //namespace w3c
}  //namespace org
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################



