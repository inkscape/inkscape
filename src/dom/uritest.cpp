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

#include "uristream.h"




static bool doURI(char *uristr)
{
    org::w3c::dom::URI uri(uristr);

    printf("URI     : '%s'\n", uri.toString().c_str());
    printf("scheme  : '%s'\n", uri.getSchemeStr().c_str());
    printf("auth    : '%s'\n", uri.getAuthority().c_str());
    printf("path    : '%s'\n", uri.getPath().c_str());
    printf("query   : '%s'\n", uri.getQuery().c_str());
    printf("fragment: '%s'\n", uri.getFragment().c_str());
    printf("absolute: '%d'\n", uri.getIsAbsolute());





    return true;
}

static bool doTest()
{
    char *uristr = "http://www.mit.edu:80/index.html?test=good#hello";
    doURI(uristr);
    uristr = "http://www.mit.edu:80";
    doURI(uristr);






    return true;
}



int main(int argc, char **argv)
{
    if (!doTest())
        return 1;
    return 0;
}
