/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See W3C License http://www.w3.org/Consortium/Legal/ for more
 * details.
 */

package org.w3c.dom.smil;

/**
 *  Declares layout type for the document. See the  LAYOUT element definition .
 *  
 */
public interface SMILLayoutElement extends SMILElement {
    /**
     *  The mime type of the layout langage used in this layout element.The 
     * default value of the type attribute is "text/smil-basic-layout". 
     */
    public String getType();

    /**
     *  <code>true</code> if the player can understand the mime type, 
     * <code>false</code> otherwise. 
     */
    public boolean getResolved();

}

