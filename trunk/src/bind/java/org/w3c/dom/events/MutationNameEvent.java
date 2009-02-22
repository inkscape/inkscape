/*
 * Copyright (c) 2003 World Wide Web Consortium,
 *
 * (Massachusetts Institute of Technology, European Research Consortium for
 * Informatics and Mathematics, Keio University). All Rights Reserved. This
 * work is distributed under the W3C(r) Software License [1] in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * [1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 */

package org.w3c.dom.events;

import org.w3c.dom.Node;

/**
 *  The <code>MutationNameEvent</code> interface provides specific contextual 
 * information associated with Mutation name event types. 
 * <p> To create an instance of the <code>MutationNameEvent</code> interface, 
 * use the <code>Document.createEvent("MutationNameEvent")</code> method 
 * call. 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 3
 */
public interface MutationNameEvent extends MutationEvent {
    /**
     *  The previous value of the <code>relatedNode</code>'s 
     * <code>namespaceURI</code>. 
     */
    public String getPrevNamespaceURI();

    /**
     *  The previous value of the <code>relatedNode</code>'s 
     * <code>nodeName</code>. 
     */
    public String getPrevNodeName();

    /**
     *  The <code>initMutationNameEvent</code> method is used to initialize 
     * the value of a <code>MutationNameEvent</code> object and has the same 
     * behavior as <code>MutationEvent.initMutationEvent()</code>. 
     * @param typeArg  Refer to the 
     *   <code>MutationEvent.initMutationEvent()</code> method for a 
     *   description of this parameter. 
     * @param canBubbleArg  Refer to the 
     *   <code>MutationEvent.initMutationEvent()</code> method for a 
     *   description of this parameter. 
     * @param cancelableArg  Refer to the 
     *   <code>MutationEvent.initMutationEvent()</code> method for a 
     *   description of this parameter. 
     * @param relatedNodeArg  Refer to the 
     *   <code>MutationEvent.initMutationEvent()</code> method for a 
     *   description of this parameter. 
     * @param prevNamespaceURIArg  Specifies 
     *   <code>MutationNameEvent.prevNamespaceURI</code>. This value may be 
     *   <code>null</code>.
     * @param prevNodeNameArg  Specifies 
     *   <code>MutationNameEvent.prevNodeName</code>. 
     * @since DOM Level 3
     */
    public void initMutationNameEvent(String typeArg, 
                                      boolean canBubbleArg, 
                                      boolean cancelableArg, 
                                      Node relatedNodeArg, 
                                      String prevNamespaceURIArg, 
                                      String prevNodeNameArg);

    /**
     *  The <code>initMutationNameEventNS</code> method is used to initialize 
     * the value of a <code>MutationNameEvent</code> object and has the same 
     * behavior as <code>MutationEvent.initMutationEventNS()</code>. 
     * @param namespaceURI  Refer to the 
     *   <code>MutationEvent.initMutationEventNS()</code> method for a 
     *   description of this parameter. 
     * @param typeArg  Refer to the 
     *   <code>MutationEvent.initMutationEventNS()</code> method for a 
     *   description of this parameter. 
     * @param canBubbleArg  Refer to the 
     *   <code>MutationEvent.initMutationEventNS()</code> method for a 
     *   description of this parameter. 
     * @param cancelableArg  Refer to the 
     *   <code>MutationEvent.initMutationEventNS()</code> method for a 
     *   description of this parameter. 
     * @param relatedNodeArg  Refer to the 
     *   <code>MutationEvent.initMutationEventNS()</code> method for a 
     *   description of this parameter. 
     * @param prevNamespaceURIArg  Refer to the 
     *   <code>MutationEvent.initMutationEvent()</code> method for a 
     *   description of this parameter. 
     * @param prevNodeNameArg  Refer to the 
     *   <code>MutationEvent.initMutationEvent()</code> method for a 
     *   description of this parameter. 
     * @since DOM Level 3
     */
    public void initMutationNameEventNS(String namespaceURI, 
                                        String typeArg, 
                                        boolean canBubbleArg, 
                                        boolean cancelableArg, 
                                        Node relatedNodeArg, 
                                        String prevNamespaceURIArg, 
                                        String prevNodeNameArg);

}
