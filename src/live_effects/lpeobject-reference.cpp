/*
 * The reference corresponding to the inkscape:live-effect attribute
 *
 * Copyright (C) 2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <string.h>

#include "enums.h"
#include "live_effects/lpeobject-reference.h"
#include "live_effects/lpeobject.h"
#include "uri.h"

namespace Inkscape {

namespace LivePathEffect {

static void lpeobjectreference_href_changed(SPObject *old_ref, SPObject *ref, LPEObjectReference *lpeobjref);
static void lpeobjectreference_delete_self(SPObject *deleted, LPEObjectReference *lpeobjref);
static void lpeobjectreference_source_modified(SPObject *iSource, guint flags, LPEObjectReference *lpeobjref);

LPEObjectReference::LPEObjectReference(SPObject* i_owner) : URIReference(i_owner)
{
    owner=i_owner;
    lpeobject_href = NULL;
    lpeobject_repr = NULL;
    lpeobject = NULL;
    _changed_connection = changedSignal().connect(sigc::bind(sigc::ptr_fun(lpeobjectreference_href_changed), this)); // listening to myself, this should be virtual instead

    user_unlink = NULL;
}

LPEObjectReference::~LPEObjectReference(void)
{
    _changed_connection.disconnect(); // to do before unlinking

    quit_listening();
    unlink();
}

bool LPEObjectReference::_acceptObject(SPObject * const obj) const
{
    if (IS_LIVEPATHEFFECT(obj)) {
        return URIReference::_acceptObject(obj);
    } else {
        return false;
    }
}

void
LPEObjectReference::link(const char *to)
{
    if ( to == NULL ) {
        quit_listening();
        unlink();
    } else {
        if ( !lpeobject_href || ( strcmp(to, lpeobject_href) != 0 ) ) {
            g_free(lpeobject_href);
            lpeobject_href = g_strdup(to);
            try {
                attach(Inkscape::URI(to));
            } catch (Inkscape::BadURIException &e) {
                /* TODO: Proper error handling as per
                 * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing.
                 */
                g_warning("%s", e.what());
                detach();
            }
        }
    }
}

void
LPEObjectReference::unlink(void)
{
    g_free(lpeobject_href);
    lpeobject_href = NULL;
    detach();
}

void
LPEObjectReference::start_listening(LivePathEffectObject* to)
{
    if ( to == NULL ) {
        return;
    }
    lpeobject = to;
    lpeobject_repr = to->getRepr();
    _delete_connection = to->connectDelete(sigc::bind(sigc::ptr_fun(&lpeobjectreference_delete_self), this));
    _modified_connection = to->connectModified(sigc::bind<2>(sigc::ptr_fun(&lpeobjectreference_source_modified), this));
}

void
LPEObjectReference::quit_listening(void)
{
    if ( lpeobject == NULL ) {
        return;
    }
    _modified_connection.disconnect();
    _delete_connection.disconnect();
    lpeobject_repr = NULL;
    lpeobject = NULL;
}

static void
lpeobjectreference_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, LPEObjectReference *lpeobjref)
{
    lpeobjref->quit_listening();
    LivePathEffectObject *refobj = LIVEPATHEFFECT( lpeobjref->getObject() );
    if ( refobj ) {
        lpeobjref->start_listening(refobj);
    }

    lpeobjref->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
lpeobjectreference_delete_self(SPObject */*deleted*/, LPEObjectReference *lpeobjref)
{
        lpeobjref->quit_listening();
        lpeobjref->unlink();
        if (lpeobjref->user_unlink)
            lpeobjref->user_unlink(lpeobjref, lpeobjref->owner);
}

static void
lpeobjectreference_source_modified(SPObject */*iSource*/, guint /*flags*/, LPEObjectReference *lpeobjref)
{
    lpeobjref->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

} //namespace LivePathEffect

} // namespace inkscape

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
