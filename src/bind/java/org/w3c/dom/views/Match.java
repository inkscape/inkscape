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
 *  The <code>Match</code> identifies <code>Segment</code>s of which a 
 * <code>Segment</code> should fetch the <code>Item</code>s. 
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 * @since DOM Level 3
 */
public interface Match {
    // MatchTestGroup
    /**
     */
    public static final short IS_EQUAL                  = 0;
    /**
     */
    public static final short IS_NOT_EQUAL              = 1;
    /**
     */
    public static final short INT_PRECEDES              = 2;
    /**
     */
    public static final short INT_PRECEDES_OR_EQUALS    = 3;
    /**
     */
    public static final short INT_FOLLOWS               = 4;
    /**
     */
    public static final short INT_FOLLOWS_OR_EQUALS     = 5;
    /**
     */
    public static final short STR_STARTS_WITH           = 6;
    /**
     */
    public static final short STR_ENDS_WITH             = 7;
    /**
     */
    public static final short STR_CONTAINS              = 8;
    /**
     */
    public static final short SET_ANY                   = 9;
    /**
     */
    public static final short SET_ALL                   = 10;
    /**
     */
    public static final short SET_NOT_ANY               = 11;
    /**
     */
    public static final short SET_NOT_ALL               = 12;

    /**
     * The <code>test</code> value of a <code>Match</code>, specified during 
     * creation, controls the test to be applied.
     */
    public short getTest();

}
