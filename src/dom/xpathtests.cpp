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



typedef struct
{
    char *xpathStr;
    char *desc;
    char *xml;
} XpathTest;

XpathTest xpathTests[] =
{

{
"/AAA",
"Select the root element AAA",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <BBB/>"
"          <BBB/>"
"          <DDD>"
"               <BBB/>"
"          </DDD>"
"          <CCC/>"
"     </AAA>"
},

{
"/AAA/CCC",
"Select all elements CCC which are children of the root element AAA",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <BBB/>"
"          <BBB/>"
"          <DDD>"
"               <BBB/>"
"          </DDD>"
"          <CCC/>"
"     </AAA>"
},

{
"/AAA/DDD/BBB",
"Select all elements BBB which are children of DDD which are children of the root element AAA",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <BBB/>"
"          <BBB/>"
"          <DDD>"
"               <BBB/>"
"          </DDD>"
"          <CCC/>"
"     </AAA>"
"//BBB",
},

{
"Select all elements BBB",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <BBB/>"
"          <DDD>"
"               <BBB/>"
"          </DDD>"
"          <CCC>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"//DDD/BBB",
"Select all elements BBB which are children of DDD",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <BBB/>"
"          <DDD>"
"               <BBB/>"
"          </DDD>"
"          <CCC>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/CCC/DDD/*",
"Select all elements enclosed by elements /AAA/CCC/DDD",
"     <AAA>"
"          <XXX>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"                    <EEE/>"
"                    <FFF/>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"                    <EEE/>"
"                    <FFF/>"
"               </DDD>"
"          </CCC>"
"          <CCC>"
"               <BBB>"
"                    <BBB>"
"                         <BBB/>"
"                    </BBB>"
"               </BBB>"
"          </CCC>"
"     </AAA>"
},

{
"/*/*/*/BBB",
"Select all elements BBB which have 3 ancestors",
"     <AAA>"
"          <XXX>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"                    <EEE/>"
"                    <FFF/>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"                    <EEE/>"
"                    <FFF/>"
"               </DDD>"
"          </CCC>"
"          <CCC>"
"               <BBB>"
"                    <BBB>"
"                         <BBB/>"
"                    </BBB>"
"               </BBB>"
"          </CCC>"
"     </AAA>"
},

{
"//*",
"Select all elements",
"     <AAA>"
"          <XXX>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"                    <EEE/>"
"                    <FFF/>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD>"
"                    <BBB/>"
"                    <BBB/>"
"                    <EEE/>"
"                    <FFF/>"
"               </DDD>"
"          </CCC>"
"          <CCC>"
"               <BBB>"
"                    <BBB>"
"                         <BBB/>"
"                    </BBB>"
"               </BBB>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/BBB[1]",
"Select the first BBB child of element AAA",
"     <AAA>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"     </AAA>"
},

{
"/AAA/BBB[last()]",
"Select the last BBB child of element AAA",
"     <AAA>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"     </AAA>"
},

{
"//@id",
"Select all attributes @id",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB id = 'b2'/>"
"          <BBB name = 'bbb'/>"
"          <BBB/>"
"     </AAA>"
},

{
"//BBB[@id]",
"Select BBB elements which have attribute id",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB id = 'b2'/>"
"          <BBB name = 'bbb'/>"
"          <BBB/>"
"     </AAA>"
},

{
"//BBB[@name]",
"Select BBB elements which have attribute name",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB id = 'b2'/>"
"          <BBB name = 'bbb'/>"
"          <BBB/>"
"     </AAA>"
},

{
"//BBB[@*]",
"Select BBB elements which have any attribute",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB id = 'b2'/>"
"          <BBB name = 'bbb'/>"
"          <BBB/>"
"     </AAA>"
},

{
"//BBB[not(@*)]",
"Select BBB elements without an attribute",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB id = 'b2'/>"
"          <BBB name = 'bbb'/>"
"          <BBB/>"
"     </AAA>"
},

{
"//BBB[@id='b1']",
"Select BBB elements which have attribute id with value b1",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB name = ' bbb '/>"
"          <BBB name = 'bbb'/>"
"     </AAA>"
},

{
"//BBB[@name='bbb']",
"Select BBB elements which have attribute name with value 'bbb'",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB name = ' bbb '/>"
"          <BBB name = 'bbb'/>"
"     </AAA>"
},

{
"//BBB[normalize-space(@name)='bbb']",
"Select BBB elements which have attribute name with value bbb, leading and trailing spaces are removed before comparison",
"     <AAA>"
"          <BBB id = 'b1'/>"
"          <BBB name = ' bbb '/>"
"          <BBB name = 'bbb'/>"
"     </AAA>"
},

{
"//*[count(BBB)=2]",
"Select elements which have two children BBB",
"     <AAA>"
"          <CCC>"
"               <BBB/>"
"               <BBB/>"
"               <BBB/>"
"          </CCC>"
"          <DDD>"
"               <BBB/>"
"               <BBB/>"
"          </DDD>"
"          <EEE>"
"               <CCC/>"
"               <DDD/>"
"          </EEE>"
"     </AAA>"
},

{
"//*[count(*)=2]",
"Select elements which have 2 children",
"     <AAA>"
"          <CCC>"
"               <BBB/>"
"               <BBB/>"
"               <BBB/>"
"          </CCC>"
"          <DDD>"
"               <BBB/>"
"               <BBB/>"
"          </DDD>"
"          <EEE>"
"               <CCC/>"
"               <DDD/>"
"          </EEE>"
"     </AAA>"
},

{
"//*[count(*)=3]",
"Select elements which have 3 children",
"     <AAA>"
"          <CCC>"
"               <BBB/>"
"               <BBB/>"
"               <BBB/>"
"          </CCC>"
"          <DDD>"
"               <BBB/>"
"               <BBB/>"
"          </DDD>"
"          <EEE>"
"               <CCC/>"
"               <DDD/>"
"          </EEE>"
"     </AAA>"
},

{
"//*[name()='BBB']",
"Select all elements with name BBB, equivalent with //BBB",
"     <AAA>"
"          <BCC>"
"               <BBB/>"
"               <BBB/>"
"               <BBB/>"
"          </BCC>"
"          <DDB>"
"               <BBB/>"
"               <BBB/>"
"          </DDB>"
"          <BEC>"
"               <CCC/>"
"               <DBD/>"
"          </BEC>"
"     </AAA>"
},

{
"//*[starts-with(name(),'B')]",
"Select all elements name of which starts with letter B",
"     <AAA>"
"          <BCC>"
"               <BBB/>"
"               <BBB/>"
"               <BBB/>"
"          </BCC>"
"          <DDB>"
"               <BBB/>"
"               <BBB/>"
"          </DDB>"
"          <BEC>"
"               <CCC/>"
"               <DBD/>"
"          </BEC>"
"     </AAA>"
},

{
"//*[contains(name(),'C')]",
"Select all elements name of which contain letter C",
"     <AAA>"
"          <BCC>"
"               <BBB/>"
"               <BBB/>"
"               <BBB/>"
"          </BCC>"
"          <DDB>"
"               <BBB/>"
"               <BBB/>"
"          </DDB>"
"          <BEC>"
"               <CCC/>"
"               <DBD/>"
"          </BEC>"
"     </AAA>"
},

{
"//*[string-length(name()) = 3]",
"Select elements with three-letter name",
"     <AAA>"
"          <Q/>"
"          <SSSS/>"
"          <BB/>"
"          <CCC/>"
"          <DDDDDDDD/>"
"          <EEEE/>"
"     </AAA>"
},

{
"//*[string-length(name()) < 3]",
"Select elements name of which has one or two characters",
"     <AAA>"
"          <Q/>"
"          <SSSS/>"
"          <BB/>"
"          <CCC/>"
"          <DDDDDDDD/>"
"          <EEEE/>"
"     </AAA>"
},

{
"//*[string-length(name()) > 3]",
"Select elements with name longer than three characters",
"     <AAA>"
"          <Q/>"
"          <SSSS/>"
"          <BB/>"
"          <CCC/>"
"          <DDDDDDDD/>"
"          <EEEE/>"
"     </AAA>"
},

{
"//CCC | //BBB",
"Select all elements CCC and BBB",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <DDD>"
"               <CCC/>"
"          </DDD>"
"          <EEE/>"
"     </AAA>"
},

{
"/AAA/EEE | //BBB",
"Select all elements BBB and elements EEE which are children of root element AAA",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <DDD>"
"               <CCC/>"
"          </DDD>"
"          <EEE/>"
"     </AAA>"
},

{
"/AAA/EEE | //DDD/CCC | /AAA | //BBB",
"Number of combinations is not restricted",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"          <DDD>"
"               <CCC/>"
"          </DDD>"
"          <EEE/>"
"     </AAA>"
},

{
"/AAA",
"Equivalent of /child::AAA",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"     </AAA>"
},

{
"/child::AAA",
"Equivalent of /AAA",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"     </AAA>"
},

{
"/AAA/BBB",
"Equivalent of /child::AAA/child::BBB",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"     </AAA>"
},

{
"/child::AAA/child::BBB",
"Equivalent of /AAA/BBB",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"     </AAA>"
},

{
"/child::AAA/BBB",
"Both possibilities can be combined",
"     <AAA>"
"          <BBB/>"
"          <CCC/>"
"     </AAA>"
},

{
"/descendant::*",
"Select all descendants of document root and therefore all elements",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/BBB/descendant::*",
"Select all descendants of /AAA/BBB",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"//CCC/descendant::*",
"Select all elements which have CCC among its ancestors",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"//CCC/descendant::DDD",
"Select elements DDD which have CCC among its ancestors",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"//DDD/parent::*",
"Select all parents of DDD element",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/BBB/DDD/CCC/EEE/ancestor::*",
"Select all elements given in this absolute path",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"//FFF/ancestor::*",
"Select ancestors of FFF element",
"     <AAA>"
"          <BBB>"
"               <DDD>"
"                    <CCC>"
"                         <DDD/>"
"                         <EEE/>"
"                    </CCC>"
"               </DDD>"
"          </BBB>"
"          <CCC>"
"               <DDD>"
"                    <EEE>"
"                         <DDD>"
"                              <FFF/>"
"                         </DDD>"
"                    </EEE>"
"               </DDD>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/BBB/following-sibling::*",
"The following-sibling axis contains all the following siblings of the context node.",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <DDD/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//CCC/following-sibling::*",
"The following-sibling axis contains all the following siblings of the context node.",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <DDD/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/XXX/preceding-sibling::*",
"The preceding-sibling axis contains all the preceding siblings of the context node.",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <DDD/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//CCC/preceding-sibling::*",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <DDD/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/XXX/following::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"                    <DDD>"
"                         <EEE/>"
"                    </DDD>"
"               </ZZZ>"
"               <FFF>"
"                    <GGG/>"
"               </FFF>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//ZZZ/following::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"                    <DDD>"
"                         <EEE/>"
"                    </DDD>"
"               </ZZZ>"
"               <FFF>"
"                    <GGG/>"
"               </FFF>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/XXX/preceding::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"               </ZZZ>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/preceding::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"               </ZZZ>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/XXX/descendant-or-self::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"               </ZZZ>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//CCC/descendant-or-self::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"               </ZZZ>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"/AAA/XXX/DDD/EEE/ancestor-or-self::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"               </ZZZ>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/ancestor-or-self::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ>"
"                    <DDD/>"
"               </ZZZ>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <DDD/>"
"                    <CCC/>"
"                    <FFF/>"
"                    <FFF>"
"                         <GGG/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/ancestor::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <FFF>"
"                         <HHH/>"
"                         <GGG>"
"                              <JJJ>"
"                                   <QQQ/>"
"                              </JJJ>"
"                              <JJJ/>"
"                         </GGG>"
"                         <HHH/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/descendant::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <FFF>"
"                         <HHH/>"
"                         <GGG>"
"                              <JJJ>"
"                                   <QQQ/>"
"                              </JJJ>"
"                              <JJJ/>"
"                         </GGG>"
"                         <HHH/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/following::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <FFF>"
"                         <HHH/>"
"                         <GGG>"
"                              <JJJ>"
"                                   <QQQ/>"
"                              </JJJ>"
"                              <JJJ/>"
"                         </GGG>"
"                         <HHH/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/preceding::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <FFF>"
"                         <HHH/>"
"                         <GGG>"
"                              <JJJ>"
"                                   <QQQ/>"
"                              </JJJ>"
"                              <JJJ/>"
"                         </GGG>"
"                         <HHH/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/self::*",
"Description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <FFF>"
"                         <HHH/>"
"                         <GGG>"
"                              <JJJ>"
"                                   <QQQ/>"
"                              </JJJ>"
"                              <JJJ/>"
"                         </GGG>"
"                         <HHH/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//GGG/ancestor::* | //GGG/descendant::* | //GGG/following::* | //GGG/preceding::* | //GGG/self::*",
"description",
"     <AAA>"
"          <BBB>"
"               <CCC/>"
"               <ZZZ/>"
"          </BBB>"
"          <XXX>"
"               <DDD>"
"                    <EEE/>"
"                    <FFF>"
"                         <HHH/>"
"                         <GGG>"
"                              <JJJ>"
"                                   <QQQ/>"
"                              </JJJ>"
"                              <JJJ/>"
"                         </GGG>"
"                         <HHH/>"
"                    </FFF>"
"               </DDD>"
"          </XXX>"
"          <CCC>"
"               <DDD/>"
"          </CCC>"
"     </AAA>"
},

{
"//BBB[position() mod 2 = 0 ]",
"Select even BBB elements",
"     <AAA>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <CCC/>"
"          <CCC/>"
"          <CCC/>"
"     </AAA>"
},

{
"//BBB[ position() = floor(last() div 2 + 0.5) or position() = ceiling(last() div 2 + 0.5) ]",
"Select middle BBB element(s)",
"     <AAA>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <CCC/>"
"          <CCC/>"
"          <CCC/>"
"     </AAA>"
},

{
"//CCC[ position() = floor(last() div 2 + 0.5) or position() = ceiling(last() div 2 + 0.5) ]",
"Select middle CCC element(s)",
"     <AAA>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <BBB/>"
"          <CCC/>"
"          <CCC/>"
"          <CCC/>"
"     </AAA>"
}

}; //end


