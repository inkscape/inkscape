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



#include "svgreader.h"
#include "lsimpl.h"

using namespace org::w3c::dom;


bool doTest(char *fileName)
{

    printf("######## PARSE ######################################\n");
    svg::SVGReader parser;
    svg::SVGDocumentPtr doc = parser.parseFile(fileName);

    if (!doc)
        {
        printf("parsing failed\n");
        return 0;
        }

    //### OUTPUT
    printf("######## SERIALIZE ##################################\n");
    ls::DOMImplementationLSImpl domImpl;
    ls::LSSerializer &serializer = domImpl.createLSSerializer();
    ls::LSOutput output = domImpl.createLSOutput();
    io::StdWriter writer;
    output.setCharacterStream(&writer);
    serializer.write(doc, output);


    return 1;
}





int main(int argc, char **argv)
{
    if (argc!=2)
       {
       printf("usage: %s xmlfile\n", argv[0]);
       return 0;
       }
    doTest(argv[1]);
    return 0;
}



