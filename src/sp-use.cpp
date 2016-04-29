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

#include <cstring>
#include <string>

#include <2geom/transforms.h>
#include <glibmm/i18n.h>
#include <glibmm/markup.h>
#include "display/drawing-group.h"
#include "attributes.h"
#include "document.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-factory.h"
#include "sp-flowregion.h"
#include "uri.h"
#include "print.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "preferences.h"
#include "style.h"
#include "sp-symbol.h"
#include "sp-root.h"
#include "sp-use.h"
#include "sp-use-reference.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "sp-flowtext.h"

SPUse::SPUse()
    : SPItem(),
      child(NULL),
      x(),
      y(),
      width(),
      height(),
      href(NULL),
      ref(new SPUseReference(this)),
      _delete_connection(),
      _changed_connection(),
      _transformed_connection()
{
    this->x.unset();
    this->y.unset();
    this->width.unset(SVGLength::PERCENT, 1.0, 1.0);
    this->height.unset(SVGLength::PERCENT, 1.0, 1.0);

    this->_changed_connection = this->ref->changedSignal().connect(
        sigc::hide(sigc::hide(sigc::mem_fun(this, &SPUse::href_changed)))
    );
}

SPUse::~SPUse() {
    if (this->child) {
        this->detach(this->child);
        this->child = NULL;
    }

    this->ref->detach();
    delete this->ref;
    this->ref = 0;
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
    // which will call SPUse::href_changed, and that will take care of the child
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
                    // First, set the href field, because SPUse::href_changed will need it.
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

    SPShape *shape = dynamic_cast<SPShape *>(child);
    if (shape) {
        shape->set_shape(); // evaluate SPCurve of child
    } else {
        SPText *text = dynamic_cast<SPText *>(child);
        if (text) {
            text->rebuildLayout(); // refresh Layout, LP Bug 1339305
        } else {
            SPFlowtext *flowtext = dynamic_cast<SPFlowtext *>(child);
            if (flowtext) {
                SPFlowregion *flowregion = dynamic_cast<SPFlowregion *>(flowtext->firstChild());
                if (flowregion) {
                    flowregion->UpdateComputed();
                }
                flowtext->rebuildLayout();
            }
        }
    }

    return repr;
}

Geom::OptRect SPUse::bbox(Geom::Affine const &transform, SPItem::BBoxType bboxtype) const {
    Geom::OptRect bbox;

    if (this->child) {
        Geom::Affine const ct(child->transform * Geom::Translate(this->x.computed, this->y.computed) * transform );

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

    if (this->child) {
        this->child->invoke_print(ctx);
    }

    if (translated) {
        sp_print_release(ctx);
    }
}

const char* SPUse::displayName() const {
    if (dynamic_cast<SPSymbol *>(child)) {
        return _("Symbol");
    } else {
        return _("Clone");
    }
}

gchar* SPUse::description() const {
    if (child) {
        if ( dynamic_cast<SPSymbol *>(child) ) {
            if (child->title()) {
                return g_strdup_printf(_("called %s"), Glib::Markup::escape_text(Glib::ustring( g_dpgettext2(NULL, "Symbol", child->title()))).c_str());
            } else if (child->getAttribute("id")) {
                return g_strdup_printf(_("called %s"), Glib::Markup::escape_text(Glib::ustring( g_dpgettext2(NULL, "Symbol", child->getAttribute("id")))).c_str());
            } else {
                return g_strdup_printf(_("called %s"), _("Unnamed Symbol"));
            }
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
        char *child_desc = this->child->detailedDescription();
        --recursion_depth;

        char *ret = g_strdup_printf(_("of: %s"), child_desc);
        g_free(child_desc);

        return ret;
    } else {
        return g_strdup(_("[orphaned]"));
    }
}

Inkscape::DrawingItem* SPUse::show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags) {

    // std::cout << "SPUse::show: " << (getId()?getId():"null") << std::endl;
    Inkscape::DrawingGroup *ai = new Inkscape::DrawingGroup(drawing);
    ai->setPickChildren(false);
    this->context_style = this->style;
    ai->setStyle(this->style, this->context_style);
    
    if (this->child) {
        Inkscape::DrawingItem *ac = this->child->invoke_show(drawing, key, flags);

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
        this->child->invoke_hide(key);
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
SPItem *SPUse::root() {
    SPItem *orig = this->child;

    SPUse *use = dynamic_cast<SPUse *>(orig);
    while (orig && use) {
        orig = use->child;
        use = dynamic_cast<SPUse *>(orig);
    }

    return orig;
}

SPItem const *SPUse::root() const {
	return const_cast<SPUse*>(this)->root();
}

/**
 * Get the number of dereferences or calls to get_original() needed to get an object
 * which is not an svg:use. Returns -1 if there is no original object.
 */
int SPUse::cloneDepth() const {
    unsigned depth = 1;
    SPItem *orig = this->child;

    while (orig && dynamic_cast<SPUse *>(orig)) {
        ++depth;
        orig = dynamic_cast<SPUse *>(orig)->child;
    }

    if (!orig) {
        return -1;
    } else {
        return depth;
    }
}

/**
 * Returns the effective transform that goes from the ultimate original to given SPUse, both ends
 * included.
 */
Geom::Affine SPUse::get_root_transform() {
    //track the ultimate source of a chain of uses
    SPObject *orig = this->child;

    GSList *chain = NULL;
    chain = g_slist_prepend(chain, this);

    while (dynamic_cast<SPUse *>(orig)) {
        chain = g_slist_prepend(chain, orig);
        orig = dynamic_cast<SPUse *>(orig)->child;
    }

    chain = g_slist_prepend(chain, orig);


    // calculate the accummulated transform, starting from the original
    Geom::Affine t(Geom::identity());

    for (GSList *i = chain; i != NULL; i = i->next) {
        SPItem *i_tem = reinterpret_cast<SPItem *>(i->data);

        // "An additional transformation translate(x,y) is appended to the end (i.e.,
        // right-side) of the transform attribute on the generated 'g', where x and y
        // represent the values of the x and y attributes on the 'use' element." - http://www.w3.org/TR/SVG11/struct.html#UseElement
        SPUse *i_use = dynamic_cast<SPUse *>(i_tem);
        if (i_use) {
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
Geom::Affine SPUse::get_parent_transform() {
    Geom::Affine t(Geom::identity());

    if ((this->x._set && this->x.computed != 0) || (this->y._set && this->y.computed != 0)) {
        t *= Geom::Translate(this->x._set ? this->x.computed : 0, this->y._set ? this->y.computed : 0);
    }

    t *= this->transform;
    return t;
}

/**
 * Sensing a movement of the original, this function attempts to compensate for it in such a way
 * that the clone stays unmoved or moves in parallel (depending on user setting) regardless of the
 * clone's transform.
 */
void SPUse::move_compensate(Geom::Affine const *mp) {
    // the clone is orphaned; or this is not a real use, but a clone of another use;
    // we skip it, otherwise duplicate compensation will occur
    if (this->cloned) {
        return;
    }

    // never compensate uses which are used in flowtext
    if (parent && dynamic_cast<SPFlowregion *>(parent)) {
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint mode = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_PARALLEL);
    // user wants no compensation
    if (mode == SP_CLONE_COMPENSATION_NONE)
        return;

    Geom::Affine m(*mp);
    Geom::Affine t = this->get_parent_transform();
    Geom::Affine clone_move = t.inverse() * m * t;

    // this is not a simple move, do not try to compensate
    if (!(m.isTranslation())){
    	//BUT move clippaths accordingly.
        //if clone has a clippath, move it accordingly
        if(clip_ref->getObject()){
            SPObject *clip = clip_ref->getObject()->firstChild() ;
            while(clip){
            	SPItem *item = (SPItem*) clip;
            	if(item){
                    item->transform *= m;
                    Geom::Affine identity;
                    item->doWriteTransform(clip->getRepr(),item->transform, &identity);
            	}
                clip = clip->getNext();
            }
        }
        if(mask_ref->getObject()){
            SPObject *mask = mask_ref->getObject()->firstChild() ;
            while(mask){
            	SPItem *item = (SPItem*) mask;
            	if(item){
                    item->transform *= m;
                    Geom::Affine identity;
                    item->doWriteTransform(mask->getRepr(),item->transform, &identity);
            	}
                mask = mask->getNext();
            }
        }
        return;
    }

    // restore item->transform field from the repr, in case it was changed by seltrans
    this->readAttr ("transform");


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

    //if clone has a clippath, move it accordingly
    if(clip_ref->getObject()){
        SPObject *clip = clip_ref->getObject()->firstChild() ;
        while(clip){
        	SPItem *item = (SPItem*) clip;
        	if(item){
                item->transform *= clone_move.inverse();
                Geom::Affine identity;
                item->doWriteTransform(clip->getRepr(),item->transform, &identity);
        	}
            clip = clip->getNext();
        }
    }
    if(mask_ref->getObject()){
        SPObject *mask = mask_ref->getObject()->firstChild() ;
        while(mask){
        	SPItem *item = (SPItem*) mask;
        	if(item){
                item->transform *= clone_move.inverse();
                Geom::Affine identity;
                item->doWriteTransform(mask->getRepr(),item->transform, &identity);
        	}
            mask = mask->getNext();
        }
    }


    // commit the compensation
    this->transform *= clone_move;
    this->doWriteTransform(this->getRepr(), this->transform, &advertized_move);
    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void SPUse::href_changed() {
    this->_delete_connection.disconnect();
    this->_transformed_connection.disconnect();

    if (this->child) {
        this->detach(this->child);
        this->child = NULL;
    }

    if (this->href) {
        SPItem *refobj = this->ref->getObject();

        if (refobj) {
            Inkscape::XML::Node *childrepr = refobj->getRepr();

            SPObject* obj = SPFactory::createObject(NodeTraits::get_type_string(*childrepr));

            SPItem *item = dynamic_cast<SPItem *>(obj);
            if (item) {
                child = item;

                this->attach(this->child, this->lastChild());
                sp_object_unref(this->child, this);

                this->child->invoke_build(this->document, childrepr, TRUE);

                for (SPItemView *v = this->display; v != NULL; v = v->next) {
                    Inkscape::DrawingItem *ai = this->child->invoke_show(v->arenaitem->drawing(), v->key, v->flags);

                    if (ai) {
                        v->arenaitem->prependChild(ai);
                    }
                }
            } else {
                delete obj;
                g_warning("Tried to create svg:use from invalid object");
            }

            this->_delete_connection = refobj->connectDelete(
                sigc::hide(sigc::mem_fun(this, &SPUse::delete_self))
            );

            this->_transformed_connection = refobj->connectTransformed(
                sigc::hide(sigc::mem_fun(this, &SPUse::move_compensate))
            );
        }
    }
}

void SPUse::delete_self() {
    // always delete uses which are used in flowtext
    if (parent && dynamic_cast<SPFlowregion *>(parent)) {
        deleteObject();
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint const mode = prefs->getInt("/options/cloneorphans/value",
                                               SP_CLONE_ORPHANS_UNLINK);

    if (mode == SP_CLONE_ORPHANS_UNLINK) {
        this->unlink();
    } else if (mode == SP_CLONE_ORPHANS_DELETE) {
        this->deleteObject();
    }
}

void SPUse::update(SPCtx *ctx, unsigned flags) {
    // std::cout << "SPUse::update: " << (getId()?getId():"null") << std::endl;
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx cctx = *ictx;

    unsigned childflags = flags;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    childflags &= SP_OBJECT_MODIFIED_CASCADE;

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

    childflags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B;

    if (this->child) {
        sp_object_ref(this->child);

        // viewport is only changed if referencing a symbol or svg element
        if( SP_IS_SYMBOL(this->child) || SP_IS_ROOT(this->child) ) {
            cctx.viewport = Geom::Rect::from_xywh(0, 0, this->width.computed, this->height.computed);
            cctx.i2vp = Geom::identity();
        }
        
        if (childflags || (this->child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            SPItem const *chi = dynamic_cast<SPItem const *>(child);
            g_assert(chi != NULL);
            cctx.i2doc = chi->transform * ictx->i2doc;
            cctx.i2vp = chi->transform * ictx->i2vp;
            this->child->updateDisplay((SPCtx *)&cctx, childflags);
        }

        sp_object_unref(this->child);
    }

    SPItem::update(ctx, flags);

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
        for (SPItemView *v = this->display; v != NULL; v = v->next) {
            Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
            this->context_style = this->style;
            g->setStyle(this->style, this->context_style);
        }
    }

    /* As last step set additional transform of arena group */
    for (SPItemView *v = this->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        Geom::Affine t(Geom::Translate(this->x.computed, this->y.computed));
        g->setChildTransform(t);
    }
}

void SPUse::modified(unsigned int flags) {
    // std::cout << "SPUse::modified: " << (getId()?getId():"null") << std::endl;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      for (SPItemView *v = this->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        this->context_style = this->style;
        g->setStyle(this->style, this->context_style);
      }
    }

    if (child) {
        sp_object_ref(child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}

SPItem *SPUse::unlink() {
    Inkscape::XML::Node *repr = this->getRepr();

    if (!repr) {
        return NULL;
    }

    Inkscape::XML::Node *parent = repr->parent();
    SPDocument *document = this->document;
    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    // Track the ultimate source of a chain of uses.
    SPItem *orig = this->root();

    if (!orig) {
        return NULL;
    }

    // Calculate the accumulated transform, starting from the original.
    Geom::Affine t = this->get_root_transform();

    Inkscape::XML::Node *copy = NULL;

    if (dynamic_cast<SPSymbol *>(orig)) { // make a group, copy children
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
    unlinked->style->merge( this->style );
    unlinked->style->cascade( unlinked->parent->style );
    unlinked->updateRepr();

    // Hold onto our SPObject and repr for now.
    sp_object_ref(this, NULL);
    Inkscape::GC::anchor(repr);

    // Remove ourselves, not propagating delete events to avoid a
    // chain-reaction with other elements that might reference us.
    this->deleteObject(false);

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
    this->setSuccessor(unlinked);
    sp_object_unref(this, NULL);

    SPItem *item = dynamic_cast<SPItem *>(unlinked);
    g_assert(item != NULL);

    // Set the accummulated transform.
    {
        Geom::Affine nomove(Geom::identity());
        // Advertise ourselves as not moving.
        item->doWriteTransform(item->getRepr(), t, &nomove);
    }

    return item;
}

SPItem *SPUse::get_original() {
    SPItem *ref = NULL;

        if (this->ref){
            ref = this->ref->getObject();
        }

    return ref;
}

void SPUse::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const {
    SPItem const *root = this->root();

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
