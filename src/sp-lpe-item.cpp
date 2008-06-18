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
#include "message-stack.h"
#include "inkscape.h"
#include "desktop.h"

#include <algorithm>

/* LPEItem base class */

static void sp_lpe_item_class_init(SPLPEItemClass *klass);
static void sp_lpe_item_init(SPLPEItem *lpe_item);
static void sp_lpe_item_finalize(GObject *object);

static void sp_lpe_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_lpe_item_release(SPObject *object);
static void sp_lpe_item_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_lpe_item_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_lpe_item_modified (SPObject *object, unsigned int flags);
static Inkscape::XML::Node *sp_lpe_item_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);

static void sp_lpe_item_child_added (SPObject * object, Inkscape::XML::Node * child, Inkscape::XML::Node * ref);
static void sp_lpe_item_remove_child (SPObject * object, Inkscape::XML::Node * child);

static void sp_lpe_item_enable_path_effects(SPLPEItem *lpeitem, bool enable);

static void lpeobject_ref_changed(SPObject *old_ref, SPObject *ref, SPLPEItem *lpeitem);
static void lpeobject_ref_modified(SPObject *href, guint flags, SPLPEItem *lpeitem);

static void sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem);
static void sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem);
typedef std::list<std::string> HRefList;
static std::string patheffectlist_write_svg(PathEffectList const & list);
static std::string hreflist_write_svg(HRefList const & list);

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
    lpeitem->path_effects_enabled = 1;

    lpeitem->path_effect_list = new PathEffectList();
    lpeitem->current_path_effect = NULL;

    new (&lpeitem->lpe_modified_connection) sigc::connection();

    lpeitem->adding_helperpaths = false;
    lpeitem->removing_helperpaths = false;
}

static void
sp_lpe_item_finalize(GObject *object)
{
    if (((GObjectClass *) (parent_class))->finalize) {
        (* ((GObjectClass *) (parent_class))->finalize)(object);
    }

    delete SP_LPE_ITEM(object)->path_effect_list;
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPLPEItem variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_lpe_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
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
            {
                lpeitem->current_path_effect = NULL;

                // Disable the path effects while populating the LPE list
                 sp_lpe_item_enable_path_effects(lpeitem, false);

                // Clear the path effect list
                PathEffectList::iterator it = lpeitem->path_effect_list->begin();
                while ( it != lpeitem->path_effect_list->end() )
                {
                    (*it)->unlink();
                    delete *it;
                    it = lpeitem->path_effect_list->erase(it);
                }

                // Parse the contents of "value" to rebuild the path effect reference list
                if ( value ) {
                    std::istringstream iss(value);
                    std::string href;
                    while (std::getline(iss, href, ';'))
                    {
                        Inkscape::LivePathEffect::LPEObjectReference *path_effect_ref;
                        path_effect_ref = new Inkscape::LivePathEffect::LPEObjectReference(SP_OBJECT(lpeitem));
                        path_effect_ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(lpeobject_ref_changed), SP_LPE_ITEM(object)));
                        // Now do the attaching, which emits the changed signal.
                        // Fixme, it should not do this changed signal and updating before all effects are added to the path_effect_list
                        try {
                            path_effect_ref->link(href.c_str());
                        } catch (Inkscape::BadURIException &e) {
                            g_warning("BadURIException: %s", e.what());
                            path_effect_ref->unlink();
                            delete path_effect_ref;
                            path_effect_ref = NULL;
                        }

                        if (path_effect_ref) {
                            lpeitem->path_effect_list->push_back(path_effect_ref);
                        }
                    }
                }

                sp_lpe_item_enable_path_effects(lpeitem, true);
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

    if (SP_IS_GROUP(object) && (flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_USER_MODIFIED_FLAG_B)) {
        sp_lpe_item_update_patheffect(SP_LPE_ITEM(object), true, true);
    }

    if (((SPObjectClass *) (parent_class))->modified) {
        (* ((SPObjectClass *) (parent_class))->modified) (object, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_lpe_item_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPLPEItem *lpeitem = (SPLPEItem *) object;

    if ( sp_lpe_item_has_path_effect(lpeitem) ) {
        std::string href = patheffectlist_write_svg(*lpeitem->path_effect_list);
        repr->setAttribute("inkscape:path-effect", href.c_str());
    } else {
        repr->setAttribute("inkscape:path-effect", NULL);
    }

    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

void sp_lpe_item_perform_path_effect(SPLPEItem *lpeitem, SPCurve *curve) {
    if (!lpeitem) return;
    if (!curve) return;

    if (sp_lpe_item_has_path_effect(lpeitem) && sp_lpe_item_path_effects_enabled(lpeitem)) {
        for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (!lpeobj) {
                g_warning("sp_lpe_item_perform_path_effect - NULL lpeobj in list!");
                return;
            }
            if (!lpeobj->lpe) {
                g_warning("sp_lpe_item_perform_path_effect - lpeobj without lpe!");
                return;
            }

            Inkscape::LivePathEffect::Effect *lpe = lpeobj->lpe;
            if (lpe->isVisible()) {
                if (lpe->acceptsNumParams() > 0 && !lpe->pathParamAccepted()) {
                    // if the effect expects mouse input before being applied and the input is not finished
                    // yet, we don't alter the path
                    return;
                }

                // Groups have their doBeforeEffect called elsewhere
                if (!SP_IS_GROUP(lpeitem)) {
                    lpe->doBeforeEffect(lpeitem);
                }

                try {
                    lpe->doEffect(curve);
                }
                catch (std::exception & e) {
                    g_warning("Exception during LPE %s execution. \n %s", lpe->getName().c_str(), e.what());
                    SP_ACTIVE_DESKTOP->messageStack()->flash( Inkscape::WARNING_MESSAGE,
                        _("An exception occurred during execution of the Path Effect.") );
                }
            }
        }
    }
}

/**
 * Calls any registered handlers for the update_patheffect action
 */
void
sp_lpe_item_update_patheffect (SPLPEItem *lpeitem, bool wholetree, bool write)
{
#ifdef SHAPE_VERBOSE
    g_message("sp_lpe_item_update_patheffect: %p\n", lpeitem);
#endif
    g_return_if_fail (lpeitem != NULL);
    g_return_if_fail (SP_IS_LPE_ITEM (lpeitem));

    if (!sp_lpe_item_path_effects_enabled(lpeitem))
        return;

    SPLPEItem *top;

    if (wholetree) {
        SPObject *prev_parent = lpeitem;
        SPObject *parent = prev_parent->parent;
        while (parent && SP_IS_LPE_ITEM(parent) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(parent))) {
            prev_parent = parent;
            parent = prev_parent->parent;
        }
        top = SP_LPE_ITEM(prev_parent);
    }
    else {
        top = lpeitem;
    }

    // TODO: ditch inkscape_active_desktop()
    SPDesktop *desktop = inkscape_active_desktop();
    if (desktop) {
        sp_lpe_item_remove_temporary_canvasitems(lpeitem, desktop);
        sp_lpe_item_add_temporary_canvasitems(lpeitem, desktop);
    }

    if (SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (top))->update_patheffect) {
        SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (top))->update_patheffect (top, write);
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
    sp_lpe_item_update_patheffect (lpeitem, true, true);
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
            sp_lpe_item_update_patheffect(lpeitem, true, true);
        }
    }
}

void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, gchar *value, bool reset)
{
    if (value) {
        // Apply the path effects here because in the casse of a group, lpe->resetDefaults
        // needs that all the subitems have their effects applied
        sp_lpe_item_update_patheffect(lpeitem, false, true);

        // Disable the path effects while preparing the new lpe
        sp_lpe_item_enable_path_effects(lpeitem, false);

        // Add the new reference to the list of LPE references
        HRefList hreflist;
        for (PathEffectList::const_iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
        {
            hreflist.push_back( std::string((*it)->lpeobject_href) );
        }
        hreflist.push_back( std::string(value) );
        std::string hrefs = hreflist_write_svg(hreflist);

        SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", hrefs.c_str());

        // make sure there is an original-d for paths!!!
        sp_lpe_item_create_original_path_recursive(lpeitem);

        LivePathEffectObject *lpeobj = lpeitem->path_effect_list->back()->lpeobject;
        if (lpeobj && lpeobj->lpe) {
            Inkscape::LivePathEffect::Effect *lpe = lpeobj->lpe;
            // Ask the path effect to reset itself if it doesn't have parameters yet
            if (reset) {
                // has to be called when all the subitems have their lpes applied
                lpe->resetDefaults(lpeitem);
            }

            // perform this once when the effect is applied
            lpe->doOnApply(SP_LPE_ITEM(lpeitem));

            // if the effect expects a number of mouse clicks to set a parameter path, perform the
            // necessary preparations
            if (lpe->acceptsNumParams() > 0) {
                lpe->doAcceptPathPreparations(lpeitem);
            }
        }

        //Enable the path effects now that everything is ready to apply the new path effect
        sp_lpe_item_enable_path_effects(lpeitem, true);

        // Apply the path effect
        sp_lpe_item_update_patheffect(lpeitem, true, true);
    }
}

void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * new_lpeobj)
{
    const gchar * repr_id = SP_OBJECT_REPR(new_lpeobj)->attribute("id");
    gchar *hrefstr = g_strdup_printf("#%s", repr_id);
    sp_lpe_item_add_path_effect(lpeitem, hrefstr, false);
    g_free(hrefstr);
}

void sp_lpe_item_remove_current_path_effect(SPLPEItem *lpeitem, bool keep_paths)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (!lperef)
        return;

    PathEffectList new_list = *lpeitem->path_effect_list;
    new_list.remove(lperef); //current lpe ref is always our 'own' pointer from the path_effect_list
    std::string r = patheffectlist_write_svg(new_list);

    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());

    if (!keep_paths) {
        sp_lpe_item_cleanup_original_path_recursive(lpeitem);
    }
}

void sp_lpe_item_remove_all_path_effects(SPLPEItem *lpeitem, bool keep_paths)
{
    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", NULL);

    if (!keep_paths) {
        sp_lpe_item_cleanup_original_path_recursive(lpeitem);
    }
}

void sp_lpe_item_down_current_path_effect(SPLPEItem *lpeitem)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (!lperef)
        return;

    PathEffectList new_list = *lpeitem->path_effect_list;
    PathEffectList::iterator cur_it = find( new_list.begin(), new_list.end(), lperef );
    if (cur_it != new_list.end()) {
        PathEffectList::iterator down_it = cur_it;
        down_it++;
        if (down_it != new_list.end()) { // perhaps current effect is already last effect
            std::iter_swap(cur_it, down_it);
        }
    }
    std::string r = patheffectlist_write_svg(new_list);
    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());

    sp_lpe_item_cleanup_original_path_recursive(lpeitem);
}

void sp_lpe_item_up_current_path_effect(SPLPEItem *lpeitem)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (!lperef)
        return;

    PathEffectList new_list = *lpeitem->path_effect_list;
    PathEffectList::iterator cur_it = find( new_list.begin(), new_list.end(), lperef );
    if (cur_it != new_list.end() && cur_it != new_list.begin()) {
        PathEffectList::iterator up_it = cur_it;
        up_it--;
        std::iter_swap(cur_it, up_it);
    }
    std::string r = patheffectlist_write_svg(new_list);

    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());

    sp_lpe_item_cleanup_original_path_recursive(lpeitem);
}


bool sp_lpe_item_has_path_effect(SPLPEItem *lpeitem)
{
    return !lpeitem->path_effect_list->empty();
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
    Inkscape::LivePathEffect::LPEObjectReference *lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (lperef && lperef->lpeobject && lperef->lpeobject->lpe) {
        lperef->lpeobject->lpe->editNextParamOncanvas(SP_ITEM(lpeitem), dt);
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

static std::string patheffectlist_write_svg(PathEffectList const & list)
{
    HRefList hreflist;
    for (PathEffectList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        hreflist.push_back( std::string((*it)->lpeobject_href) );
    }
    return hreflist_write_svg(hreflist);
}

/**
 *  THE function that should be used to generate any patheffectlist string.
 * one of the methods to change the effect list:
 *  - create temporary href list
 *  - populate the templist with the effects from the old list that you want to have and their order
 *  - call this function with temp list as param
 */
static std::string hreflist_write_svg(HRefList const & list)
{
    std::string r;
    bool semicolon_first = false;
    for (HRefList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        if (semicolon_first) {
            r += ';';
        }
        semicolon_first = true;

        r += (*it);
    }
    return r;
}

// Return a copy of the effect list
PathEffectList sp_lpe_item_get_effect_list(SPLPEItem *lpeitem)
{
    return *lpeitem->path_effect_list;
}

Inkscape::LivePathEffect::LPEObjectReference* sp_lpe_item_get_current_lpereference(SPLPEItem *lpeitem)
{
    if (!lpeitem->current_path_effect && !lpeitem->path_effect_list->empty())
        sp_lpe_item_set_current_path_effect(lpeitem, lpeitem->path_effect_list->back());

    return lpeitem->current_path_effect;
}

Inkscape::LivePathEffect::Effect* sp_lpe_item_get_current_lpe(SPLPEItem *lpeitem)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);

    if (lperef && lperef->lpeobject)
        return lperef->lpeobject->lpe;
    else
        return NULL;
}

bool sp_lpe_item_set_current_path_effect(SPLPEItem *lpeitem, Inkscape::LivePathEffect::LPEObjectReference* lperef)
{
    for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); it++) {
        if ((*it)->lpeobject_repr == lperef->lpeobject_repr) {
            lpeobject_ref_changed(NULL, (*it)->lpeobject, SP_LPE_ITEM(lpeitem)); // FIXME: explain why this is here?
            lpeitem->current_path_effect = (*it);  // current_path_effect should always be a pointer from the path_effect_list !
            return true;
        }
    }

    return false;
}

void sp_lpe_item_replace_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * old_lpeobj,
                                        LivePathEffectObject * new_lpeobj)
{
    HRefList hreflist;
    for (PathEffectList::const_iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
    {
        if ((*it)->lpeobject == old_lpeobj) {
            const gchar * repr_id = SP_OBJECT_REPR(new_lpeobj)->attribute("id");
            gchar *hrefstr = g_strdup_printf("#%s", repr_id);
            hreflist.push_back( std::string(hrefstr) );
            g_free(hrefstr);
        }
        else {
            hreflist.push_back( std::string((*it)->lpeobject_href) );
        }
    }
    std::string r = hreflist_write_svg(hreflist);
    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());
}

// Enable or disable the path effects of the item.
// The counter allows nested calls
static void sp_lpe_item_enable_path_effects(SPLPEItem *lpeitem, bool enable)
{
    if (enable) {
        lpeitem->path_effects_enabled++;
    }
    else {
        lpeitem->path_effects_enabled--;
    }
}

// Are the path effects enabled on this item ?
bool sp_lpe_item_path_effects_enabled(SPLPEItem *lpeitem)
{
    return lpeitem->path_effects_enabled > 0;
}

void
sp_lpe_item_add_temporary_canvasitems(SPLPEItem *lpeitem, SPDesktop *desktop)
{
    if (lpeitem->adding_helperpaths) {
        return;
    }
    lpeitem->adding_helperpaths = true;
    // FIXME: for some reason it seems that we must create the variable lpe AFTER checking
    //        for adding_helperpaths == true; otherwise we get a crash on startup. why??
    Inkscape::LivePathEffect::Effect *lpe = sp_lpe_item_get_current_lpe(lpeitem);
    if (lpe) {
        // TODO: can we just update the tempitem's SPCurve instead of recreating it each time?
        lpe->addHelperPaths(lpeitem, desktop);
    }
    lpeitem->adding_helperpaths = false;
}

void
sp_lpe_item_remove_temporary_canvasitems(SPLPEItem *lpeitem, SPDesktop *desktop)
{
    g_return_if_fail(lpeitem);
    g_return_if_fail(desktop);

    if (lpeitem->removing_helperpaths) {
        return;
    }
    lpeitem->removing_helperpaths = true;

    // destroy all temporary canvasitems created by LPEs
    std::vector<Inkscape::Display::TemporaryItem*>::iterator i;
    for (i = lpeitem->lpe_helperpaths.begin(); i != lpeitem->lpe_helperpaths.end(); ++i) {
        desktop->remove_temporary_canvasitem(*i);
    }

    lpeitem->removing_helperpaths = false;
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
