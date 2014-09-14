/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2006 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <cstring>
#include <string>

#include "display/drawing-group.h"
#include "display/curve.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "document.h"
#include "document-undo.h"
#include "style.h"
#include "attributes.h"
#include "sp-item-transform.h"
#include "sp-root.h"
#include "sp-use.h"
#include "sp-offset.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-path.h"
#include "box3d.h"
#include "persp3d.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "selection.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "sp-title.h"
#include "sp-desc.h"
#include "sp-switch.h"
#include "sp-defs.h"
#include "verbs.h"
#include "layer-model.h"
#include "sp-textpath.h"
#include "sp-flowtext.h"

using Inkscape::DocumentUndo;

static void sp_group_perform_patheffect(SPGroup *group, SPGroup *topgroup, bool write);

#include "sp-factory.h"

namespace {
	SPObject* createGroup() {
		return new SPGroup();
	}

	bool groupRegistered = SPFactory::instance().registerObject("svg:g", createGroup);
}

SPGroup::SPGroup() : SPLPEItem() {
    this->_layer_mode = SPGroup::GROUP;
}

SPGroup::~SPGroup() {
}

void SPGroup::build(SPDocument *document, Inkscape::XML::Node *repr) {
    this->readAttr( "inkscape:groupmode" );

    SPLPEItem::build(document, repr);
}

void SPGroup::release() {
    if (this->_layer_mode == SPGroup::LAYER) {
        this->document->removeResource("layer", this);
    }

    SPLPEItem::release();
}

void SPGroup::child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) {
    SPLPEItem::child_added(child, ref);

    SPObject *last_child = this->lastChild();

    if (last_child && last_child->getRepr() == child) {
        // optimization for the common special case where the child is being added at the end
        SPObject *ochild = last_child;
        if ( SP_IS_ITEM(ochild) ) {
            /* TODO: this should be moved into SPItem somehow */
            SPItemView *v;
            Inkscape::DrawingItem *ac;

            for (v = this->display; v != NULL; v = v->next) {
                ac = SP_ITEM (ochild)->invoke_show (v->arenaitem->drawing(), v->key, v->flags);

                if (ac) {
                    v->arenaitem->appendChild(ac);
                }
            }
        }
    } else {    // general case
        SPObject *ochild = this->get_child_by_repr(child);
        if ( ochild && SP_IS_ITEM(ochild) ) {
            /* TODO: this should be moved into SPItem somehow */
            SPItemView *v;
            Inkscape::DrawingItem *ac;

            unsigned position = SP_ITEM(ochild)->pos_in_parent();

            for (v = this->display; v != NULL; v = v->next) {
                ac = SP_ITEM (ochild)->invoke_show (v->arenaitem->drawing(), v->key, v->flags);

                if (ac) {
                    v->arenaitem->prependChild(ac);
                    ac->setZOrder(position);
                }
            }
        }
    }

    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

void SPGroup::remove_child(Inkscape::XML::Node *child) {
    SPLPEItem::remove_child(child);

    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void SPGroup::order_changed (Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref)
{
	SPLPEItem::order_changed(child, old_ref, new_ref);

    SPObject *ochild = this->get_child_by_repr(child);
    if ( ochild && SP_IS_ITEM(ochild) ) {
        /* TODO: this should be moved into SPItem somehow */
        SPItemView *v;
        unsigned position = SP_ITEM(ochild)->pos_in_parent();
        for ( v = SP_ITEM (ochild)->display ; v != NULL ; v = v->next ) {
            v->arenaitem->setZOrder(position);
        }
    }

    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void SPGroup::update(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx, cctx;

    ictx = (SPItemCtx *) ctx;
    cctx = *ictx;

    unsigned childflags = flags;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
      childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = g_slist_reverse(this->childList(true, SPObject::ActionUpdate));
    while (l) {
        SPObject *child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);

        if (childflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM (child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, childflags);
            } else {
                child->updateDisplay(ctx, childflags);
            }
        }

        sp_object_unref(child);
    }

    // For a group, we need to update ourselves *after* updating children.
    // this is because the group might contain shapes such as rect or ellipse,
    // which recompute their equivalent path (a.k.a curve) in the update callback,
    // and this is in turn used when computing bbox.
    SPLPEItem::update(ctx, flags);

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
        for (SPItemView *v = this->display; v != NULL; v = v->next) {
            Inkscape::DrawingGroup *group = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
            group->setStyle(this->style);
        }
    }
}

void SPGroup::modified(guint flags) {
    SPLPEItem::modified(flags);

    SPObject *child;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
    	flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = g_slist_reverse(this->childList(true));

    while (l) {
        child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}

Inkscape::XML::Node* SPGroup::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
	SPGroup* object = this;
    SPGroup *group = SP_GROUP(object);

    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l;

        if (!repr) {
            if (SP_IS_SWITCH(object)) {
                repr = xml_doc->createElement("svg:switch");
            } else {
                repr = xml_doc->createElement("svg:g");
            }
        }

        l = NULL;

        for (SPObject *child = object->firstChild(); child; child = child->getNext() ) {
            if ( !SP_IS_TITLE(child) && !SP_IS_DESC(child) ) {
                Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);

                if (crepr) {
                    l = g_slist_prepend (l, crepr);
                }
            }
        }

        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove (l, l->data);
        }
    } else {
        for (SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
            if ( !SP_IS_TITLE(child) && !SP_IS_DESC(child) ) {
                child->updateRepr(flags);
            }
        }
    }

    if ( flags & SP_OBJECT_WRITE_EXT ) {
        const char *value;
        if ( group->_layer_mode == SPGroup::LAYER ) {
            value = "layer";
        } else if ( group->_layer_mode == SPGroup::MASK_HELPER ) {
            value = "maskhelper";
        } else if ( flags & SP_OBJECT_WRITE_ALL ) {
            value = "group";
        } else {
            value = NULL;
        }

        repr->setAttribute("inkscape:groupmode", value);
    }

    SPLPEItem::write(xml_doc, repr, flags);

    return repr;
}

Geom::OptRect SPGroup::bbox(Geom::Affine const &transform, SPItem::BBoxType bboxtype) const
{
    Geom::OptRect bbox;

    // CPPIFY: replace this const_cast later
    GSList *l = const_cast<SPGroup*>(this)->childList(false, SPObject::ActionBBox);

    while (l) {
        SPObject *o = SP_OBJECT (l->data);

        if (SP_IS_ITEM(o) && !SP_ITEM(o)->isHidden()) {
            SPItem *child = SP_ITEM(o);
            Geom::Affine const ct(child->transform * transform);
            bbox |= child->bounds(bboxtype, ct);
        }

        l = g_slist_remove (l, o);
    }

    return bbox;
}

void SPGroup::print(SPPrintContext *ctx) {
    GSList *l = g_slist_reverse(this->childList(false));

    while (l) {
        SPObject *o = SP_OBJECT (l->data);

        if (SP_IS_ITEM(o)) {
            SP_ITEM(o)->invoke_print (ctx);
        }

        l = g_slist_remove (l, o);
    }
}

const char *SPGroup::displayName() const {
    return _("Group");
}

gchar *SPGroup::description() const {
    gint len = this->getItemCount();
    return g_strdup_printf(
        ngettext(_("of <b>%d</b> object"), _("of <b>%d</b> objects"), len), len);
}

void SPGroup::set(unsigned int key, gchar const* value) {
    switch (key) {
        case SP_ATTR_INKSCAPE_GROUPMODE:
            if ( value && !strcmp(value, "layer") ) {
                this->setLayerMode(SPGroup::LAYER);
            } else if ( value && !strcmp(value, "maskhelper") ) {
                this->setLayerMode(SPGroup::MASK_HELPER);
            } else {
                this->setLayerMode(SPGroup::GROUP);
            }
            break;

        default:
            SPLPEItem::set(key, value);
            break;
    }
}

Inkscape::DrawingItem *SPGroup::show (Inkscape::Drawing &drawing, unsigned int key, unsigned int flags) {
    Inkscape::DrawingGroup *ai;

    ai = new Inkscape::DrawingGroup(drawing);
    ai->setPickChildren(this->effectiveLayerMode(key) == SPGroup::LAYER);
    ai->setStyle(this->style);

    this->_showChildren(drawing, ai, key, flags);
    return ai;
}

void SPGroup::hide (unsigned int key) {
    SPItem * child;

    GSList *l = g_slist_reverse(this->childList(false, SPObject::ActionShow));

    while (l) {
        SPObject *o = SP_OBJECT (l->data);

        if (SP_IS_ITEM (o)) {
            child = SP_ITEM (o);
            child->invoke_hide (key);
        }

        l = g_slist_remove (l, o);
    }

//    SPLPEItem::onHide(key);
}


void SPGroup::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const {
    for ( SPObject const *o = this->firstChild(); o; o = o->getNext() )
    {
        if (SP_IS_ITEM(o)) {
            SP_ITEM(o)->getSnappoints(p, snapprefs);
        }
    }
}


void
sp_item_group_ungroup (SPGroup *group, GSList **children, bool do_done)
{
    g_return_if_fail (group != NULL);
    g_return_if_fail (SP_IS_GROUP (group));

    SPDocument *doc = group->document;
    SPRoot *root = doc->getRoot();
    SPObject *defs = root->defs;

    SPItem *gitem = group;
    Inkscape::XML::Node *grepr = gitem->getRepr();

    g_return_if_fail (!strcmp (grepr->name(), "svg:g")
                   || !strcmp (grepr->name(), "svg:a")
                   || !strcmp (grepr->name(), "svg:switch")
                   || !strcmp (grepr->name(), "svg:svg"));

    // this converts the gradient/pattern fill/stroke on the group, if any, to userSpaceOnUse
    gitem->adjust_paint_recursive (Geom::identity(), Geom::identity(), false);

    SPItem *pitem = SP_ITEM(gitem->parent);
    Inkscape::XML::Node *prepr = pitem->getRepr();

	if (SP_IS_BOX3D(gitem)) {
		group = box3d_convert_to_group(SP_BOX3D(gitem));
		gitem = group;
	}

    SP_LPE_ITEM(group)->removeAllPathEffects(false);

    /* Step 1 - generate lists of children objects */
    GSList *items = NULL;
    GSList *objects = NULL;
    for (SPObject *child = group->firstChild() ; child; child = child->getNext() ) {

        if (SP_IS_ITEM (child)) {

            SPItem *citem = SP_ITEM (child);

            /* Merging of style */
            // this converts the gradient/pattern fill/stroke, if any, to userSpaceOnUse; we need to do
            // it here _before_ the new transform is set, so as to use the pre-transform bbox
            citem->adjust_paint_recursive (Geom::identity(), Geom::identity(), false);

            sp_style_merge_from_dying_parent(child->style, gitem->style);
            /*
             * fixme: We currently make no allowance for the case where child is cloned
             * and the group has any style settings.
             *
             * (This should never occur with documents created solely with the current
             * version of inkscape without using the XML editor: we usually apply group
             * style changes to children rather than to the group itself.)
             *
             * If the group has no style settings, then
             * sp_style_merge_from_dying_parent should be a no-op.  Otherwise (i.e. if
             * we change the child's style to compensate for its parent going away)
             * then those changes will typically be reflected in any clones of child,
             * whereas we'd prefer for Ungroup not to affect the visual appearance.
             *
             * The only way of preserving styling appearance in general is for child to
             * be put into a new group -- a somewhat surprising response to an Ungroup
             * command.  We could add a new groupmode:transparent that would mostly
             * hide the existence of such groups from the user (i.e. editing behaves as
             * if the transparent group's children weren't in a group), though that's
             * extra complication & maintenance burden and this case is rare.
             */

            child->updateRepr();

            Inkscape::XML::Node *nrepr = child->getRepr()->duplicate(prepr->document());

            // Merging transform
            Geom::Affine ctrans;
            Geom::Affine const g(gitem->transform);
            if (SP_IS_USE(citem) && SP_USE(citem)->get_original() &&
                SP_USE(citem)->get_original()->parent == SP_OBJECT(group)) {
                // make sure a clone's effective transform is the same as was under group
                ctrans = g.inverse() * citem->transform * g;
            } else {
                // We should not apply the group's transformation to both a linked offset AND to its source
                if (SP_IS_OFFSET(citem)) { // Do we have an offset at hand (whether it's dynamic or linked)?
                    SPItem *source = sp_offset_get_source(SP_OFFSET(citem));
                    // When dealing with a chain of linked offsets, the transformation of an offset will be
                    // tied to the transformation of the top-most source, not to any of the intermediate
                    // offsets. So let's find the top-most source
                    while (source != NULL && SP_IS_OFFSET(source)) {
                        source = sp_offset_get_source(SP_OFFSET(source));
                    }
                    if (source != NULL && // If true then we must be dealing with a linked offset ...
                        group->isAncestorOf(source) == false) { // ... of which the source is not in the same group
                        ctrans = citem->transform * g; // then we should apply the transformation of the group to the offset
                    } else {
                        ctrans = citem->transform;
                    }
                } else {
                    ctrans = citem->transform * g;
                }
            }

            // FIXME: constructing a transform that would fully preserve the appearance of a
            // textpath if it is ungrouped with its path seems to be impossible in general
            // case. E.g. if the group was squeezed, to keep the ungrouped textpath squeezed
            // as well, we'll need to relink it to some "virtual" path which is inversely
            // stretched relative to the actual path, and then squeeze the textpath back so it
            // would both fit the actual path _and_ be squeezed as before. It's a bummer.

            // This is just a way to temporarily remember the transform in repr. When repr is
            // reattached outside of the group, the transform will be written more properly
            // (i.e. optimized into the object if the corresponding preference is set)
            gchar *affinestr=sp_svg_transform_write(ctrans);
            nrepr->setAttribute("transform", affinestr);
            g_free(affinestr);

            items = g_slist_prepend (items, nrepr);

        } else {
            Inkscape::XML::Node *nrepr = child->getRepr()->duplicate(prepr->document());
            objects = g_slist_prepend (objects, nrepr);
        }
    }

    /* Step 2 - clear group */
    // remember the position of the group
    gint pos = group->getRepr()->position();

    // the group is leaving forever, no heir, clones should take note; its children however are going to reemerge
    group->deleteObject(true, false);

    /* Step 3 - add nonitems */
    if (objects) {
        Inkscape::XML::Node *last_def = defs->getRepr()->lastChild();
        while (objects) {
            Inkscape::XML::Node *repr = (Inkscape::XML::Node *) objects->data;
            if (!sp_repr_is_meta_element(repr)) {
                defs->getRepr()->addChild(repr, last_def);
            }
            Inkscape::GC::release(repr);
            objects = g_slist_remove (objects, objects->data);
        }
    }

    /* Step 4 - add items */
    while (items) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) items->data;
        // add item
        prepr->appendChild(repr);
        // restore position; since the items list was prepended (i.e. reverse), we now add
        // all children at the same pos, which inverts the order once again
        repr->setPosition(pos > 0 ? pos : 0);

        // fill in the children list if non-null
        SPItem *item = static_cast<SPItem *>(doc->getObjectByRepr(repr));

        item->doWriteTransform(repr, item->transform, NULL, false);

        Inkscape::GC::release(repr);
        if (children && SP_IS_ITEM(item)) {
            *children = g_slist_prepend(*children, item);
        }

        items = g_slist_remove (items, items->data);
    }

    if (do_done) {
        DocumentUndo::done(doc, SP_VERB_NONE, _("Ungroup"));
    }
}

/*
 * some API for list aspect of SPGroup
 */

GSList *sp_item_group_item_list(SPGroup * group)
{
    g_return_val_if_fail(group != NULL, NULL);
    g_return_val_if_fail(SP_IS_GROUP(group), NULL);

    GSList *s = NULL;

    for (SPObject *o = group->firstChild() ; o ; o = o->getNext() ) {
        if ( SP_IS_ITEM(o) ) {
            s = g_slist_prepend(s, o);
        }
    }

    return g_slist_reverse (s);
}

SPObject *sp_item_group_get_child_by_name(SPGroup *group, SPObject *ref, const gchar *name)
{
    SPObject *child = (ref) ? ref->getNext() : group->firstChild();
    while ( child && strcmp(child->getRepr()->name(), name) ) {
        child = child->getNext();
    }
    return child;
}

void SPGroup::setLayerMode(LayerMode mode) {
    if ( _layer_mode != mode ) {
        if ( mode == LAYER ) {
            this->document->addResource("layer", this);
        } else if ( _layer_mode == LAYER ) {
            this->document->removeResource("layer", this);
        }
        _layer_mode = mode;
        _updateLayerMode();
    }
}

SPGroup::LayerMode SPGroup::layerDisplayMode(unsigned int dkey) const {
    std::map<unsigned int, LayerMode>::const_iterator iter;
    iter = _display_modes.find(dkey);
    if ( iter != _display_modes.end() ) {
        return (*iter).second;
    } else {
        return GROUP;
    }
}

void SPGroup::setLayerDisplayMode(unsigned int dkey, SPGroup::LayerMode mode) {
    if ( layerDisplayMode(dkey) != mode ) {
        _display_modes[dkey] = mode;
        _updateLayerMode(dkey);
    }
}

void SPGroup::_updateLayerMode(unsigned int display_key) {
    SPItemView *view;
    for ( view = this->display ; view ; view = view->next ) {
        if ( !display_key || view->key == display_key ) {
            Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(view->arenaitem);
            if (g) {
                g->setPickChildren(effectiveLayerMode(view->key) == SPGroup::LAYER);
            }
        }
    }
}

void SPGroup::translateChildItems(Geom::Translate const &tr)
{
    if ( hasChildren() ) {
        for (SPObject *o = firstChild() ; o ; o = o->getNext() ) {
            if ( SP_IS_ITEM(o) ) {
                sp_item_move_rel(reinterpret_cast<SPItem *>(o), tr);
            }
        }
    }
}

// Recursively (or not) scale child items around a point
void SPGroup::scaleChildItemsRec(Geom::Scale const &sc, Geom::Point const &p, bool noRecurse)
{
    if ( hasChildren() ) {
        for (SPObject *o = firstChild() ; o ; o = o->getNext() ) {
            if ( SP_IS_ITEM(o) ) {
                if (SP_IS_GROUP(o) && !SP_IS_BOX3D(o)) {
                    /* Using recursion breaks clipping because transforms are applied 
                       in coordinates for draws but nothing in defs is changed
                       instead change the transform on the entire group, and the transform
                       is applied after any references to clipping paths.  However NOT using
                       recursion apparently breaks as of r13544 other parts of Inkscape
                       involved with showing/modifying units.  So offer both for use
                       in different contexts.
                    */
                    if(noRecurse) {
                        // used for EMF import
                        SPItem *item = SP_ITEM(o);
                        Geom::Translate const s(p);
                        Geom::Affine final = s.inverse() * sc * s;
                        Geom::Affine tAff = item->i2dt_affine() * final;
                        item->set_i2d_affine(tAff);
                        tAff = item->transform;
                        // Eliminate common rounding error affecting EMF/WMF input.
                        // When the rounding error persists it converts the simple 
                        //    transform=scale() to transform=matrix().
                        if(std::abs(tAff[4]) < 1.0e-5 && std::abs(tAff[5]) < 1.0e-5){
                           tAff[4] = 0.0;
                           tAff[5] = 0.0;
                        }
                        item->doWriteTransform(item->getRepr(), tAff, NULL, true);
                    } else {
                        // used for other import
                        SP_GROUP(o)->scaleChildItemsRec(sc, p, false);
                    }
                } else {
                    SPItem *item = SP_ITEM(o);
                    Geom::OptRect bbox = item->desktopVisualBounds();
                    if (bbox) {
                        // Scale item
                        Geom::Translate const s(p);
                        Geom::Affine final = s.inverse() * sc * s;
                        
                        gchar const *conn_type = NULL;
                        if (SP_IS_TEXT_TEXTPATH(item)) {
                            SP_TEXT(item)->optimizeTextpathText();
                        } else if (SP_IS_FLOWTEXT(item)) {
                            SP_FLOWTEXT(item)->optimizeScaledText();
                        } else if (SP_IS_BOX3D(item)) {
                            // Force recalculation from perspective
                            box3d_position_set(SP_BOX3D(item));
                        } else if (item->getAttribute("inkscape:connector-type") != NULL
                            && (item->getAttribute("inkscape:connection-start") == NULL
                                || item->getAttribute("inkscape:connection-end") == NULL)) {
                            // Remove and store connector type for transform if disconnected
                            conn_type = item->getAttribute("inkscape:connector-type");
                            item->removeAttribute("inkscape:connector-type");
                        }
                        
                        if (SP_IS_PERSP3D(item)) {
                            persp3d_apply_affine_transformation(SP_PERSP3D(item), final);
                        } else if ((SP_IS_TEXT_TEXTPATH(item) || SP_IS_FLOWTEXT(item)) && !item->transform.isIdentity()) {
                            // Save and reset current transform
                            Geom::Affine tmp(item->transform);
                            item->transform = Geom::Affine();
                            // Apply scale
                            item->set_i2d_affine(item->i2dt_affine() * sc);
                            item->doWriteTransform(item->getRepr(), item->transform, NULL, true);
                            // Scale translation and restore original transform
                            tmp[4] *= sc[0];
                            tmp[5] *= sc[1];
                            item->doWriteTransform(item->getRepr(), tmp, NULL, true);
                        } else if (SP_IS_USE(item)) {
                            // calculate the matrix we need to apply to the clone
                            // to cancel its induced transform from its original
                            Geom::Affine move = final.inverse() * item->transform * final;
                            item->doWriteTransform(item->getRepr(), move, &move, true);
                        } else {
                            item->set_i2d_affine(item->i2dt_affine() * final);
                            item->doWriteTransform(item->getRepr(), item->transform, NULL, true);
                        }
                        
                        if (conn_type != NULL) {
                            item->setAttribute("inkscape:connector-type", conn_type);
                        }
                        
                        if (item->isCenterSet() && !(final.isTranslation() || final.isIdentity())) {
                            item->scaleCenter(sc); // All coordinates have been scaled, so also the center must be scaled
                            item->updateRepr();
                        }
                    }
                }
            }
        }
    }
}

gint SPGroup::getItemCount() const {
    gint len = 0;
    for (SPObject const *o = this->firstChild() ; o ; o = o->getNext() ) {
        if (SP_IS_ITEM(o)) {
            len++;
        }
    }

    return len;
}

void SPGroup::_showChildren (Inkscape::Drawing &drawing, Inkscape::DrawingItem *ai, unsigned int key, unsigned int flags) {
    Inkscape::DrawingItem *ac = NULL;
    SPItem * child = NULL;
    GSList *l = g_slist_reverse(this->childList(false, SPObject::ActionShow));
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM (o)) {
            child = SP_ITEM (o);
            ac = child->invoke_show (drawing, key, flags);
            if (ac) {
                ai->appendChild(ac);
            }
        }
        l = g_slist_remove (l, o);
    }
}

void SPGroup::update_patheffect(bool write) {
#ifdef GROUP_VERBOSE
    g_message("sp_group_update_patheffect: %p\n", lpeitem);
#endif

    GSList const *item_list = sp_item_group_item_list(this);

    for ( GSList const *iter = item_list; iter; iter = iter->next ) {
        SPObject *subitem = static_cast<SPObject *>(iter->data);

        if (SP_IS_LPE_ITEM(subitem)) {
        	((SPLPEItem*)subitem)->update_patheffect(write);
        }
    }

    if (hasPathEffect() && pathEffectsEnabled()) {
        for (PathEffectList::iterator it = this->path_effect_list->begin(); it != this->path_effect_list->end(); it++)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;

            if (lpeobj && lpeobj->get_lpe()) {
                lpeobj->get_lpe()->doBeforeEffect(this);
            }
        }

        sp_group_perform_patheffect(this, this, write);
    }
}

static void
sp_group_perform_patheffect(SPGroup *group, SPGroup *topgroup, bool write)
{
    GSList const *item_list = sp_item_group_item_list(SP_GROUP(group));

    for ( GSList const *iter = item_list; iter; iter = iter->next ) {
        SPObject *subitem = static_cast<SPObject *>(iter->data);

        if (SP_IS_GROUP(subitem)) {
            sp_group_perform_patheffect(SP_GROUP(subitem), topgroup, write);
        } else if (SP_IS_SHAPE(subitem)) {
            SPCurve * c = NULL;

            if (SP_IS_PATH(subitem)) {
                c = SP_PATH(subitem)->get_original_curve();
            } else {
                c = SP_SHAPE(subitem)->getCurve();
            }

            // only run LPEs when the shape has a curve defined
            if (c) {
                c->transform(i2anc_affine(subitem, topgroup));
                SP_LPE_ITEM(topgroup)->performPathEffect(c);
                c->transform(i2anc_affine(subitem, topgroup).inverse());
                SP_SHAPE(subitem)->setCurve(c, TRUE);

                if (write) {
                    Inkscape::XML::Node *repr = subitem->getRepr();
                    gchar *str = sp_svg_write_path(c->get_pathvector());
                    repr->setAttribute("d", str);
#ifdef GROUP_VERBOSE
g_message("sp_group_perform_patheffect writes 'd' attribute");
#endif
                    g_free(str);
                }

                c->unref();
            }
        }
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
