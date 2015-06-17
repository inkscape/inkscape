#ifndef SEEN_SP_TAG_USE_REFERENCE_H
#define SEEN_SP_TAG_USE_REFERENCE_H

/*
 * The reference corresponding to href of <inkscape:tagref> element.
 *
 * Copyright (C) Theodore Janeczko 2012-2014 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <glib.h>
#include "sp-object.h"
#include "sp-item.h"
#include <uri-references.h>
#include <stddef.h>
#include <sigc++/sigc++.h>

class Path;

namespace Inkscape {
namespace XML {
    class Node;
}
}


class SPTagUseReference : public Inkscape::URIReference {
public:
    SPTagUseReference(SPObject *owner) : URIReference(owner) {}

    SPItem *getObject() const {
        return static_cast<SPItem *>(URIReference::getObject());
    }

protected:
    virtual bool _acceptObject(SPObject * const obj) const;

};


class SPTagUsePath : public SPTagUseReference {
public:
    Path *originalPath;
    bool sourceDirty;

    SPObject            *owner;
    gchar               *sourceHref;
    Inkscape::XML::Node *sourceRepr;
    SPObject            *sourceObject;

    sigc::connection _delete_connection;
    sigc::connection _changed_connection;

    SPTagUsePath(SPObject* i_owner);
    ~SPTagUsePath(void);

    void link(char* to);
    void unlink(void);
    void start_listening(SPObject* to);
    void quit_listening(void);
    void refresh_source(void);

    void (*user_unlink) (SPObject *user);
};

#endif /* !SEEN_SP_USE_REFERENCE_H */

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
