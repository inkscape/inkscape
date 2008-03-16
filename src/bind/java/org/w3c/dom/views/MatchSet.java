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
 * The <code>MatchSet</code> identifies <code>Segment</code>s where a set of 
 * matches evaluate in a specified way.
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 * @since DOM level 3
 */
public interface MatchSet extends Match {
    /**
     * The Node value to be compared against, which is specified during 
     * construction.
     */
    public Node getNodeArg();

    /**
     * Adds a specific <code>Match</code> to the set.
     * @param add The <code>Match</code> to be added.After adding a match, 
     *   the results of any related call to <code>getNext</code> are 
     *   unpredictable until the segment has been requested again by calling 
     *   <code>matchFirstSegment</code>.
     */
    public void addMatch(Match add);

    /**
     * Returns a specific <code>Match</code>, of the set, which is to be 
     * matched during <code>MatchSet</code> evaluation, or returns null if 
     * the specified index does not correspond to a <code>Match</code>.
     * @param index The index of the <code>Match</code> to be retrieved.
     * @return The requested match, if any, or null.
     */
    public Match getMatch(int index);

}
