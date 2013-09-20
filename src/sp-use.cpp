/*
 * SVG <use> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
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

#include <2geom/transforms.h>
#include <glibmm/i18n.h>
#include "display/drawing-group.h"
#include "attributes.h"
#include "document.h"
#include "sp-factory.h"
#include "sp-flowregion.h"
#include "uri.h"
#include "print.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "preferences.h"
#include "style.h"
#include "sp-symbol.h"
#include "sp-use.h"
#include "sp-use-reference.h"

/* fixme: */
static void sp_use_href_changed(SPObject *old_ref, SPObject *ref, SPUse *use);
static void sp_use_delete_self(SPObject *deleted, SPUse *self);

#include "sp-factory.h"

namespace {
    SPObject* createUse() {
        return new SPUse();
    }

    bool useRegistered = SPFactory::instance().registerObject("svg:use", createUse);
}

SPUse::SPUse() : SPItem() {
    this->child = NULL;

    this->x.unset();
    this->y.unset();
    this->width.unset(SVGLength::PERCENT, 1.0, 1.0);
    this->height.unset(SVGLength::PERCENT, 1.0, 1.0);
    this->href = NULL;

    //new (&this->_delete_connection) sigc::connection();
    //new (&this->_changed_connection) sigc::connection();

    //new (&this->_transformed_connection) sigc::connection();

    this->ref = new SPUseReference(this);

    this->_changed_connection = this->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(sp_use_href_changed), this));
}

SPUse::~SPUse() {
    if (this->child) {
        this->detach(this->child);
        this->child = NULL;
    }

    this->ref->detach();
    delete this->ref;
    this->ref = 0;

    //this->_delete_connection.~connection();
    //this->_changed_connection.~connection();

    //this->_transformed_connection.~connection();
}

void SPUse::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPItem::build(document, repr);

    this->readAttr( "x" );
    this->readAttr( "y" );
    this->readAttr( "width" );
    this->readAttr( "height" );
    this->readAttr( "xlink:href" );

    // We don't need to create child here:
    // reading xlink:href will attach ref, and that will cause the changed signal to be emitted,
    // which will call sp_use_href_changed, and that will take care of the child
}

void SPUse::release() {
    if (this->child) {
        this->detach(this->child);
        this->child = NULL;
    }

    this->_delete_connection.disconnect();
    this->_changed_connection.disconnect();
    this->_transformed_connection.disconnect();

    g_free(this->href);
    this->href = NULL;

    this->ref->detach();

    SPItem::release();
}

void SPUse::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_X:
            this->x.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_Y:
            this->y.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_WIDTH:
            this->width.readOrUnset(value, SVGLength::PERCENT, 1.0, 1.0);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_HEIGHT:
            this->height.readOrUnset(value, SVGLength::PERCENT, 1.0, 1.0);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_XLINK_HREF: {
            if ( value && this->href && ( strcmp(value, this->href) == 0 ) ) {
                /* No change, do nothing. */
            } else {
                g_free(this->href);
                this->href = NULL;

                if (value) {
                    // First, set the href field, because sp_use_href_changed will need it.
                    this->href = g_strdup(value);

                    // Now do the attaching, which emits the changed signal.
                    try {
                        this->ref->attach(Inkscape::URI(value));
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        this->ref->detach();
                    }
                } else {
                    this->ref->detach();
                }
            }
            break;
        }

        default:
            SPItem::set(key, value);
            break;
    }
}

Inkscape::XML::Node* SPUse::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:use");
    }

    SPItem::write(xml_doc, repr, flags);

    sp_repr_set_svg_double(repr, "x", this->x.computed);
    sp_repr_set_svg_double(repr, "y", this->y.computed);
    repr->setAttribute("width", sp_svg_length_write_with_units(this->width).c_str());
    repr->setAttribute("height", sp_svg_length_write_with_units(this->height).c_str());

    if (this->ref->getURI()) {
        gchar *uri_string = this->ref->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    return repr;
}

Geom::OptRect SPUse::bbox(Geom::Affine const &transform, SPItem::BBoxType bboxtype) {
    Geom::OptRect bbox;

    if (this->child && SP_IS_ITEM(this->child)) {
        SPItem *child = SP_ITEM(this->child);
        Geom::Affine const ct( child->transform
                             * Geom::Translate(this->x.computed,
                                               this->y.computed)
                             * transform );

        bbox = child->bounds(bboxtype, ct);
    }

    return bbox;
}

void SPUse::print(SPPrintContext* ctx) {
    bool translated = false;

    if ((this->x._set && this->x.computed != 0) || (this->y._set && this->y.computed != 0)) {
        Geom::Affine tp(Geom::Translate(this->x.computed, this->y.computed));
        sp_print_bind(ctx, tp, 1.0);
        translated = true;
    }

    if (this->child && SP_IS_ITEM(this->child)) {
        SP_ITEM(this->child)->invoke_print(ctx);
    }

    if (translated) {
        sp_print_release(ctx);
    }
}

const char* SPUse::display_name() {
    if(this->child && SP_IS_SYMBOL( this->child )) {
        return _("Symbol");
    }
    return _("Clone");
}

gchar* SPUse::description() {
    if (this->child) {
        if( SP_IS_SYMBOL( this->child ) ) {
            return g_strdup_printf(_("called %s"), SP_ITEM(this->child)->title());
        }

        static unsigned recursion_depth = 0;

        if (recursion_depth >= 4) {
            /* TRANSLATORS: Used for statusbar description for long <use> chains:
             * "Clone of: Clone of: ... in Layer 1". */
            return g_strdup(_("..."));
            /* We could do better, e.g. chasing the href chain until we reach something other than
             * a <use>, and giving its description. */
        }

        ++recursion_depth;
        char *child_desc = SP_ITEM(this->child)->getDetailedDescription();
        --recursion_depth;

        char *ret = g_strdup_printf(_("of: %s"), child_desc);
        g_free(child_desc);

        return ret;
    } else {
        return g_strdup(_("[orphaned]"));
    }
}

Inkscape::DrawingItem* SPUse::show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags) {
    Inkscape::DrawingGroup *ai = new Inkscape::DrawingGroup(drawing);
    ai->setPickChildren(false);
    ai->setStyle(this->style);

    if (this->child) {
        Inkscape::DrawingItem *ac = SP_ITEM(this->child)->invoke_show(drawing, key, flags);
        if (ac) {
            ai->prependChild(ac);
        }

        Geom::Translate t(this->x.computed, this->y.computed);
        ai->setChildTransform(t);
    }

    return ai;
}

void SPUse::hide(unsigned int key) {
    if (this->child) {
        SP_ITEM(this->child)->invoke_hide(key);
    }

//  SPItem::onHide(key);
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
    while (orig && SP_IS_USE(orig)) {
        orig = SP_USE(orig)->child;
    }
    if (!orig || !SP_IS_ITEM(orig))
        return NULL;
    return SP_ITEM(orig);
}

/**
 * Returns the effective transform that goes from the ultimate original to given SPUse, both ends
 * included.
 */
Geom::Affine
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
    Geom::Affine t(Geom::identity());
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
Geom::Affine
sp_use_get_parent_transform(SPUse *use)
{
    Geom::Affine t(Geom::identity());
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
sp_use_move_compensate(Geom::Affine const *mp, SPItem */*original*/, SPUse *self)
{
    // the clone is orphaned; or this is not a real use, but a clone of another use;
    // we skip it, otherwise duplicate compensation will occur
    if (self->cloned) {
        return;
    }

    // never compensate uses which are used in flowtext
    if (self->parent && SP_IS_FLOWREGION(self->parent)) {
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint mode = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_PARALLEL);
    // user wants no compensation
    if (mode == SP_CLONE_COMPENSATION_NONE)
        return;

    Geom::Affine m(*mp);

    // this is not a simple move, do not try to compensate
    if (!(m.isTranslation()))
        return;

    // restore item->transform field from the repr, in case it was changed by seltrans
    self->readAttr ("transform");

    Geom::Affine t = sp_use_get_parent_transform(self);
    Geom::Affine clone_move = t.inverse() * m * t;

    // calculate the compensation matrix and the advertized movement matrix
    Geom::Affine advertized_move;
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
    self->transform *= clone_move;
    self->doWriteTransform(self->getRepr(), self->transform, &advertized_move);
    self->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_use_href_changed(SPObject */*old_ref*/, SPObject */*ref*/, SPUse *use)
{
    SPItem *item = SP_ITEM(use);

    use->_delete_connection.disconnect();
    use->_transformed_connection.disconnect();

    if (use->child) {
        use->detach(use->child);
        use->child = NULL;
    }

    if (use->href) {
        SPItem *refobj = use->ref->getObject();
        if (refobj) {
            Inkscape::XML::Node *childrepr = refobj->getRepr();

//            GType type = sp_repr_type_lookup(childrepr);
//            g_return_if_fail(type > G_TYPE_NONE);
//            if (g_type_is_a(type, SP_TYPE_ITEM)) {
//                use->child = (SPObject*) g_object_new(type, 0);
//                use->attach(use->child, use->lastChild());
//                sp_object_unref(use->child, use);
//                (use->child)->invoke_build(use->document, childrepr, TRUE);
//
//                for (SPItemView *v = item->display; v != NULL; v = v->next) {
//                    Inkscape::DrawingItem *ai;
//                    ai = SP_ITEM(use->child)->invoke_show(v->arenaitem->drawing(), v->key, v->flags);
//                    if (ai) {
//                        v->arenaitem->prependChild(ai);
//                    }
//                }
//            }

            SPObject* obj = SPFactory::instance().createObject(NodeTraits::get_type_string(*childrepr));
            if (SP_IS_ITEM(obj)) {
                use->child = obj;

                use->attach(use->child, use->lastChild());
                sp_object_unref(use->child, use);
                (use->child)->invoke_build(use->document, childrepr, TRUE);

                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    Inkscape::DrawingItem *ai;
                    ai = SP_ITEM(use->child)->invoke_show(v->arenaitem->drawing(), v->key, v->flags);
                    if (ai) {
                        v->arenaitem->prependChild(ai);
                    }
                }
            } else {
                delete obj;
            }

            use->_delete_connection = refobj->connectDelete(sigc::bind(sigc::ptr_fun(&sp_use_delete_self), use));
            use->_transformed_connection = SP_ITEM(refobj)->connectTransformed(sigc::bind(sigc::ptr_fun(&sp_use_move_compensate), use));
        }
    }
}

static void
sp_use_delete_self(SPObject */*deleted*/, SPUse *self)
{
    // always delete uses which are used in flowtext
    if (self->parent && SP_IS_FLOWREGION(self->parent)) {
        self->deleteObject();
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint const mode = prefs->getInt("/options/cloneorphans/value",
                                               SP_CLONE_ORPHANS_UNLINK);

    if (mode == SP_CLONE_ORPHANS_UNLINK) {
        sp_use_unlink(self);
    } else if (mode == SP_CLONE_ORPHANS_DELETE) {
        self->deleteObject();
    }
}

void SPUse::update(SPCtx *ctx, unsigned flags) {
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx cctx = *ictx;

    SPItem::update(ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      for (SPItemView *v = SP_ITEM(this)->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        g->setStyle(this->style);
      }
    }

    /* Set up child viewport */
    if (this->x.unit == SVGLength::PERCENT) {
        this->x.computed = this->x.value * ictx->viewport.width();
    }

    if (this->y.unit == SVGLength::PERCENT) {
        this->y.computed = this->y.value * ictx->viewport.height();
    }

    if (this->width.unit == SVGLength::PERCENT) {
        this->width.computed = this->width.value * ictx->viewport.width();
    }

    if (this->height.unit == SVGLength::PERCENT) {
        this->height.computed = this->height.value * ictx->viewport.height();
    }

    cctx.viewport = Geom::Rect::from_xywh(0, 0, this->width.computed, this->height.computed);
    cctx.i2vp = Geom::identity();
    flags&=~SP_OBJECT_USER_MODIFIED_FLAG_B;

    if (this->child) {
        sp_object_ref(this->child);

        if (flags || (this->child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(this->child)) {
                SPItem const &chi = *SP_ITEM(this->child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                this->child->updateDisplay((SPCtx *)&cctx, flags);
            } else {
                this->child->updateDisplay(ctx, flags);
            }
        }

        sp_object_unref(this->child);
    }

    /* As last step set additional transform of arena group */
    for (SPItemView *v = this->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        Geom::Affine t(Geom::Translate(this->x.computed, this->y.computed));
        g->setChildTransform(t);
    }
}

void SPUse::modified(unsigned int flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      for (SPItemView *v = SP_ITEM(this)->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        g->setStyle(this->style);
      }
    }

    SPObject *child = this->child;

    if (child) {
        sp_object_ref(child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}

SPItem *sp_use_unlink(SPUse *use)
{
    if (!use) {
        return NULL;
    }

    Inkscape::XML::Node *repr = use->getRepr();
    if (!repr) {
        return NULL;
    }

    Inkscape::XML::Node *parent = repr->parent();
    SPDocument *document = use->document;
    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    // Track the ultimate source of a chain of uses.
    SPItem *orig = sp_use_root(use);

    if (!orig) {
        return NULL;
    }

    // Calculate the accumulated transform, starting from the original.
    Geom::Affine t = sp_use_get_root_transform(use);

    Inkscape::XML::Node *copy = NULL;

    if (SP_IS_SYMBOL(orig)) { // make a group, copy children
        copy = xml_doc->createElement("svg:g");

        for (Inkscape::XML::Node *child = orig->getRepr()->firstChild() ; child != NULL; child = child->next()) {
                Inkscape::XML::Node *newchild = child->duplicate(xml_doc);
                copy->appendChild(newchild);
        }
    } else { // just copy
        copy = orig->getRepr()->duplicate(xml_doc);
    }

    // Add the duplicate repr just after the existing one.
    parent->addChild(copy, repr);

    // Retrieve the SPItem of the resulting repr.
    SPObject *unlinked = document->getObjectByRepr(copy);

    // Merge style from the use.
    SPStyle *unli_sty = unlinked->style;
    SPStyle const *use_sty = use->style;
    sp_style_merge_from_dying_parent(unli_sty, use_sty);
    sp_style_merge_from_parent(unli_sty, unlinked->parent->style);

    unlinked->updateRepr();

    // Hold onto our SPObject and repr for now.
    sp_object_ref(use, NULL);
    Inkscape::GC::anchor(repr);

    // Remove ourselves, not propagating delete events to avoid a
    // chain-reaction with other elements that might reference us.
    use->deleteObject(false);

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
    use->setSuccessor(unlinked);
    sp_object_unref(use, NULL);

    SPItem *item = SP_ITEM(unlinked);
    // Set the accummulated transform.
    {
        Geom::Affine nomove(Geom::identity());
        // Advertise ourselves as not moving.
        item->doWriteTransform(item->getRepr(), t, &nomove);
    }

    return item;
}

SPItem *sp_use_get_original(SPUse *use)
{
    SPItem *ref = NULL;

    if (use) {
        if (use->ref){
            ref = use->ref->getObject();
        }
    }

    return ref;
}

void SPUse::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) {
    SPItem *root = sp_use_root(this);

    if (!root) {
        return;
    }

    root->snappoints(p, snapprefs);
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
