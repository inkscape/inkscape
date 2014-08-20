/** @file
 * @brief Interface for XML documents
 */
/* Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SP_REPR_DOC_H
#define SEEN_INKSCAPE_XML_SP_REPR_DOC_H

#include "xml/node.h"

namespace Inkscape {
namespace XML {

/**
 * @brief Interface for XML documents
 *
 * This class represents a complete document tree. You have to go through this class
 * to create new nodes. It also contains transaction support, which forms the base
 * of the undo system.
 *
 * The document is also a node. It usually contains only two child nodes - a processing
 * instruction node (PINode) containing the XML prolog, and the root node. You can get
 * the root node of the document by calling the root() method.
 *
 * The name "transaction" can be misleading, because they are not atomic. Their main feature
 * is that they provide rollback. After starting a transaction,
 * all changes made to the document are stored in an internal event log. At any time
 * after starting the transaction, you can call the rollback() method, which restores
 * the document to the state it was before starting the transaction. Calling the commit()
 * method causes the internal event log to be discarded, and you can estabilish a new
 * "restore point" by calling beginTransaction() again. There can be only one active
 * transaction at a time for a given document.
 */
struct Document : virtual public Node {
public:
    /**
     * @name Document transactions
     * @{
     */
    /**
     * @brief Checks whether there is an active transaction for this document
     * @return true if there's an established transaction for this document, false otherwise
     */
    virtual bool inTransaction()=0;
    /**
     * @brief Begin a transaction and start recording changes
     *
     * By calling this method you effectively establish a resotre point.
     * You can undo all changes made to the document after this call using rollback().
     */
    virtual void beginTransaction()=0;
    /**
     * @brief Restore the state of the document prior to the transaction
     *
     * This method applies the inverses of all recorded changes in reverse order,
     * restoring the document state from before the transaction. For some implementations,
     * this function may do nothing.
     */
    virtual void rollback()=0;
    /**
     * @brief Commit a transaction and discard change data
     *
     * This method finishes the active transaction and discards the recorded changes.
     */
    virtual void commit()=0;
    /**
     * @brief Commit a transaction and store the events for later use
     *
     * This method finishes a transaction and returns an event chain
     * that describes the changes made to the document. This method may return NULL,
     * which means that the document implementation doesn't support event logging,
     * or that no changes were made.
     *
     * @return Event chain describing the changes, or NULL
     */
    virtual Event *commitUndoable()=0;
    /*@}*/

    /**
     * @name Create new nodes
     * @{
     */
    virtual Node *createElement(char const *name)=0;
    virtual Node *createTextNode(char const *content)=0;
    virtual Node *createTextNode(char const *content, bool is_CData)=0;
    virtual Node *createComment(char const *content)=0;
    virtual Node *createPI(char const *target, char const *content)=0;
    /*@}*/

    /**
     * @brief Get the event logger for this document
     *
     * This is an implementation detail that should not be used outside of node implementations.
     * It should be made non-public in the future.
     */
    virtual NodeObserver *logger()=0;
};

}
}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
