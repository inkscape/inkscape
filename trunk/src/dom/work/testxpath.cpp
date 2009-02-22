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



#include "dom.h"
#include "lsimpl.h"
#include "xpathparser.h"

#include <stdio.h>


typedef org::w3c::dom::Node          Node;
typedef org::w3c::dom::NodePtr       NodePtr;
typedef org::w3c::dom::NodeList      NodeList;
typedef org::w3c::dom::DOMString     DOMString;
typedef org::w3c::dom::Document      Document;
typedef org::w3c::dom::DocumentPtr   DocumentPtr;
typedef org::w3c::dom::io::StdWriter StdWriter;
typedef org::w3c::dom::ls::DOMImplementationLSImpl DOMImplementationLSImpl;
typedef org::w3c::dom::ls::LSSerializer LSSerializer;
typedef org::w3c::dom::ls::LSOutput  LSOutput;
typedef org::w3c::dom::ls::LSInput   LSInput;
typedef org::w3c::dom::ls::LSParser  LSParser;
typedef org::w3c::dom::xpath::XPathParser XPathParser;



typedef struct
{
    const char *xpathStr;
    const char *desc;
    const char *xml;
} XpathTest;

XpathTest xpathTests[] =
{

{
"/AAA",
"Select the root element AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <DDD>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/AAA/CCC",
"Select all elements CCC which are children of the root element AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <DDD>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/AAA/DDD/BBB",
"Select all elements BBB which are children of DDD which are children of the root element AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <DDD>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"//BBB",
"Select all elements BBB",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <BBB/>\n"
"          <DDD>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//DDD/BBB",
"Select all elements BBB which are children of DDD",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <BBB/>\n"
"          <DDD>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/CCC/DDD/*",
"Select all elements enclosed by elements /AAA/CCC/DDD",
"     <AAA>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"                    <EEE/>\n"
"                    <FFF/>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"                    <EEE/>\n"
"                    <FFF/>\n"
"               </DDD>\n"
"          </CCC>\n"
"          <CCC>\n"
"               <BBB>\n"
"                    <BBB>\n"
"                         <BBB/>\n"
"                    </BBB>\n"
"               </BBB>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/*/*/*/BBB",
"Select all elements BBB which have 3 ancestors",
"     <AAA>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"                    <EEE/>\n"
"                    <FFF/>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"                    <EEE/>\n"
"                    <FFF/>\n"
"               </DDD>\n"
"          </CCC>\n"
"          <CCC>\n"
"               <BBB>\n"
"                    <BBB>\n"
"                         <BBB/>\n"
"                    </BBB>\n"
"               </BBB>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//*",
"Select all elements",
"     <AAA>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"                    <EEE/>\n"
"                    <FFF/>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <BBB/>\n"
"                    <BBB/>\n"
"                    <EEE/>\n"
"                    <FFF/>\n"
"               </DDD>\n"
"          </CCC>\n"
"          <CCC>\n"
"               <BBB>\n"
"                    <BBB>\n"
"                         <BBB/>\n"
"                    </BBB>\n"
"               </BBB>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/BBB[1]",
"Select the first BBB child of element AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"/AAA/BBB[last()]",
"Select the last BBB child of element AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"//@id",
"Select all attributes @id",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB id = 'b2'/>\n"
"          <BBB name = 'bbb'/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"//BBB[@id]",
"Select BBB elements which have attribute id",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB id = 'b2'/>\n"
"          <BBB name = 'bbb'/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"//BBB[@name]",
"Select BBB elements which have attribute name",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB id = 'b2'/>\n"
"          <BBB name = 'bbb'/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"//BBB[@*]",
"Select BBB elements which have any attribute",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB id = 'b2'/>\n"
"          <BBB name = 'bbb'/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"//BBB[not(@*)]",
"Select BBB elements without an attribute",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB id = 'b2'/>\n"
"          <BBB name = 'bbb'/>\n"
"          <BBB/>\n"
"     </AAA>\n"
},

{
"//BBB[@id='b1']",
"Select BBB elements which have attribute id with value b1",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB name = ' bbb '/>\n"
"          <BBB name = 'bbb'/>\n"
"     </AAA>\n"
},

{
"//BBB[@name='bbb']",
"Select BBB elements which have attribute name with value 'bbb'",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB name = ' bbb '/>\n"
"          <BBB name = 'bbb'/>\n"
"     </AAA>\n"
},

{
"//BBB[normalize-space(@name)='bbb']",
"Select BBB elements which have attribute name with value bbb, leading and trailing spaces are removed before comparison",
"     <AAA>\n"
"          <BBB id = 'b1'/>\n"
"          <BBB name = ' bbb '/>\n"
"          <BBB name = 'bbb'/>\n"
"     </AAA>\n"
},

{
"//*[count(BBB)=2]",
"Select elements which have two children BBB",
"     <AAA>\n"
"          <CCC>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </CCC>\n"
"          <DDD>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <EEE>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </EEE>\n"
"     </AAA>\n"
},

{
"//*[count(*)=2]",
"Select elements which have 2 children",
"     <AAA>\n"
"          <CCC>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </CCC>\n"
"          <DDD>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <EEE>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </EEE>\n"
"     </AAA>\n"
},

{
"//*[count(*)=3]",
"Select elements which have 3 children",
"     <AAA>\n"
"          <CCC>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </CCC>\n"
"          <DDD>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </DDD>\n"
"          <EEE>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </EEE>\n"
"     </AAA>\n"
},

{
"//*[name()='BBB']",
"Select all elements with name BBB, equivalent with //BBB",
"     <AAA>\n"
"          <BCC>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </BCC>\n"
"          <DDB>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </DDB>\n"
"          <BEC>\n"
"               <CCC/>\n"
"               <DBD/>\n"
"          </BEC>\n"
"     </AAA>\n"
},

{
"//*[starts-with(name(),'B')]",
"Select all elements name of which starts with letter B",
"     <AAA>\n"
"          <BCC>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </BCC>\n"
"          <DDB>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </DDB>\n"
"          <BEC>\n"
"               <CCC/>\n"
"               <DBD/>\n"
"          </BEC>\n"
"     </AAA>\n"
},

{
"//*[contains(name(),'C')]",
"Select all elements name of which contain letter C",
"     <AAA>\n"
"          <BCC>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </BCC>\n"
"          <DDB>\n"
"               <BBB/>\n"
"               <BBB/>\n"
"          </DDB>\n"
"          <BEC>\n"
"               <CCC/>\n"
"               <DBD/>\n"
"          </BEC>\n"
"     </AAA>\n"
},

{
"//*[string-length(name()) = 3]",
"Select elements with three-letter name",
"     <AAA>\n"
"          <Q/>\n"
"          <SSSS/>\n"
"          <BB/>\n"
"          <CCC/>\n"
"          <DDDDDDDD/>\n"
"          <EEEE/>\n"
"     </AAA>\n"
},

{
"//*[string-length(name()) < 3]",
"Select elements name of which has one or two characters",
"     <AAA>\n"
"          <Q/>\n"
"          <SSSS/>\n"
"          <BB/>\n"
"          <CCC/>\n"
"          <DDDDDDDD/>\n"
"          <EEEE/>\n"
"     </AAA>\n"
},

{
"//*[string-length(name()) > 3]",
"Select elements with name longer than three characters",
"     <AAA>\n"
"          <Q/>\n"
"          <SSSS/>\n"
"          <BB/>\n"
"          <CCC/>\n"
"          <DDDDDDDD/>\n"
"          <EEEE/>\n"
"     </AAA>\n"
},

{
"//CCC | //BBB",
"Select all elements CCC and BBB",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <DDD>\n"
"               <CCC/>\n"
"          </DDD>\n"
"          <EEE/>\n"
"     </AAA>\n"
},

{
"/AAA/EEE | //BBB",
"Select all elements BBB and elements EEE which are children of root element AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <DDD>\n"
"               <CCC/>\n"
"          </DDD>\n"
"          <EEE/>\n"
"     </AAA>\n"
},

{
"/AAA/EEE | //DDD/CCC | /AAA | //BBB",
"Number of combinations is not restricted",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <DDD>\n"
"               <CCC/>\n"
"          </DDD>\n"
"          <EEE/>\n"
"     </AAA>\n"
},

{
"/AAA",
"Equivalent of /child::AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/child::AAA",
"Equivalent of /AAA",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/AAA/BBB",
"Equivalent of /child::AAA/child::BBB",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/child::AAA/child::BBB",
"Equivalent of /AAA/BBB",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/child::AAA/BBB",
"Both possibilities can be combined",
"     <AAA>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"/descendant::*",
"Select all descendants of document root and therefore all elements",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/BBB/descendant::*",
"Select all descendants of /AAA/BBB",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//CCC/descendant::*",
"Select all elements which have CCC among its ancestors",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//CCC/descendant::DDD",
"Select elements DDD which have CCC among its ancestors",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//DDD/parent::*",
"Select all parents of DDD element",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/BBB/DDD/CCC/EEE/ancestor::*",
"Select all elements given in this absolute path",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//FFF/ancestor::*",
"Select ancestors of FFF element",
"     <AAA>\n"
"          <BBB>\n"
"               <DDD>\n"
"                    <CCC>\n"
"                         <DDD/>\n"
"                         <EEE/>\n"
"                    </CCC>\n"
"               </DDD>\n"
"          </BBB>\n"
"          <CCC>\n"
"               <DDD>\n"
"                    <EEE>\n"
"                         <DDD>\n"
"                              <FFF/>\n"
"                         </DDD>\n"
"                    </EEE>\n"
"               </DDD>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/BBB/following-sibling::*",
"The following-sibling axis contains all the following siblings of the context node.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//CCC/following-sibling::*",
"The following-sibling axis contains all the following siblings of the context node.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/XXX/preceding-sibling::*",
"The preceding-sibling axis contains all the preceding siblings of the context node.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//CCC/preceding-sibling::*",
"The preceding-sibling axis contains all the preceding siblings of the context node",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <DDD/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/XXX/following::*",
"The following axis contains all nodes in the same document as the context "
"node that are after the context node in document order, "
"excluding any descendants and excluding attribute nodes and namespace nodes.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"                    <DDD>\n"
"                         <EEE/>\n"
"                    </DDD>\n"
"               </ZZZ>\n"
"               <FFF>\n"
"                    <GGG/>\n"
"               </FFF>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//ZZZ/following::*",
"The following axis contains all nodes in the same document as the context "
"node that are after the context node in document order, "
"excluding any descendants and excluding attribute nodes and namespace nodes.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"                    <DDD>\n"
"                         <EEE/>\n"
"                    </DDD>\n"
"               </ZZZ>\n"
"               <FFF>\n"
"                    <GGG/>\n"
"               </FFF>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/XXX/preceding::*",
"The preceding axis contains all nodes in the same document as the "
"context node that are before the context node in document order, "
"excluding any ancestors and excluding attribute nodes and namespace nodes",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"               </ZZZ>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/preceding::*",
"The preceding axis contains all nodes in the same document as the "
"context node that are before the context node in document order, "
"excluding any ancestors and excluding attribute nodes and namespace nodes",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"               </ZZZ>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/XXX/descendant-or-self::*",
"The descendant-or-self axis contains the "
"context node and the descendants of the context node",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"               </ZZZ>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//CCC/descendant-or-self::*",
"The descendant-or-self axis contains the "
"context node and the descendants of the context node",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"               </ZZZ>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"/AAA/XXX/DDD/EEE/ancestor-or-self::*",
"The ancestor-or-self axis contains the context node and the "
"ancestors of the context node; thus, the ancestor-or-self axis "
"will always include the root node.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"               </ZZZ>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/ancestor-or-self::*",
"The ancestor-or-self axis contains the context node and the "
"ancestors of the context node; thus, the ancestor-or-self axis "
"will always include the root node.",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ>\n"
"                    <DDD/>\n"
"               </ZZZ>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <DDD/>\n"
"                    <CCC/>\n"
"                    <FFF/>\n"
"                    <FFF>\n"
"                         <GGG/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/ancestor::*",
"The ancestor, descendant, following, preceding and self axes partition a document",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <FFF>\n"
"                         <HHH/>\n"
"                         <GGG>\n"
"                              <JJJ>\n"
"                                   <QQQ/>\n"
"                              </JJJ>\n"
"                              <JJJ/>\n"
"                         </GGG>\n"
"                         <HHH/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/descendant::*",
"The ancestor, descendant, following, preceding and self axes partition a document",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <FFF>\n"
"                         <HHH/>\n"
"                         <GGG>\n"
"                              <JJJ>\n"
"                                   <QQQ/>\n"
"                              </JJJ>\n"
"                              <JJJ/>\n"
"                         </GGG>\n"
"                         <HHH/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/following::*",
"The ancestor, descendant, following, preceding and self axes partition a document",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <FFF>\n"
"                         <HHH/>\n"
"                         <GGG>\n"
"                              <JJJ>\n"
"                                   <QQQ/>\n"
"                              </JJJ>\n"
"                              <JJJ/>\n"
"                         </GGG>\n"
"                         <HHH/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/preceding::*",
"The ancestor, descendant, following, preceding and self axes partition a document",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <FFF>\n"
"                         <HHH/>\n"
"                         <GGG>\n"
"                              <JJJ>\n"
"                                   <QQQ/>\n"
"                              </JJJ>\n"
"                              <JJJ/>\n"
"                         </GGG>\n"
"                         <HHH/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/self::*",
"The ancestor, descendant, following, preceding and self axes partition a document",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <FFF>\n"
"                         <HHH/>\n"
"                         <GGG>\n"
"                              <JJJ>\n"
"                                   <QQQ/>\n"
"                              </JJJ>\n"
"                              <JJJ/>\n"
"                         </GGG>\n"
"                         <HHH/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//GGG/ancestor::* | //GGG/descendant::* | //GGG/following::* | //GGG/preceding::* | //GGG/self::*",
"The ancestor, descendant, following, preceding and self axes partition a document",
"     <AAA>\n"
"          <BBB>\n"
"               <CCC/>\n"
"               <ZZZ/>\n"
"          </BBB>\n"
"          <XXX>\n"
"               <DDD>\n"
"                    <EEE/>\n"
"                    <FFF>\n"
"                         <HHH/>\n"
"                         <GGG>\n"
"                              <JJJ>\n"
"                                   <QQQ/>\n"
"                              </JJJ>\n"
"                              <JJJ/>\n"
"                         </GGG>\n"
"                         <HHH/>\n"
"                    </FFF>\n"
"               </DDD>\n"
"          </XXX>\n"
"          <CCC>\n"
"               <DDD/>\n"
"          </CCC>\n"
"     </AAA>\n"
},

{
"//BBB[position() mod 2 = 0 ]",
"Select even BBB elements",
"     <AAA>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <CCC/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"//BBB[ position() = floor(last() div 2 + 0.5) or position() = ceiling(last() div 2 + 0.5) ]",
"Select middle BBB element(s)",
"     <AAA>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <CCC/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{
"//CCC[ position() = floor(last() div 2 + 0.5) or position() = ceiling(last() div 2 + 0.5) ]",
"Select middle CCC element(s)",
"     <AAA>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <BBB/>\n"
"          <CCC/>\n"
"          <CCC/>\n"
"          <CCC/>\n"
"     </AAA>\n"
},

{  //end data
NULL,
NULL,
NULL,
}

};



bool doStringTest(const char *str)
{
    XPathParser xp;
    xp.setDebug(true);

    if (!xp.parse(str))
        return false;


    return true;
}



bool doStringTests()
{
    for (XpathTest *xpt = xpathTests ; xpt->xpathStr ; xpt++)
        {
        if (!doStringTest(xpt->xpathStr))
            return false;
        }
    return true;
}

bool dumpDoc(DocumentPtr doc)
{
    DOMImplementationLSImpl domImpl;
    LSSerializer &serializer = domImpl.createLSSerializer();
    LSOutput output = domImpl.createLSOutput();
    StdWriter writer;
    output.setCharacterStream(&writer);
    serializer.write(doc, output);

    return true;
}


bool doXmlTest(XpathTest *xpt)
{
    printf("################################################################\n");

    //### READ
    DOMImplementationLSImpl domImpl;
    LSInput input = domImpl.createLSInput();
    LSParser &parser = domImpl.createLSParser(0, "");
    input.setStringData(xpt->xml);
    DocumentPtr doc = parser.parse(input);

    //### XPATH
    XPathParser xp;
    xp.setDebug(true);

    DOMString xpathStr = xpt->xpathStr;
    NodeList list = xp.evaluate(doc, xpathStr);
    for (unsigned int i=0 ; i<list.getLength() ; i++)
        {
        NodePtr n = list.item(i);
        printf("@@ node: %s\n", n->getNodeName().c_str());
        }

    //dumpDoc(doc);

    return true;
}

bool doXmlTests()
{
    for (XpathTest *xpt = xpathTests ; xpt->xpathStr ; xpt++)
        {
        if (!doXmlTest(xpt))
            return false;
        }
    return true;
}

bool doTests()
{
    /*
    if (!doStringTests())
        {
        printf("## Failed string tests\n");
        return false;
        }
    */
    if (!doXmlTests())
        {
        printf("## Failed xml tests\n");
        return false;
        }
    return true;
}



int main(int argc, char **argv)
{
    doTests();
    return 0;
}
