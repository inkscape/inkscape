/*
 * The reference corresponding to the inkscape:perspectiveID attribute
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2007 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "persp3d-reference.h"
#include "persp3d.h"
#include "uri.h"

static void persp3dreference_href_changed(SPObject *old_ref, SPObject *ref, Persp3DReference *persp3dref);
static void persp3dreference_delete_self(SPObject *deleted, Persp3DReference *persp3dref);
static void persp3dreference_source_modified(SPObject *iSource, guint flags, Persp3DReference *persp3dref);

Persp3DReference::Persp3DReference(SPObject* i_owner) : URIReference(i_owner)
{
    owner=i_owner;
    persp_href = NULL;
    persp_repr = NULL;
    persp = NULL;
    _changed_connection = changedSignal().connect(sigc::bind(sigc::ptr_fun(persp3dreference_href_changed), this)); // listening to myself, this should be virtual instead
}

Persp3DReference::~Persp3DReference(void)
{
    _changed_connection.disconnect(); // to do before unlinking

    quit_listening();
    unlink();
}

bool
Persp3DReference::_acceptObject(SPObject *obj) const
{
    return SP_IS_PERSP3D(obj) && URIReference::_acceptObject(obj);
;
    /* effic: Don't bother making this an inline function: _acceptObject is a virtual function,
       typically called from a context where the runtime type is not known at compile time. */
}

void
Persp3DReference::unlink(void)
{
    g_free(persp_href);
    persp_href = NULL;
    detach();
}

void
Persp3DReference::start_listening(Persp3D* to)
{
    if ( to == NULL ) {
        return;
    }
    persp = to;
    persp_repr = to->getRepr();
    _delete_connection = to->connectDelete(sigc::bind(sigc::ptr_fun(&persp3dreference_delete_self), this));
    _modified_connection = to->connectModified(sigc::bind<2>(sigc::ptr_fun(&persp3dreference_source_modified), this));
}

void
Persp3DReference::quit_listening(void)
{
    if ( persp == NULL ) {
        return;
    }
    _modified_connection.disconnect();
    _delete_connection.disconnect();
    persp_repr = NULL;
    persp = NULL;
}

static void
persp3dreference_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, Persp3DReference *persp3dref)
{
    persp3dref->quit_listening();
    Persp3D *refobj = SP_PERSP3D(persp3dref->getObject());
    if ( refobj ) {
        persp3dref->start_listening(refobj);
    }

    persp3dref->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
persp3dreference_delete_self(SPObject */*deleted*/, Persp3DReference *persp3dref)
{
    g_return_if_fail(persp3dref->owner);
    persp3dref->owner->deleteObject();
}

static void
persp3dreference_source_modified(SPObject */*iSource*/, guint /*flags*/, Persp3DReference *persp3dref)
{
    persp3dref->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}


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
