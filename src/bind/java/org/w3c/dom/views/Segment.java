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

import org.w3c.dom.Node;

/**
 *  <code>Segment</code> is used to retrieve specific items from specific 
 * segments. Segments may be nested as a match and may be repeatedly applied 
 * for traversing multiple matching segments. 
 * <p ><b>Note:</b>  Types and names of properties of segments of Visual media 
 * types 
 * <pre> Integer TopOffset Integer BottomOffset Integer LeftOffset Integer 
 * RightOffset Integer Width Integer Height Boolean Visible Boolean Selected 
 * Integer ForegroundColor Integer BackgroundColor String FontName String 
 * FontHeight String FontBaseline String FontSpace Width String FontMaximum 
 * Width </pre>
 * 
 * <p ><b>Note:</b> Segment types
 * <pre> // Display info and root (the default 
 * segment) Display // An area that objects or text lines flow in // or are 
 * anchored to Frame // A single character Character // 
 * Sequentially-appearing characters // with identical properties 
 * CharacterRun FormField {Text | Label | Button | Menu ...} Embedded Object 
 * Image </pre>
 * 
 * <p ><b>Note:</b> Possible properties of specific types:
 * <pre> (Image) String URL 
 * (Image) Boolean isLoaded (Image) Integer ScalingFactor (Button) Boolean 
 * isPressed (Frame) Boolean isScrollable </pre>
 * 
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 * @since DOM Level 3
 */
public interface Segment extends Match {
    /**
     *  The <code>criteria</code> <code>Match</code> of a <code>Segment</code>
     * , specified during creation, controls which <code>Segment</code>s 
     * will match.
     * <br>After setting this attribute, the results of any related call to 
     * <code>getNext</code> are unpredictable until the segment has been 
     * requested again by calling <code>matchFirstSegment</code>.
     */
    public Match getCriteria();
    /**
     *  The <code>criteria</code> <code>Match</code> of a <code>Segment</code>
     * , specified during creation, controls which <code>Segment</code>s 
     * will match.
     * <br>After setting this attribute, the results of any related call to 
     * <code>getNext</code> are unpredictable until the segment has been 
     * requested again by calling <code>matchFirstSegment</code>.
     */
    public void setCriteria(Match criteria);

    /**
     * The <code>order</code> string of a <code>Segment</code>, specified 
     * during creation, controls the order in which matching segments will 
     * be returned. If this attribute is not specified, the order defaults 
     * to an implementation-specific order.
     * <br>After setting this attribute, the results of any related call to 
     * <code>getNext</code> are unpredictable until the segment has been 
     * requested again by calling <code>matchFirstSegment</code>.
     */
    public String getOrder();
    /**
     * The <code>order</code> string of a <code>Segment</code>, specified 
     * during creation, controls the order in which matching segments will 
     * be returned. If this attribute is not specified, the order defaults 
     * to an implementation-specific order.
     * <br>After setting this attribute, the results of any related call to 
     * <code>getNext</code> are unpredictable until the segment has been 
     * requested again by calling <code>matchFirstSegment</code>.
     */
    public void setOrder(String order);

    /**
     * Adds a specific <code>Item</code> to the <code>Segment</code>.
     * @param add The <code>Item</code> to be added.After adding a result, 
     *   the results of any related call to <code>getNext</code> are 
     *   unpredictable until the segment has been requested again by calling 
     *   <code>matchFirstSegment</code>.
     */
    public void addItem(Item add);

    /**
     * Creates a match for a string value, which can be used to specify a 
     * criterium to find desired segments.
     * @param test The match test desired.
     * @param name The name of a string property to be compared against.
     * @param value The string value to be compared against.
     * @return The requested <code>MatchString</code>.
     */
    public MatchString createMatchString(short test, 
                                         String name, 
                                         String value);

    /**
     * Creates a match for an integral value, which can be used to specify a 
     * criterium to find desired segments.
     * @param test The match test desired.
     * @param name The name of an integer property to be compared against.
     * @param value The integer value to be compared against.
     * @return The requested <code>MatchInteger</code>.
     */
    public MatchInteger createMatchInteger(short test, 
                                           String name, 
                                           int value);

    /**
     * Creates a match for a boolean value, which can be used to specify a 
     * criterium to find desired segments.
     * @param test The match test desired.
     * @param name The name of a boolean property to be compared against.
     * @param value The boolean value to be compared against.
     * @return The requested <code>MatchBoolean</code>.
     */
    public MatchBoolean createMatchBoolean(short test, 
                                           String name, 
                                           boolean value);

    /**
     * Creates a match for a content value, which can be used to specify a 
     * criterium to find desired segments.
     * @param test The match test desired.
     * @param name The name of an integer property to be compared against.
     * @param offset The offset of the content value to be compared against.
     * @param nodeArg The Node of the content value to be compared against.
     * @return The requested <code>MatchContent</code>.
     */
    public MatchContent createMatchContent(short test, 
                                           String name, 
                                           int offset, 
                                           Node nodeArg);

    /**
     * Creates a match for an set of matches, which can be used to specify a 
     * criterium to find desired segments.
     * @param test The match test desired.
     * @return The requested <code>MatchSet</code>.
     */
    public MatchSet createMatchSet(short test);

    /**
     * Creates an item for a segment that can receive a string value.
     * @param name The name of a string property to be received.
     * @return The requested <code>StringItem</code>.
     */
    public StringItem createStringItem(String name);

    /**
     * Creates an item for a segment that can receive an integral value.
     * @param name The name of an integral property to be received.
     * @return The requested <code>IntegerItem</code>.
     */
    public IntegerItem createIntegerItem(String name);

    /**
     * Creates an item for a segment that can receive a boolean value.
     * @param name The name of a boolean property to be received.
     * @return The requested <code>BooleanItem</code>.
     */
    public BooleanItem createBooleanItem(String name);

    /**
     * Creates an item for a segment that can receive a content value.
     * @param name The name of a content property to be received.
     * @return The requested <code>ContentItem</code>.
     */
    public ContentItem createContentItem(String name);

    /**
     *  Returns a specific <code>Item</code>, of the list specified during the 
     * creation of the <code>Segment</code>, which is to be fetched during 
     * <code>Segment</code> execution, or returns null if the specified 
     * index does not correspond to a <code>Item</code>. 
     * @param index The index of the <code>Item</code> to be retrieved.
     */
    public void getItem(int index);

    /**
     * Fetches the results of the next matching <code>Segment</code>, if any.
     * @return  <code>true</code> if another match, otherwise 
     *   <code>false</code> (same value as <code>exists</code>). 
     */
    public boolean getNext();

}
