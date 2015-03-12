/** \file
 * Base class for live path effect items
 */
/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Bastien Bouclet <bgkweb@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ui/tool/multi-path-manipulator.h"

#include <glibmm/i18n.h>

#include "live_effects/effect.h"
#include "live_effects/lpe-path_length.h"
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
#include "ui/shape-editor.h"
#include "sp-ellipse.h"
#include "display/curve.h"
#include "svg/svg.h"
#include <2geom/pathvector.h>
#include "sp-clippath.h"
#include "sp-mask.h"
#include "ui/tools-switch.h"
#include "ui/tools/node-tool.h"
#include "ui/tools/tool-base.h"

#include <algorithm>

/* LPEItem base class */
static void sp_lpe_item_enable_path_effects(SPLPEItem *lpeitem, bool enable);

static void lpeobject_ref_modified(SPObject *href, guint flags, SPLPEItem *lpeitem);

static void sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem);
static void sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem);

typedef std::list<std::string> HRefList;
static std::string patheffectlist_svg_string(PathEffectList const & list);
static std::string hreflist_svg_string(HRefList const & list);

SPLPEItem::SPLPEItem()
    : SPItem()
    , path_effects_enabled(1)
    , path_effect_list(new PathEffectList())
    , lpe_modified_connection_list(new std::list<sigc::connection>())
    , current_path_effect(NULL)
    , lpe_helperpaths()
{
}

SPLPEItem::~SPLPEItem() {
}

void SPLPEItem::build(SPDocument *document, Inkscape::XML::Node *repr) {
    this->readAttr( "inkscape:path-effect" );

    SPItem::build(document, repr);
}

void SPLPEItem::release() {
    // disconnect all modified listeners:
    for (std::list<sigc::connection>::iterator mod_it = this->lpe_modified_connection_list->begin();
         mod_it != this->lpe_modified_connection_list->end(); ++mod_it)
    {
        mod_it->disconnect();
    }

    delete this->lpe_modified_connection_list;
    this->lpe_modified_connection_list = NULL;

    PathEffectList::iterator it = this->path_effect_list->begin();

    while ( it != this->path_effect_list->end() ) {
        // unlink and delete all references in the list
        (*it)->unlink();
        delete *it;
        it = this->path_effect_list->erase(it);
    }

    // delete the list itself
    delete this->path_effect_list;
    this->path_effect_list = NULL;

    SPItem::release();
}

void SPLPEItem::set(unsigned int key, gchar const* value) {
    switch (key) {
        case SP_ATTR_INKSCAPE_PATH_EFFECT:
            {
                this->current_path_effect = NULL;

                // Disable the path effects while populating the LPE list
                 sp_lpe_item_enable_path_effects(this, false);

                // disconnect all modified listeners:
                for ( std::list<sigc::connection>::iterator mod_it = this->lpe_modified_connection_list->begin();
                      mod_it != this->lpe_modified_connection_list->end();
                      ++mod_it)
                {
                    mod_it->disconnect();
                }

                this->lpe_modified_connection_list->clear();
                // Clear the path effect list
                PathEffectList::iterator it = this->path_effect_list->begin();

                while ( it != this->path_effect_list->end() )
                {
                    (*it)->unlink();
                    delete *it;
                    it = this->path_effect_list->erase(it);
                }

                // Parse the contents of "value" to rebuild the path effect reference list
                if ( value ) {
                    std::istringstream iss(value);
                    std::string href;

                    while (std::getline(iss, href, ';'))
                    {
                        Inkscape::LivePathEffect::LPEObjectReference *path_effect_ref = new Inkscape::LivePathEffect::LPEObjectReference(this);

                        try {
                            path_effect_ref->link(href.c_str());
                        } catch (Inkscape::BadURIException &e) {
                            g_warning("BadURIException when trying to find LPE: %s", e.what());
                            path_effect_ref->unlink();
                            delete path_effect_ref;
                            path_effect_ref = NULL;
                        }

                        this->path_effect_list->push_back(path_effect_ref);

                        if ( path_effect_ref->lpeobject && path_effect_ref->lpeobject->get_lpe() ) {
                            // connect modified-listener
                            this->lpe_modified_connection_list->push_back(
                                                path_effect_ref->lpeobject->connectModified(sigc::bind(sigc::ptr_fun(&lpeobject_ref_modified), this)) );
                        } else {
                            // something has gone wrong in finding the right patheffect.
                            g_warning("Unknown LPE type specified, LPE stack effectively disabled");
                            // keep the effect in the lpestack, so the whole stack is effectively disabled but maintained
                        }
                    }
                }

                sp_lpe_item_enable_path_effects(this, true);
            }
            break;

        default:
            SPItem::set(key, value);
            break;
    }
}

void SPLPEItem::update(SPCtx* ctx, unsigned int flags) {
    SPItem::update(ctx, flags);

    // update the helperpaths of all LPEs applied to the item
    // TODO: re-add for the new node tool
}

void SPLPEItem::modified(unsigned int flags) {
    if (SP_IS_GROUP(this) && (flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_USER_MODIFIED_FLAG_B)) {
        sp_lpe_item_update_patheffect(this, true, true);
    }

//    SPItem::onModified(flags);
}

Inkscape::XML::Node* SPLPEItem::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if (flags & SP_OBJECT_WRITE_EXT) {
        if ( hasPathEffect() ) {
            repr->setAttribute("inkscape:path-effect", patheffectlist_svg_string(*this->path_effect_list));
        } else {
            repr->setAttribute("inkscape:path-effect", NULL);
        }
    }

    SPItem::write(xml_doc, repr, flags);

    return repr;
}

/**
 * returns true when LPE was successful.
 */
bool SPLPEItem::performPathEffect(SPCurve *curve) {
    if (!this) {
        return false;
    }

    if (!curve) {
        return false;
    }

    if (this->hasPathEffect() && this->pathEffectsEnabled()) {
        for (PathEffectList::iterator it = this->path_effect_list->begin(); it != this->path_effect_list->end(); ++it)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (!lpeobj) {
                /** \todo Investigate the cause of this.
                 * For example, this happens when copy pasting an object with LPE applied. Probably because the object is pasted while the effect is not yet pasted to defs, and cannot be found.
                 */
                g_warning("SPLPEItem::performPathEffect - NULL lpeobj in list!");
                return false;
            }
            Inkscape::LivePathEffect::Effect *lpe = lpeobj->get_lpe();
            if (!lpe) {
                /** \todo Investigate the cause of this.
                 * Not sure, but I think this can happen when an unknown effect type is specified...
                 */
                g_warning("SPLPEItem::performPathEffect - lpeobj with invalid lpe in the stack!");
                return false;
            }

            if (lpe->isVisible()) {
                if (lpe->acceptsNumClicks() > 0 && !lpe->isReady()) {
                    // if the effect expects mouse input before being applied and the input is not finished
                    // yet, we don't alter the path
                    return false;
                }

                // Groups have their doBeforeEffect called elsewhere
                if (!SP_IS_GROUP(this)) {
                    lpe->doBeforeEffect_impl(this);
                }

                try {
                    lpe->doEffect(curve);
                }
                catch (std::exception & e) {
                    g_warning("Exception during LPE %s execution. \n %s", lpe->getName().c_str(), e.what());
                    if (SP_ACTIVE_DESKTOP && SP_ACTIVE_DESKTOP->messageStack()) {
                        SP_ACTIVE_DESKTOP->messageStack()->flash( Inkscape::WARNING_MESSAGE,
                                        _("An exception occurred during execution of the Path Effect.") );
                    }
                    return false;
                }
                if (!SP_IS_GROUP(this)) {
                    lpe->doAfterEffect(this);
                }
            }
        }
    }

    return true;
}

// CPPIFY: make pure virtual
void SPLPEItem::update_patheffect(bool /*write*/) {
    //throw;
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

    if (!lpeitem->pathEffectsEnabled())
        return;

    // TODO: hack! this will be removed when path length measuring is reimplemented in a better way
    PathEffectList lpelist = lpeitem->getEffectList();
    std::list<Inkscape::LivePathEffect::LPEObjectReference *>::iterator i;
    for (i = lpelist.begin(); i != lpelist.end(); ++i) {
        if ((*i)->lpeobject) {
            Inkscape::LivePathEffect::Effect *lpe = (*i)->lpeobject->get_lpe();
            if (dynamic_cast<Inkscape::LivePathEffect::LPEPathLength *>(lpe)) {
                if (!lpe->isVisible()) {
                    // we manually disable text for LPEPathLength
                    // use static_cast, because we already checked for the right type above
                    static_cast<Inkscape::LivePathEffect::LPEPathLength *>(lpe)->hideCanvasText();
                }
            }
        }
    }

    SPLPEItem *top = NULL;

    if (wholetree) {
        SPLPEItem *prev_parent = lpeitem;
        SPLPEItem *parent = dynamic_cast<SPLPEItem*>(prev_parent->parent);
        while (parent && parent->hasPathEffectRecursive()) {
            prev_parent = parent;
            parent = dynamic_cast<SPLPEItem*>(prev_parent->parent);
        }
        top = prev_parent;
    }
    else {
        top = lpeitem;
    }

    top->update_patheffect(write);
}

/**
 * Gets called when any of the lpestack's lpeobject repr contents change: i.e. parameter change in any of the stacked LPEs
 */
static void
lpeobject_ref_modified(SPObject */*href*/, guint /*flags*/, SPLPEItem *lpeitem)
{
#ifdef SHAPE_VERBOSE
    g_message("lpeobject_ref_modified");
#endif
    sp_lpe_item_update_patheffect (lpeitem, true, true);
}

static void
sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem)
{
    g_return_if_fail(lpeitem != NULL);

    SPMask * mask = lpeitem->mask_ref->getObject();
    if(mask)
    {
        sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(mask->firstChild()));
    }
    SPClipPath * clipPath = lpeitem->clip_ref->getObject();
    if(clipPath)
    {
        sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(clipPath->firstChild()));
    }
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
        Inkscape::XML::Node *pathrepr = lpeitem->getRepr();
        if ( !pathrepr->attribute("inkscape:original-d") ) {
            pathrepr->setAttribute("inkscape:original-d", pathrepr->attribute("d"));
        }
    }
}

static void
sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem)
{
    g_return_if_fail(lpeitem != NULL);

    if (SP_IS_GROUP(lpeitem)) {
        if (!lpeitem->hasPathEffectRecursive()) {
            SPMask * mask = lpeitem->mask_ref->getObject();
            if(mask)
            {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(mask->firstChild()));
            }
            SPClipPath * clipPath = lpeitem->clip_ref->getObject();
            if(clipPath)
            {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(clipPath->firstChild()));
            }
        }
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(lpeitem));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            if (SP_IS_LPE_ITEM(subitem)) {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(subitem));
            }
        }
    }
    else if (SP_IS_PATH(lpeitem)) {
        Inkscape::XML::Node *repr = lpeitem->getRepr();
        if (!lpeitem->hasPathEffectRecursive() && repr->attribute("inkscape:original-d")) {
            SPMask * mask = lpeitem->mask_ref->getObject();
            if(mask)
            {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(mask->firstChild()));
            }
            SPClipPath * clipPath = lpeitem->clip_ref->getObject();
            if(clipPath)
            {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(clipPath->firstChild()));
            }
            repr->setAttribute("d", repr->attribute("inkscape:original-d"));
            repr->setAttribute("inkscape:original-d", NULL);
        }
        else {
            sp_lpe_item_update_patheffect(lpeitem, true, true);
        }
    }
}

void SPLPEItem::addPathEffect(std::string value, bool reset)
{
    if (!value.empty()) {
        // Apply the path effects here because in the casse of a group, lpe->resetDefaults
        // needs that all the subitems have their effects applied
        sp_lpe_item_update_patheffect(this, false, true);

        // Disable the path effects while preparing the new lpe
        sp_lpe_item_enable_path_effects(this, false);

        // Add the new reference to the list of LPE references
        HRefList hreflist;
        for (PathEffectList::const_iterator it = this->path_effect_list->begin(); it != this->path_effect_list->end(); ++it)
        {
            hreflist.push_back( std::string((*it)->lpeobject_href) );
        }
        hreflist.push_back(value); // C++11: should be emplace_back std::move'd  (also the reason why passed by value to addPathEffect)

        this->getRepr()->setAttribute("inkscape:path-effect", hreflist_svg_string(hreflist));

        // Make sure that ellipse is stored as <svg:path>
        if( SP_IS_GENERICELLIPSE(this)) {
            SP_GENERICELLIPSE(this)->write( this->getRepr()->document(), this->getRepr(), SP_OBJECT_WRITE_EXT );
        }
        // make sure there is an original-d for paths!!!
        sp_lpe_item_create_original_path_recursive(this);

        LivePathEffectObject *lpeobj = this->path_effect_list->back()->lpeobject;
        if (lpeobj && lpeobj->get_lpe()) {
            Inkscape::LivePathEffect::Effect *lpe = lpeobj->get_lpe();
            // Ask the path effect to reset itself if it doesn't have parameters yet
            if (reset) {
                // has to be called when all the subitems have their lpes applied
                lpe->resetDefaults(this);
            }

            // perform this once when the effect is applied
            lpe->doOnApply(this);

            // indicate that all necessary preparations are done and the effect can be performed
            lpe->setReady();
        }

        //Enable the path effects now that everything is ready to apply the new path effect
        sp_lpe_item_enable_path_effects(this, true);

        // Apply the path effect
        sp_lpe_item_update_patheffect(this, true, true);
        
        //fix bug 1219324
        if (SP_ACTIVE_DESKTOP ) {
        Inkscape::UI::Tools::ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
            if (INK_IS_NODE_TOOL(ec)) {
                tools_switch(SP_ACTIVE_DESKTOP, TOOLS_LPETOOL); //mhh
                tools_switch(SP_ACTIVE_DESKTOP, TOOLS_NODES);
            }
        }
    }
}

void SPLPEItem::addPathEffect(LivePathEffectObject * new_lpeobj)
{
    const gchar * repr_id = new_lpeobj->getRepr()->attribute("id");
    gchar *hrefstr = g_strdup_printf("#%s", repr_id);
    this->addPathEffect(hrefstr, false);
    g_free(hrefstr);
}

/**
 *  If keep_path == true, the item should not be updated, effectively 'flattening' the LPE.
 */
void SPLPEItem::removeCurrentPathEffect(bool keep_paths)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = this->getCurrentLPEReference();
    if (!lperef)
        return;

    if (Inkscape::LivePathEffect::Effect* effect_ = this->getCurrentLPE()) {
        effect_->doOnRemove(this);
    }    
    PathEffectList new_list = *this->path_effect_list;
    new_list.remove(lperef); //current lpe ref is always our 'own' pointer from the path_effect_list
    this->getRepr()->setAttribute("inkscape:path-effect", patheffectlist_svg_string(new_list));

    if (!keep_paths) {
        // Make sure that ellipse is stored as <svg:circle> or <svg:ellipse> if possible.
        if( SP_IS_GENERICELLIPSE(this)) {
            SP_GENERICELLIPSE(this)->write( this->getRepr()->document(), this->getRepr(), SP_OBJECT_WRITE_EXT );
        }

        sp_lpe_item_cleanup_original_path_recursive(this);
    }
}

/**
 *  If keep_path == true, the item should not be updated, effectively 'flattening' the LPE.
 */
void SPLPEItem::removeAllPathEffects(bool keep_paths)
{
    this->getRepr()->setAttribute("inkscape:path-effect", NULL);

    if (!keep_paths) {
        // Make sure that ellipse is stored as <svg:circle> or <svg:ellipse> if possible.
        if (SP_IS_GENERICELLIPSE(this)) {
            SP_GENERICELLIPSE(this)->write(this->getRepr()->document(), this->getRepr(), SP_OBJECT_WRITE_EXT);
        }

        sp_lpe_item_cleanup_original_path_recursive(this);
    }
}

void SPLPEItem::downCurrentPathEffect()
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = getCurrentLPEReference();
    if (!lperef)
        return;

    PathEffectList new_list = *this->path_effect_list;
    PathEffectList::iterator cur_it = find( new_list.begin(), new_list.end(), lperef );
    if (cur_it != new_list.end()) {
        PathEffectList::iterator down_it = cur_it;
        ++down_it;
        if (down_it != new_list.end()) { // perhaps current effect is already last effect
            std::iter_swap(cur_it, down_it);
        }
    }

    this->getRepr()->setAttribute("inkscape:path-effect", patheffectlist_svg_string(new_list));

    sp_lpe_item_cleanup_original_path_recursive(this);
}

void SPLPEItem::upCurrentPathEffect()
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = getCurrentLPEReference();
    if (!lperef)
        return;

    PathEffectList new_list = *this->path_effect_list;
    PathEffectList::iterator cur_it = find( new_list.begin(), new_list.end(), lperef );
    if (cur_it != new_list.end() && cur_it != new_list.begin()) {
        PathEffectList::iterator up_it = cur_it;
        --up_it;
        std::iter_swap(cur_it, up_it);
    }

    this->getRepr()->setAttribute("inkscape:path-effect", patheffectlist_svg_string(new_list));

    sp_lpe_item_cleanup_original_path_recursive(this);
}

/** used for shapes so they can see if they should also disable shape calculation and read from d= */
bool SPLPEItem::hasBrokenPathEffect() const
{
    if (path_effect_list->empty()) {
        return false;
    }

    // go through the list; if some are unknown or invalid, return true
    for (PathEffectList::const_iterator it = path_effect_list->begin(); it != path_effect_list->end(); ++it)
    {
        LivePathEffectObject *lpeobj = (*it)->lpeobject;
        if (!lpeobj || !lpeobj->get_lpe()) {
            return true;
        }
    }

    return false;
}


bool SPLPEItem::hasPathEffect() const
{
    if (path_effect_list->empty()) {
        return false;
    }

    // go through the list; if some are unknown or invalid, we are not an LPE item!
    for (PathEffectList::const_iterator it = path_effect_list->begin(); it != path_effect_list->end(); ++it)
    {
        LivePathEffectObject *lpeobj = (*it)->lpeobject;
        if (!lpeobj || !lpeobj->get_lpe()) {
            return false;
        }
    }

    return true;
}

bool SPLPEItem::hasPathEffectOfType(int const type) const
{
    if (path_effect_list->empty()) {
        return false;
    }

    for (PathEffectList::const_iterator it = path_effect_list->begin(); it != path_effect_list->end(); ++it)
    {
        LivePathEffectObject const *lpeobj = (*it)->lpeobject;
        if (lpeobj) {
            Inkscape::LivePathEffect::Effect const* lpe = lpeobj->get_lpe();
            if (lpe && (lpe->effectType() == type)) {
                return true;
            }
        }
    }

    return false;
}

bool SPLPEItem::hasPathEffectRecursive() const
{
    if (parent && SP_IS_LPE_ITEM(parent)) {
        return hasPathEffect() || SP_LPE_ITEM(parent)->hasPathEffectRecursive();
    }
    else {
        return hasPathEffect();
    }
}
void
SPLPEItem::apply_to_clippath(SPItem *item)
{
    SPClipPath *clipPath = item->clip_ref->getObject();
    if(clipPath) {
        SPObject * clip_data = clipPath->firstChild();
        SPCurve * clip_curve = NULL;

        if (SP_IS_PATH(clip_data)) {
            clip_curve = SP_PATH(clip_data)->get_original_curve();
        } else if(SP_IS_SHAPE(clip_data)) {
            clip_curve = SP_SHAPE(clip_data)->getCurve();
        } else if(SP_IS_GROUP(clip_data)) {
            apply_to_clip_or_mask_group(SP_ITEM(clip_data), item);
            return;
        }
        if(clip_curve) {
            bool success = false;
            if(SP_IS_GROUP(this)){
                clip_curve->transform(i2anc_affine(SP_GROUP(item), SP_GROUP(this)));
                success = this->performPathEffect(clip_curve);
                clip_curve->transform(i2anc_affine(SP_GROUP(item), SP_GROUP(this)).inverse());
            } else {
                success = this->performPathEffect(clip_curve);
            }
            Inkscape::XML::Node *reprClip = clip_data->getRepr();
            if (success) {
                gchar *str = sp_svg_write_path(clip_curve->get_pathvector());
                reprClip->setAttribute("d", str);
                g_free(str);
            } else {
                // LPE was unsuccesfull. Read the old 'd'-attribute.
                if (gchar const * value = reprClip->attribute("d")) {
                    Geom::PathVector pv = sp_svg_read_pathv(value);
                    SPCurve *oldcurve = new SPCurve(pv);
                    if (oldcurve) {
                        SP_SHAPE(clip_data)->setCurve(oldcurve, TRUE);
                        oldcurve->unref();
                    }
                }
            }
            clip_curve->unref();
        }
    }
    if(SP_IS_GROUP(item)){
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(item));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            apply_to_clippath(SP_ITEM(subitem));
        }
    }
}

void
SPLPEItem::apply_to_mask(SPItem *item)
{
    SPMask *mask = item->mask_ref->getObject();
    if(mask) {
        SPObject *mask_data = mask->firstChild();
        SPCurve * mask_curve = NULL;
        if (SP_IS_PATH(mask_data)) {
            mask_curve = SP_PATH(mask_data)->get_original_curve();
        } else if(SP_IS_SHAPE(mask_data)) {
            mask_curve = SP_SHAPE(mask_data)->getCurve();
        } else if(SP_IS_GROUP(mask_data)) {
            apply_to_clip_or_mask_group(SP_ITEM(mask_data), item);
            return;
        }
        if(mask_curve) {
            bool success = false;
            if(SP_IS_GROUP(this)){
                mask_curve->transform(i2anc_affine(SP_GROUP(item), SP_GROUP(this)));
                success = this->performPathEffect(mask_curve);
                mask_curve->transform(i2anc_affine(SP_GROUP(item), SP_GROUP(this)).inverse());
            } else {
                success = this->performPathEffect(mask_curve);
            }
            Inkscape::XML::Node *reprmask = mask_data->getRepr();
            if (success) {
                gchar *str = sp_svg_write_path(mask_curve->get_pathvector());
                reprmask->setAttribute("d", str);
                g_free(str);
            } else {
                // LPE was unsuccesfull. Read the old 'd'-attribute.
                if (gchar const * value = reprmask->attribute("d")) {
                    Geom::PathVector pv = sp_svg_read_pathv(value);
                    SPCurve *oldcurve = new SPCurve(pv);
                    if (oldcurve) {
                        SP_SHAPE(mask_data)->setCurve(oldcurve, TRUE);
                        oldcurve->unref();
                    }
                }
            }
            mask_curve->unref();
        }
    }
    if(SP_IS_GROUP(item)){
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(item));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            apply_to_mask(SP_ITEM(subitem));
        }
    }
}

void
SPLPEItem::apply_to_clip_or_mask_group(SPItem *group, SPItem *item)
{
    if (!SP_IS_GROUP(group)) {
        return;
    }
    GSList *item_list = sp_item_group_item_list(SP_GROUP(group));
    for ( GSList *iter = item_list; iter; iter = iter->next ) {
        SPObject *subitem = static_cast<SPObject *>(iter->data);
        if (SP_IS_GROUP(subitem)) {
            apply_to_clip_or_mask_group(SP_ITEM(subitem), item);
        } else if (SP_IS_SHAPE(subitem)) {
            SPCurve * c = NULL;

            if (SP_IS_PATH(subitem)) {
                c = SP_PATH(subitem)->get_original_curve();
            } else {
                c = SP_SHAPE(subitem)->getCurve();
            }
            if (c) {
                bool success = false;
                if(SP_IS_GROUP(group)){
                    c->transform(i2anc_affine(SP_GROUP(item), SP_GROUP(this)));
                    success = this->performPathEffect(c);
                    c->transform(i2anc_affine(SP_GROUP(item), SP_GROUP(this)).inverse());
                } else {
                    success = this->performPathEffect(c);
                }
                Inkscape::XML::Node *repr = subitem->getRepr();
                if (success) {
                    gchar *str = sp_svg_write_path(c->get_pathvector());
                    repr->setAttribute("d", str);
                    g_free(str);
                } else {
                    // LPE was unsuccesfull. Read the old 'd'-attribute.
                    if (gchar const * value = repr->attribute("d")) {
                        Geom::PathVector pv = sp_svg_read_pathv(value);
                        SPCurve *oldcurve = new SPCurve(pv);
                        if (oldcurve) {
                            SP_SHAPE(subitem)->setCurve(oldcurve, TRUE);
                            oldcurve->unref();
                        }
                    }
                }
                c->unref();
            }
        }
    }
}

Inkscape::LivePathEffect::Effect*
SPLPEItem::getPathEffectOfType(int type)
{
    std::list<Inkscape::LivePathEffect::LPEObjectReference *>::iterator i;
    for (i = path_effect_list->begin(); i != path_effect_list->end(); ++i) {
        LivePathEffectObject *lpeobj = (*i)->lpeobject;
        if (lpeobj) {
            Inkscape::LivePathEffect::Effect* lpe = lpeobj->get_lpe();
            if (lpe && (lpe->effectType() == type)) {
                return lpe;
            }
        }
    }
    return NULL;
}

Inkscape::LivePathEffect::Effect const*
SPLPEItem::getPathEffectOfType(int type) const
{
    std::list<Inkscape::LivePathEffect::LPEObjectReference *>::const_iterator i;
    for (i = path_effect_list->begin(); i != path_effect_list->end(); ++i) {
        LivePathEffectObject const *lpeobj = (*i)->lpeobject;
        if (lpeobj) {
            Inkscape::LivePathEffect::Effect const *lpe = lpeobj->get_lpe();
            if (lpe && (lpe->effectType() == type)) {
                return lpe;
            }
        }
    }
    return NULL;
}

void SPLPEItem::editNextParamOncanvas(SPDesktop *dt)
{
    Inkscape::LivePathEffect::LPEObjectReference *lperef = this->getCurrentLPEReference();
    if (lperef && lperef->lpeobject && lperef->lpeobject->get_lpe()) {
        lperef->lpeobject->get_lpe()->editNextParamOncanvas(this, dt);
    }
}

void SPLPEItem::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
    SPItem::child_added(child, ref);

    if (this->hasPathEffectRecursive()) {
        SPObject *ochild = this->get_child_by_repr(child);

        if ( ochild && SP_IS_LPE_ITEM(ochild) ) {
            sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(ochild));
        }
    }
}
void SPLPEItem::remove_child(Inkscape::XML::Node * child) {
    if (this->hasPathEffectRecursive()) {
        SPObject *ochild = this->get_child_by_repr(child);

        if ( ochild && SP_IS_LPE_ITEM(ochild) ) {
            sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(ochild));
        }
    }

    SPItem::remove_child(child);
}

static std::string patheffectlist_svg_string(PathEffectList const & list)
{
    HRefList hreflist;

    for (PathEffectList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        hreflist.push_back( std::string((*it)->lpeobject_href) ); // C++11: use emplace_back
    }

    return hreflist_svg_string(hreflist);
}

/**
 *  THE function that should be used to generate any patheffectlist string.
 * one of the methods to change the effect list:
 *  - create temporary href list
 *  - populate the templist with the effects from the old list that you want to have and their order
 *  - call this function with temp list as param
 */
static std::string hreflist_svg_string(HRefList const & list)
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
PathEffectList SPLPEItem::getEffectList()
{
    return *path_effect_list;
}

// Return a copy of the effect list
PathEffectList const SPLPEItem::getEffectList() const
{
    return *path_effect_list;
}

Inkscape::LivePathEffect::LPEObjectReference* SPLPEItem::getCurrentLPEReference()
{
    if (!this->current_path_effect && !this->path_effect_list->empty()) {
        setCurrentPathEffect(this->path_effect_list->back());
    }

    return this->current_path_effect;
}

Inkscape::LivePathEffect::Effect* SPLPEItem::getCurrentLPE()
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = getCurrentLPEReference();

    if (lperef && lperef->lpeobject)
        return lperef->lpeobject->get_lpe();
    else
        return NULL;
}

bool SPLPEItem::setCurrentPathEffect(Inkscape::LivePathEffect::LPEObjectReference* lperef)
{
    for (PathEffectList::iterator it = path_effect_list->begin(); it != path_effect_list->end(); ++it) {
        if ((*it)->lpeobject_repr == lperef->lpeobject_repr) {
            this->current_path_effect = (*it);  // current_path_effect should always be a pointer from the path_effect_list !
            return true;
        }
    }

    return false;
}

/**
 * Writes a new "inkscape:path-effect" string to xml, where the old_lpeobjects are substituted by the new ones.
 *  Note that this method messes up the item's \c PathEffectList.
 */
void SPLPEItem::replacePathEffects( std::vector<LivePathEffectObject const *> const &old_lpeobjs,
                                    std::vector<LivePathEffectObject const *> const &new_lpeobjs )
{
    HRefList hreflist;
    for (PathEffectList::const_iterator it = this->path_effect_list->begin(); it != this->path_effect_list->end(); ++it)
    {
        LivePathEffectObject const * current_lpeobj = (*it)->lpeobject;
        std::vector<LivePathEffectObject const *>::const_iterator found_it(std::find(old_lpeobjs.begin(), old_lpeobjs.end(), current_lpeobj));

        if ( found_it != old_lpeobjs.end() ) {
            std::vector<LivePathEffectObject const *>::difference_type found_index = std::distance (old_lpeobjs.begin(), found_it);
            const gchar * repr_id = new_lpeobjs[found_index]->getRepr()->attribute("id");
            gchar *hrefstr = g_strdup_printf("#%s", repr_id);
            hreflist.push_back( std::string(hrefstr) );
            g_free(hrefstr);
        }
        else {
            hreflist.push_back( std::string((*it)->lpeobject_href) );
        }
    }

    this->getRepr()->setAttribute("inkscape:path-effect", hreflist_svg_string(hreflist));
}

/**
 *  Check all effects in the stack if they are used by other items, and fork them if so.
 *  It is not recommended to fork the effects by yourself calling LivePathEffectObject::fork_private_if_necessary,
 *  use this method instead.
 *  Returns true if one or more effects were forked; returns false if nothing was done.
 */
bool SPLPEItem::forkPathEffectsIfNecessary(unsigned int nr_of_allowed_users)
{
    bool forked = false;

    if ( this->hasPathEffect() ) {
        // If one of the path effects is used by 2 or more items, fork it
        // so that each object has its own independent copy of the effect.
        // Note: replacing path effects messes up the path effect list

        // Clones of the LPEItem will increase the refcount of the lpeobjects.
        // Therefore, nr_of_allowed_users should be increased with the number of clones (i.e. refs to the lpeitem)
        nr_of_allowed_users += this->hrefcount;

        std::vector<LivePathEffectObject const *> old_lpeobjs, new_lpeobjs;
        PathEffectList effect_list = this->getEffectList();
        for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); ++it)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (lpeobj) {
                LivePathEffectObject *forked_lpeobj = lpeobj->fork_private_if_necessary(nr_of_allowed_users);
                if (forked_lpeobj != lpeobj) {
                    forked = true;
                    old_lpeobjs.push_back(lpeobj);
                    new_lpeobjs.push_back(forked_lpeobj);
                }
            }
        }

        if (forked) {
            this->replacePathEffects(old_lpeobjs, new_lpeobjs);
        }
    }

    return forked;
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
bool SPLPEItem::pathEffectsEnabled() const
{
    return path_effects_enabled > 0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
