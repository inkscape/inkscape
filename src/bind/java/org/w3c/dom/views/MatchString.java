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
 * The <code>MatchString</code> identifies <code>Segment</code>s where a 
 * string property matches a specific value.
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 * @since DOM level 3
 */
public interface MatchString extends Match {
    /**
     * The name of a string property of each <code>Segment</code> to be 
     * compared against, which is specified during construction.
     */
    public String getName();

    /**
     * The string value to be compared against, which is specified during 
     * construction.
     */
    public String getValue();

}
