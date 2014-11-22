#ifndef SEEN_SP_TREF_REFERENCE_H
#define SEEN_SP_TREF_REFERENCE_H

/*
 * The reference corresponding to href of <tref> element.
 * 
 * This file was created based on sp-use-reference.h
 *
 * Copyright (C) 2007 Gail Banaszkiewicz
 *   Abhishek Sharma
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <cstddef>
#include <sigc++/sigc++.h>

#include "sp-item.h"
#include "uri-references.h"

#include "util/share.h"
#include "xml/node-observer.h"
#include "xml/subtree.h"

typedef unsigned int GQuark;

class SPTRefReference : public Inkscape::URIReference,
                        public Inkscape::XML::NodeObserver {
public:
    SPTRefReference(SPObject *owner) : URIReference(owner), subtreeObserved(NULL) {
        updateObserver();
    }
    
    virtual ~SPTRefReference() {
        if (subtreeObserved) {
            subtreeObserved->removeObserver(*this);
            delete subtreeObserved;
        }   
    }

    SPItem *getObject() const {
        return static_cast<SPItem *>(URIReference::getObject());
    }
   
    void updateObserver();
    
    /////////////////////////////////////////////////////////////////////
    // Node Observer Functions
    // -----------------------
    virtual void notifyChildAdded(Inkscape::XML::Node &node, Inkscape::XML::Node &child, Inkscape::XML::Node *prev);
    virtual void notifyChildRemoved(Inkscape::XML::Node &node, Inkscape::XML::Node &child, Inkscape::XML::Node *prev);
    virtual void notifyChildOrderChanged(Inkscape::XML::Node &node, Inkscape::XML::Node &child,
                                         Inkscape::XML::Node *old_prev, Inkscape::XML::Node *new_prev);
    virtual void notifyContentChanged(Inkscape::XML::Node &node,
                                      Inkscape::Util::ptr_shared<char> old_content,
                                      Inkscape::Util::ptr_shared<char> new_content);
    virtual void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name,
                                        Inkscape::Util::ptr_shared<char> old_value,
                                        Inkscape::Util::ptr_shared<char> new_value);
    /////////////////////////////////////////////////////////////////////

protected:
    virtual bool _acceptObject(SPObject * obj) const; 
    
    Inkscape::XML::Subtree *subtreeObserved; 
};

#endif /* !SEEN_SP_TREF_REFERENCE_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
