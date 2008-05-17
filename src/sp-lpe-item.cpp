#define __SP_LPE_ITEM_CPP__

/** \file
 * Base class for live path effect items
 */
/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Bastien Bouclet <bgkweb@gmail.com>
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"

#include "sp-path.h"
#include "sp-item-group.h"
#include "streq.h"
#include "macros.h"
#include "attributes.h"
#include "sp-lpe-item.h"
#include "xml/repr.h"
#include "uri.h"

/* LPEItem base class */

static void sp_lpe_item_class_init(SPLPEItemClass *klass);
static void sp_lpe_item_init(SPLPEItem *lpe_item);
static void sp_lpe_item_finalize(GObject *object);

static void sp_lpe_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_lpe_item_release(SPObject *object);
static void sp_lpe_item_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_lpe_item_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_lpe_item_modified (SPObject *object, unsigned int flags);
static Inkscape::XML::Node *sp_lpe_item_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static void sp_lpe_item_child_added (SPObject * object, Inkscape::XML::Node * child, Inkscape::XML::Node * ref);
static void sp_lpe_item_remove_child (SPObject * object, Inkscape::XML::Node * child);

static void lpeobject_ref_changed(SPObject *old_ref, SPObject *ref, SPLPEItem *lpeitem);
static void lpeobject_ref_modified(SPObject *href, guint flags, SPLPEItem *lpeitem);

static void sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem);
static void sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem);

static SPItemClass *parent_class;

GType
sp_lpe_item_get_type()
{
    static GType lpe_item_type = 0;

    if (!lpe_item_type) {
        GTypeInfo lpe_item_info = {
            sizeof(SPLPEItemClass),
            NULL, NULL,
            (GClassInitFunc) sp_lpe_item_class_init,
            NULL, NULL,
            sizeof(SPLPEItem),
            16,
            (GInstanceInitFunc) sp_lpe_item_init,
            NULL,    /* value_table */
        };
        lpe_item_type = g_type_register_static(SP_TYPE_ITEM, "SPLPEItem", &lpe_item_info, (GTypeFlags)0);
    }
    return lpe_item_type;
}

static void
sp_lpe_item_class_init(SPLPEItemClass *klass)
{    
    GObjectClass *gobject_class;
    SPObjectClass *sp_object_class;

    gobject_class = (GObjectClass *) klass;
    sp_object_class = (SPObjectClass *) klass;
    parent_class = (SPItemClass *)g_type_class_peek_parent (klass);

    gobject_class->finalize = sp_lpe_item_finalize;

    sp_object_class->build = sp_lpe_item_build;
    sp_object_class->release = sp_lpe_item_release;
    sp_object_class->set = sp_lpe_item_set;
    sp_object_class->update = sp_lpe_item_update;
    sp_object_class->modified = sp_lpe_item_modified;
    sp_object_class->write = sp_lpe_item_write;
    sp_object_class->child_added = sp_lpe_item_child_added;
    sp_object_class->remove_child = sp_lpe_item_remove_child;
    
    klass->update_patheffect = NULL;
}

static void
sp_lpe_item_init(SPLPEItem *lpeitem)
{
    lpeitem->path_effect_ref  = new Inkscape::LivePathEffect::LPEObjectReference(SP_OBJECT(lpeitem));
    new (&lpeitem->lpe_modified_connection) sigc::connection();
}

static void
sp_lpe_item_finalize(GObject *object)
{
    if (((GObjectClass *) (parent_class))->finalize) {
        (* ((GObjectClass *) (parent_class))->finalize)(object);
    }
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPLPEItem variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_lpe_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    SP_LPE_ITEM(object)->path_effect_ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(lpeobject_ref_changed), SP_LPE_ITEM(object)));

    sp_object_read_attr(object, "inkscape:path-effect");
    
    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }
}

/**
 * Drops any allocated memory.
 */
static void
sp_lpe_item_release(SPObject *object)
{
    SPLPEItem *lpeitem;
    lpeitem = (SPLPEItem *) object;

    lpeitem->path_effect_ref->detach();

    lpeitem->lpe_modified_connection.disconnect();
    lpeitem->lpe_modified_connection.~connection();

    if (((SPObjectClass *) parent_class)->release)
        ((SPObjectClass *) parent_class)->release(object);
}

/**
 * Sets a specific value in the SPLPEItem.
 */
static void
sp_lpe_item_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPLPEItem *lpeitem = (SPLPEItem *) object;

    switch (key) {
        case SP_ATTR_INKSCAPE_PATH_EFFECT:
            if ( value && lpeitem->path_effect_ref->lpeobject_href 
                 && streq(value, lpeitem->path_effect_ref->lpeobject_href) ) {
                /* No change, do nothing. */
            } else {
                if (value) {
                    // Now do the attaching, which emits the changed signal.
                    try {
                        lpeitem->path_effect_ref->link((gchar*)value);
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        lpeitem->path_effect_ref->detach();
                    }
                } else {
                    // Detach, which emits the changed signal.
                    lpeitem->path_effect_ref->detach();
                }
            }
            break;
        default:
            if (((SPObjectClass *) parent_class)->set) {
                ((SPObjectClass *) parent_class)->set(object, key, value);
            }
            break;
    }
}

/**
 * Receives update notifications.
 */
static void
sp_lpe_item_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }
}

/**
 * Sets modified flag for all sub-item views.
 */
static void
sp_lpe_item_modified (SPObject *object, unsigned int flags)
{
	if (((SPObjectClass *) (parent_class))->modified) {
	  (* ((SPObjectClass *) (parent_class))->modified) (object, flags);
	}
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_lpe_item_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPLPEItem *lpeitem = (SPLPEItem *) object;

    if ( sp_lpe_item_has_path_effect(lpeitem) ) {
        repr->setAttribute("inkscape:path-effect", lpeitem->path_effect_ref->lpeobject_href);
    } else {
        repr->setAttribute("inkscape:path-effect", NULL);
    }

    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, repr, flags);
    }

    return repr;
}


LivePathEffectObject *
sp_lpe_item_get_livepatheffectobject(SPLPEItem *lpeitem) {
    if (!lpeitem) return NULL;

    if (sp_lpe_item_has_path_effect(lpeitem)) {
        return lpeitem->path_effect_ref->lpeobject;
    } else {
        return NULL;
    }
}

Inkscape::LivePathEffect::Effect *
sp_lpe_item_get_livepatheffect(SPLPEItem *lpeitem) {
    if (!lpeitem) return NULL;

    LivePathEffectObject * lpeobj = sp_lpe_item_get_livepatheffectobject(lpeitem);
    if (lpeobj)
        return lpeobj->lpe;
    else
        return NULL;
}

void sp_lpe_item_perform_path_effect(SPLPEItem *lpeitem, SPCurve *curve) {
    if (!lpeitem) return;
    if (!curve) return;

    if (sp_lpe_item_has_path_effect(lpeitem)) {
        LivePathEffectObject *lpeobj = sp_lpe_item_get_livepatheffectobject(lpeitem);
        lpeobj->lpe->doBeforeEffect(lpeitem);
        lpeobj->lpe->doEffect(curve);
    }

    SPObject *parent = lpeitem->parent;
    if (parent && SP_IS_LPE_ITEM(parent))
        sp_lpe_item_perform_path_effect(SP_LPE_ITEM(parent), curve);
}

/**
 * Calls any registered handlers for the update_patheffect action
 */
void
sp_lpe_item_update_patheffect (SPLPEItem *lpeitem, bool write)
{
#ifdef SHAPE_VERBOSE
    g_message("sp_lpe_item_update_patheffect: %p\n", lpeitem);
#endif
    g_return_if_fail (lpeitem != NULL);
    g_return_if_fail (SP_IS_LPE_ITEM (lpeitem));

    if (SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (lpeitem))->update_patheffect) {
        SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (lpeitem))->update_patheffect (lpeitem, write);
    }
}

/**
 * Gets called when (re)attached to another lpeobject.
 */
static void
lpeobject_ref_changed(SPObject *old_ref, SPObject *ref, SPLPEItem *lpeitem)
{
    if (old_ref) {
        sp_signal_disconnect_by_data(old_ref, lpeitem);
    }
    if ( IS_LIVEPATHEFFECT(ref) && ref != lpeitem )
    {
        lpeitem->lpe_modified_connection.disconnect();
        lpeitem->lpe_modified_connection = ref->connectModified(sigc::bind(sigc::ptr_fun(&lpeobject_ref_modified), lpeitem));
        lpeobject_ref_modified(ref, 0, lpeitem);
    }
}

/**
 * Gets called when lpeobject repr contents change: i.e. parameter change.
 */
static void
lpeobject_ref_modified(SPObject */*href*/, guint /*flags*/, SPLPEItem *lpeitem)
{
    sp_lpe_item_update_patheffect (lpeitem, true);
}

static void
sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem)
{
    if (SP_IS_GROUP(lpeitem)) {
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(lpeitem));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            if (SP_IS_LPE_ITEM(subitem)) {
                sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(subitem));
            }
        }
    }
    else if (SP_IS_PATH(lpeitem)) {
        Inkscape::XML::Node *pathrepr = SP_OBJECT_REPR(lpeitem);
        if ( !pathrepr->attribute("inkscape:original-d") ) {
            pathrepr->setAttribute("inkscape:original-d", pathrepr->attribute("d"));
        }
    }
}

static void
sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem)
{
    if (SP_IS_GROUP(lpeitem)) {
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(lpeitem));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            if (SP_IS_LPE_ITEM(subitem)) {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(subitem));
            }
        }
    }
    else if (SP_IS_PATH(lpeitem)) {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(lpeitem);
        if (!sp_lpe_item_has_path_effect_recursive(lpeitem) 
                && repr->attribute("inkscape:original-d")) {
            repr->setAttribute("d", repr->attribute("inkscape:original-d"));
            repr->setAttribute("inkscape:original-d", NULL);
        }
        else {
            sp_lpe_item_update_patheffect(lpeitem, true);
        }
    }
}

void sp_lpe_item_set_path_effect(SPLPEItem *lpeitem, gchar *value, bool reset)
{
    if (!value) {
        sp_lpe_item_remove_path_effect(lpeitem, false);
    } else {
        SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", value);
        
        // Ask the path effect to reset itself if it doesn't have parameters yet
        if (lpeitem->path_effect_ref && reset) {
            LivePathEffectObject *lpeobj = lpeitem->path_effect_ref->lpeobject;
            if (lpeobj && lpeobj->lpe) {
                // has to be called when all the subitems have their lpes applied
                lpeobj->lpe->resetDefaults(lpeitem);
            }
        }
        
        // make sure there is an original-d for paths!!!
        sp_lpe_item_create_original_path_recursive(lpeitem);
    }
}

void sp_lpe_item_set_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * new_lpeobj)
{
    const gchar * repr_id = SP_OBJECT_REPR(new_lpeobj)->attribute("id");
    gchar *hrefstr = g_strdup_printf("#%s", repr_id);
    sp_lpe_item_set_path_effect(lpeitem, hrefstr, false);
    g_free(hrefstr);
}

void sp_lpe_item_remove_path_effect(SPLPEItem *lpeitem, bool keep_paths)
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(lpeitem);
    repr->setAttribute("inkscape:path-effect", NULL);
    
    if (!keep_paths) {
        sp_lpe_item_cleanup_original_path_recursive(lpeitem);
    }
}

bool sp_lpe_item_has_path_effect(SPLPEItem *lpeitem)
{
    return lpeitem->path_effect_ref && lpeitem->path_effect_ref->lpeobject;
}

bool sp_lpe_item_has_path_effect_recursive(SPLPEItem *lpeitem)
{
    SPObject *parent = lpeitem->parent;
    if (parent && SP_IS_LPE_ITEM(parent)) {
        return sp_lpe_item_has_path_effect(lpeitem) || sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(parent));
    }
    else {
        return sp_lpe_item_has_path_effect(lpeitem);
    }
}

void sp_lpe_item_edit_next_param_oncanvas(SPLPEItem *lpeitem, SPDesktop *dt)
{
    LivePathEffectObject *lpeobj = sp_lpe_item_get_livepatheffectobject(lpeitem);
    if (lpeobj && lpeobj->lpe) {
        lpeobj->lpe->editNextParamOncanvas(SP_ITEM(lpeitem), dt);
    }
}

static void
sp_lpe_item_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    if (((SPObjectClass *) (parent_class))->child_added)
        (* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);
        
    if (SP_IS_LPE_ITEM(object) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(object))) {
        SPObject *ochild = sp_object_get_child_by_repr(object, child);
        if ( ochild && SP_IS_LPE_ITEM(ochild) ) {
            sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(ochild));
        }
    }
}

static void
sp_lpe_item_remove_child (SPObject * object, Inkscape::XML::Node * child)
{
    if (SP_IS_LPE_ITEM(object) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(object))) {
        SPObject *ochild = sp_object_get_child_by_repr(object, child);
        if ( ochild && SP_IS_LPE_ITEM(ochild) ) {
            sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(ochild));
        }
    }

    if (((SPObjectClass *) (parent_class))->remove_child)
        (* ((SPObjectClass *) (parent_class))->remove_child) (object, child);
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
