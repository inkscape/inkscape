#ifndef SEEN_LPEOBJECT_REFERENCE_H
#define SEEN_LPEOBJECT_REFERENCE_H

/*
 * The reference corresponding to the inkscape:live-effect attribute
 *
 * Copyright (C) 2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <sigc++/sigc++.h>

#include "uri-references.h"

namespace Inkscape {
namespace XML {
class Node;
}
}

class LivePathEffectObject;

namespace Inkscape {

namespace LivePathEffect {

class LPEObjectReference : public Inkscape::URIReference {
public:
    LPEObjectReference(SPObject *owner);
    virtual ~LPEObjectReference();

    SPObject       *owner;

    // concerning the LPEObject that is refered to:
    char                 *lpeobject_href;
    Inkscape::XML::Node  *lpeobject_repr;
    LivePathEffectObject *lpeobject;

    sigc::connection _modified_connection;
    sigc::connection _delete_connection;
    sigc::connection _changed_connection;

    void            link(const char* to);
    void            unlink(void);
    void            start_listening(LivePathEffectObject* to);
    void            quit_listening(void);

    void (*user_unlink) (LPEObjectReference *me, SPObject *user);

protected:
    bool _acceptObject(SPObject * const obj) const;

};

} //namespace LivePathEffect

} // namespace inkscape

#endif /* !SEEN_LPEOBJECT_REFERENCE_H */

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
