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
 * Visual segments contain match criteria attributes and result attributes 
 * common to visual views of a document. When this structure is created, all 
 * booleans are set to false, all integral values are set to 0, and all 
 * strings and object references are set to null. Match criteria are then 
 * set. After setting match criteria, <code>matchSegment</code> is called 
 * passing this segment or another segment that references this segment, 
 * which finds a matching segment and sets result attributes.
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 */
public interface VisualSegment extends VisualResource {
    /**
     * May be set to cause the corresponding segment to be matched only if it 
     * contains the specified <code>matchX</code> and <code>matchY</code> 
     * positions.
     */
    public boolean getMatchPosition();
    /**
     * May be set to cause the corresponding segment to be matched only if it 
     * contains the specified <code>matchX</code> and <code>matchY</code> 
     * positions.
     */
    public void setMatchPosition(boolean matchPosition);

    /**
     * May be set to cause the corresponding segment to be matched only if it 
     * is inside the specified rectangular region bounded by 
     * <code>matchX</code>, <code>matchY</code>, <code>matchXR</code>, and 
     * <code>matchYR</code>.
     */
    public boolean getMatchInside();
    /**
     * May be set to cause the corresponding segment to be matched only if it 
     * is inside the specified rectangular region bounded by 
     * <code>matchX</code>, <code>matchY</code>, <code>matchXR</code>, and 
     * <code>matchYR</code>.
     */
    public void setMatchInside(boolean matchInside);

    /**
     * May be set to cause the corresponding segment to be matched only if it 
     * contains the specified rectangular region bounded by 
     * <code>matchX</code>, <code>matchY</code>, <code>matchXR</code>, and 
     * <code>matchYR</code>.
     */
    public boolean getMatchContaining();
    /**
     * May be set to cause the corresponding segment to be matched only if it 
     * contains the specified rectangular region bounded by 
     * <code>matchX</code>, <code>matchY</code>, <code>matchXR</code>, and 
     * <code>matchYR</code>.
     */
    public void setMatchContaining(boolean matchContaining);

    /**
     * An integral X coordinate, specified in horizontal view units, that may 
     * be used to match a point or region.
     */
    public int getMatchX();
    /**
     * An integral X coordinate, specified in horizontal view units, that may 
     * be used to match a point or region.
     */
    public void setMatchX(int matchX);

    /**
     * An integral Y coordinate, specified in vertical view units, that may be 
     * used to match a point or region.
     */
    public int getMatchY();
    /**
     * An integral Y coordinate, specified in vertical view units, that may be 
     * used to match a point or region.
     */
    public void setMatchY(int matchY);

    /**
     * An integral X coordinate, specified in horizontal view units, that may 
     * be used to match a region.
     */
    public int getMatchXR();
    /**
     * An integral X coordinate, specified in horizontal view units, that may 
     * be used to match a region.
     */
    public void setMatchXR(int matchXR);

    /**
     * An integral Y coordinate, specified in vertical view units, that may be 
     * used to match a region.
     */
    public int getMatchYR();
    /**
     * An integral Y coordinate, specified in vertical view units, that may be 
     * used to match a region.
     */
    public void setMatchYR(int matchYR);

    /**
     * May be set to cause the corresponding segment to only be matched if it 
     * presents the <code>matchNode</code> content, offset by 
     * <code>matchOffset</code>.
     */
    public boolean getMatchContent();
    /**
     * May be set to cause the corresponding segment to only be matched if it 
     * presents the <code>matchNode</code> content, offset by 
     * <code>matchOffset</code>.
     */
    public void setMatchContent(boolean matchContent);

    /**
     * May be set to cause the corresponding segment to only be matched if the 
     * content it presents is within the range of content between Node 
     * <code>matchNode</code> offset <code>matchOffset</code> and Node 
     * <code>matchNodeR</code> offset <code>matchOffsetR</code>.
     */
    public boolean getMatchRange();
    /**
     * May be set to cause the corresponding segment to only be matched if the 
     * content it presents is within the range of content between Node 
     * <code>matchNode</code> offset <code>matchOffset</code> and Node 
     * <code>matchNodeR</code> offset <code>matchOffsetR</code>.
     */
    public void setMatchRange(boolean matchRange);

    /**
     * The node, or first node in a range to use to match segments which 
     * present specified content.
     * <br>If matching content is enabled, but this is set to null, then only 
     * segments that are not associated with content will be matched.
     */
    public Node getMatchNode();
    /**
     * The node, or first node in a range to use to match segments which 
     * present specified content.
     * <br>If matching content is enabled, but this is set to null, then only 
     * segments that are not associated with content will be matched.
     */
    public void setMatchNode(Node matchNode);

    /**
     * The offset, or first offset in a range to use to match segments which 
     * present specified content.
     */
    public int getMatchOffset();
    /**
     * The offset, or first offset in a range to use to match segments which 
     * present specified content.
     */
    public void setMatchOffset(int matchOffset);

    /**
     * The second node in a range to use to match segments which present 
     * specified content.
     * <br>If matching a content range is enabled, but this is set to null, 
     * then only segments that are not associated with content will be 
     * matched.
     */
    public Node getMatchNodeR();
    /**
     * The second node in a range to use to match segments which present 
     * specified content.
     * <br>If matching a content range is enabled, but this is set to null, 
     * then only segments that are not associated with content will be 
     * matched.
     */
    public void setMatchNodeR(Node matchNodeR);

    /**
     * The offset, or first offset in a range to use to match segments which 
     * present specified content.
     */
    public int getMatchOffsetR();
    /**
     * The offset, or first offset in a range to use to match segments which 
     * present specified content.
     */
    public void setMatchOffsetR(int matchOffsetR);

    /**
     * May be set to cause the corresponding segment to only be matched if the 
     * content being presented contains a cursor or part of a selected 
     * region.
     */
    public boolean getMatchContainsSelected();
    /**
     * May be set to cause the corresponding segment to only be matched if the 
     * content being presented contains a cursor or part of a selected 
     * region.
     */
    public void setMatchContainsSelected(boolean matchContainsSelected);

    /**
     * May be set to cause the corresponding segment to only be matched if the 
     * segment being presented contains some part that is visible.
     */
    public boolean getMatchContainsVisible();
    /**
     * May be set to cause the corresponding segment to only be matched if the 
     * segment being presented contains some part that is visible.
     */
    public void setMatchContainsVisible(boolean matchContainsVisible);

    /**
     * Returns true result if the desired segment was located, or false if it 
     * was not. If this value is set to false, no other results are set. If 
     * this value is set to true, all other results are set.
     */
    public boolean getExists();

    /**
     * Whenever a segment is matched, this is set to the first node presented 
     * by the matched segment or null if the segment does not present any 
     * specific document content.
     */
    public Node getStartNode();

    /**
     * Whenever a segment is matched, this is set to the first offset 
     * presented within the first node presented by the matched segment or 0 
     * if the segment does not present any specific document content.
     */
    public int getStartOffset();

    /**
     * Whenever a segment is matched, this is set to the last node presented 
     * by the matched segment or null if the segment does not present any 
     * specific document content.
     */
    public Node getEndNode();

    /**
     * Whenever a segment is matched, this is set to first offset not 
     * presented within the last node presented by the matched segment or 0 
     * if the segment does not present any specific document content.
     */
    public int getEndOffset();

    /**
     * Whenever a segment is matched, this is set to the top offset of the 
     * segment within the view, specified in vertical view units.
     */
    public int getTopOffset();

    /**
     * Whenever a segment is matched, this is set to the bottom offset of the 
     * segment within the view, specified in vertical view units.
     */
    public int getBottomOffset();

    /**
     * Whenever a segment is matched, this is set to the left offset of the 
     * segment within the view, specified in horizontal view units.
     */
    public int getLeftOffset();

    /**
     * Whenever a segment is matched, this is set to the right offset of the 
     * segment within the view, specified in horizontal view units.
     */
    public int getRightOffset();

    /**
     * Whenever a segment is matched, this is set to the width of the segment 
     * within the view, specified in horizontal view units.
     */
    public int getWidth();

    /**
     * Whenever a segment is matched, this is set to the width of the segment 
     * within the view, specified in vertical view units.
     */
    public int getHeight();

    /**
     * Whenever a segment is matched, this is set to true if the segment 
     * presents the content with the cursor or selected content, otherwise, 
     * this is set to false.
     */
    public boolean getSelected();

    /**
     * Whenever a segment is matched, this is set to true if the segment 
     * contains some part that is visible, otherwise, this is set to false.
     */
    public boolean getVisible();

    /**
     * Whenever a segment is matched, this is set to the integral value of the 
     * foreground color of that segment, or transparent if there is no 
     * foreground color. The 32 bits of this value are divided into the 
     * following 8-bit sub-fields, from most significant to least 
     * significant: alpha, red, green, blue. The color fields range from 0 
     * for no intensity to 255 to indicate the contribution of each color. 
     * The alpha field ranges from 0 for transparent to 255 for completely 
     * opaque. For complete transparency, the color fields will be 
     * normalized to 0 as well.
     */
    public int getForegroundColor();

    /**
     * Whenever a segment is matched, this is set to the integral value of the 
     * background color of that segment, or transparent if there is no 
     * background color. The 32 bits of this value are divided into the 
     * following 8-bit sub-fields, from most significant to least 
     * significant: alpha, red, green, blue. The color fields range from 0 
     * for no intensity to 255 to indicate the contribution of each color. 
     * The alpha field ranges from 0 for transparent to 255 for completely 
     * opaque. For a transparent alpha value of 0, the color fields are be 
     * normalized to 0 as well.
     */
    public int getBackgroundColor();

    /**
     * The font name is a view-specific designation of the font name.
     */
    public String getFontName();

    /**
     */
    public String getFontHeight();

    /**
     * Fetches the results of the next matching <code>VisualResource</code>, 
     * if any.
     * @return 
     */
    public boolean getNext();

}
