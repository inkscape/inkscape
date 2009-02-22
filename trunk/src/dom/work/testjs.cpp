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



#include "jsengine.h"
#include "lsimpl.h"

using namespace org::w3c::dom;


bool doTest1()
{

    char *filename = "test.xml";
    ls::DOMImplementationLSImpl domImpl;
    ls::LSInput input  = domImpl.createLSInput();
    ls::LSParser &parser = domImpl.createLSParser(0, "");

    DOMString buf;
    FILE *f = fopen(filename, "rb");
    if (!f)
        {
        printf("Cannot open %s for reading\n", filename);
        return false;
        }
    while (!feof(f))
        {
        int ch = fgetc(f);
        buf.push_back(ch);
        }
    fclose(f);
    input.setStringData(buf);

    printf("######## PARSE ######################################\n");
    DocumentPtr doc = parser.parse(input);

    if (!doc)
        {
        printf("parsing failed\n");
        return 0;
        }

    //### OUTPUT
    printf("######## SERIALIZE ##################################\n");
    ls::LSSerializer &serializer = domImpl.createLSSerializer();
    ls::LSOutput output = domImpl.createLSOutput();
    io::StdWriter writer;
    output.setCharacterStream(&writer);
    serializer.write(doc, output);

    printf("####### Namespace check\n");
    DOMString svgNamespace = "http://www.w3.org/2000/svg";
    NodeList list = doc->getElementsByTagNameNS(svgNamespace, "svg");
    int nodeCount = list.getLength();
    printf("Nodes:%d\n", nodeCount);
    for (int i=0; i<nodeCount ; i++)
        {
        NodePtr node = list.item(i);
        serializer.write(node, output);
        }

    return true;
}


bool doTest2()
{
    JavascriptEngine js;
    bool ret = js.evaluate("print(3 * 5);");
    if (!ret)
        return false;

    ret = js.evaluate("var s = new CDATASection();");
    if (!ret)
        return false;
        
    return true;
}




bool doTests()
{
    //if (!doTest1())
    //    return false;
    if (!doTest2())
        return false;
    return true;
}


int main(int argc, char **argv)
{
    if (!doTests())
        return 1;
    return 0;
}



