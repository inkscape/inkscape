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
 * Copyright (C) 2006-2008 Bob Jamison
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

#include "io/uristream.h"


typedef org::w3c::dom::URI URI;

static bool doURI(const char *uristr)
{
    org::w3c::dom::URI uri(uristr);
    printf("##################################\n");
    printf("String  : '%s'\n", uristr);
    printf("URI     : '%s'\n", uri.toString().c_str());
    printf("scheme  : '%s'\n", uri.getSchemeStr().c_str());
    printf("auth    : '%s'\n", uri.getAuthority().c_str());
    printf("path    : '%s'\n", uri.getPath().c_str());
    printf("query   : '%s'\n", uri.getQuery().c_str());
    printf("fragment: '%s'\n", uri.getFragment().c_str());
    printf("absolute: '%d'\n", uri.isAbsolute());
    printf("opaque  : '%d'\n", uri.isOpaque());

    return true;
}



static bool test1()
{
    printf("########################################\n");
    printf("## TEST 1\n");
    printf("########################################\n");
    const char *uristr = "http://www.mit.edu:80/index.html?test=good#hello";
    doURI(uristr);
    uristr = "http://www.mit.edu:80";
    doURI(uristr);
    uristr = "http://r&#xE9;sum&#xE9;.example.org";
    doURI(uristr);

    printf("\n\n");
    return true;
}

static bool test2()
{
    printf("########################################\n");
    printf("## TEST 2\n");
    printf("########################################\n");
    printf("############ uri.resolve() #######\n");
    URI absUri("file:/this/is/an/./absolute/path.sfx");
    printf("absUri:%s\n", absUri.getPath().c_str());
    URI relUri("../this/is/a/./relative/path.sfx");
    printf("relUri:%s\n", relUri.getPath().c_str());
    URI resUri = absUri.resolve(relUri);
    printf("resUri:%s\n", resUri.getPath().c_str());

    printf("\n\n");
    return true;
}

static bool test3()
{
    printf("########################################\n");
    printf("## TEST 3\n");
    printf("########################################\n");
    printf("############ windows-style uri.resolve() #######\n");
    URI absUri("file:C:\\this\\is\\an\\.\\absolute/path.sfx");
    printf("absUri:%s\n", absUri.getPath().c_str());
    printf("absUri:%s\n", absUri.getNativePath().c_str());
    URI relUri("..\\this\\is\\a\\.\\relative\\path.sfx");
    printf("relUri:%s\n", relUri.getPath().c_str());
    printf("relUri:%s\n", relUri.getNativePath().c_str());
    URI resUri = absUri.resolve(relUri);
    printf("resUri:%s\n", resUri.getPath().c_str());
    printf("resUri:%s\n", resUri.getNativePath().c_str());

    printf("\n\n");
    return true;
}


int main(int argc, char **argv)
{
    if (!test1())
        return 1;
    if (!test2())
        return 1;
    if (!test3())
        return 1;
    return 0;
}
