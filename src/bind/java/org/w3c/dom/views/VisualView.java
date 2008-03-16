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
 * Presents a flatter model of a visual view.
 * <p>See also the <a href='http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226'>Document Object Model (DOM) Level 3 Views and Formatting
Specification</a>.
 */
public interface VisualView {
    /**
     * A string identifying the type of fonts on the system so that font name 
     * strings may be properly interpreted.
     */
    public String getFontScheme();

    /**
     * The width, in horizontal units, of the view.
     */
    public int getWidth();

    /**
     * The height, in vertical units, of the view.
     */
    public int getHeight();

    /**
     * The number of horizontal dots per inch in the view, used to interpret 
     * horizontal values.
     */
    public int getHorizontalDPI();

    /**
     * The number of vertical dots per inch in the view, used to interpret 
     * vertical values.
     */
    public int getVerticalDPI();

    /**
     * Creates a visual character to match and return information on a single 
     * visual character of the view.
     * @return The requested <code>VisualCharacter</code>.
     */
    public VisualCharacter createVisualCharacter();

    /**
     * Creates a visual character run to match and return information on a run 
     * of similar ajdacent visual characters of the view.
     * <br>This will match the largest character run that meets the specified 
     * criteria, is not contiguously displayed on the view and has 
     * homogeneous display properties.
     * @return The requested <code>VisualCharacterRun</code>.
     */
    public VisualCharacterRun createVisualCharacterRun();

    /**
     * Creates a visual frame to match and return information on a frame of 
     * the view.
     * @return The requested <code>VisualFrame</code>.
     */
    public VisualFrame createVisualFrame();

    /**
     * Creates a visual image to match and return information on an image of 
     * the view.
     * @return The requested <code>VisualImage</code>.
     */
    public VisualImage createVisualImage();

    /**
     * Creates a visual form button to match and return information on a form 
     * button of the view.
     * @return The requested <code>VisualFormButton</code>.
     */
    public VisualFormButton createVisualFormButton();

    /**
     * Creates a visual form field to match and return information on a form 
     * field of the view.
     * @return The requested <code>VisualFormField</code>.
     */
    public VisualFormField createVisualFormField();

    /**
     * @param boundary 
     * @param offset 
     * @param extend 
     * @param add 
     */
    public void select(Node boundary, 
                       int offset, 
                       boolean extend, 
                       boolean add);

    /**
     * @param segment 
     */
    public void matchSegment(VisualResource segment);

}
