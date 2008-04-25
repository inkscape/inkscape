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
 * Copyright (C) 2005-2007 Bob Jamison
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
 *  
 * =======================================================================
 * NOTES
 * 
 * Some definitions are taken from the URI RFC:
 * http://www.ietf.org/rfc/rfc2396.txt      
 */

#include "dom.h"


namespace org
{
namespace w3c
{
namespace dom
{


/**
 *  A class that implements the W3C URI resource reference.  Although this
 *  API attempts to process URIs as closely as possible to the needs of W3,
 *  this model is not based on any official W3C spec.  
 */
class URI
{
public:

    /**
     * Code that indicates the scheme type.
     */     
    typedef enum
        {
        SCHEME_NONE =0,
        SCHEME_DATA,
        SCHEME_HTTP,
        SCHEME_HTTPS,
        SCHEME_FTP,
        SCHEME_FILE,
        SCHEME_LDAP,
        SCHEME_MAILTO,
        SCHEME_NEWS,
        SCHEME_TELNET
        } SchemeTypes;

    /**
     * Simple constructor
     */
    URI();

    /**
     * Copy constructor
     */
    URI(DOMString const &str);


    /**
     * Parsing constructor
     */
    URI(char const *str);

    /**
     * Copy constructor
     */
    URI(URI const &other);

    /**
     *  Assignment operator
     */
    URI &operator=(URI const &other);

    /**
     * Destructor
     */
    virtual ~URI();

    /**
     * Parse a string to initialize this URI.
     */
    virtual bool parse(DOMString const &str);

    /**
     * Produce a string displaying this URI's current value, in W3C format.
     */
    virtual DOMString toString() const;

    /**
     * Return the scheme (SchemeTypes above) of this URI as an enumeration
     */
    virtual int getScheme() const;

    /**
     * Return the scheme value as a string
     * From the RFC:
     * Just as there are many different methods of access to resources,
     * there are a variety of schemes for identifying such resources.  The
     * URI syntax consists of a sequence of components separated by reserved
     * characters, with the first component defining the semantics for the
     * remainder of the URI string.
     * 
     * Scheme names consist of a sequence of characters beginning with a
     * lower case letter and followed by any combination of lower case
     * letters, digits, plus ("+"), period ("."), or hyphen ("-").  For
     * resiliency, programs interpreting URI should treat upper case letters
     * as equivalent to lower case in scheme names (e.g., allow "HTTP" as
     * well as "http").
     * 
     *   scheme        = alpha *( alpha | digit | "+" | "-" | "." )
     * 
     * Relative URI references are distinguished from absolute URI in that
     * they do not begin with a scheme name.  Instead, the scheme is
     * inherited from the base URI, as described in Section 5.2.
     * 	      
     */
    virtual DOMString getSchemeStr() const;

    /**
     * From the RFC:
     * Many URI schemes include a top hierarchical element for a naming
     * authority, such that the namespace defined by the remainder of the
     * URI is governed by that authority.  This authority component is
     * typically defined by an Internet-based server or a scheme-specific
     * registry of naming authorities.
     * 
     *  authority     = server | reg_name
     * 
     * The authority component is preceded by a double slash "//" and is
     * terminated by the next slash "/", question-mark "?", or by the end of
     * the URI.  Within the authority component, the characters ";", ":",
     * "@", "?", and "/" are reserved.
     * 
     * An authority component is not required for a URI scheme to make use
     * of relative references.  A base URI without an authority component
     * implies that any relative reference will also be without an authority
     * component.
     */
    virtual DOMString getAuthority() const;

    /**
     *  Same as getAuthority, but if the port has been specified
     *  as host:port , the port will not be included
     */
    virtual DOMString getHost() const;

    /**
     * Return the port (TCPIP port for transport-type schemes)
     */
    virtual int getPort() const;

    /**
     * From the RFC:
     * The path component contains data, specific to the authority (or the
     * scheme if there is no authority component), identifying the resource
     * within the scope of that scheme and authority.
     * 
     * path          = [ abs_path | opaque_part ]
     * 
     * path_segments = segment *( "/" segment )
     * segment       = *pchar *( ";" param )
     * param         = *pchar
     * 
     * pchar         = unreserved | escaped |
     *                  ":" | "@" | "&" | "=" | "+" | "$" | ","
     * 
     * The path may consist of a sequence of path segments separated by a
     * single slash "/" character.  Within a path segment, the characters
     * "/", ";", "=", and "?" are reserved.  Each path segment may include a
     * sequence of parameters, indicated by the semicolon ";" character.
     * The parameters are not significant to the parsing of relative
     * references.
     */
    virtual DOMString getPath() const;

    /**
     * Converts the URI's internal canonical representation of the path to
     * what is meaningful on the architecture on which this method is called.     
     */
    virtual DOMString getNativePath() const;

    /**
     * An absolute URI contains the name of the scheme being used (<scheme>)
     * followed by a colon (":") and then a string (the <scheme-specific-part>)
     * whose interpretation depends on the scheme.
     */    
    virtual bool isAbsolute() const;

    /**
     * URI that do not make use of the slash "/" character for separating
     *   hierarchical components are considered opaque
     */     
    virtual bool isOpaque() const;

    /**
     * The part of the URI following a ? in the path.
     * 	     
     * From the RFC:    
     * The query component is a string of information to be interpreted by
     * the resource.
     *
     *      query         = *uric
     *
     * Within a query component, the characters ";", "/", "?", ":", "@",
     * "&", "=", "+", ",", and "$" are reserved.
     *     
     */
    virtual DOMString getQuery() const;

    /**
     * From the RFC:
     * When a URI reference is used to perform a retrieval action on the
     * identified resource, the optional fragment identifier, separated from
     * the URI by a crosshatch ("#") character, consists of additional
     * reference information to be interpreted by the user agent after the
     * retrieval action has been successfully completed.  As such, it is not
     * part of a URI, but is often used in conjunction with a URI.
     * 
     *    fragment      = *uric
     * 
     * The semantics of a fragment identifier is a property of the data
     * resulting from a retrieval action, regardless of the type of URI used
     * in the reference.  Therefore, the format and interpretation of
     * fragment identifiers is dependent on the media type [RFC2046] of the
     * retrieval result.  The character restrictions described in Section 2
     * for URI also apply to the fragment in a URI-reference.  Individual
     * media types may define additional restrictions or structure within
     * the fragment for specifying different types of "partial views" that
     * can be identified within that media type.
     * 
     * A fragment identifier is only meaningful when a URI reference is
     * intended for retrieval and the result of that retrieval is a document
     * for which the identified fragment is consistently defined.
     */
    virtual DOMString getFragment() const;

    /**
     * resolve()
     * This is by far the most useful feature of a URI.  It defines a set
     * of rules for finding one resource relative to another, so that your
     * resource search is well-defined and much easier.
     * 
     * From the RFC:
     * 	 	 	 	 	 	     
     *  The base URI is established according to the rules of Section 5.1 and
     *  parsed into the four main components as described in Section 3.  Note
     *  that only the scheme component is required to be present in the base
     *  URI; the other components may be empty or undefined.  A component is
     *  undefined if its preceding separator does not appear in the URI
     *  reference; the path component is never undefined, though it may be
     *  empty.  The base URI's query component is not used by the resolution
     *  algorithm and may be discarded.
     * 
     *  For each URI reference, the following steps are performed in order:
     * 
     *  1) The URI reference is parsed into the potential four components and
     *     fragment identifier, as described in Section 4.3.
     * 
     *  2) If the path component is empty and the scheme, authority, and
     *     query components are undefined, then it is a reference to the
     *     current document and we are done.  Otherwise, the reference URI's
     *     query and fragment components are defined as found (or not found)
     *     within the URI reference and not inherited from the base URI.
     * 
     *  3) If the scheme component is defined, indicating that the reference
     *     starts with a scheme name, then the reference is interpreted as an
     *     absolute URI and we are done.  Otherwise, the reference URI's
     *     scheme is inherited from the base URI's scheme component.
     * 
     *     Due to a loophole in prior specifications [RFC1630], some parsers
     *     allow the scheme name to be present in a relative URI if it is the
     *     same as the base URI scheme.  Unfortunately, this can conflict
     *     with the correct parsing of non-hierarchical URI.  For backwards
     *     compatibility, an implementation may work around such references
     *     by removing the scheme if it matches that of the base URI and the
     *     scheme is known to always use the <hier_part> syntax.  The parser
     *     can then continue with the steps below for the remainder of the
     *     reference components.  Validating parsers should mark such a
     *     misformed relative reference as an error.
     * 
     *  4) If the authority component is defined, then the reference is a
     *     network-path and we skip to step 7.  Otherwise, the reference
     *     URI's authority is inherited from the base URI's authority
     *     component, which will also be undefined if the URI scheme does not
     *     use an authority component.
     * 
     *  5) If the path component begins with a slash character ("/"), then
     *     the reference is an absolute-path and we skip to step 7.
     * 
     *  6) If this step is reached, then we are resolving a relative-path
     *     reference.  The relative path needs to be merged with the base
     *     URI's path.  Although there are many ways to do this, we will
     *     describe a simple method using a separate string buffer.
     * 
     *     a) All but the last segment of the base URI's path component is
     *        copied to the buffer.  In other words, any characters after the
     *        last (right-most) slash character, if any, are excluded.
     * 
     *     b) The reference's path component is appended to the buffer
     *        string.
     * 
     *     c) All occurrences of "./", where "." is a complete path segment,
     *        are removed from the buffer string.
     * 
     *     d) If the buffer string ends with "." as a complete path segment,
     *        that "." is removed.
     * 
     *     e) All occurrences of "<segment>/../", where <segment> is a
     *        complete path segment not equal to "..", are removed from the
     *        buffer string.  Removal of these path segments is performed
     *        iteratively, removing the leftmost matching pattern on each
     *        iteration, until no matching pattern remains.
     * 
     *     f) If the buffer string ends with "<segment>/..", where <segment>
     *        is a complete path segment not equal to "..", that
     *        "<segment>/.." is removed.
     * 
     *     g) If the resulting buffer string still begins with one or more
     *        complete path segments of "..", then the reference is
     *        considered to be in error.  Implementations may handle this
     *        error by retaining these components in the resolved path (i.e.,
     *        treating them as part of the final URI), by removing them from
     *        the resolved path (i.e., discarding relative levels above the
     *        root), or by avoiding traversal of the reference.
     * 
     *     h) The remaining buffer string is the reference URI's new path
     *        component.
     * 
     *  7) The resulting URI components, including any inherited from the
     *     base URI, are recombined to give the absolute form of the URI
     *     reference.  Using pseudocode, this would be
     * 
     *        result = ""
     * 
     *        if scheme is defined then
     *            append scheme to result
     *            append ":" to result
     * 
     *        if authority is defined then
     *            append "//" to result
     *            append authority to result
     * 
     *        append path to result
     * 
     *        if query is defined then
     *            append "?" to result
     *            append query to result
     * 
     *        if fragment is defined then
     *            append "#" to result
     *            append fragment to result
     * 
     *        return result
     * 
     *     Note that we must be careful to preserve the distinction between a
     *     component that is undefined, meaning that its separator was not
     *     present in the reference, and a component that is empty, meaning
     *     that the separator was present and was immediately followed by the
     *     next component separator or the end of the reference.
     * 
     *  The above algorithm is intended to provide an example by which the
     *  output of implementations can be tested -- implementation of the
     *  algorithm itself is not required.  For example, some systems may find
     *  it more efficient to implement step 6 as a pair of segment stacks
     *  being merged, rather than as a series of string pattern replacements.
     * 
     *     Note: Some WWW client applications will fail to separate the
     *     reference's query component from its path component before merging
     *     the base and reference paths in step 6 above.  This may result in
     *     a loss of information if the query component contains the strings
     *     "/../" or "/./".
     * 
     */
    virtual URI resolve(URI const &other) const;

    /**
     * "Mends" a URI by examining the path, and converting it to canonical
     *  form.  In particular, it takes patterns like "/./" and "/a/../b/../c"
     *  and simplifies them.	      
     */
    virtual void normalize();


private:

    void init();

    //assign values of other to this. used by copy constructor
    void assign(URI const &other);

    int scheme;

    DOMString schemeStr;

    std::vector<int> authority;

    bool portSpecified;

    int port;

    std::vector<int> path;

    bool absolute;

    bool opaque;

    std::vector<int> query;

    std::vector<int> fragment;

    void error(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    void trace(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    int peek(int p);

    int match(int p, char const *key);

    int parseHex(int p, int &result);

    int parseEntity(int p, int &result);

    int parseAsciiEntity(int p, int &result);

    int parseScheme(int p);

    int parseHierarchicalPart(int p0);

    int parseQuery(int p0);

    int parseFragment(int p0);

    int parse(int p);

    int *parsebuf;

    int parselen;

};



}  //namespace dom
}  //namespace w3c
}  //namespace org



#endif /* __URI_H__ */

