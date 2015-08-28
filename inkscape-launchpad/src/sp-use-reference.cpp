/*
 * The reference corresponding to href of <use> element.
 *
 * Copyright (C) 2004 Bulia Byak
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <cstring>
#include <string>
#include <string.h>

#include "enums.h"
#include "sp-use-reference.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "preferences.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "uri.h"



bool SPUseReference::_acceptObject(SPObject * const obj) const
{
	return URIReference::_acceptObject(obj);
}


static void sp_usepath_href_changed(SPObject *old_ref, SPObject *ref, SPUsePath *offset);
static void sp_usepath_move_compensate(Geom::Affine const *mp, SPItem *original, SPUsePath *self);
static void sp_usepath_delete_self(SPObject *deleted, SPUsePath *offset);
static void sp_usepath_source_modified(SPObject *iSource, guint flags, SPUsePath *offset);

SPUsePath::SPUsePath(SPObject* i_owner):SPUseReference(i_owner)
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

SPUsePath::~SPUsePath(void)
{
    delete originalPath;
    originalPath = NULL;

    _changed_connection.disconnect(); // to do before unlinking

    quit_listening();
    unlink();
}

void
SPUsePath::link(char *to)
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
SPUsePath::unlink(void)
{
    g_free(sourceHref);
    sourceHref = NULL;
    detach();
}

void
SPUsePath::start_listening(SPObject* to)
{
    if ( to == NULL ) {
        return;
    }
    sourceObject = to;
    sourceRepr = to->getRepr();
    _delete_connection = to->connectDelete(sigc::bind(sigc::ptr_fun(&sp_usepath_delete_self), this));
    _transformed_connection = SP_ITEM(to)->connectTransformed(sigc::bind(sigc::ptr_fun(&sp_usepath_move_compensate), this));
    _modified_connection = to->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_usepath_source_modified), this));
}

void
SPUsePath::quit_listening(void)
{
    if ( sourceObject == NULL ) {
        return;
    }
    _modified_connection.disconnect();
    _delete_connection.disconnect();
    _transformed_connection.disconnect();
    sourceRepr = NULL;
    sourceObject = NULL;
}

static void
sp_usepath_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, SPUsePath *offset)
{
    offset->quit_listening();
    SPItem *refobj = offset->getObject();
    if ( refobj ) {
        offset->start_listening(refobj);
    }
    offset->sourceDirty=true;
    offset->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_usepath_move_compensate(Geom::Affine const *mp, SPItem *original, SPUsePath *self)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint mode = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_PARALLEL);
    if (mode == SP_CLONE_COMPENSATION_NONE) {
        return;
    }
    SPItem *item = SP_ITEM(self->owner);

// TODO kill naughty naughty #if 0
#if 0
    Geom::Affine m(*mp);
    if (!(m.is_translation())) {
        return;
    }
    Geom::Affine const t(item->transform);
    Geom::Affine clone_move = t.inverse() * m * t;

    // Calculate the compensation matrix and the advertized movement matrix.
    Geom::Affine advertized_move;
    if (mode == SP_CLONE_COMPENSATION_PARALLEL) {
        //clone_move = clone_move.inverse();
        advertized_move.set_identity();
    } else if (mode == SP_CLONE_COMPENSATION_UNMOVED) {
        clone_move = clone_move.inverse() * m;
        advertized_move = m;
    } else {
        g_assert_not_reached();
    }

    // Commit the compensation.
    item->transform *= clone_move;
    sp_item_write_transform(item, item->getRepr(), item->transform, &advertized_move);
#else
    (void)mp;
    (void)original;
#endif

    self->sourceDirty = true;
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_usepath_delete_self(SPObject */*deleted*/, SPUsePath *offset)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint const mode = prefs->getInt("/options/cloneorphans/value", SP_CLONE_ORPHANS_UNLINK);

    if (mode == SP_CLONE_ORPHANS_UNLINK) {
        // leave it be. just forget about the source
        offset->quit_listening();
        offset->unlink();
        if (offset->user_unlink)
            offset->user_unlink(offset->owner);
    } else if (mode == SP_CLONE_ORPHANS_DELETE) {
        offset->owner->deleteObject();
    }
}

static void
sp_usepath_source_modified(SPObject */*iSource*/, guint /*flags*/, SPUsePath *offset)
{
    offset->sourceDirty = true;
    offset->owner->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void SPUsePath::refresh_source()
{
    sourceDirty = false;
    delete originalPath;
    originalPath = NULL;

    // le mauvais cas: pas d'attribut d => il faut verifier que c'est une SPShape puis prendre le contour
    // [tr: The bad case: no d attribute.  Must check that it's a SPShape and then take the outline.]
    SPObject *refobj = sourceObject;
    if ( refobj == NULL ) return;
    
    SPItem *item = SP_ITEM(refobj);
    SPCurve *curve = NULL;

    if (SP_IS_SHAPE(item))
    {
        curve = SP_SHAPE(item)->getCurve();
    }
    else if (SP_IS_TEXT(item))
    {
        curve = SP_TEXT(item)->getNormalizedBpath();
    }
    else
    {
        return;
    }
        
    if (curve == NULL)
        return;

    originalPath = new Path;
    originalPath->LoadPathVector(curve->get_pathvector(), item->transform, true);
    curve->unref();
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
