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

// for testing:
#include "xml/repr.h"
#include "box3d.h"

static void persp3dreference_href_changed(SPObject *old_ref, SPObject *ref, Persp3DReference *persp3dref);
static void persp3dreference_delete_self(SPObject *deleted, Persp3DReference *persp3dref);
static void persp3dreference_source_modified(SPObject *iSource, guint flags, Persp3DReference *persp3dref);

Persp3DReference::Persp3DReference(SPObject* i_owner) : URIReference(i_owner)
{
    owner=i_owner;
    /**
    if (owner) {
        g_print ("Owner of newly created Persp3DReference is box #%d ", SP_BOX3D(owner)->my_counter);
        g_print ("(no ID yet because we are calling from box3d_init()...\n");
    }
    **/
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
    return SP_IS_PERSP3D(obj);
    /* effic: Don't bother making this an inline function: _acceptObject is a virtual function,
       typically called from a context where the runtime type is not known at compile time. */
}

/***
void
Persp3DReference::link(char *to)
{
    if ( to == NULL ) {
        quit_listening();
        unlink();
    } else {
        if ( !persp_href || ( strcmp(to, persp_href) != 0 ) ) {
            g_free(persp_href);
            persp_href = g_strdup(to);
            try {
                attach(Inkscape::URI(to));
            } catch (Inkscape::BadURIException &e) {
                 // TODO: Proper error handling as per
                 // http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing.
                 //
                g_warning("%s", e.what());
                detach();
            }
        }
    }
}
***/

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
    persp_repr = SP_OBJECT_REPR(to);
    _delete_connection = to->connectDelete(sigc::bind(sigc::ptr_fun(&persp3dreference_delete_self), this));
    _modified_connection = to->connectModified(sigc::bind<2>(sigc::ptr_fun(&persp3dreference_source_modified), this));
    //box3d_start_listening_to_persp_change (SP_BOX3D(this->owner), to);
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
    //g_print ("persp3dreference_href_changed:\n");
    persp3dref->quit_listening();
    /**
    if (SP_IS_PERSP3D(persp3dref->getObject())){
        g_print ("referenced object is a perspective\n");
    } else {
        g_print ("referenced object is NOT a perspective!!!!\n");
    }
    **/
    Persp3D *refobj = SP_PERSP3D(persp3dref->getObject());
    if ( refobj ) {
        persp3dref->start_listening(refobj);
        //g_print ("     start listening to %s\n", SP_OBJECT_REPR(refobj)->attribute("id"));
    }

    /**
    if (persp3dref->owner) {
        g_print ("Requesting display update of owner box #%d (%s) from persp3dreference_href_changed()\n",
                 SP_BOX3D(persp3dref->owner)->my_counter,
                 SP_OBJECT_REPR(persp3dref->owner)->attribute("id"));
    }
    **/
    persp3dref->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
persp3dreference_delete_self(SPObject */*deleted*/, Persp3DReference *persp3dref)
{
    g_print ("persp3dreference_delete_self; FIXME: Can we leave this to the parent URIReference?\n");
    if (persp3dref->owner) {
        g_print ("Deleting box #%d (%s) (?) from Persp3DReference\n",
                 SP_BOX3D(persp3dref->owner)->my_counter,
                 SP_OBJECT_REPR(persp3dref->owner)->attribute("id"));
    }
    persp3dref->owner->deleteObject();
}

static void
persp3dreference_source_modified(SPObject *iSource, guint flags, Persp3DReference *persp3dref)
{
    /**
    g_print ("persp3dreference_source_modified; FIXME: Can we leave this to the parent URIReference?\n");
    if (persp3dref->owner) {
        g_print ("Requesting display update of box #%d (%s) from persp3dreference_source_modified\n",
                 SP_BOX3D(persp3dref->owner)->my_counter,
                 SP_OBJECT_REPR(persp3dref->owner)->attribute("id"));
    }
    **/
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
