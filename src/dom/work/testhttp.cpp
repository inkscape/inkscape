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



#include "io/uristream.h"

using namespace org::w3c::dom;


bool doTest1(char *filename)
{
    return true;
}

bool doTest2()
{
    io::UriInputStream instream("http://www.mit.edu");

    DOMString res;

    while (true)
        {
        int ch = instream.get();
        if (ch<0)
            break;
        res.push_back(ch);
        }

    instream.close();

    printf("result: %s\n", res.c_str());

    return true;
}





int main(int argc, char **argv)
{
    /*
    if (argc!=2)
       {
       printf("usage: %s xmlfile\n", argv[0]);
       return 0;
       }
    doTest1(argv[1]);
    */
    doTest2();
    return 0;
}



