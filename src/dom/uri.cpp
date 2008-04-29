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
 * Copyright (C) 2005-2008 Bob Jamison
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
#include "ucd.h"

#include <stdio.h>
#include <stdarg.h>
#include <vector>


namespace org
{
namespace w3c
{
namespace dom
{


typedef struct
{
    int  ival;
    char const *sval;
    int  port;
} LookupEntry;

static LookupEntry schemes[] =
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
    schemeStr.clear();
    port      = 0;
    authority.clear();
    path.clear();
    absolute  = false;
    opaque    = false;
    query.clear();
    fragment.clear();
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
static const char *hexChars = "0123456789abcdef";

static DOMString toStr(const std::vector<int> &arr)
{
    DOMString buf;
    std::vector<int>::const_iterator iter;
    for (iter=arr.begin() ; iter!=arr.end() ; iter++)
        {
        int ch = *iter;
        if (isprint(ch))
            buf.push_back((XMLCh)ch);
        else
            {
            buf.push_back('%');
            int hi = ((ch>>4) & 0xf);
            buf.push_back(hexChars[hi]);
            int lo = ((ch   ) & 0xf);
            buf.push_back(hexChars[lo]);
            }
        }
    return buf;
}


DOMString URI::toString() const
{
    DOMString str = schemeStr;
    if (authority.size() > 0)
        {
        str.append("//");
        str.append(toStr(authority));
        }
    str.append(toStr(path));
    if (query.size() > 0)
        {
        str.append("?");
        str.append(toStr(query));
        }
    if (fragment.size() > 0)
        {
        str.append("#");
        str.append(toStr(fragment));
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
    DOMString ret = toStr(authority);
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
    DOMString str = toStr(authority);
    return str;
}

int URI::getPort() const
{
    return port;
}


DOMString URI::getPath() const
{
    DOMString str = toStr(path);
    return str;
}

DOMString URI::getNativePath() const
{
    DOMString pathStr = toStr(path);
    DOMString npath;
#ifdef __WIN32__
    unsigned int firstChar = 0;
    if (pathStr.size() >= 3)
        {
        if (pathStr[0] == '/' &&
            uni_is_letter(pathStr[1]) &&
            pathStr[2] == ':')
            firstChar++;
         }
    for (unsigned int i=firstChar ; i<pathStr.size() ; i++)
        {
        XMLCh ch = (XMLCh) pathStr[i];
        if (ch == '/')
            npath.push_back((XMLCh)'\\');
        else
            npath.push_back(ch);
        }
#else
    npath = pathStr;
#endif
    return npath;
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
    DOMString str = toStr(query);
    return str;
}


DOMString URI::getFragment() const
{
    DOMString str = toStr(fragment);
    return str;
}




static int find(const std::vector<int> &str, int ch, int startpos)
{
    for (unsigned int i = startpos ; i < str.size() ; i++)
        {
        if (ch == str[i])
            return i;
        }
    return -1;
}


static int findLast(const std::vector<int> &str, int ch)
{
    /**
     * Fixed.  Originally I used an unsigned int for str.size(),
     * which was dumb, since i>=0 would always be true.
     */
    for (int i = ((int)str.size())-1 ; i>=0 ; i--)
        {
        if (ch == str[i])
            return i;
        }
    return -1;
}


static bool sequ(const std::vector<int> &str, const char *key)
{
    char *c = (char *)key;
    for (unsigned int i=0 ; i<str.size() ; i++)
        {
        if (! (*c))
            return false;
        if (*c != str[i])
            return false;
        }
    return true;
}


static std::vector<int> substr(const std::vector<int> &str,
                      int startpos, int len)
{
    std::vector<int> buf;
    unsigned int pos = startpos;
    for (int i=0 ; i<len ; i++)
        {
        if (pos >= str.size())
            break;
        buf.push_back(str[pos++]);
        }
    return buf;
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
            int pos = findLast(path, '/');
            if (pos >= 0)
                {
                newUri.path.clear();
                //# append my path up to and including the '/'
                for (int i = 0; i<=pos ; i++)
                       newUri.path.push_back(path[i]);
                //# append other path
                for (unsigned int i = 0; i<other.path.size() ; i++)
                       newUri.path.push_back(other.path[i]);
                }
            else
                newUri.path = other.path;
            }
        }

    newUri.normalize();

    return newUri;
}


/**
 *  This follows the Java URI algorithm:
 *   1. All "." segments are removed.
 *   2. If a ".." segment is preceded by a non-".." segment
 *          then both of these segments are removed. This step
 *          is repeated until it is no longer applicable.
 *   3. If the path is relative, and if its first segment
 *          contains a colon character (':'), then a "." segment
 *          is prepended. This prevents a relative URI with a path
 *          such as "a:b/c/d" from later being re-parsed as an
 *          opaque URI with a scheme of "a" and a scheme-specific
 *          part of "b/c/d". (Deviation from RFC 2396)
 */
void URI::normalize()
{
    std::vector< std::vector<int> > segments;

    //## Collect segments
    if (path.size()<2)
        return;
    bool abs = false;
    int pos=0;
    int len = (int) path.size();

    if (path[0]=='/')
        {
        abs = true;
        pos++;
        }

    while (pos < len)
        {
        int pos2 = find(path, '/', pos);
        if (pos2 < 0)
            {
            std::vector<int> seg = substr(path, pos, path.size()-pos);
            //printf("last segment:%s\n", toStr(seg).c_str());
            segments.push_back(seg);
            break;
            }
        if (pos2>pos)
            {
            std::vector<int> seg = substr(path, pos, pos2-pos);
            //printf("segment:%s\n", toStr(seg).c_str());
            segments.push_back(seg);
            }
        pos = pos2;
        pos++;
        }

    //## Clean up (normalize) segments
    bool edited = false;
    std::vector< std::vector<int> >::iterator iter;
    for (iter=segments.begin() ; iter!=segments.end() ; )
        {
        std::vector<int> s = *iter;
        if (sequ(s,"."))
            {
            iter = segments.erase(iter);
            edited = true;
            }
        else if (sequ(s, "..") && iter != segments.begin() &&
                 !sequ(*(iter-1), ".."))
            {
            iter--; //back up, then erase two entries
            iter = segments.erase(iter);
            iter = segments.erase(iter);
            edited = true;
            }
        else
            iter++;
        }

    //## Rebuild path, if necessary
    if (edited)
        {
        path.clear();
        if (abs)
            {
            path.push_back('/');
            }
        std::vector< std::vector<int> >::iterator iter;
        for (iter=segments.begin() ; iter!=segments.end() ; iter++)
            {
            if (iter != segments.begin())
                path.push_back('/');
            std::vector<int> seg = *iter;
            for (unsigned int i = 0; i<seg.size() ; i++)
                path.push_back(seg[i]);
            }
        }

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



int URI::match(int p0, char const *key)
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

int URI::parseHex(int p0, int &result)
{
    int p = p0;
    int val = 0;

    //# Upper 4
    int ch = peek(p);
    if (ch >= '0' && ch <= '9')
        val += (ch - '0');
    else if (ch >= 'a' && ch <= 'f')
        val += (10 + ch - 'a');
    else if (ch >= 'A' && ch <= 'F')
        val += (10 + ch - 'A');
    else
        {
        error("parseHex : unexpected character : %c", ch);
        return -1;
        }
    p++;
    val <<= 4;

    //# Lower 4
    ch = peek(p);
    if (ch >= '0' && ch <= '9')
        val += (ch - '0');
    else if (ch >= 'a' && ch <= 'f')
        val += (10 + ch - 'a');
    else if (ch >= 'A' && ch <= 'F')
        val += (10 + ch - 'A');
    else
        {
        error("parseHex : unexpected character : %c", ch);
        return -1;
        }
    p++;
    result = val;
    return p;
}



int URI::parseEntity(int p0, int &result)
{
    int p = p0;
    int ch = peek(p);
    if (ch != '&')
        return p0;
    p++;
    if (!match(p, "#x"))
        {
        error("parseEntity: expected '#x'");
        return -1;
        }
    p += 2;
    int val;
    p = parseHex(p, val);
    if (p<0)
        return -1;
    ch = peek(p);
    if (ch != ';')
        {
        error("parseEntity: expected ';'");
        return -1;
        }
    p++;
    result = val;
    return p;
}

int URI::parseAsciiEntity(int p0, int &result)
{
    int p = p0;
    int ch = peek(p);
    if (ch != '%')
        return p0;
    p++;
    int val;
    p = parseHex(p, val);
    if (p<0)
        return -1;
    result = val;
    return p;
}


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
            else if (ch == '&') //IRI entity
                {
                int val;
                p2 = parseEntity(p, val);
                if (p2<p)
                    {
                    return -1;
                    }
                p = p2;
                authority.push_back((XMLCh)val);
                }
            else if (ch == '%') //ascii hex excape
                {
                int val;
                p2 = parseAsciiEntity(p, val);
                if (p2<p)
                    {
                    return -1;
                    }
                p = p2;
                authority.push_back((XMLCh)val);
                }
            else if (ch == ':')
                {
                portSpecified = true;
                p++;
                }
            else if (portSpecified)
                {
                portStr.push_back((XMLCh)ch);
                p++;
                }
            else
                {
                authority.push_back((XMLCh)ch);
                p++;
                }
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
    if (uni_is_letter(ch) && peek(p+1)==':')
        {
        absolute = true;
        path.push_back((XMLCh)'/');
        }
    else if (ch == '/')
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
        else if (ch == '&') //IRI entity
            {
            int val;
            p2 = parseEntity(p, val);
            if (p2<p)
                {
                return -1;
                }
            p = p2;
            path.push_back((XMLCh)val);
            }
        else if (ch == '%') //ascii hex excape
            {
            int val;
            p2 = parseAsciiEntity(p, val);
            if (p2<p)
                {
                return -1;
                }
            p = p2;
            path.push_back((XMLCh)val);
            }
        else
            {
            path.push_back((XMLCh)ch);
            p++;
            }
        }
    //trace("path:%s", toStr(path).c_str());
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
        fragment.push_back(ch);
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
    parsebuf = new int[str.size()];
    if (!parsebuf)
        {
        error("parse : could not allocate parsebuf");
        return false;
        }

    DOMString::const_iterator iter;
    unsigned int i=0;
    for (iter= str.begin() ; iter!=str.end() ; iter++)
        {
        int ch = *iter;
        if (ch == '\\')
            parsebuf[i++] = '/';
        else
            parsebuf[i++] = ch;
        }


    int p = parse(0);
    normalize();

    delete[] parsebuf;

    if (p < 0)
        {
        error("Syntax error");
        return false;
        }

    //printf("uri:%s\n", toString().c_str());
    //printf("parse:%s\n", toStr(path).c_str());

    return true;

}





}  //namespace dom
}  //namespace w3c
}  //namespace org
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################



