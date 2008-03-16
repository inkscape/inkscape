/*
 * Copyright (c) 2004 World Wide Web Consortium,
 *
 * (Massachusetts Institute of Technology, European Research Consortium for
 * Informatics and Mathematics, Keio University). All Rights Reserved. This
 * work is distributed under the W3C(r) Software License [1] in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * [1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 */

package org.w3c.dom.views;

/**
 * The <code>Item</code> represents information to be fetched by a 
 * <code>Segment</code>.
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 * @since DOM Level 3
 */
public interface Item {
    /**
     * The <code>exists</code> boolean of a <code>Segment</code>, initially 
     * set to false during creation, is set after an attempt to fetch the 
     * values of a <code>Item</code> to indicate whether or not the required 
     * data was present. A true value indicates that it was.
     */
    public boolean getExists();

    /**
     * The name of a property of the matched <code>Segment</code> to be 
     * fetched, which is specified during construction.
     */
    public String getName();

}
