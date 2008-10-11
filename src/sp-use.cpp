#define __SP_USE_C__

/*
 * SVG <use> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cstring>
#include <string>

#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <2geom/transforms.h>
#include <glibmm/i18n.h>
#include "display/nr-arena-group.h"
#include "attributes.h"
#include "document.h"
#include "sp-object-repr.h"
#include "sp-flowregion.h"
#include "uri.h"
#include "print.h"
#include "xml/repr.h"
#include "prefs-utils.h"
#include "style.h"
#include "sp-symbol.h"
#include "sp-use.h"
#include "sp-use-reference.h"

/* fixme: */

static void sp_use_class_init(SPUseClass *classname);
static void sp_use_init(SPUse *use);
static void sp_use_finalize(GObject *obj);

static void sp_use_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_use_release(SPObject *object);
static void sp_use_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *sp_use_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_use_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_use_modified(SPObject *object, guint flags);

static void sp_use_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static void sp_use_snappoints(SPItem const *item, SnapPointsIter p);
static void sp_use_print(SPItem *item, SPPrintContext *ctx);
static gchar *sp_use_description(SPItem *item);
static NRArenaItem *sp_use_show(SPItem *item, NRArena *arena, unsigned key, unsigned flags);
static void sp_use_hide(SPItem *item, unsigned key);

static void sp_use_href_changed(SPObject *old_ref, SPObject *ref, SPUse *use);

static void sp_use_delete_self(SPObject *deleted, SPUse *self);

static SPItemClass *parent_class;

//void m_print(gchar *say, NR::Matrix m)
//{ g_print("%s %g %g %g %g %g %g\n", say, m[0], m[1], m[2], m[3], m[4], m[5]); }

GType
sp_use_get_type(void)
{
    static GType use_type = 0;
    if (!use_type) {
        GTypeInfo use_info = {
            sizeof(SPUseClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_use_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPUse),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_use_init,
            NULL,   /* value_table */
        };
        use_type = g_type_register_static(SP_TYPE_ITEM, "SPUse", &use_info, (GTypeFlags)0);
    }
    return use_type;
}

static void
sp_use_class_init(SPUseClass *classname)
{
    GObjectClass *gobject_class = (GObjectClass *) classname;
    SPObjectClass *sp_object_class = (SPObjectClass *) classname;
    SPItemClass *item_class = (SPItemClass *) classname;

    parent_class = (SPItemClass*) g_type_class_ref(SP_TYPE_ITEM);

    gobject_class->finalize = sp_use_finalize;

    sp_object_class->build = sp_use_build;
    sp_object_class->release = sp_use_release;
    sp_object_class->set = sp_use_set;
    sp_object_class->write = sp_use_write;
    sp_object_class->update = sp_use_update;
    sp_object_class->modified = sp_use_modified;

    item_class->bbox = sp_use_bbox;
    item_class->description = sp_use_description;
    item_class->print = sp_use_print;
    item_class->show = sp_use_show;
    item_class->hide = sp_use_hide;
    item_class->snappoints = sp_use_snappoints;
}

static void
sp_use_init(SPUse *use)
{
    use->x.unset();
    use->y.unset();
    use->width.unset(SVGLength::PERCENT, 1.0, 1.0);
    use->height.unset(SVGLength::PERCENT, 1.0, 1.0);
    use->href = NULL;

    new (&use->_delete_connection) sigc::connection();
    new (&use->_changed_connection) sigc::connection();

    new (&use->_transformed_connection) sigc::connection();

    use->ref = new SPUseReference(SP_OBJECT(use));

    use->_changed_connection = use->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_use_href_changed), use));
}

static void
sp_use_finalize(GObject *obj)
{
    SPUse *use = (SPUse *) obj;

    if (use->child) {
        sp_object_detach(SP_OBJECT(obj), use->child);
        use->child = NULL;
    }

    use->ref->detach();
    delete use->ref;

    use->_delete_connection.~connection();
    use->_changed_connection.~connection();

    use->_transformed_connection.~connection();
}

static void
sp_use_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) parent_class)->build) {
        (* ((SPObjectClass *) parent_class)->build)(object, document, repr);
    }

    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "width");
    sp_object_read_attr(object, "height");
    sp_object_read_attr(object, "xlink:href");

    // We don't need to create child here:
    // reading xlink:href will attach ref, and that will cause the changed signal to be emitted,
    // which will call sp_use_href_changed, and that will take care of the child
}

static void
sp_use_release(SPObject *object)
{
    SPUse *use = SP_USE(object);

    if (use->child) {
        sp_object_detach(object, use->child);
        use->child = NULL;
    }

    use->_delete_connection.disconnect();
    use->_changed_connection.disconnect();
    use->_transformed_connection.disconnect();

    g_free(use->href);
    use->href = NULL;

    use->ref->detach();

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

static void
sp_use_set(SPObject *object, unsigned key, gchar const *value)
{
    SPUse *use = SP_USE(object);

    switch (key) {
        case SP_ATTR_X:
            use->x.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            use->y.readOrUnset(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_WIDTH:
            use->width.readOrUnset(value, SVGLength::PERCENT, 1.0, 1.0);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_HEIGHT:
            use->height.readOrUnset(value, SVGLength::PERCENT, 1.0, 1.0);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_XLINK_HREF: {
            if ( value && use->href && ( strcmp(value, use->href) == 0 ) ) {
                /* No change, do nothing. */
            } else {
                g_free(use->href);
                use->href = NULL;
                if (value) {
                    // First, set the href field, because sp_use_href_changed will need it.
                    use->href = g_strdup(value);

                    // Now do the attaching, which emits the changed signal.
                    try {
                        use->ref->attach(Inkscape::URI(value));
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        use->ref->detach();
                    }
                } else {
                    use->ref->detach();
                }
            }
            break;
        }

        default:
            if (((SPObjectClass *) parent_class)->set) {
                ((SPObjectClass *) parent_class)->set(object, key, value);
            }
            break;
    }
}

static Inkscape::XML::Node *
sp_use_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPUse *use = SP_USE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:use");
    }

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, xml_doc, repr, flags);
    }

    sp_repr_set_svg_double(repr, "x", use->x.computed);
    sp_repr_set_svg_double(repr, "y", use->y.computed);
    sp_repr_set_svg_double(repr, "width", use->width.computed);
    sp_repr_set_svg_double(repr, "height", use->height.computed);

    if (use->ref->getURI()) {
        gchar *uri_string = use->ref->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    return repr;
}

static void
sp_use_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags)
{
    SPUse const *use = SP_USE(item);

    if (use->child && SP_IS_ITEM(use->child)) {
        SPItem *child = SP_ITEM(use->child);
        Geom::Matrix const ct( child->transform
                             * Geom::Translate(use->x.computed,
                                               use->y.computed)
                             * transform );
        sp_item_invoke_bbox_full(child, bbox, ct, flags, FALSE);
    }
}

static void
sp_use_print(SPItem *item, SPPrintContext *ctx)
{
    bool translated = false;
    SPUse *use = SP_USE(item);

    if ((use->x._set && use->x.computed != 0) || (use->y._set && use->y.computed != 0)) {
        NR::Matrix tp(Geom::Translate(use->x.computed, use->y.computed));
        sp_print_bind(ctx, tp, 1.0);
        translated = true;
    }

    if (use->child && SP_IS_ITEM(use->child)) {
        sp_item_invoke_print(SP_ITEM(use->child), ctx);
    }

    if (translated) {
        sp_print_release(ctx);
    }
}

static gchar *
sp_use_description(SPItem *item)
{
    SPUse *use = SP_USE(item);

    char *ret;
    if (use->child) {
        static unsigned recursion_depth = 0;
        if (recursion_depth >= 4) {
            /* TRANSLATORS: Used for statusbar description for long <use> chains:
             * "Clone of: Clone of: ... in Layer 1". */
            return g_strdup(_("..."));
            /* We could do better, e.g. chasing the href chain until we reach something other than
             * a <use>, and giving its description. */
        }
        ++recursion_depth;
        char *child_desc = sp_item_description(SP_ITEM(use->child));
        --recursion_depth;

        ret = g_strdup_printf(_("<b>Clone</b> of: %s"), child_desc);
        g_free(child_desc);
        return ret;
    } else {
        return g_strdup(_("<b>Orphaned clone</b>"));
    }
}

static NRArenaItem *
sp_use_show(SPItem *item, NRArena *arena, unsigned key, unsigned flags)
{
    SPUse *use = SP_USE(item);

    NRArenaItem *ai = NRArenaGroup::create(arena);
    nr_arena_group_set_transparent(NR_ARENA_GROUP(ai), FALSE);
    nr_arena_group_set_style(NR_ARENA_GROUP(ai), SP_OBJECT_STYLE(item));

    if (use->child) {
        NRArenaItem *ac = sp_item_invoke_show(SP_ITEM(use->child), arena, key, flags);
        if (ac) {
            nr_arena_item_add_child(ai, ac, NULL);
        }
        Geom::Translate t(use->x.computed,
                        use->y.computed);
        nr_arena_group_set_child_transform(NR_ARENA_GROUP(ai), NR::Matrix(t));
    }

    return ai;
}

static void
sp_use_hide(SPItem *item, unsigned key)
{
    SPUse *use = SP_USE(item);

    if (use->child) {
        sp_item_invoke_hide(SP_ITEM(use->child), key);
    }

    if (((SPItemClass *) parent_class)->hide) {
        ((SPItemClass *) parent_class)->hide(item, key);
    }
}

/**
 * Returns the ultimate original of a SPUse (i.e. the first object in the chain of its originals
 * which is not an SPUse). If no original is found, NULL is returned (it is the responsibility
 * of the caller to make sure that this is handled correctly).
 *
 * Note that the returned is the clone object, i.e. the child of an SPUse (of the argument one for
 * the trivial case) and not the "true original".
 */
SPItem *
sp_use_root(SPUse *use)
{
    SPObject *orig = use->child;
    while (SP_IS_USE(orig)) {
        orig = SP_USE(orig)->child;
    }
    g_return_val_if_fail(SP_IS_ITEM(orig), NULL);
    return SP_ITEM(orig);
}

/**
 * Returns the effective transform that goes from the ultimate original to given SPUse, both ends
 * included.
 */
NR::Matrix
sp_use_get_root_transform(SPUse *use)
{
    //track the ultimate source of a chain of uses
    SPObject *orig = use->child;
    GSList *chain = NULL;
    chain = g_slist_prepend(chain, use);
    while (SP_IS_USE(orig)) {
        chain = g_slist_prepend(chain, orig);
        orig = SP_USE(orig)->child;
    }
    chain = g_slist_prepend(chain, orig);


    //calculate the accummulated transform, starting from the original
    NR::Matrix t(NR::identity());
    for (GSList *i = chain; i != NULL; i = i->next) {
        SPItem *i_tem = SP_ITEM(i->data);

        // "An additional transformation translate(x,y) is appended to the end (i.e.,
        // right-side) of the transform attribute on the generated 'g', where x and y
        // represent the values of the x and y attributes on the 'use' element." - http://www.w3.org/TR/SVG11/struct.html#UseElement
        if (SP_IS_USE(i_tem)) {
            SPUse *i_use = SP_USE(i_tem);
            if ((i_use->x._set && i_use->x.computed != 0) || (i_use->y._set && i_use->y.computed != 0)) {
                t = t * Geom::Translate(i_use->x._set ? i_use->x.computed : 0, i_use->y._set ? i_use->y.computed : 0);
            }
        }

        t *= i_tem->transform;
    }

    g_slist_free(chain);
    return t;
}

/**
 * Returns the transform that leads to the use from its immediate original.
 * Does not inlcude the original's transform if any.
 */
NR::Matrix
sp_use_get_parent_transform(SPUse *use)
{
    Geom::Matrix t(Geom::identity());
    if ((use->x._set && use->x.computed != 0) || (use->y._set && use->y.computed != 0)) {
        t *= Geom::Translate(use->x._set ? use->x.computed : 0,
                           use->y._set ? use->y.computed : 0);
    }

    t *= SP_ITEM(use)->transform;
    return t;
}

/**
 * Sensing a movement of the original, this function attempts to compensate for it in such a way
 * that the clone stays unmoved or moves in parallel (depending on user setting) regardless of the
 * clone's transform.
 */
static void
sp_use_move_compensate(Geom::Matrix const *mp, SPItem */*original*/, SPUse *self)
{
    // the clone is orphaned; or this is not a real use, but a clone of another use;
    // we skip it, otherwise duplicate compensation will occur
    if (SP_OBJECT_IS_CLONED(self)) {
        return;
    }

    // never compensate uses which are used in flowtext
    if (SP_OBJECT_PARENT(self) && SP_IS_FLOWREGION(SP_OBJECT_PARENT(self))) {
        return;
    }

    guint mode = prefs_get_int_attribute("options.clonecompensation", "value", SP_CLONE_COMPENSATION_PARALLEL);
    // user wants no compensation
    if (mode == SP_CLONE_COMPENSATION_NONE)
        return;

    Geom::Matrix m(*mp);

    // this is not a simple move, do not try to compensate
    if (!(m.isTranslation()))
        return;

    // restore item->transform field from the repr, in case it was changed by seltrans
    sp_object_read_attr (SP_OBJECT (self), "transform");

    Geom::Matrix t = sp_use_get_parent_transform(self);
    Geom::Matrix clone_move = t.inverse() * m * t;

    // calculate the compensation matrix and the advertized movement matrix
    Geom::Matrix advertized_move;
    if (mode == SP_CLONE_COMPENSATION_PARALLEL) {
        clone_move = clone_move.inverse() * m;
        advertized_move = m;
    } else if (mode == SP_CLONE_COMPENSATION_UNMOVED) {
        clone_move = clone_move.inverse();
        advertized_move.setIdentity();
    } else {
        g_assert_not_reached();
    }

    // commit the compensation
    SPItem *item = SP_ITEM(self);
    item->transform *= clone_move;
    sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform, &advertized_move);
    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_use_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, SPUse *use)
{
    SPItem *item = SP_ITEM(use);

    use->_delete_connection.disconnect();
    use->_transformed_connection.disconnect();

    if (use->child) {
        sp_object_detach(SP_OBJECT(use), use->child);
        use->child = NULL;
    }

    if (use->href) {
        SPItem *refobj = use->ref->getObject();
        if (refobj) {
            Inkscape::XML::Node *childrepr = SP_OBJECT_REPR(refobj);
            GType type = sp_repr_type_lookup(childrepr);
            g_return_if_fail(type > G_TYPE_NONE);
            if (g_type_is_a(type, SP_TYPE_ITEM)) {
                use->child = (SPObject*) g_object_new(type, 0);
                sp_object_attach(SP_OBJECT(use), use->child, use->lastChild());
                sp_object_unref(use->child, SP_OBJECT(use));
                sp_object_invoke_build(use->child, SP_OBJECT(use)->document, childrepr, TRUE);

                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    NRArenaItem *ai;
                    ai = sp_item_invoke_show(SP_ITEM(use->child), NR_ARENA_ITEM_ARENA(v->arenaitem), v->key, v->flags);
                    if (ai) {
                        nr_arena_item_add_child(v->arenaitem, ai, NULL);
                    }
                }

            }
            use->_delete_connection = SP_OBJECT(refobj)->connectDelete(sigc::bind(sigc::ptr_fun(&sp_use_delete_self), use));
            use->_transformed_connection = SP_ITEM(refobj)->connectTransformed(sigc::bind(sigc::ptr_fun(&sp_use_move_compensate), use));
        }
    }
}

static void
sp_use_delete_self(SPObject */*deleted*/, SPUse *self)
{
    // always delete uses which are used in flowtext
    if (SP_OBJECT_PARENT(self) && SP_IS_FLOWREGION(SP_OBJECT_PARENT(self))) {
        SP_OBJECT(self)->deleteObject();
        return;
    }

    guint const mode = prefs_get_int_attribute("options.cloneorphans", "value",
                                               SP_CLONE_ORPHANS_UNLINK);

    if (mode == SP_CLONE_ORPHANS_UNLINK) {
        sp_use_unlink(self);
    } else if (mode == SP_CLONE_ORPHANS_DELETE) {
        SP_OBJECT(self)->deleteObject();
    }
}

static void
sp_use_update(SPObject *object, SPCtx *ctx, unsigned flags)
{
    SPItem *item = SP_ITEM(object);
    SPUse *use = SP_USE(object);
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx cctx = *ictx;

    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      for (SPItemView *v = SP_ITEM(object)->display; v != NULL; v = v->next) {
        nr_arena_group_set_style(NR_ARENA_GROUP(v->arenaitem), SP_OBJECT_STYLE(object));
      }
    }

    /* Set up child viewport */
    if (use->x.unit == SVGLength::PERCENT) {
        use->x.computed = use->x.value * (ictx->vp.x1 - ictx->vp.x0);
    }
    if (use->y.unit == SVGLength::PERCENT) {
        use->y.computed = use->y.value * (ictx->vp.y1 - ictx->vp.y0);
    }
    if (use->width.unit == SVGLength::PERCENT) {
        use->width.computed = use->width.value * (ictx->vp.x1 - ictx->vp.x0);
    }
    if (use->height.unit == SVGLength::PERCENT) {
        use->height.computed = use->height.value * (ictx->vp.y1 - ictx->vp.y0);
    }
    cctx.vp.x0 = 0.0;
    cctx.vp.y0 = 0.0;
    cctx.vp.x1 = use->width.computed;
    cctx.vp.y1 = use->height.computed;
    cctx.i2vp = NR::identity();
    flags&=~SP_OBJECT_USER_MODIFIED_FLAG_B;

    if (use->child) {
        g_object_ref(G_OBJECT(use->child));
        if (flags || (use->child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(use->child)) {
                SPItem const &chi = *SP_ITEM(use->child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                use->child->updateDisplay((SPCtx *)&cctx, flags);
            } else {
                use->child->updateDisplay(ctx, flags);
            }
        }
        g_object_unref(G_OBJECT(use->child));
    }

    /* As last step set additional transform of arena group */
    for (SPItemView *v = item->display; v != NULL; v = v->next) {
        NR::Matrix t(Geom::Translate(use->x.computed, use->y.computed));
        nr_arena_group_set_child_transform(NR_ARENA_GROUP(v->arenaitem), t);
    }
}

static void
sp_use_modified(SPObject *object, guint flags)
{
    SPUse *use_obj = SP_USE(object);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      for (SPItemView *v = SP_ITEM(object)->display; v != NULL; v = v->next) {
        nr_arena_group_set_style(NR_ARENA_GROUP(v->arenaitem), SP_OBJECT_STYLE(object));
      }
    }

    SPObject *child = use_obj->child;
    if (child) {
        g_object_ref(G_OBJECT(child));
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref(G_OBJECT(child));
    }
}

SPItem *
sp_use_unlink(SPUse *use)
{
    if (!use) return NULL;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(use);
    if (!repr) return NULL;

    Inkscape::XML::Node *parent = sp_repr_parent(repr);
    SPDocument *document = SP_OBJECT(use)->document;
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    // Track the ultimate source of a chain of uses.
    SPItem *orig = sp_use_root(use);
    g_return_val_if_fail(orig, NULL);

    // Calculate the accumulated transform, starting from the original.
    Geom::Matrix t = sp_use_get_root_transform(use);

    Inkscape::XML::Node *copy = NULL;
    if (SP_IS_SYMBOL(orig)) { // make a group, copy children
        copy = xml_doc->createElement("svg:g");
        for (Inkscape::XML::Node *child = SP_OBJECT_REPR(orig)->firstChild() ; child != NULL; child = child->next()) {
                Inkscape::XML::Node *newchild = child->duplicate(xml_doc);
                copy->appendChild(newchild);
        }
    } else { // just copy
        copy = SP_OBJECT_REPR(orig)->duplicate(xml_doc);
    }

    // Add the duplicate repr just after the existing one.
    parent->addChild(copy, repr);

    // Retrieve the SPItem of the resulting repr.
    SPObject *unlinked = document->getObjectByRepr(copy);

    // Merge style from the use.
    SPStyle *unli_sty = SP_OBJECT_STYLE(unlinked);
    SPStyle const *use_sty = SP_OBJECT_STYLE(use);
    sp_style_merge_from_dying_parent(unli_sty, use_sty);
    sp_style_merge_from_parent(unli_sty, unlinked->parent->style);

    SP_OBJECT(unlinked)->updateRepr();

    // Hold onto our SPObject and repr for now.
    sp_object_ref(SP_OBJECT(use), NULL);
    Inkscape::GC::anchor(repr);

    // Remove ourselves, not propagating delete events to avoid a
    // chain-reaction with other elements that might reference us.
    SP_OBJECT(use)->deleteObject(false);

    // Give the copy our old id and let go of our old repr.
    copy->setAttribute("id", repr->attribute("id"));
    Inkscape::GC::release(repr);

    // Remove tiled clone attrs.
    copy->setAttribute("inkscape:tiled-clone-of", NULL);
    copy->setAttribute("inkscape:tile-w", NULL);
    copy->setAttribute("inkscape:tile-h", NULL);
    copy->setAttribute("inkscape:tile-cx", NULL);
    copy->setAttribute("inkscape:tile-cy", NULL);

    // Establish the succession and let go of our object.
    SP_OBJECT(use)->setSuccessor(unlinked);
    sp_object_unref(SP_OBJECT(use), NULL);

    SPItem *item = SP_ITEM(unlinked);
    // Set the accummulated transform.
    {
        Geom::Matrix nomove(Geom::identity());
        // Advertise ourselves as not moving.
        sp_item_write_transform(item, SP_OBJECT_REPR(item), t, &nomove);
    }
    return item;
}

SPItem *
sp_use_get_original(SPUse *use)
{
    SPItem *ref = use->ref->getObject();
    return ref;
}

static void
sp_use_snappoints(SPItem const *item, SnapPointsIter p)
{
    g_assert (item != NULL);
    g_assert (SP_IS_ITEM(item));
    g_assert (SP_IS_USE(item));
    
    SPUse *use = SP_USE(item);
    SPItem *root = sp_use_root(use);
    g_return_if_fail(root);
    
    SPItemClass const &item_class = *(SPItemClass const *) G_OBJECT_GET_CLASS(root);
    if (item_class.snappoints) {
        item_class.snappoints(root, p);
    }
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
