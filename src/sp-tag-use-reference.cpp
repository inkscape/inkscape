/*
 * The reference corresponding to href of <inkscape:tagref> element.
 *
 * Copyright (C) Theodore Janeczko 2012-2014 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <cstring>
#include <string>
#include <string.h>

#include "enums.h"
#include "sp-tag-use-reference.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "preferences.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "uri.h"


bool SPTagUseReference::_acceptObject(SPObject * const obj) const
{
    if (SP_IS_ITEM(obj)) {
        return URIReference::_acceptObject(obj);
    } else {
        return false;
    }
}


static void sp_usepath_href_changed(SPObject *old_ref, SPObject *ref, SPTagUsePath *offset);
static void sp_usepath_delete_self(SPObject *deleted, SPTagUsePath *offset);

SPTagUsePath::SPTagUsePath(SPObject* i_owner):SPTagUseReference(i_owner)
{
    owner=i_owner;
    originalPath = NULL;
    sourceDirty=false;
    sourceHref = NULL;
    sourceRepr = NULL;
    sourceObject = NULL;
    _changed_connection = changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_usepath_href_changed), this)); // listening to myself, this should be virtual instead

    user_unlink = NULL;
}

SPTagUsePath::~SPTagUsePath(void)
{
    delete originalPath;
    originalPath = NULL;

    _changed_connection.disconnect(); // to do before unlinking

    quit_listening();
    unlink();
}

void
SPTagUsePath::link(char *to)
{
    if ( to == NULL ) {
        quit_listening();
        unlink();
    } else {
        if ( !sourceHref || ( strcmp(to, sourceHref) != 0 ) ) {
            g_free(sourceHref);
            sourceHref = g_strdup(to);
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
SPTagUsePath::unlink(void)
{
    g_free(sourceHref);
    sourceHref = NULL;
    detach();
}

void
SPTagUsePath::start_listening(SPObject* to)
{
    if ( to == NULL ) {
        return;
    }
    sourceObject = to;
    sourceRepr = to->getRepr();
    _delete_connection = to->connectDelete(sigc::bind(sigc::ptr_fun(&sp_usepath_delete_self), this));
}

void
SPTagUsePath::quit_listening(void)
{
    if ( sourceObject == NULL ) {
        return;
    }
    _delete_connection.disconnect();
    sourceRepr = NULL;
    sourceObject = NULL;
}

static void
sp_usepath_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, SPTagUsePath *offset)
{
    offset->quit_listening();
    SPItem *refobj = offset->getObject();
    if ( refobj ) {
        offset->start_listening(refobj);
    }
}

static void
sp_usepath_delete_self(SPObject */*deleted*/, SPTagUsePath *offset)
{
    offset->owner->deleteObject();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
