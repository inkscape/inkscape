#ifndef SEEN_SP_USE_REFERENCE_H
#define SEEN_SP_USE_REFERENCE_H

/*
 * The reference corresponding to href of <use> element.
 *
 * Copyright (C) 2004 Bulia Byak
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <forward.h>
#include <uri-references.h>
#include <sigc++/sigc++.h>

class Path;

namespace Inkscape {
namespace XML {
    struct Node;
}
}


class SPUseReference : public Inkscape::URIReference {
public:
    SPUseReference(SPObject *owner) : URIReference(owner) {}

    SPItem *getObject() const {
        return (SPItem *)URIReference::getObject();
    }

protected:
    virtual bool _acceptObject(SPObject * const obj) const;

};


class SPUsePath : public SPUseReference {
public:
    Path *originalPath;
    bool sourceDirty;

    SPObject            *owner;
    gchar               *sourceHref;
    Inkscape::XML::Node *sourceRepr;
    SPObject            *sourceObject;

    sigc::connection _modified_connection;
    sigc::connection _delete_connection;
    sigc::connection _changed_connection;
    sigc::connection _transformed_connection;

    SPUsePath(SPObject* i_owner);
    ~SPUsePath(void);

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
