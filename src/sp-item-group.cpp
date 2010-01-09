#define __SP_GROUP_C__

/*
 * SVG <g> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
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

#include "display/nr-arena-group.h"
#include "display/curve.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-matrix-fns.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "document.h"
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

static void sp_group_class_init (SPGroupClass *klass);
static void sp_group_init (SPGroup *group);
static void sp_group_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_group_release(SPObject *object);
static void sp_group_dispose(GObject *object);

static void sp_group_child_added (SPObject * object, Inkscape::XML::Node * child, Inkscape::XML::Node * ref);
static void sp_group_remove_child (SPObject * object, Inkscape::XML::Node * child);
static void sp_group_order_changed (SPObject * object, Inkscape::XML::Node * child, Inkscape::XML::Node * old_ref, Inkscape::XML::Node * new_ref);
static void sp_group_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_group_modified (SPObject *object, guint flags);
static Inkscape::XML::Node *sp_group_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_group_set(SPObject *object, unsigned key, char const *value);

static void sp_group_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static void sp_group_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_group_description (SPItem * item);
static NRArenaItem *sp_group_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_group_hide (SPItem * item, unsigned int key);
static void sp_group_snappoints (SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs);

static void sp_group_update_patheffect(SPLPEItem *lpeitem, bool write);
static void sp_group_perform_patheffect(SPGroup *group, SPGroup *topgroup, bool write);

static SPLPEItemClass * parent_class;

GType
sp_group_get_type (void)
{
    static GType group_type = 0;
    if (!group_type) {
        GTypeInfo group_info = {
            sizeof (SPGroupClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_group_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof (SPGroup),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_group_init,
            NULL,	/* value_table */
        };
        group_type = g_type_register_static (SP_TYPE_LPE_ITEM, "SPGroup", &group_info, (GTypeFlags)0);
    }
    return group_type;
}

static void
sp_group_class_init (SPGroupClass *klass)
{
    GObjectClass * object_class;
    SPObjectClass * sp_object_class;
    SPItemClass * item_class;
    SPLPEItemClass * lpe_item_class;

    object_class = (GObjectClass *) klass;
    sp_object_class = (SPObjectClass *) klass;
    item_class = (SPItemClass *) klass;
    lpe_item_class = (SPLPEItemClass *) klass;

    parent_class = (SPLPEItemClass *)g_type_class_ref (SP_TYPE_LPE_ITEM);

    object_class->dispose = sp_group_dispose;

    sp_object_class->child_added = sp_group_child_added;
    sp_object_class->remove_child = sp_group_remove_child;
    sp_object_class->order_changed = sp_group_order_changed;
    sp_object_class->update = sp_group_update;
    sp_object_class->modified = sp_group_modified;
    sp_object_class->set = sp_group_set;
    sp_object_class->write = sp_group_write;
    sp_object_class->release = sp_group_release;
    sp_object_class->build = sp_group_build;

    item_class->bbox = sp_group_bbox;
    item_class->print = sp_group_print;
    item_class->description = sp_group_description;
    item_class->show = sp_group_show;
    item_class->hide = sp_group_hide;
    item_class->snappoints = sp_group_snappoints;

    lpe_item_class->update_patheffect = sp_group_update_patheffect;
}

static void
sp_group_init (SPGroup *group)
{
    group->_layer_mode = SPGroup::GROUP;
    group->group = new CGroup(group);
    new (&group->_display_modes) std::map<unsigned int, SPGroup::LayerMode>();
}

static void sp_group_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    sp_object_read_attr(object, "inkscape:groupmode");

    if (((SPObjectClass *)parent_class)->build) {
        ((SPObjectClass *)parent_class)->build(object, document, repr);
    }
}

static void sp_group_release(SPObject *object) {
    if ( SP_GROUP(object)->_layer_mode == SPGroup::LAYER ) {
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "layer", object);
    }
    if (((SPObjectClass *)parent_class)->release) {
        ((SPObjectClass *)parent_class)->release(object);
    }
}

static void
sp_group_dispose(GObject *object)
{
    SP_GROUP(object)->_display_modes.~map();
    delete SP_GROUP(object)->group;
}

static void
sp_group_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPItem *item;

    item = SP_ITEM (object);

    if (((SPObjectClass *) (parent_class))->child_added)
        (* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);

    SP_GROUP(object)->group->onChildAdded(child);
}

/* fixme: hide (Lauris) */

static void
sp_group_remove_child (SPObject * object, Inkscape::XML::Node * child)
{
    if (((SPObjectClass *) (parent_class))->remove_child)
        (* ((SPObjectClass *) (parent_class))->remove_child) (object, child);

    SP_GROUP(object)->group->onChildRemoved(child);
}

static void
sp_group_order_changed (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref)
{
    if (((SPObjectClass *) (parent_class))->order_changed)
        (* ((SPObjectClass *) (parent_class))->order_changed) (object, child, old_ref, new_ref);

    SP_GROUP(object)->group->onOrderChanged(child, old_ref, new_ref);
}

static void
sp_group_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update (object, ctx, flags);

    SP_GROUP(object)->group->onUpdate(ctx, flags);
}

static void
sp_group_modified (SPObject *object, guint flags)
{
    if (((SPObjectClass *) (parent_class))->modified)
        ((SPObjectClass *) (parent_class))->modified (object, flags);

    SP_GROUP(object)->group->onModified(flags);
}

static Inkscape::XML::Node *
sp_group_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPGroup *group;
    SPObject *child;
    Inkscape::XML::Node *crepr;

    group = SP_GROUP (object);

    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l;
        if (!repr) {
            if (SP_IS_SWITCH(object))
                repr = xml_doc->createElement("svg:switch");
            else
                repr = xml_doc->createElement("svg:g");
        }
        l = NULL;
        for (child = sp_object_first_child(object); child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_TITLE(child) || SP_IS_DESC(child)) continue;
            crepr = child->updateRepr(xml_doc, NULL, flags);
            if (crepr) l = g_slist_prepend (l, crepr);
        }
        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove (l, l->data);
        }
    } else {
        for (child = sp_object_first_child(object) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_TITLE(child) || SP_IS_DESC(child)) continue;
            child->updateRepr(flags);
        }
    }

    if ( flags & SP_OBJECT_WRITE_EXT ) {
        const char *value;
        if ( group->_layer_mode == SPGroup::LAYER ) {
            value = "layer";
        } else if ( flags & SP_OBJECT_WRITE_ALL ) {
            value = "group";
        } else {
            value = NULL;
        }
        repr->setAttribute("inkscape:groupmode", value);
    }

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags);

    return repr;
}

static void
sp_group_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags)
{
    SP_GROUP(item)->group->calculateBBox(bbox, transform, flags);
}

static void
sp_group_print (SPItem * item, SPPrintContext *ctx)
{
    SP_GROUP(item)->group->onPrint(ctx);
}

static gchar * sp_group_description (SPItem * item)
{
    return SP_GROUP(item)->group->getDescription();
}

static void sp_group_set(SPObject *object, unsigned key, char const *value) {
    SPGroup *group = SP_GROUP(object);

    switch (key) {
        case SP_ATTR_INKSCAPE_GROUPMODE:
            if ( value && !strcmp(value, "layer") ) {
                group->setLayerMode(SPGroup::LAYER);
            } else {
                group->setLayerMode(SPGroup::GROUP);
            }
            break;
        default: {
            if (((SPObjectClass *) (parent_class))->set) {
                (* ((SPObjectClass *) (parent_class))->set)(object, key, value);
            }
        }
    }
}

static NRArenaItem *
sp_group_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
    return SP_GROUP(item)->group->show(arena, key, flags);
}

static void
sp_group_hide (SPItem *item, unsigned int key)
{
    SP_GROUP(item)->group->hide(key);
}

static void sp_group_snappoints (SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs)
{
    for (SPObject const *o = sp_object_first_child(SP_OBJECT(item));
         o != NULL;
         o = SP_OBJECT_NEXT(o))
    {
        if (SP_IS_ITEM(o)) {
            sp_item_snappoints(SP_ITEM(o), p, snapprefs);
        }
    }
}


void
sp_item_group_ungroup (SPGroup *group, GSList **children, bool do_done)
{
    g_return_if_fail (group != NULL);
    g_return_if_fail (SP_IS_GROUP (group));

    SPDocument *doc = SP_OBJECT_DOCUMENT (group);
    SPObject *root = SP_DOCUMENT_ROOT (doc);
    SPObject *defs = SP_OBJECT (SP_ROOT (root)->defs);

    SPItem *gitem = SP_ITEM (group);
    Inkscape::XML::Node *grepr = SP_OBJECT_REPR (gitem);

    g_return_if_fail (!strcmp (grepr->name(), "svg:g") || !strcmp (grepr->name(), "svg:a") || !strcmp (grepr->name(), "svg:switch"));

    // this converts the gradient/pattern fill/stroke on the group, if any, to userSpaceOnUse
    sp_item_adjust_paint_recursive (gitem, Geom::identity(), Geom::identity(), false);

    SPItem *pitem = SP_ITEM (SP_OBJECT_PARENT (gitem));
    Inkscape::XML::Node *prepr = SP_OBJECT_REPR (pitem);

	if (SP_IS_BOX3D(gitem)) {
		group = box3d_convert_to_group(SP_BOX3D(gitem));
		gitem = SP_ITEM(group);
	}

	sp_lpe_item_remove_all_path_effects(SP_LPE_ITEM(group), false);

    /* Step 1 - generate lists of children objects */
    GSList *items = NULL;
    GSList *objects = NULL;
    for (SPObject *child = sp_object_first_child(SP_OBJECT(group)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {

        if (SP_IS_ITEM (child)) {

            SPItem *citem = SP_ITEM (child);

            /* Merging of style */
            // this converts the gradient/pattern fill/stroke, if any, to userSpaceOnUse; we need to do
            // it here _before_ the new transform is set, so as to use the pre-transform bbox
            sp_item_adjust_paint_recursive (citem, Geom::identity(), Geom::identity(), false);

            sp_style_merge_from_dying_parent(SP_OBJECT_STYLE(child), SP_OBJECT_STYLE(gitem));
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

            Inkscape::XML::Node *nrepr = SP_OBJECT_REPR (child)->duplicate(prepr->document());

            // Merging transform
            Geom::Matrix ctrans;
            Geom::Matrix const g(gitem->transform);
            if (SP_IS_USE(citem) && sp_use_get_original (SP_USE(citem)) &&
                    SP_OBJECT_PARENT (sp_use_get_original (SP_USE(citem))) == SP_OBJECT(group)) {
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
                			SP_OBJECT(group)->isAncestorOf(SP_OBJECT(source)) == false) { // ... of which the source is not in the same group
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
            Inkscape::XML::Node *nrepr = SP_OBJECT_REPR (child)->duplicate(prepr->document());
            objects = g_slist_prepend (objects, nrepr);
        }
    }

    /* Step 2 - clear group */
    // remember the position of the group
    gint pos = SP_OBJECT_REPR(group)->position();

    // the group is leaving forever, no heir, clones should take note; its children however are going to reemerge
    SP_OBJECT (group)->deleteObject(true, false);

    /* Step 3 - add nonitems */
    if (objects) {
        Inkscape::XML::Node *last_def = SP_OBJECT_REPR(defs)->lastChild();
        while (objects) {
			Inkscape::XML::Node *repr = (Inkscape::XML::Node *) objects->data;
			if (!sp_repr_is_meta_element(repr))
				SP_OBJECT_REPR(defs)->addChild(repr, last_def);
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
        SPItem *item = (SPItem *) doc->getObjectByRepr(repr);

        sp_item_write_transform(item, repr, item->transform, NULL, false);

        Inkscape::GC::release(repr);
        if (children && SP_IS_ITEM (item))
            *children = g_slist_prepend (*children, item);

        items = g_slist_remove (items, items->data);
    }

    if (do_done)
        sp_document_done (doc, SP_VERB_NONE, _("Ungroup"));
}

/*
 * some API for list aspect of SPGroup
 */

GSList *
sp_item_group_item_list (SPGroup * group)
{
    g_return_val_if_fail (group != NULL, NULL);
    g_return_val_if_fail (SP_IS_GROUP (group), NULL);

    GSList *s = NULL;

    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_ITEM (o)) {
            s = g_slist_prepend (s, o);
        }
    }

    return g_slist_reverse (s);
}

SPObject *
sp_item_group_get_child_by_name (SPGroup *group, SPObject *ref, const gchar *name)
{
    SPObject *child;
    child = (ref) ? SP_OBJECT_NEXT(ref) : sp_object_first_child(SP_OBJECT(group));
    while ( child && strcmp (SP_OBJECT_REPR(child)->name(), name) ) {
        child = SP_OBJECT_NEXT(child);
    }
    return child;
}

void SPGroup::setLayerMode(LayerMode mode) {
    if ( _layer_mode != mode ) {
        if ( mode == LAYER ) {
            sp_document_add_resource(SP_OBJECT_DOCUMENT(this), "layer", this);
        } else {
            sp_document_remove_resource(SP_OBJECT_DOCUMENT(this), "layer", this);
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
            NRArenaGroup *arena_group=NR_ARENA_GROUP(view->arenaitem);
            if (arena_group) {
                nr_arena_group_set_transparent(arena_group, effectiveLayerMode(view->key) == SPGroup::LAYER);
            }
        }
    }
}

void SPGroup::translateChildItems(Geom::Translate const &tr)
{
    if (this->hasChildren())
    {
        SPObject *o = NULL;
        for (o = sp_object_first_child(SP_OBJECT(this)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
            if (SP_IS_ITEM (o)) {
                sp_item_move_rel(static_cast<SPItem *>(o), tr);
            }
        }
    }
}

CGroup::CGroup(SPGroup *group) {
    _group = group;
}

CGroup::~CGroup() {
}

void CGroup::onChildAdded(Inkscape::XML::Node *child) {
    SPObject *last_child = _group->lastChild();
    if (last_child && SP_OBJECT_REPR(last_child) == child) {
        // optimization for the common special case where the child is being added at the end
        SPObject *ochild = last_child;
        if ( SP_IS_ITEM(ochild) ) {
            /* TODO: this should be moved into SPItem somehow */
            SPItemView *v;
            NRArenaItem *ac;

            for (v = _group->display; v != NULL; v = v->next) {
                ac = sp_item_invoke_show (SP_ITEM (ochild), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);

                if (ac) {
                    nr_arena_item_append_child (v->arenaitem, ac);
                }
            }
        }
    } else {    // general case
        SPObject *ochild = sp_object_get_child_by_repr(_group, child);
        if ( ochild && SP_IS_ITEM(ochild) ) {
            /* TODO: this should be moved into SPItem somehow */
            SPItemView *v;
            NRArenaItem *ac;

            unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));

            for (v = _group->display; v != NULL; v = v->next) {
                ac = sp_item_invoke_show (SP_ITEM (ochild), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);

                if (ac) {
                    nr_arena_item_add_child (v->arenaitem, ac, NULL);
                    nr_arena_item_set_order (ac, position);
                }
            }
        }
    }

    _group->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void CGroup::onChildRemoved(Inkscape::XML::Node */*child*/) {
    _group->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void CGroup::onUpdate(SPCtx *ctx, unsigned int flags) {
    SPItemCtx *ictx, cctx;

    ictx = (SPItemCtx *) ctx;
    cctx = *ictx;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
      flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      SPObject *object = SP_OBJECT(_group);
      for (SPItemView *v = SP_ITEM(_group)->display; v != NULL; v = v->next) {
    nr_arena_group_set_style(NR_ARENA_GROUP(v->arenaitem), SP_OBJECT_STYLE(object));
      }
    }

    GSList *l = g_slist_reverse(_group->childList(true, SPObject::ActionUpdate));
    while (l) {
        SPObject *child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);
        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM (child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, flags);
            } else {
                child->updateDisplay(ctx, flags);
            }
        }
        g_object_unref (G_OBJECT (child));
    }
}

void CGroup::onModified(guint flags) {
    SPObject *child;

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
      SPObject *object = SP_OBJECT(_group);
      for (SPItemView *v = SP_ITEM(_group)->display; v != NULL; v = v->next) {
    nr_arena_group_set_style(NR_ARENA_GROUP(v->arenaitem), SP_OBJECT_STYLE(object));
      }
    }

    GSList *l = g_slist_reverse(_group->childList(true));
    while (l) {
        child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref (G_OBJECT (child));
    }
}

void CGroup::calculateBBox(NRRect *bbox, Geom::Matrix const &transform, unsigned const flags) {

    Geom::OptRect dummy_bbox;

    GSList *l = _group->childList(false, SPObject::ActionBBox);
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM(o) && !SP_ITEM(o)->isHidden()) {
            SPItem *child = SP_ITEM(o);
            Geom::Matrix const ct(to_2geom(child->transform) * transform);
            sp_item_invoke_bbox_full(child, dummy_bbox, ct, flags, FALSE);
        }
        l = g_slist_remove (l, o);
    }

    *bbox = NRRect(dummy_bbox);
}

void CGroup::onPrint(SPPrintContext *ctx) {
    GSList *l = g_slist_reverse(_group->childList(false));
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM(o)) {
            sp_item_invoke_print (SP_ITEM (o), ctx);
        }
        l = g_slist_remove (l, o);
    }
}

gint CGroup::getItemCount() {
    gint len = 0;
    for (SPObject *o = sp_object_first_child(SP_OBJECT(_group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_ITEM(o)) {
            len++;
        }
    }

    return len;
}

gchar *CGroup::getDescription() {
    gint len = getItemCount();
    return g_strdup_printf(
            ngettext("<b>Group</b> of <b>%d</b> object",
                 "<b>Group</b> of <b>%d</b> objects",
                 len), len);
}

NRArenaItem *CGroup::show (NRArena *arena, unsigned int key, unsigned int flags) {
    NRArenaItem *ai;
    SPObject *object = SP_OBJECT(_group);

    ai = NRArenaGroup::create(arena);

    nr_arena_group_set_transparent(NR_ARENA_GROUP (ai),
                                   _group->effectiveLayerMode(key) ==
                         SPGroup::LAYER);
    nr_arena_group_set_style(NR_ARENA_GROUP(ai), SP_OBJECT_STYLE(object));

    _showChildren(arena, ai, key, flags);
    return ai;
}

void CGroup::_showChildren (NRArena *arena, NRArenaItem *ai, unsigned int key, unsigned int flags) {
    NRArenaItem *ac = NULL;
    NRArenaItem *ar = NULL;
    SPItem * child = NULL;
    GSList *l = g_slist_reverse(_group->childList(false, SPObject::ActionShow));
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM (o)) {
            child = SP_ITEM (o);
            ac = sp_item_invoke_show (child, arena, key, flags);
            if (ac) {
                nr_arena_item_add_child (ai, ac, ar);
                ar = ac;
            }
        }
        l = g_slist_remove (l, o);
    }
}

void CGroup::hide (unsigned int key) {
    SPItem * child;

    GSList *l = g_slist_reverse(_group->childList(false, SPObject::ActionShow));
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM (o)) {
            child = SP_ITEM (o);
            sp_item_invoke_hide (child, key);
        }
        l = g_slist_remove (l, o);
    }

    if (((SPItemClass *) parent_class)->hide)
        ((SPItemClass *) parent_class)->hide (_group, key);
}

void CGroup::onOrderChanged (Inkscape::XML::Node *child, Inkscape::XML::Node *, Inkscape::XML::Node *)
{
    SPObject *ochild = sp_object_get_child_by_repr(_group, child);
    if ( ochild && SP_IS_ITEM(ochild) ) {
        /* TODO: this should be moved into SPItem somehow */
        SPItemView *v;
        unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));
        for ( v = SP_ITEM (ochild)->display ; v != NULL ; v = v->next ) {
            nr_arena_item_set_order (v->arenaitem, position);
        }
    }

    _group->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_group_update_patheffect (SPLPEItem *lpeitem, bool write)
{
#ifdef GROUP_VERBOSE
    g_message("sp_group_update_patheffect: %p\n", lpeitem);
#endif
    g_return_if_fail (lpeitem != NULL);
    g_return_if_fail (SP_IS_GROUP (lpeitem));

    GSList const *item_list = sp_item_group_item_list(SP_GROUP(lpeitem));
    for ( GSList const *iter = item_list; iter; iter = iter->next ) {
        SPObject *subitem = static_cast<SPObject *>(iter->data);
        if (SP_IS_LPE_ITEM(subitem)) {
            if (SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (subitem))->update_patheffect) {
                SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (subitem))->update_patheffect (SP_LPE_ITEM(subitem), write);
            }
        }
    }

    if (sp_lpe_item_has_path_effect(lpeitem) && sp_lpe_item_path_effects_enabled(lpeitem)) {
        for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); it++)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (lpeobj && lpeobj->get_lpe()) {
                lpeobj->get_lpe()->doBeforeEffect(lpeitem);
            }
        }

        sp_group_perform_patheffect(SP_GROUP(lpeitem), SP_GROUP(lpeitem), write);
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
                c = sp_path_get_original_curve(SP_PATH(subitem));
            } else {
                c = sp_shape_get_curve(SP_SHAPE(subitem));
            }
            // only run LPEs when the shape has a curve defined
            if (c) {
                sp_lpe_item_perform_path_effect(SP_LPE_ITEM(topgroup), c);
                sp_shape_set_curve(SP_SHAPE(subitem), c, TRUE);

                if (write) {
                    Inkscape::XML::Node *repr = SP_OBJECT_REPR(subitem);
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
