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
import org.w3c.dom.DOMException;

/**
 * <code>View</code> is used as the root <code>Segment</code>, as well as 
 * providing additional global functionality such as selection.
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 * @since DOM Level 3
 */
public interface View {
    /**
     * Selects a new region of the document or adds to the existing selection.
     * @param boundary The Node at which to create or extend the selection.
     * @param offset The offset within the node at which to create or extend 
     *   the selection.
     * @param extend If false, sets a selection anchor. If true, extends the 
     *   selection with respect to the most-recently-set anchor.
     * @param add If false, clears any existing selection. If true adds a new 
     *   region to existing selection regions.
     */
    public void select(Node boundary, 
                       int offset, 
                       boolean extend, 
                       boolean add);

    /**
     * Creates a segment that can be used to obtain segment items from the 
     * view.
     * @return A new segment object, that can be set up to obtain information 
     *   about the view.
     */
    public Segment createSegment();

    /**
     *  Executes a <code>Segment</code> against all nested <code>Segment</code>
     * s, fetching<code>Item</code>s associated the requested match number, 
     * if it exists. 
     * @param todo The <code>Segment</code> to match within the view.
     * @return <code>true</code> if the desired match number was found, 
     *   otherwise <code>false</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: If the segment request could not be interpreted.
     */
    public boolean matchFirstSegment(Segment todo)
                                     throws DOMException;

    /**
     * Returns the value of an integer property of the segment, used by 
     * <code>Match</code>es and <code>Item</code>s.
     * @param name The name of the integer property of the segment to be 
     *   retrieved.
     * @return The value of the named property of the <code>Segment</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the named property does not exist on the 
     *   view or is not an integer.
     */
    public int getIntegerProperty(String name)
                                  throws DOMException;

    /**
     * Returns the value of a string property of the segment, used by 
     * <code>Match</code>es and <code>Item</code>s.
     * @param name The name of the string property of the segment to be 
     *   retrieved.
     * @return The value of the named property of the <code>Segment</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the named property does not exist on the 
     *   view or is not a string.
     */
    public String getStringProperty(String name)
                                    throws DOMException;

    /**
     * Returns the value of a boolean property of the segment, used by 
     * <code>Match</code>es and <code>Item</code>s.
     * @param name The name of the boolean property of the segment to be 
     *   retrieved.
     * @return The value of the named property of the <code>Segment</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the named property does not exist on the 
     *   view or is not a boolean.
     */
    public boolean getBooleanProperty(boolean name)
                                      throws DOMException;

    /**
     * Returns the Node value of a content property of the segment, used by 
     * <code>Match</code>es and <code>Item</code>s.
     * @param name The name of the content property of the segment to be 
     *   retrieved.
     * @return The Node value of the named property of the 
     *   <code>Segment</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the named property does not exist on the 
     *   view or is not content.
     */
    public Node getContentPropertyNode(String name)
                                       throws DOMException;

    /**
     * Returns the offset value of a content property of the segment, used by 
     * <code>Match</code>es and <code>Item</code>s.
     * @param name The name of the content property of the segment to be 
     *   retrieved.
     * @return The offset value of the named property of the 
     *   <code>Segment</code>.
     * @exception DOMException
     *   NOT_SUPPORTED_ERR: Raised if the named property does not exist on the 
     *   view or is not content.
     */
    public int getContentPropertyOffset(String name)
                                        throws DOMException;

}
