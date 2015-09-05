/** @file
 * Miscellanous operations on selected items.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Andrius R. <knutux@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Martin Sucha <martin.sucha-inkscape@jts-sro.sk>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr> (Symbol additions)
 *
 * Copyright (C) 1999-2010,2012 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/clipboard.h>

#include "file.h"
#include "selection-chemistry.h"

// TOOD fixme: This should be moved into preference repr
SPCycleType SP_CYCLING = SP_CYCLE_FOCUS;

#include "svg/svg.h"
#include "desktop.h"
#include "desktop-style.h"
#include "dir-util.h"
#include "layer-model.h"
#include "selection.h"
#include "ui/tools-switch.h"

#include "message-stack.h"
#include "sp-item-transform.h"
#include "sp-marker.h"
#include "sp-use.h"
#include "sp-textpath.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "sp-flowtext.h"
#include "sp-flowregion.h"
#include "sp-image.h"
#include "sp-rect.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-switch.h"
#include "sp-polyline.h"
#include "sp-line.h"
#include "text-editing.h"
#include "display/sp-canvas.h"
#include "ui/tools/text-tool.h"
#include "ui/tools/connector-tool.h"
#include "sp-path.h"
#include "sp-conn-end.h"
#include "ui/tools/dropper-tool.h"
#include <glibmm/i18n.h>
#include <2geom/transforms.h>
#include "xml/repr.h"
#include "xml/rebase-hrefs.h"
#include "style.h"
#include "document-private.h"
#include "document-undo.h"
#include "sp-gradient.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-pattern.h"
#include "sp-symbol.h"
#include "sp-radial-gradient.h"
#include "ui/tools/gradient-tool.h"
#include "sp-namedview.h"
#include "preferences.h"
#include "sp-offset.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "helper/png-write.h"
#include "layer-fns.h"
#include "context-fns.h"
#include <map>
#include <cstring>
#include <string>
#include "sp-item.h"
#include "box3d.h"
#include "persp3d.h"
#include "util/units.h"
#include "xml/simple-document.h"
#include "sp-filter-reference.h"
#include "gradient-drag.h"
#include "uri-references.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "display/cairo-utils.h"
#include "inkscape.h"
#include "path-chemistry.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/multi-path-manipulator.h"
#include "sp-lpe-item.h"
#include "live_effects/effect.h"
#include "live_effects/effect-enum.h"
#include "live_effects/parameter/originalpath.h"
#include "layer-manager.h"

#include "enums.h"
#include "sp-item-group.h"

// For clippath editing
#include "ui/tools-switch.h"
#include "ui/tools/node-tool.h"

#include "ui/clipboard.h"
#include "verbs.h"

using Inkscape::DocumentUndo;
using Geom::X;
using Geom::Y;
using Inkscape::UI::Tools::NodeTool;

/* The clipboard handling is in ui/clipboard.cpp now. There are some legacy functions left here,
because the layer manipulation code uses them. It should be rewritten specifically
for that purpose. */


// helper for printing error messages, regardless of whether we have a GUI or not
// If desktop == NULL, errors will be shown on stderr
static void
selection_display_message(SPDesktop *desktop, Inkscape::MessageType msgType, Glib::ustring const &msg)
{
    if (desktop) {
        desktop->messageStack()->flash(msgType, msg);
    } else {
        if (msgType == Inkscape::IMMEDIATE_MESSAGE ||
            msgType == Inkscape::WARNING_MESSAGE ||
            msgType == Inkscape::ERROR_MESSAGE) {
            g_printerr("%s\n", msg.c_str());
        }
    }
}

namespace Inkscape {

void SelectionHelper::selectAll(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        if (!nt->_multipath->empty()) {
            nt->_multipath->selectSubpaths();
            return;
        }
    }
    sp_edit_select_all(dt);
}

void SelectionHelper::selectAllInAll(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        nt->_selected_nodes->selectAll();
    } else {
        sp_edit_select_all_in_all_layers(dt);
    }
}

void SelectionHelper::selectNone(SPDesktop *dt)
{
    NodeTool *nt = NULL;
    if (tools_isactive(dt, TOOLS_NODES)) {
        nt = static_cast<NodeTool*>(dt->event_context);
    }

    if (nt && !nt->_selected_nodes->empty()) {
        nt->_selected_nodes->clear();
    } else if (!dt->getSelection()->isEmpty()) {
        dt->getSelection()->clear();
    } else {
        // If nothing selected switch to selection tool
        tools_switch(dt, TOOLS_SELECT);
    }
}

void SelectionHelper::selectSameFillStroke(SPDesktop *dt)
{
    sp_select_same_fill_stroke_style(dt, true, true, true);
}

void SelectionHelper::selectSameFillColor(SPDesktop *dt)
{
    sp_select_same_fill_stroke_style(dt, true, false, false);
}

void SelectionHelper::selectSameStrokeColor(SPDesktop *dt)
{
    sp_select_same_fill_stroke_style(dt, false, true, false);
}

void SelectionHelper::selectSameStrokeStyle(SPDesktop *dt)
{
    sp_select_same_fill_stroke_style(dt, false, false, true);
}

void SelectionHelper::selectSameObjectType(SPDesktop *dt)
{
    sp_select_same_object_type(dt);
}

void SelectionHelper::invert(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        nt->_multipath->invertSelectionInSubpaths();
    } else {
        sp_edit_invert(dt);
    }
}

void SelectionHelper::invertAllInAll(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        nt->_selected_nodes->invertSelection();
    } else {
        sp_edit_invert_in_all_layers(dt);
    }
}

void SelectionHelper::reverse(SPDesktop *dt)
{
    // TODO make this a virtual method of event context!
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        nt->_multipath->reverseSubpaths();
    } else {
        sp_selected_path_reverse(dt);
    }
}

void SelectionHelper::selectNext(SPDesktop *dt)
{
    Inkscape::UI::Tools::ToolBase *ec = dt->event_context;
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        nt->_multipath->shiftSelection(1);
    } else if (tools_isactive(dt, TOOLS_GRADIENT)
               && ec->_grdrag->isNonEmpty()) {
        Inkscape::UI::Tools::sp_gradient_context_select_next(ec);
    } else {
        sp_selection_item_next(dt);
    }
}

void SelectionHelper::selectPrev(SPDesktop *dt)
{
    Inkscape::UI::Tools::ToolBase *ec = dt->event_context;
    if (tools_isactive(dt, TOOLS_NODES)) {
        NodeTool *nt = static_cast<NodeTool*>(dt->event_context);
        nt->_multipath->shiftSelection(-1);
    } else if (tools_isactive(dt, TOOLS_GRADIENT)
               && ec->_grdrag->isNonEmpty()) {
        Inkscape::UI::Tools::sp_gradient_context_select_prev(ec);
    } else {
        sp_selection_item_prev(dt);
    }
}

/*
 * Fixes the current selection, removing locked objects from it
 */
void SelectionHelper::fixSelection(SPDesktop *dt) 
{
    if(!dt)
        return;

    Inkscape::Selection *selection = dt->getSelection();
    
    std::vector<SPItem*> items ;

    std::vector<SPItem*> const selList = selection->itemList();

    for( std::vector<SPItem*>::const_reverse_iterator i = selList.rbegin(); i != selList.rend(); i++ ) {
        SPItem *item = *i;
        if( item &&
            !dt->isLayer(item) &&
            (!item->isLocked()))
        {
            items.push_back(item);
        }
    }

    selection->setList(items);
}

} // namespace Inkscape


/**
 * Copies repr and its inherited css style elements, along with the accumulated transform 'full_t',
 * then prepends the copy to 'clip'.
 */
static void sp_selection_copy_one(Inkscape::XML::Node *repr, Geom::Affine full_t, std::vector<Inkscape::XML::Node*> &clip, Inkscape::XML::Document* xml_doc)
{
    Inkscape::XML::Node *copy = repr->duplicate(xml_doc);

    // copy complete inherited style
    SPCSSAttr *css = sp_repr_css_attr_inherited(repr, "style");
    sp_repr_css_set(copy, css, "style");
    sp_repr_css_attr_unref(css);

    // write the complete accumulated transform passed to us
    // (we're dealing with unattached repr, so we write to its attr
    // instead of using sp_item_set_transform)
    gchar *affinestr=sp_svg_transform_write(full_t);
    copy->setAttribute("transform", affinestr);
    g_free(affinestr);

    clip.insert(clip.begin(),copy);
}

static void sp_selection_copy_impl(std::vector<SPItem*> const &items, std::vector<Inkscape::XML::Node*> &clip, Inkscape::XML::Document* xml_doc)
{
    // Sort items:
    std::vector<SPItem*> sorted_items(items);
    sort(sorted_items.begin(),sorted_items.end(),sp_object_compare_position_bool);

    // Copy item reprs:
    for (std::vector<SPItem*>::const_iterator i = sorted_items.begin(); i != sorted_items.end(); i++) {
        SPItem *item = *i;
        if (item) {
            sp_selection_copy_one(item->getRepr(), item->i2doc_affine(), clip, xml_doc);
        } else {
            g_assert_not_reached();
        }
    }
    reverse(clip.begin(),clip.end());
}

// TODO check if parent parameter should be changed to SPItem, of if the code should handle non-items.
static std::vector<Inkscape::XML::Node*> sp_selection_paste_impl(SPDocument *doc, SPObject *parent, std::vector<Inkscape::XML::Node*> &clip)
{
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    SPItem *parentItem = dynamic_cast<SPItem *>(parent);
    g_assert(parentItem != NULL);

    std::vector<Inkscape::XML::Node*> copied;
    // add objects to document
    for (std::vector<Inkscape::XML::Node*>::const_iterator l = clip.begin(); l != clip.end(); l++) {
        Inkscape::XML::Node *repr = *l;
        Inkscape::XML::Node *copy = repr->duplicate(xml_doc);

        // premultiply the item transform by the accumulated parent transform in the paste layer
        Geom::Affine local(parentItem->i2doc_affine());
        if (!local.isIdentity()) {
            gchar const *t_str = copy->attribute("transform");
            Geom::Affine item_t(Geom::identity());
            if (t_str)
                sp_svg_transform_read(t_str, &item_t);
            item_t *= local.inverse();
            // (we're dealing with unattached repr, so we write to its attr instead of using sp_item_set_transform)
            gchar *affinestr=sp_svg_transform_write(item_t);
            copy->setAttribute("transform", affinestr);
            g_free(affinestr);
        }

        parent->appendChildRepr(copy);
        copied.push_back(copy);
        Inkscape::GC::release(copy);
    }
    return copied;
}

static void sp_selection_delete_impl(std::vector<SPItem*> const &items, bool propagate = true, bool propagate_descendants = true)
{
    for (std::vector<SPItem*>::const_iterator i = items.begin(); i != items.end(); i++) {
        sp_object_ref(*i, NULL);
    }
    for (std::vector<SPItem*>::const_iterator i = items.begin(); i != items.end(); i++) {
        SPItem *item = *i;
        item->deleteObject(propagate, propagate_descendants);
        sp_object_unref(item, NULL);
    }
}


void sp_selection_delete(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    if (tools_isactive(desktop, TOOLS_TEXT))
        if (Inkscape::UI::Tools::sp_text_delete_selection(desktop->event_context)) {
            DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_TEXT,
                               _("Delete text"));
            return;
        }

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("<b>Nothing</b> was deleted."));
        return;
    }
    std::vector<SPItem*> selected(selection->itemList());
    selection->clear();
    sp_selection_delete_impl(selected);
    desktop->currentLayer()->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

    /* a tool may have set up private information in it's selection context
     * that depends on desktop items.  I think the only sane way to deal with
     * this currently is to reset the current tool, which will reset it's
     * associated selection context.  For example: deleting an object
     * while moving it around the canvas.
     */
    tools_switch( desktop, tools_active( desktop ) );

    DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_DELETE,
                       _("Delete"));
}

static void add_ids_recursive(std::vector<const gchar *> &ids, SPObject *obj)
{
    if (obj) {
        ids.push_back(obj->getId());

        if (dynamic_cast<SPGroup *>(obj)) {
            for (SPObject *child = obj->firstChild() ; child; child = child->getNext() ) {
                add_ids_recursive(ids, child);
            }
        }
    }
}

void sp_selection_duplicate(SPDesktop *desktop, bool suppressDone, bool duplicateLayer)
{
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->doc();
    Inkscape::XML::Document* xml_doc = doc->getReprDoc();
    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty() && !duplicateLayer) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to duplicate."));
        return;
    }
    std::vector<Inkscape::XML::Node*> reprs(selection->reprList());

    if(duplicateLayer){
        reprs.clear();
        reprs.push_back(desktop->currentLayer()->getRepr());
    }

    selection->clear();

    // sorting items from different parents sorts each parent's subset without possibly mixing
    // them, just what we need
    sort(reprs.begin(),reprs.end(),sp_repr_compare_position_bool);

    std::vector<Inkscape::XML::Node*> newsel;

    std::vector<const gchar *> old_ids;
    std::vector<const gchar *> new_ids;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool relink_clones = prefs->getBool("/options/relinkclonesonduplicate/value");
    const bool fork_livepatheffects = prefs->getBool("/options/forklpeonduplicate/value", true);

    for(std::vector<Inkscape::XML::Node*>::const_iterator i=reprs.begin();i!=reprs.end();i++){
        Inkscape::XML::Node *old_repr = *i;
        Inkscape::XML::Node *parent = old_repr->parent();
        Inkscape::XML::Node *copy = old_repr->duplicate(xml_doc);

        if(! duplicateLayer)
            parent->appendChild(copy);
        else
        	parent->addChild(copy, old_repr);

        if (relink_clones) {
            SPObject *old_obj = doc->getObjectByRepr(old_repr);
            SPObject *new_obj = doc->getObjectByRepr(copy);
            add_ids_recursive(old_ids, old_obj);
            add_ids_recursive(new_ids, new_obj);
        }

        if (fork_livepatheffects) {
            SPObject *new_obj = doc->getObjectByRepr(copy);
            SPLPEItem *newLPEObj = dynamic_cast<SPLPEItem *>(new_obj);
            if (newLPEObj) {
                newLPEObj->forkPathEffectsIfNecessary(1);
            }
        }

        newsel.push_back(copy);
        Inkscape::GC::release(copy);
    }

    if (relink_clones) {

        g_assert(old_ids.size() == new_ids.size());

        for (unsigned int i = 0; i < old_ids.size(); i++) {
            const gchar *id = old_ids[i];
            SPObject *old_clone = doc->getObjectById(id);
            SPUse *use = dynamic_cast<SPUse *>(old_clone);
            if (use) {
                SPItem *orig = use->get_original();
                if (!orig) // orphaned
                    continue;
                for (unsigned int j = 0; j < old_ids.size(); j++) {
                    if (!strcmp(orig->getId(), old_ids[j])) {
                        // we have both orig and clone in selection, relink
                        // std::cout << id  << " old, its ori: " << orig->getId() << "; will relink:" << new_ids[i] << " to " << new_ids[j] << "\n";
                        SPObject *new_clone = doc->getObjectById(new_ids[i]);
                        new_clone->getRepr()->setAttribute("xlink:href", Glib::ustring("#") + new_ids[j]);
                        new_clone->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                    }
                }
            } else {
                SPOffset *offset = dynamic_cast<SPOffset *>(old_clone);
                if (offset) {
                    for (guint j = 0; j < old_ids.size(); j++) {
                        gchar *source_href = offset->sourceHref;
                        if (source_href && source_href[0]=='#' && !strcmp(source_href+1, old_ids[j])) {
                            doc->getObjectById(new_ids[i])->getRepr()->setAttribute("xlink:href", Glib::ustring("#") + new_ids[j]);
                        }
                    }
                }
            }
        }
    }


    if ( !suppressDone ) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_DUPLICATE,
                           _("Duplicate"));
    }
    if(!duplicateLayer)
        selection->setReprList(newsel);
    else{
    	SPObject* new_layer = doc->getObjectByRepr(newsel[0]);
        gchar* name = g_strdup_printf(_("%s copy"), new_layer->label());
        desktop->layer_manager->renameLayer( new_layer, name, TRUE );
        g_free(name);
    }
}

void sp_edit_clear_all(Inkscape::Selection *selection)
{
    if (!selection)
        return;

    SPDocument *doc = selection->layers()->getDocument();
    selection->clear();

    SPGroup *group = dynamic_cast<SPGroup *>(selection->layers()->currentLayer());
    g_return_if_fail(group != NULL);
    std::vector<SPItem*> items = sp_item_group_item_list(group);

    for(unsigned int i = 0; i < items.size(); i++){
        items[i]->deleteObject();
    }

    DocumentUndo::done(doc, SP_VERB_EDIT_CLEAR_ALL,
                       _("Delete all"));
}

/*
 * Return a list of SPItems that are the children of 'list'
 *
 * list - source list of items to search in
 * desktop - desktop associated with the source list
 * exclude - list of items to exclude from result
 * onlyvisible - TRUE includes only items visible on canvas
 * onlysensitive - TRUE includes only non-locked items
 * ingroups - TRUE to recursively get grouped items children
 */
std::vector<SPItem*> &get_all_items(std::vector<SPItem*> &list, SPObject *from, SPDesktop *desktop, bool onlyvisible, bool onlysensitive, bool ingroups, std::vector<SPItem*> const &exclude)
{
    for ( SPObject *child = from->firstChild() ; child; child = child->getNext() ) {
        SPItem *item = dynamic_cast<SPItem *>(child);
        if (item &&
            !desktop->isLayer(item) &&
            (!onlysensitive || !item->isLocked()) &&
            (!onlyvisible || !desktop->itemIsHidden(item)) &&
            (exclude.empty() || exclude.end() == std::find(exclude.begin(),exclude.end(),child))
            )
        {
            list.insert(list.begin(),item);
        }

        if (ingroups || (item && desktop->isLayer(item))) {
            list = get_all_items(list, child, desktop, onlyvisible, onlysensitive, ingroups, exclude);
        }
    }

    return list;
}

static void sp_edit_select_all_full(SPDesktop *dt, bool force_all_layers, bool invert)
{
    if (!dt)
        return;

    Inkscape::Selection *selection = dt->getSelection();

    g_return_if_fail(dynamic_cast<SPGroup *>(dt->currentLayer()));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    PrefsSelectionContext inlayer = (PrefsSelectionContext) prefs->getInt("/options/kbselection/inlayer", PREFS_SELECTION_LAYER);
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);

    std::vector<SPItem*> items ;

    std::vector<SPItem*> exclude;
    if (invert) {
        exclude = selection->itemList();
    }

    if (force_all_layers)
        inlayer = PREFS_SELECTION_ALL;

    switch (inlayer) {
        case PREFS_SELECTION_LAYER: {
        if ( (onlysensitive && dynamic_cast<SPItem *>(dt->currentLayer())->isLocked()) ||
             (onlyvisible && dt->itemIsHidden(dynamic_cast<SPItem *>(dt->currentLayer()))) )
        return;

        std::vector<SPItem*> all_items = sp_item_group_item_list(dynamic_cast<SPGroup *>(dt->currentLayer()));

        for (std::vector<SPItem*>::const_reverse_iterator i=all_items.rbegin();i!=all_items.rend();i++) {
            SPItem *item = *i;

            if (item && (!onlysensitive || !item->isLocked())) {
                if (!onlyvisible || !dt->itemIsHidden(item)) {
                    if (!dt->isLayer(item)) {
                        if (!invert || exclude.end() == std::find(exclude.begin(),exclude.end(),item)) {
                            items.push_back(item); // leave it in the list
                        }
                    }
                }
            }
        }

            break;
        }
        case PREFS_SELECTION_LAYER_RECURSIVE: {
        	std::vector<SPItem*> x;
            items = get_all_items(x, dt->currentLayer(), dt, onlyvisible, onlysensitive, FALSE, exclude);
            break;
        }
        default: {
            std::vector<SPItem*> x;
            items = get_all_items(x, dt->currentRoot(), dt, onlyvisible, onlysensitive, FALSE, exclude);
            break;
    }
    }

    selection->setList(items);

}

void sp_edit_select_all(SPDesktop *desktop)
{
    sp_edit_select_all_full(desktop, false, false);
}

void sp_edit_select_all_in_all_layers(SPDesktop *desktop)
{
    sp_edit_select_all_full(desktop, true, false);
}

void sp_edit_invert(SPDesktop *desktop)
{
    sp_edit_select_all_full(desktop, false, true);
}

void sp_edit_invert_in_all_layers(SPDesktop *desktop)
{
    sp_edit_select_all_full(desktop, true, true);
}

static void sp_selection_group_impl(std::vector<Inkscape::XML::Node*> p, Inkscape::XML::Node *group, Inkscape::XML::Document *xml_doc, SPDocument *doc) {

    sort(p.begin(),p.end(),sp_repr_compare_position_bool);

    // Remember the position and parent of the topmost object.
    gint topmost = p.back()->position();
    Inkscape::XML::Node *topmost_parent = p.back()->parent();

    for(std::vector<Inkscape::XML::Node*>::const_iterator i = p.begin(); i != p.end(); i++){
        Inkscape::XML::Node *current = *i;

        if (current->parent() == topmost_parent) {
            Inkscape::XML::Node *spnew = current->duplicate(xml_doc);
            sp_repr_unparent(current);
            group->appendChild(spnew);
            Inkscape::GC::release(spnew);
            topmost --; // only reduce count for those items deleted from topmost_parent
        } else { // move it to topmost_parent first
            std::vector<Inkscape::XML::Node*> temp_clip;

            // At this point, current may already have no item, due to its being a clone whose original is already moved away
            // So we copy it artificially calculating the transform from its repr->attr("transform") and the parent transform
            gchar const *t_str = current->attribute("transform");
            Geom::Affine item_t(Geom::identity());
            if (t_str)
                sp_svg_transform_read(t_str, &item_t);
            item_t *= dynamic_cast<SPItem *>(doc->getObjectByRepr(current->parent()))->i2doc_affine();
            // FIXME: when moving both clone and original from a transformed group (either by
            // grouping into another parent, or by cut/paste) the transform from the original's
            // parent becomes embedded into original itself, and this affects its clones. Fix
            // this by remembering the transform diffs we write to each item into an array and
            // then, if this is clone, looking up its original in that array and pre-multiplying
            // it by the inverse of that original's transform diff.

            sp_selection_copy_one(current, item_t, temp_clip, xml_doc);
            sp_repr_unparent(current);

            // paste into topmost_parent (temporarily)
            std::vector<Inkscape::XML::Node*> copied = sp_selection_paste_impl(doc, doc->getObjectByRepr(topmost_parent), temp_clip);
            if (!temp_clip.empty())temp_clip.clear() ;
            if (!copied.empty()) { // if success,
                // take pasted object (now in topmost_parent)
                Inkscape::XML::Node *in_topmost = copied.back();
                // make a copy
                Inkscape::XML::Node *spnew = in_topmost->duplicate(xml_doc);
                // remove pasted
                sp_repr_unparent(in_topmost);
                // put its copy into group
                group->appendChild(spnew);
                Inkscape::GC::release(spnew);
                copied.clear();
            }
        }
    }

    // Add the new group to the topmost members' parent
    topmost_parent->appendChild(group);

    // Move to the position of the topmost, reduced by the number of items deleted from topmost_parent
    group->setPosition(topmost + 1);
}

void sp_selection_group(Inkscape::Selection *selection, SPDesktop *desktop)
{
    SPDocument *doc = selection->layers()->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    // Check if something is selected.
    if (selection->isEmpty()) {
        selection_display_message(desktop, Inkscape::WARNING_MESSAGE, _("Select <b>some objects</b> to group."));
        return;
    }

    std::vector<Inkscape::XML::Node*> p (selection->reprList());

    selection->clear();

    Inkscape::XML::Node *group = xml_doc->createElement("svg:g");

    sp_selection_group_impl(p, group, xml_doc, doc);

    DocumentUndo::done(doc, SP_VERB_SELECTION_GROUP,
                       C_("Verb", "Group"));

    selection->set(group);
    Inkscape::GC::release(group);
}

static gint clone_depth_descending(gconstpointer a, gconstpointer b) {
    SPUse *use_a = static_cast<SPUse *>(const_cast<gpointer>(a));
    SPUse *use_b = static_cast<SPUse *>(const_cast<gpointer>(b));
    int depth_a = use_a->cloneDepth();
    int depth_b = use_b->cloneDepth();
    if (depth_a < depth_b) {
        return 1;
    } else if (depth_a == depth_b) {
        return 0;
    } else {
        return -1;
    }
}

void sp_selection_ungroup(Inkscape::Selection *selection, SPDesktop *desktop)
{
    if (selection->isEmpty()) {
        selection_display_message(desktop, Inkscape::WARNING_MESSAGE, _("Select a <b>group</b> to ungroup."));
    }

    // first check whether there is anything to ungroup
    std::vector<SPItem*> old_select = selection->itemList();
    std::vector<SPItem*> new_select;
    GSList *groups = NULL;
    for (std::vector<SPItem*>::const_iterator item = old_select.begin(); item!=old_select.end(); item++) {
        SPItem *obj = *item;
        if (dynamic_cast<SPGroup *>(obj)) {
            groups = g_slist_prepend(groups, obj);
        }
    }

    if (groups == NULL) {
        selection_display_message(desktop, Inkscape::ERROR_MESSAGE, _("<b>No groups</b> to ungroup in the selection."));
        g_slist_free(groups);
        return;
    }

    std::vector<SPItem*> items(old_select);
    selection->clear();

    // If any of the clones refer to the groups, unlink them and replace them with successors
    // in the items list.
    GSList *clones_to_unlink = NULL;
    for (std::vector<SPItem*>::const_iterator item = items.begin(); item != items.end(); item++) {
        SPUse *use = dynamic_cast<SPUse *>(*item);

        SPItem *original = use;
        while (dynamic_cast<SPUse *>(original)) {
            original = dynamic_cast<SPUse *>(original)->get_original();
        }

        if (g_slist_find(groups, original) != NULL) {
            clones_to_unlink = g_slist_prepend(clones_to_unlink, *item);
        }
    }

    // Unlink clones beginning from those with highest clone depth.
    // This way we can be sure than no additional automatic unlinking happens,
    // and the items in the list remain valid
    clones_to_unlink = g_slist_sort(clones_to_unlink, clone_depth_descending);

    for (GSList *item = clones_to_unlink; item; item = item->next) {
        SPUse *use = static_cast<SPUse *>(item->data);
        std::vector<SPItem*>::iterator items_node = std::find(items.begin(),items.end(), item->data);
        *items_node = use->unlink();
    }
    g_slist_free(clones_to_unlink);

    // do the actual work
    for (std::vector<SPItem*>::iterator item = items.begin(); item != items.end(); item++) {
        SPItem *obj = *item;

        // ungroup only the groups marked earlier
        if (g_slist_find(groups, *item) != NULL) {
        	std::vector<SPItem*> children;
            sp_item_group_ungroup(dynamic_cast<SPGroup *>(obj), children, false);
            // add the items resulting from ungrouping to the selection
            new_select.insert(new_select.end(),children.begin(),children.end());
            *item = NULL; // zero out the original pointer, which is no longer valid
        } else {
            // if not a group, keep in the selection
            new_select.push_back(*item);
        }
    }

    selection->addList(new_select);

    DocumentUndo::done(selection->layers()->getDocument(), SP_VERB_SELECTION_UNGROUP,
                       _("Ungroup"));
}

/** Replace all groups in the list with their member objects, recursively; returns a new list, frees old */
std::vector<SPItem*>
sp_degroup_list(std::vector<SPItem*> &items)
{
	std::vector<SPItem*> out;
    bool has_groups = false;
    for (std::vector<SPItem*>::const_iterator item=items.begin();item!=items.end();item++) {
        SPGroup *group = dynamic_cast<SPGroup *>(*item);
        if (!group) {
            out.push_back(*item);
        } else {
            has_groups = true;
            std::vector<SPItem*> members = sp_item_group_item_list(group);
            for (std::vector<SPItem*>::const_iterator member=members.begin();member!=members.end();member++) {
                out.push_back(*member);
            }
            members.clear();
        }
    }

    if (has_groups) { // recurse if we unwrapped a group - it may have contained others
        out = sp_degroup_list(out);
    }

    return out;
}


/** If items in the list have a common parent, return it, otherwise return NULL */
static SPGroup *
sp_item_list_common_parent_group(std::vector<SPItem*> const items)
{
    if (items.empty()) {
        return NULL;
    }
    SPObject *parent = items[0]->parent;
    // Strictly speaking this CAN happen, if user selects <svg> from Inkscape::XML editor
    if (!dynamic_cast<SPGroup *>(parent)) {
        return NULL;
    }
    for (std::vector<SPItem*>::const_iterator item=items.begin();item!=items.end();item++) {
    	if((*item)==items[0])continue;
        if ((*item)->parent != parent) {
            return NULL;
        }
    }

    return dynamic_cast<SPGroup *>(parent);
}

/** Finds out the minimum common bbox of the selected items. */
static Geom::OptRect
enclose_items(std::vector<SPItem*> const &items)
{
    g_assert(!items.empty());

    Geom::OptRect r;
    for (std::vector<SPItem*>::const_iterator i = items.begin();i!=items.end();i++) {
        r.unionWith((*i)->desktopVisualBounds());
    }
    return r;
}

// TODO determine if this is intentionally different from SPObject::getPrev()
static SPObject *prev_sibling(SPObject *child)
{
    SPObject *prev = 0;
    if ( child && dynamic_cast<SPGroup *>(child->parent) ) {
        prev = child->getPrev();
    }
    return prev;
}

bool sp_item_repr_compare_position_bool(SPObject const *first, SPObject const *second)
{
    return sp_repr_compare_position(((SPItem*)first)->getRepr(),
    		((SPItem*)second)->getRepr())<0;
}

void
sp_selection_raise(Inkscape::Selection *selection, SPDesktop *desktop)
{
    std::vector<SPItem*> items= selection->itemList();
    if (items.empty()) {
        selection_display_message(desktop, Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to raise."));
        return;
    }

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        selection_display_message(desktop, Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    Inkscape::XML::Node *grepr = const_cast<Inkscape::XML::Node *>(group->getRepr());

    /* Construct reverse-ordered list of selected children. */
    std::vector<SPItem*> rev(items);
    sort(rev.begin(),rev.end(),sp_item_repr_compare_position_bool);

    // Determine the common bbox of the selected items.
    Geom::OptRect selected = enclose_items(items);

    // Iterate over all objects in the selection (starting from top).
    if (selected) {
        for (std::vector<SPItem*>::const_iterator item=rev.begin();item!=rev.end();item++) {
            SPObject *child = *item;
            // for each selected object, find the next sibling
            for (SPObject *newref = child->next; newref; newref = newref->next) {
                // if the sibling is an item AND overlaps our selection,
                SPItem *newItem = dynamic_cast<SPItem *>(newref);
                if (newItem) {
                    Geom::OptRect newref_bbox = newItem->desktopVisualBounds();
                    if ( newref_bbox && selected->intersects(*newref_bbox) ) {
                        // AND if it's not one of our selected objects,
                        if ( std::find(items.begin(),items.end(),newref)==items.end()) {
                            // move the selected object after that sibling
                            grepr->changeOrder(child->getRepr(), newref->getRepr());
                        }
                        break;
                    }
                }
            }
        }
    }
    DocumentUndo::done(selection->layers()->getDocument(), SP_VERB_SELECTION_RAISE,
                       //TRANSLATORS: "Raise" means "to raise an object" in the undo history
                       C_("Undo action", "Raise"));
}

void sp_selection_raise_to_top(Inkscape::Selection *selection, SPDesktop *desktop)
{
    SPDocument *document = selection->layers()->getDocument();

    if (selection->isEmpty()) {
        selection_display_message(desktop, Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to raise to top."));
        return;
    }

    std::vector<SPItem*> items = selection->itemList();

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        selection_display_message(desktop, Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    std::vector<Inkscape::XML::Node*> rl(selection->reprList());
    sort(rl.begin(),rl.end(),sp_repr_compare_position_bool);

    for (std::vector<Inkscape::XML::Node*>::const_iterator l=rl.begin(); l!=rl.end();l++) {
        Inkscape::XML::Node *repr =(*l);
        repr->setPosition(-1);
    }

    DocumentUndo::done(document, SP_VERB_SELECTION_TO_FRONT,
                       _("Raise to top"));
}

void sp_selection_lower(Inkscape::Selection *selection, SPDesktop *desktop)
{
    std::vector<SPItem*> items = selection->itemList();
    if (items.empty()) {
        selection_display_message(desktop, Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to lower."));
        return;
    }

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        selection_display_message(desktop, Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    Inkscape::XML::Node *grepr = const_cast<Inkscape::XML::Node *>(group->getRepr());

    // Determine the common bbox of the selected items.
    Geom::OptRect selected = enclose_items(items);

    /* Construct direct-ordered list of selected children. */
    std::vector<SPItem*> rev(items);
    sort(rev.begin(),rev.end(),sp_item_repr_compare_position_bool);

    // Iterate over all objects in the selection (starting from top).
    if (selected) {
        for (std::vector<SPItem*>::const_reverse_iterator item=rev.rbegin();item!=rev.rend();item++) {
            SPObject *child = *item;
            // for each selected object, find the prev sibling
            for (SPObject *newref = prev_sibling(child); newref; newref = prev_sibling(newref)) {
                // if the sibling is an item AND overlaps our selection,
                SPItem *newItem = dynamic_cast<SPItem *>(newref);
                if (newItem) {
                    Geom::OptRect ref_bbox = newItem->desktopVisualBounds();
                    if ( ref_bbox && selected->intersects(*ref_bbox) ) {
                        // AND if it's not one of our selected objects,
                        if (items.end()==std::find(items.begin(),items.end(),newref)) {
                            // move the selected object before that sibling
                            SPObject *put_after = prev_sibling(newref);
                            if (put_after)
                                grepr->changeOrder(child->getRepr(), put_after->getRepr());
                            else
                                child->getRepr()->setPosition(0);
                        }
                        break;
                    }
                }
            }
        }
    }

    DocumentUndo::done(selection->layers()->getDocument(), SP_VERB_SELECTION_LOWER,
                       //TRANSLATORS: "Lower" means "to lower an object" in the undo history
                       C_("Undo action", "Lower"));
}

void sp_selection_lower_to_bottom(Inkscape::Selection *selection, SPDesktop *desktop)
{
    SPDocument *document = selection->layers()->getDocument();

    if (selection->isEmpty()) {
        selection_display_message(desktop, Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to lower to bottom."));
        return;
    }

    std::vector<SPItem*> items =selection->itemList();

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        selection_display_message(desktop, Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    std::vector<Inkscape::XML::Node*> rl(selection->reprList());
    sort(rl.begin(),rl.end(),sp_repr_compare_position_bool);

    for (std::vector<Inkscape::XML::Node*>::const_reverse_iterator l=rl.rbegin();l!=rl.rend();l++) {
        gint minpos;
        SPObject *pp, *pc;
        Inkscape::XML::Node *repr = (*l);
        pp = document->getObjectByRepr(repr->parent());
        minpos = 0;
        g_assert(dynamic_cast<SPGroup *>(pp));
        pc = pp->firstChild();
        while (!dynamic_cast<SPItem *>(pc)) {
            minpos += 1;
            pc = pc->next;
        }
        repr->setPosition(minpos);
    }

    DocumentUndo::done(document, SP_VERB_SELECTION_TO_BACK,
                       _("Lower to bottom"));
}

void
sp_undo(SPDesktop *desktop, SPDocument *)
{
    // No re/undo while dragging, too dangerous.
    if(desktop->getCanvas()->is_dragging) return;

    if (!DocumentUndo::undo(desktop->getDocument())) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Nothing to undo."));
    }
}

void
sp_redo(SPDesktop *desktop, SPDocument *)
{
    // No re/undo while dragging, too dangerous.
    if(desktop->getCanvas()->is_dragging) return;

    if (!DocumentUndo::redo(desktop->getDocument())) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Nothing to redo."));
    }
}

void sp_selection_cut(SPDesktop *desktop)
{
    sp_selection_copy(desktop);
    sp_selection_delete(desktop);
}

/**
 * \pre item != NULL
 */
SPCSSAttr *
take_style_from_item(SPObject *object)
{
    // CPPIFY:
    // This function should only take SPItems, but currently SPString is not an Item.

    // write the complete cascaded style, context-free
    SPCSSAttr *css = sp_css_attr_from_object(object, SP_STYLE_FLAG_ALWAYS);
    if (css == NULL)
        return NULL;

    if ((dynamic_cast<SPGroup *>(object) && object->children) ||
        (dynamic_cast<SPText *>(object) && object->children && object->children->next == NULL)) {
        // if this is a text with exactly one tspan child, merge the style of that tspan as well
        // If this is a group, merge the style of its topmost (last) child with style
        for (SPObject *last_element = object->lastChild(); last_element != NULL; last_element = last_element->getPrev()) {
            if ( last_element->style ) {
                SPCSSAttr *temp = sp_css_attr_from_object(last_element, SP_STYLE_FLAG_IFSET);
                if (temp) {
                    sp_repr_css_merge(css, temp);
                    sp_repr_css_attr_unref(temp);
                }
                break;
            }
        }
    }

    // Remove black-listed properties (those that should not be used in a default style)
    css = sp_css_attr_unset_blacklist(css);

    if (!(dynamic_cast<SPText *>(object) || dynamic_cast<SPTSpan *>(object) || dynamic_cast<SPTRef *>(object) || dynamic_cast<SPString *>(object))) {
        // do not copy text properties from non-text objects, it's confusing
        css = sp_css_attr_unset_text(css);
    }


    SPItem *item = dynamic_cast<SPItem *>(object);
    if (item) {
        // FIXME: also transform gradient/pattern fills, by forking? NO, this must be nondestructive
        double ex = item->i2doc_affine().descrim();
        if (ex != 1.0) {
            css = sp_css_attr_scale(css, ex);
        }
    }

    return css;
}


void sp_selection_copy(SPDesktop *desktop)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    cm->copy(desktop);
}

void sp_selection_paste(SPDesktop *desktop, bool in_place)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->paste(desktop, in_place)) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_PASTE, _("Paste"));
    }
}

void sp_selection_paste_style(SPDesktop *desktop)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pasteStyle(desktop)) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_PASTE_STYLE, _("Paste style"));
    }
}


void sp_selection_paste_livepatheffect(SPDesktop *desktop)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pastePathEffect(desktop)) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_PASTE_LIVEPATHEFFECT,
                           _("Paste live path effect"));
    }
}


static void sp_selection_remove_livepatheffect_impl(SPItem *item)
{
    if ( SPLPEItem *lpeitem = dynamic_cast<SPLPEItem*>(item) ) {
        if ( lpeitem->hasPathEffect() ) {
            lpeitem->removeAllPathEffects(false);
        }
    }
}

void sp_selection_remove_livepatheffect(SPDesktop *desktop)
{
    if (desktop == NULL) return;

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to remove live path effects from."));
        return;
    }
    std::vector<SPItem*> list=selection->itemList();
    for ( std::vector<SPItem*>::const_iterator itemlist=list.begin();itemlist!=list.end();itemlist++) {
        SPItem *item = *itemlist;

        sp_selection_remove_livepatheffect_impl(item);

    }

    DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_REMOVE_LIVEPATHEFFECT,
                       _("Remove live path effect"));
}

void sp_selection_remove_filter(SPDesktop *desktop)
{
    if (desktop == NULL) return;

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to remove filters from."));
        return;
    }

    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_unset_property(css, "filter");
    sp_desktop_set_style(desktop, css);
    sp_repr_css_attr_unref(css);

    DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_REMOVE_FILTER,
                       _("Remove filter"));
}


void sp_selection_paste_size(SPDesktop *desktop, bool apply_x, bool apply_y)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pasteSize(desktop, false, apply_x, apply_y)) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_PASTE_SIZE,
                           _("Paste size"));
    }
}

void sp_selection_paste_size_separately(SPDesktop *desktop, bool apply_x, bool apply_y)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pasteSize(desktop, true, apply_x, apply_y)) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_PASTE_SIZE_SEPARATELY,
                           _("Paste size separately"));
    }
}

/**
 * Ensures that the clones of objects are not modified when moving objects between layers.
 * Calls the same function as ungroup
 */
void sp_selection_change_layer_maintain_clones(std::vector<SPItem*> const &items,SPObject *where)
{
    for (std::vector<SPItem*>::const_iterator i = items.begin(); i != items.end(); i++) {
        SPItem *item = *i;
        if (item) {
            SPItem *oldparent = dynamic_cast<SPItem *>(item->parent);
            SPItem *newparent = dynamic_cast<SPItem *>(where);
            sp_item_group_ungroup_handle_clones(item,
                    (oldparent->i2doc_affine())
                    *((newparent->i2doc_affine()).inverse()));
        }
    }
}


void sp_selection_to_next_layer(SPDesktop *dt, bool suppressDone)
{
    Inkscape::Selection *selection = dt->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to move to the layer above."));
        return;
    }

    std::vector<SPItem*> items(selection->itemList());

    bool no_more = false; // Set to true, if no more layers above
    SPObject *next=Inkscape::next_layer(dt->currentRoot(), dt->currentLayer());
    if (next) {
        sp_selection_change_layer_maintain_clones(items,next);
        std::vector<Inkscape::XML::Node*> temp_clip;
        sp_selection_copy_impl(items, temp_clip, dt->doc()->getReprDoc());
        sp_selection_delete_impl(items, false, false);
        next=Inkscape::next_layer(dt->currentRoot(), dt->currentLayer()); // Fixes bug 1482973: crash while moving layers
        std::vector<Inkscape::XML::Node*> copied;
        if (next) {
            copied = sp_selection_paste_impl(dt->getDocument(), next, temp_clip);
        } else {
            copied = sp_selection_paste_impl(dt->getDocument(), dt->currentLayer(), temp_clip);
            no_more = true;
        }
        selection->setReprList(copied);
        if (next) dt->setCurrentLayer(next);
        if ( !suppressDone ) {
            DocumentUndo::done(dt->getDocument(), SP_VERB_LAYER_MOVE_TO_NEXT,
                               _("Raise to next layer"));
        }
    } else {
        no_more = true;
    }

    if (no_more) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No more layers above."));
    }

}

void sp_selection_to_prev_layer(SPDesktop *dt, bool suppressDone)
{
    Inkscape::Selection *selection = dt->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to move to the layer below."));
        return;
    }

    const std::vector<SPItem*> items(selection->itemList());

    bool no_more = false; // Set to true, if no more layers below
    SPObject *next=Inkscape::previous_layer(dt->currentRoot(), dt->currentLayer());
    if (next) {
        sp_selection_change_layer_maintain_clones(items,next);
        std::vector<Inkscape::XML::Node*> temp_clip;
        sp_selection_copy_impl(items, temp_clip, dt->doc()->getReprDoc()); // we're in the same doc, so no need to copy defs
        sp_selection_delete_impl(items, false, false);
        next=Inkscape::previous_layer(dt->currentRoot(), dt->currentLayer()); // Fixes bug 1482973: crash while moving layers
        std::vector<Inkscape::XML::Node*> copied;
        if (next) {
            copied = sp_selection_paste_impl(dt->getDocument(), next, temp_clip);
        } else {
            copied = sp_selection_paste_impl(dt->getDocument(), dt->currentLayer(), temp_clip);
            no_more = true;
        }
        selection->setReprList( copied);
        if (next) dt->setCurrentLayer(next);
        if ( !suppressDone ) {
            DocumentUndo::done(dt->getDocument(), SP_VERB_LAYER_MOVE_TO_PREV,
                               _("Lower to previous layer"));
        }
    } else {
        no_more = true;
    }

    if (no_more) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No more layers below."));
    }
}

void sp_selection_to_layer(SPDesktop *dt, SPObject *moveto, bool suppressDone)
{
    Inkscape::Selection *selection = dt->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to move."));
        return;
    }

    std::vector<SPItem*> items(selection->itemList());

    if (moveto) {
        sp_selection_change_layer_maintain_clones(items,moveto);
        std::vector<Inkscape::XML::Node*> temp_clip;
        sp_selection_copy_impl(items, temp_clip, dt->doc()->getReprDoc()); // we're in the same doc, so no need to copy defs
        sp_selection_delete_impl(items, false, false);
        std::vector<Inkscape::XML::Node*> copied = sp_selection_paste_impl(dt->getDocument(), moveto, temp_clip);
        selection->setReprList(copied);
        if (!temp_clip.empty()) temp_clip.clear();
        if (moveto) dt->setCurrentLayer(moveto);
        if ( !suppressDone ) {
            DocumentUndo::done(dt->getDocument(), SP_VERB_LAYER_MOVE_TO,
                               _("Move selection to layer"));
        }
    }
}

static bool
selection_contains_original(SPItem *item, Inkscape::Selection *selection)
{
    bool contains_original = false;

    SPItem *item_use = item;
    SPItem *item_use_first = item;
    SPUse *use = dynamic_cast<SPUse *>(item_use);
    while (use && item_use && !contains_original)
    {
        item_use = use->get_original();
        use = dynamic_cast<SPUse *>(item_use);
        contains_original |= selection->includes(item_use);
        if (item_use == item_use_first)
            break;
    }

    // If it's a tref, check whether the object containing the character
    // data is part of the selection
    SPTRef *tref = dynamic_cast<SPTRef *>(item);
    if (!contains_original && tref) {
        contains_original = selection->includes(tref->getObjectReferredTo());
    }

    return contains_original;
}


static bool
selection_contains_both_clone_and_original(Inkscape::Selection *selection)
{
    bool clone_with_original = false;
    std::vector<SPItem*> items = selection->itemList();
    for (std::vector<SPItem*>::const_iterator l=items.begin();l!=items.end() ;l++) {
        SPItem *item = *l;
        if (item) {
            clone_with_original |= selection_contains_original(item, selection);
            if (clone_with_original)
                break;
        }
    }
    return clone_with_original;
}

/** Apply matrix to the selection.  \a set_i2d is normally true, which means objects are in the
original transform, synced with their reprs, and need to jump to the new transform in one go. A
value of set_i2d==false is only used by seltrans when it's dragging objects live (not outlines); in
that case, items are already in the new position, but the repr is in the old, and this function
then simply updates the repr from item->transform.
 */
void sp_selection_apply_affine(Inkscape::Selection *selection, Geom::Affine const &affine, bool set_i2d, bool compensate, bool adjust_transf_center)
{
    if (selection->isEmpty())
        return;

    // For each perspective with a box in selection, check whether all boxes are selected and
    // unlink all non-selected boxes.
    Persp3D *persp;
    Persp3D *transf_persp;
    std::list<Persp3D *> plist = selection->perspList();
    for (std::list<Persp3D *>::iterator i = plist.begin(); i != plist.end(); ++i) {
        persp = (Persp3D *) (*i);

        if (!persp3d_has_all_boxes_in_selection (persp, selection)) {
            std::list<SPBox3D *> selboxes = selection->box3DList(persp);

            // create a new perspective as a copy of the current one and link the selected boxes to it
            transf_persp = persp3d_create_xml_element (persp->document, persp->perspective_impl);

            for (std::list<SPBox3D *>::iterator b = selboxes.begin(); b != selboxes.end(); ++b)
                box3d_switch_perspectives(*b, persp, transf_persp);
        } else {
            transf_persp = persp;
        }

        persp3d_apply_affine_transformation(transf_persp, affine);
    }
    std::vector<SPItem*> items = selection->itemList();
    for (std::vector<SPItem*>::const_iterator l=items.begin();l!=items.end() ;l++) {
        SPItem *item = *l;

        if( dynamic_cast<SPRoot *>(item) ) {
            // An SVG element cannot have a transform. We could change 'x' and 'y' in response
            // to a translation... but leave that for another day.
            selection->desktop()->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot transform an embedded SVG."));
            break;
        }

        Geom::Point old_center(0,0);
        if (set_i2d && item->isCenterSet())
            old_center = item->getCenter();

#if 0 /* Re-enable this once persistent guides have a graphical indication.
         At the time of writing, this is the only place to re-enable. */
        sp_item_update_cns(*item, selection->desktop());
#endif

        // we're moving both a clone and its original or any ancestor in clone chain?
        bool transform_clone_with_original = selection_contains_original(item, selection);

        // ...both a text-on-path and its path?
        bool transform_textpath_with_path = ((dynamic_cast<SPText *>(item) && item->firstChild() && dynamic_cast<SPTextPath *>(item->firstChild()))
                                             && selection->includes( sp_textpath_get_path_item(dynamic_cast<SPTextPath *>(item->firstChild())) ));

        // ...both a flowtext and its frame?
        bool transform_flowtext_with_frame = (dynamic_cast<SPFlowtext *>(item) && selection->includes( dynamic_cast<SPFlowtext *>(item)->get_frame(NULL))); // (only the first frame is checked so far)

        // ...both an offset and its source?
        bool transform_offset_with_source = (dynamic_cast<SPOffset *>(item) && dynamic_cast<SPOffset *>(item)->sourceHref) && selection->includes( sp_offset_get_source(dynamic_cast<SPOffset *>(item)) );

        // If we're moving a connector, we want to detach it
        // from shapes that aren't part of the selection, but
        // leave it attached if they are
        if (Inkscape::UI::Tools::cc_item_is_connector(item)) {
            SPPath *path = dynamic_cast<SPPath *>(item);
            if (path) {
                SPItem *attItem[2] = {0, 0};
                path->connEndPair.getAttachedItems(attItem);
                for (int n = 0; n < 2; ++n) {
                    if (!selection->includes(attItem[n])) {
                        sp_conn_end_detach(item, n);
                    }
                }
            } else {
                g_assert_not_reached();
            }
        }

        // "clones are unmoved when original is moved" preference
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        bool prefs_unmoved = (compensation == SP_CLONE_COMPENSATION_UNMOVED);
        bool prefs_parallel = (compensation == SP_CLONE_COMPENSATION_PARALLEL);

        /* If this is a clone and it's selected along with its original, do not move it;
         * it will feel the transform of its original and respond to it itself.
         * Without this, a clone is doubly transformed, very unintuitive.
         *
         * Same for textpath if we are also doing ANY transform to its path: do not touch textpath,
         * letters cannot be squeezed or rotated anyway, they only refill the changed path.
         * Same for linked offset if we are also moving its source: do not move it. */
        if (transform_textpath_with_path) {
            // Restore item->transform field from the repr, in case it was changed by seltrans.
            item->readAttr( "transform" );
        } else if (transform_flowtext_with_frame) {
            // apply the inverse of the region's transform to the <use> so that the flow remains
            // the same (even though the output itself gets transformed)
            for ( SPObject *region = item->firstChild() ; region ; region = region->getNext() ) {
                if (dynamic_cast<SPFlowregion *>(region) || dynamic_cast<SPFlowregionExclude *>(region)) {
                    for ( SPObject *item = region->firstChild() ; item ; item = item->getNext() ) {
                        SPUse *use = dynamic_cast<SPUse *>(item);
                        if ( use ) {
                            use->doWriteTransform(use->getRepr(), use->transform.inverse(), NULL, compensate);
                        }
                    }
                }
            }
        } else if (transform_clone_with_original || transform_offset_with_source) {
            // We are transforming a clone along with its original. The below matrix juggling is
            // necessary to ensure that they transform as a whole, i.e. the clone's induced
            // transform and its move compensation are both cancelled out.

            // restore item->transform field from the repr, in case it was changed by seltrans
            item->readAttr( "transform" );

            // calculate the matrix we need to apply to the clone to cancel its induced transform from its original
            Geom::Affine parent2dt;
            {
                SPItem *parentItem = dynamic_cast<SPItem *>(item->parent);
                if (parentItem) {
                    parent2dt = parentItem->i2dt_affine();
                } else {
                    g_assert_not_reached();
                }
            }
            Geom::Affine t = parent2dt * affine * parent2dt.inverse();
            Geom::Affine t_inv = t.inverse();
            Geom::Affine result = t_inv * item->transform * t;

            if (transform_clone_with_original && (prefs_parallel || prefs_unmoved) && affine.isTranslation()) {
                // we need to cancel out the move compensation, too

                // find out the clone move, same as in sp_use_move_compensate
                Geom::Affine parent;
                {
                    SPUse *use = dynamic_cast<SPUse *>(item);
                    if (use) {
                        parent = use->get_parent_transform();
                    } else {
                        g_assert_not_reached();
                    }
                }
                Geom::Affine clone_move = parent.inverse() * t * parent;

                if (prefs_parallel) {
                    Geom::Affine move = result * clone_move * t_inv;
                    item->doWriteTransform(item->getRepr(), move, &move, compensate);

                } else if (prefs_unmoved) {
                    //if (dynamic_cast<SPUse *>(sp_use_get_original(dynamic_cast<SPUse *>(item))))
                    //    clone_move = Geom::identity();
                    Geom::Affine move = result * clone_move;
                    item->doWriteTransform(item->getRepr(), move, &t, compensate);
                }

            } else if (transform_offset_with_source && (prefs_parallel || prefs_unmoved) && affine.isTranslation()){
                Geom::Affine parent = item->transform;
                Geom::Affine offset_move = parent.inverse() * t * parent;

                if (prefs_parallel) {
                    Geom::Affine move = result * offset_move * t_inv;
                    item->doWriteTransform(item->getRepr(), move, &move, compensate);

                } else if (prefs_unmoved) {
                    Geom::Affine move = result * offset_move;
                    item->doWriteTransform(item->getRepr(), move, &t, compensate);
                }

            } else {
                // just apply the result
                item->doWriteTransform(item->getRepr(), result, &t, compensate);
            }

        } else {
            if (set_i2d) {
                item->set_i2d_affine(item->i2dt_affine() * (Geom::Affine)affine);
            }
            item->doWriteTransform(item->getRepr(), item->transform, NULL, compensate);
        }

        if (adjust_transf_center) { // The transformation center should not be touched in case of pasting or importing, which is allowed by this if clause
            // if we're moving the actual object, not just updating the repr, we can transform the
            // center by the same matrix (only necessary for non-translations)
            if (set_i2d && item->isCenterSet() && !(affine.isTranslation() || affine.isIdentity())) {
                item->setCenter(old_center * affine);
                item->updateRepr();
            }
        }
    }
}

void sp_selection_remove_transform(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = desktop->getSelection();

    std::vector<Inkscape::XML::Node*> items = selection->reprList();
    for (std::vector<Inkscape::XML::Node*>::const_iterator l=items.begin();l!=items.end() ;l++) {
        (*l)->setAttribute("transform", NULL, false);
    }

    DocumentUndo::done(desktop->getDocument(), SP_VERB_OBJECT_FLATTEN,
                       _("Remove transform"));
}

void
sp_selection_scale_absolute(Inkscape::Selection *selection,
                            double const x0, double const x1,
                            double const y0, double const y1)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect bbox = selection->visualBounds();
    if ( !bbox ) {
        return;
    }

    Geom::Translate const p2o(-bbox->min());

    Geom::Scale const newSize(x1 - x0,
                              y1 - y0);
    Geom::Scale const scale( newSize * Geom::Scale(bbox->dimensions()).inverse() );
    Geom::Translate const o2n(x0, y0);
    Geom::Affine const final( p2o * scale * o2n );

    sp_selection_apply_affine(selection, final);
}


void sp_selection_scale_relative(Inkscape::Selection *selection, Geom::Point const &align, Geom::Scale const &scale)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect bbox = selection->visualBounds();

    if ( !bbox ) {
        return;
    }

    // FIXME: ARBITRARY LIMIT: don't try to scale above 1 Mpx, it won't display properly and will crash sooner or later anyway
    if ( bbox->dimensions()[Geom::X] * scale[Geom::X] > 1e6  ||
         bbox->dimensions()[Geom::Y] * scale[Geom::Y] > 1e6 )
    {
        return;
    }

    Geom::Translate const n2d(-align);
    Geom::Translate const d2n(align);
    Geom::Affine const final( n2d * scale * d2n );
    sp_selection_apply_affine(selection, final);
}

void
sp_selection_rotate_relative(Inkscape::Selection *selection, Geom::Point const &center, gdouble const angle_degrees)
{
    Geom::Translate const d2n(center);
    Geom::Translate const n2d(-center);
    Geom::Rotate const rotate(Geom::Rotate::from_degrees(angle_degrees));
    Geom::Affine const final( Geom::Affine(n2d) * rotate * d2n );
    sp_selection_apply_affine(selection, final);
}

void
sp_selection_skew_relative(Inkscape::Selection *selection, Geom::Point const &align, double dx, double dy)
{
    Geom::Translate const d2n(align);
    Geom::Translate const n2d(-align);
    Geom::Affine const skew(1, dy,
                            dx, 1,
                            0, 0);
    Geom::Affine const final( n2d * skew * d2n );
    sp_selection_apply_affine(selection, final);
}

void sp_selection_move_relative(Inkscape::Selection *selection, Geom::Point const &move, bool compensate)
{
    sp_selection_apply_affine(selection, Geom::Affine(Geom::Translate(move)), true, compensate);
}

void sp_selection_move_relative(Inkscape::Selection *selection, double dx, double dy)
{
    sp_selection_apply_affine(selection, Geom::Affine(Geom::Translate(dx, dy)));
}

/**
 * Rotates selected objects 90 degrees, either clock-wise or counter-clockwise, depending on the value of ccw.
 */
void sp_selection_rotate_90(SPDesktop *desktop, bool ccw)
{
    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty())
        return;

    std::vector<SPItem*> items = selection->itemList();
    Geom::Rotate const rot_90(Geom::Point(0, ccw ? 1 : -1)); // pos. or neg. rotation, depending on the value of ccw
    for (std::vector<SPItem*>::const_iterator l=items.begin();l!=items.end() ;l++) {
        SPItem *item = *l;
        if (item) {
            sp_item_rotate_rel(item, rot_90);
        } else {
            g_assert_not_reached();
        }
    }

    DocumentUndo::done(desktop->getDocument(),
                       ccw ? SP_VERB_OBJECT_ROTATE_90_CCW : SP_VERB_OBJECT_ROTATE_90_CW,
                       ccw ? _("Rotate 90\xc2\xb0 CCW") : _("Rotate 90\xc2\xb0 CW"));
}

void
sp_selection_rotate(Inkscape::Selection *selection, gdouble const angle_degrees)
{
    if (selection->isEmpty())
        return;

    boost::optional<Geom::Point> center = selection->center();
    if (!center) {
        return;
    }

    sp_selection_rotate_relative(selection, *center, angle_degrees);

    DocumentUndo::maybeDone(selection->desktop()->getDocument(),
                            ( ( angle_degrees > 0 )
                              ? "selector:rotate:ccw"
                              : "selector:rotate:cw" ),
                            SP_VERB_CONTEXT_SELECT,
                            _("Rotate"));
}

/*
 * Selects all the visible items with the same fill and/or stroke color/style as the items in the current selection
 *
 * Params:
 * desktop - set the selection on this desktop
 * fill - select objects matching fill
 * stroke - select objects matching stroke
 */
void sp_select_same_fill_stroke_style(SPDesktop *desktop, gboolean fill, gboolean stroke, gboolean style)
{
    if (!desktop) {
        return;
    }

    if (!fill && !stroke && !style) {
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);
    bool ingroups = TRUE;
    std::vector<SPItem*> x,y;
    std::vector<SPItem*> all_list = get_all_items(x, desktop->currentRoot(), desktop, onlyvisible, onlysensitive, ingroups, y);
    std::vector<SPItem*> all_matches;

    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> items = selection->itemList();

    std::vector<SPItem*> tmp;
    for (std::vector<SPItem*>::const_iterator iter=all_list.begin();iter!=all_list.end();iter++) {
        if(!SP_IS_GROUP(*iter)){
            tmp.push_back(*iter);
        }
    }
    all_list=tmp;

    for (std::vector<SPItem*>::const_iterator sel_iter=items.begin();sel_iter!=items.end();sel_iter++) {
        SPItem *sel = *sel_iter;
        std::vector<SPItem*> matches = all_list;
        if (fill && stroke && style) {
            matches = sp_get_same_style(sel, matches);
        }
        else if (fill) {
            matches = sp_get_same_style(sel, matches, SP_FILL_COLOR);
        }
        else if (stroke) {
            matches = sp_get_same_style(sel, matches, SP_STROKE_COLOR);
        }
        else if (style) {
            matches = sp_get_same_style(sel, matches,SP_STROKE_STYLE_ALL);
        }
        all_matches.insert(all_matches.end(), matches.begin(),matches.end());
    }

    selection->clear();
    selection->setList(all_matches);

}


/*
 * Selects all the visible items with the same object type as the items in the current selection
 *
 * Params:
 * desktop - set the selection on this desktop
 */
void sp_select_same_object_type(SPDesktop *desktop)
{
    if (!desktop) {
        return;
    }


    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);
    bool ingroups = TRUE;
    std::vector<SPItem*> x,y;
    std::vector<SPItem*> all_list = get_all_items(x, desktop->currentRoot(), desktop, onlyvisible, onlysensitive, ingroups, y);
    std::vector<SPItem*> matches = all_list;

    Inkscape::Selection *selection = desktop->getSelection();

    std::vector<SPItem*> items=selection->itemList();
    for (std::vector<SPItem*>::const_iterator sel_iter=items.begin();sel_iter!=items.end();sel_iter++) {
        SPItem *sel = *sel_iter;
        if (sel) {
            matches = sp_get_same_object_type(sel, matches);
        } else {
            g_assert_not_reached();
        }
    }

    selection->clear();
    selection->setList(matches);

}



/*
 * Find all items in src list that have the same fill or stroke style as sel
 * Return the list of matching items
 */
std::vector<SPItem*> sp_get_same_fill_or_stroke_color(SPItem *sel, std::vector<SPItem*> &src, SPSelectStrokeStyleType type)
{
    std::vector<SPItem*> matches ;
    gboolean match = false;

    SPIPaint *sel_paint = (type == SP_FILL_COLOR) ? &(sel->style->fill) : &(sel->style->stroke);

    for (std::vector<SPItem*>::const_reverse_iterator i=src.rbegin();i!=src.rend();i++) {
        SPItem *iter = *i;
        if (iter) {
            SPIPaint *iter_paint = (type == SP_FILL_COLOR) ? &(iter->style->fill) : &(iter->style->stroke);
            match = false;
            if (sel_paint->isColor() && iter_paint->isColor() // color == color comparision doesnt seem to work here.
                && (sel_paint->value.color.toRGBA32(1.0) == iter_paint->value.color.toRGBA32(1.0))) {
                match = true;
            } else if (sel_paint->isPaintserver() && iter_paint->isPaintserver()) {

                SPPaintServer *sel_server =
                    (type == SP_FILL_COLOR) ? sel->style->getFillPaintServer() : sel->style->getStrokePaintServer();
                SPPaintServer *iter_server =
                    (type == SP_FILL_COLOR) ? iter->style->getFillPaintServer() : iter->style->getStrokePaintServer();

                if ((dynamic_cast<SPLinearGradient *>(sel_server) || dynamic_cast<SPRadialGradient *>(sel_server) ||
                     (dynamic_cast<SPGradient *>(sel_server) && dynamic_cast<SPGradient *>(sel_server)->getVector()->isSwatch()))
                    &&
                    (dynamic_cast<SPLinearGradient *>(iter_server) || dynamic_cast<SPRadialGradient *>(iter_server) ||
                     (dynamic_cast<SPGradient *>(iter_server) && dynamic_cast<SPGradient *>(iter_server)->getVector()->isSwatch()))) {
                    SPGradient *sel_vector = dynamic_cast<SPGradient *>(sel_server)->getVector();
                    SPGradient *iter_vector = dynamic_cast<SPGradient *>(iter_server)->getVector();
                    if (sel_vector == iter_vector) {
                        match = true;
                    }

                } else if (dynamic_cast<SPPattern *>(sel_server) && dynamic_cast<SPPattern *>(iter_server)) {
                    SPPattern *sel_pat = dynamic_cast<SPPattern *>(sel_server)->rootPattern();
                    SPPattern *iter_pat = dynamic_cast<SPPattern *>(iter_server)->rootPattern();
                    if (sel_pat == iter_pat) {
                        match = true;
                    }
                }
            } else if (sel_paint->isNone() && iter_paint->isNone()) {
                match = true;
            } else if (sel_paint->isNoneSet() && iter_paint->isNoneSet()) {
                match = true;
            }

            if (match) {
                matches.push_back(iter);
            }
        } else {
            g_assert_not_reached();
        }
    }

    return matches;
}

static bool item_type_match (SPItem *i, SPItem *j)
{
    if ( dynamic_cast<SPRect *>(i)) {
        return ( dynamic_cast<SPRect *>(j) );

    } else if (dynamic_cast<SPGenericEllipse *>(i)) {
        return (dynamic_cast<SPGenericEllipse *>(j));

    } else if (dynamic_cast<SPStar *>(i) || dynamic_cast<SPPolygon *>(i)) {
        return (dynamic_cast<SPStar *>(j) || dynamic_cast<SPPolygon *>(j)) ;

    } else if (dynamic_cast<SPSpiral *>(i)) {
        return (dynamic_cast<SPSpiral *>(j));

    } else if (dynamic_cast<SPPath *>(i) || dynamic_cast<SPLine *>(i) || dynamic_cast<SPPolyLine *>(i)) {
        return (dynamic_cast<SPPath *>(j) || dynamic_cast<SPLine *>(j) || dynamic_cast<SPPolyLine *>(j));

    } else if (dynamic_cast<SPText *>(i) || dynamic_cast<SPFlowtext *>(i) || dynamic_cast<SPTSpan *>(i) || dynamic_cast<SPTRef *>(i) || dynamic_cast<SPString *>(i)) {
        return (dynamic_cast<SPText *>(j) || dynamic_cast<SPFlowtext *>(j) || dynamic_cast<SPTSpan *>(j) || dynamic_cast<SPTRef *>(j) || dynamic_cast<SPString *>(j));

    }  else if (dynamic_cast<SPUse *>(i)) {
        return (dynamic_cast<SPUse *>(j)) ;

    } else if (dynamic_cast<SPImage *>(i)) {
        return (dynamic_cast<SPImage *>(j));

    } else if (dynamic_cast<SPOffset *>(i) && dynamic_cast<SPOffset *>(i)->sourceHref) {   // Linked offset
        return (dynamic_cast<SPOffset *>(j) && dynamic_cast<SPOffset *>(j)->sourceHref);

    }  else if (dynamic_cast<SPOffset *>(i) && !dynamic_cast<SPOffset *>(i)->sourceHref) { // Dynamic offset
        return (dynamic_cast<SPOffset *>(j) && !dynamic_cast<SPOffset *>(j)->sourceHref);

    }

    return false;
}

/*
 * Find all items in src list that have the same object type as sel by type
 * Return the list of matching items
 */
std::vector<SPItem*> sp_get_same_object_type(SPItem *sel, std::vector<SPItem*> &src)
{
    std::vector<SPItem*> matches;

    for (std::vector<SPItem*>::const_reverse_iterator i=src.rbegin();i!=src.rend();i++) {
        SPItem *item = *i;
        if (item && item_type_match(sel, item) && !item->cloned) {
            matches.push_back(item);
        }
    }
    return matches;
}

GSList *sp_get_same_fill_or_stroke_color(SPItem *sel, GSList *src, SPSelectStrokeStyleType type);

/*
 * Find all items in src list that have the same stroke style as sel by type
 * Return the list of matching items
 */
std::vector<SPItem*> sp_get_same_style(SPItem *sel, std::vector<SPItem*> &src, SPSelectStrokeStyleType type)
{
    std::vector<SPItem*> matches;
    bool match = false;

    SPStyle *sel_style = sel->style;

    if (type == SP_FILL_COLOR || type == SP_STYLE_ALL) {
        src = sp_get_same_fill_or_stroke_color(sel, src, SP_FILL_COLOR);
    }
    if (type == SP_STROKE_COLOR || type == SP_STYLE_ALL) {
        src = sp_get_same_fill_or_stroke_color(sel, src, SP_STROKE_COLOR);
    }

    /*
     * Stroke width needs to handle transformations, so call this function
     * to get the transformed stroke width
     */
    std::vector<SPItem*> objects;
    SPStyle *sel_style_for_width = NULL;
    if (type == SP_STROKE_STYLE_WIDTH || type == SP_STROKE_STYLE_ALL || type==SP_STYLE_ALL ) {
        objects.push_back(sel);
        sel_style_for_width = new SPStyle(SP_ACTIVE_DOCUMENT);
        objects_query_strokewidth (objects, sel_style_for_width);
    }
    bool match_g;
    for (std::vector<SPItem*>::const_iterator i=src.begin();i!=src.end();i++) {
        SPItem *iter = *i;
        if (iter) {
            match_g=true;
            SPStyle *iter_style = iter->style;
            match = true;

            if (type == SP_STROKE_STYLE_WIDTH|| type == SP_STROKE_STYLE_ALL|| type==SP_STYLE_ALL) {
                match = (sel_style->stroke_width.set == iter_style->stroke_width.set);
                if (sel_style->stroke_width.set && iter_style->stroke_width.set) {
                    std::vector<SPItem*> objects;
                    objects.insert(objects.begin(),iter);
                    SPStyle tmp_style(SP_ACTIVE_DOCUMENT);
                    objects_query_strokewidth (objects, &tmp_style);

                    if (sel_style_for_width) {
                        match = (sel_style_for_width->stroke_width.computed == tmp_style.stroke_width.computed);
                    }
                }
            }
            match_g = match_g && match;
            if (type == SP_STROKE_STYLE_DASHES|| type == SP_STROKE_STYLE_ALL || type==SP_STYLE_ALL) {
                match = (sel_style->stroke_dasharray.set == iter_style->stroke_dasharray.set);
                if (sel_style->stroke_dasharray.set && iter_style->stroke_dasharray.set) {
                    match = (sel_style->stroke_dasharray.values == iter_style->stroke_dasharray.values);
                }
            }
            match_g = match_g && match;
            if (type == SP_STROKE_STYLE_MARKERS|| type == SP_STROKE_STYLE_ALL|| type==SP_STYLE_ALL) {
                match = true;
                int len = sizeof(sel_style->marker)/sizeof(SPIString);
                for (int i = 0; i < len; i++) {
                    match = (sel_style->marker_ptrs[i]->set == iter_style->marker_ptrs[i]->set);
                    if (sel_style->marker_ptrs[i]->set && iter_style->marker_ptrs[i]->set &&
                        (strcmp(sel_style->marker_ptrs[i]->value, iter_style->marker_ptrs[i]->value))) {
                        match = false;
                        break;
                    }
                }
            }
			match_g = match_g && match;
            if (match_g) {
                while (iter->cloned) iter=dynamic_cast<SPItem *>(iter->parent);
                matches.insert(matches.begin(),iter);
            }
        } else {
            g_assert_not_reached();
        }
    }

    if( sel_style_for_width != NULL ) delete sel_style_for_width;
    return matches;
}

// helper function:
static
Geom::Point
cornerFarthestFrom(Geom::Rect const &r, Geom::Point const &p){
    Geom::Point m = r.midpoint();
    unsigned i = 0;
    if (p[X] < m[X]) {
        i = 1;
    }
    if (p[Y] < m[Y]) {
        i = 3 - i;
    }
    return r.corner(i);
}

/**
\param  angle   the angle in "angular pixels", i.e. how many visible pixels must move the outermost point of the rotated object
*/
void
sp_selection_rotate_screen(Inkscape::Selection *selection, gdouble angle)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect bbox = selection->visualBounds();
    boost::optional<Geom::Point> center = selection->center();

    if ( !bbox || !center ) {
        return;
    }

    gdouble const zoom = selection->desktop()->current_zoom();
    gdouble const zmove = angle / zoom;
    gdouble const r = Geom::L2(cornerFarthestFrom(*bbox, *center) - *center);

    gdouble const zangle = 180 * atan2(zmove, r) / M_PI;

    sp_selection_rotate_relative(selection, *center, zangle);

    DocumentUndo::maybeDone(selection->desktop()->getDocument(),
                            ( (angle > 0)
                              ? "selector:rotate:ccw"
                              : "selector:rotate:cw" ),
                            SP_VERB_CONTEXT_SELECT,
                            _("Rotate by pixels"));
}

void
sp_selection_scale(Inkscape::Selection *selection, gdouble grow)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect bbox = selection->visualBounds();
    if (!bbox) {
        return;
    }

    Geom::Point const center(bbox->midpoint());

    // you can't scale "do nizhe pola" (below zero)
    double const max_len = bbox->maxExtent();
    if ( max_len + grow <= 1e-3 ) {
        return;
    }

    double const times = 1.0 + grow / max_len;
    sp_selection_scale_relative(selection, center, Geom::Scale(times, times));

    DocumentUndo::maybeDone(selection->desktop()->getDocument(),
                            ( (grow > 0)
                              ? "selector:scale:larger"
                              : "selector:scale:smaller" ),
                            SP_VERB_CONTEXT_SELECT,
                            _("Scale"));
}

void
sp_selection_scale_screen(Inkscape::Selection *selection, gdouble grow_pixels)
{
    sp_selection_scale(selection,
                       grow_pixels / selection->desktop()->current_zoom());
}

void
sp_selection_scale_times(Inkscape::Selection *selection, gdouble times)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect sel_bbox = selection->visualBounds();

    if (!sel_bbox) {
        return;
    }

    Geom::Point const center(sel_bbox->midpoint());
    sp_selection_scale_relative(selection, center, Geom::Scale(times, times));
    DocumentUndo::done(selection->desktop()->getDocument(), SP_VERB_CONTEXT_SELECT,
                       _("Scale by whole factor"));
}

void
sp_selection_move(Inkscape::Selection *selection, gdouble dx, gdouble dy)
{
    if (selection->isEmpty()) {
        return;
    }

    sp_selection_move_relative(selection, dx, dy);

    SPDocument *doc = selection->layers()->getDocument();
    if (dx == 0) {
        DocumentUndo::maybeDone(doc, "selector:move:vertical", SP_VERB_CONTEXT_SELECT,
                                _("Move vertically"));
    } else if (dy == 0) {
        DocumentUndo::maybeDone(doc, "selector:move:horizontal", SP_VERB_CONTEXT_SELECT,
                                _("Move horizontally"));
    } else {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_SELECT,
                           _("Move"));
    }
}

void
sp_selection_move_screen(Inkscape::Selection *selection, gdouble dx, gdouble dy)
{
    if (selection->isEmpty() || !selection->desktop()) {
        return;
    }

    // same as sp_selection_move but divide deltas by zoom factor
    gdouble const zoom = selection->desktop()->current_zoom();
    gdouble const zdx = dx / zoom;
    gdouble const zdy = dy / zoom;
    sp_selection_move_relative(selection, zdx, zdy);

    SPDocument *doc = selection->layers()->getDocument();
    if (dx == 0) {
        DocumentUndo::maybeDone(doc, "selector:move:vertical", SP_VERB_CONTEXT_SELECT,
                                _("Move vertically by pixels"));
    } else if (dy == 0) {
        DocumentUndo::maybeDone(doc, "selector:move:horizontal", SP_VERB_CONTEXT_SELECT,
                                _("Move horizontally by pixels"));
    } else {
        DocumentUndo::done(doc, SP_VERB_CONTEXT_SELECT,
                           _("Move"));
    }
}



typedef struct Forward {
    typedef SPObject *Iterator;

    static Iterator children(SPObject *o) { return o->firstChild(); }
    static Iterator siblings_after(SPObject *o) { return o->getNext(); }
    static void dispose(Iterator /*i*/) {}

    static SPObject *object(Iterator i) { return i; }
    static Iterator next(Iterator i) { return i->getNext(); }
} Forward;

typedef struct ListReverse {
    typedef GSList *Iterator;

    static Iterator children(SPObject *o) {
        return make_list(o->firstChild(), NULL);
    }
    static Iterator siblings_after(SPObject *o) {
        return make_list(o->parent->firstChild(), o);
    }
    static void dispose(Iterator i) {
        g_slist_free(i);
    }

    static SPObject *object(Iterator i) {
        return reinterpret_cast<SPObject *>(i->data);
    }
    static Iterator next(Iterator i) { return i->next; }

private:
    static GSList *make_list(SPObject *object, SPObject *limit) {
        GSList *list = NULL;
        while ( object != limit ) {
            if (!object) { // TODO check if this happens in practice
                g_warning("Unexpected list overrun");
                break;
            }
            list = g_slist_prepend(list, object);
            object = object->getNext();
        }
        return list;
    }
} ListReverse;



template <typename D>
SPItem *next_item(SPDesktop *desktop, GSList *path, SPObject *root,
                  bool only_in_viewport, PrefsSelectionContext inlayer, bool onlyvisible, bool onlysensitive)
{
    typename D::Iterator children;
    typename D::Iterator iter;

    SPItem *found=NULL;

    if (path) {
        SPObject *object=reinterpret_cast<SPObject *>(path->data);
        g_assert(object->parent == root);
        if (desktop->isLayer(object)) {
            found = next_item<D>(desktop, path->next, object, only_in_viewport, inlayer, onlyvisible, onlysensitive);
        }
        iter = children = D::siblings_after(object);
    } else {
        iter = children = D::children(root);
    }

    while ( iter && !found ) {
        SPObject *object=D::object(iter);
        if (desktop->isLayer(object)) {
            if (PREFS_SELECTION_LAYER != inlayer) { // recurse into sublayers
                found = next_item<D>(desktop, NULL, object, only_in_viewport, inlayer, onlyvisible, onlysensitive);
            }
        } else {
            SPItem *item = dynamic_cast<SPItem *>(object);
            if ( item &&
                 ( !only_in_viewport || desktop->isWithinViewport(item) ) &&
                 ( !onlyvisible || !desktop->itemIsHidden(item)) &&
                 ( !onlysensitive || !item->isLocked()) &&
                 !desktop->isLayer(item) )
            {
                found = item;
            }
        }
        iter = D::next(iter);
    }

    D::dispose(children);

    return found;
}


template <typename D>
SPItem *next_item_from_list(SPDesktop *desktop, std::vector<SPItem*> const items,
                            SPObject *root, bool only_in_viewport, PrefsSelectionContext inlayer, bool onlyvisible, bool onlysensitive)
{
    SPObject *current=root;
    for(std::vector<SPItem*>::const_iterator i = items.begin();i!=items.end();i++) {
        SPItem *item = *i;
        if ( root->isAncestorOf(item) &&
             ( !only_in_viewport || desktop->isWithinViewport(item) ) )
        {
            current = item;
            break;
        }
    }

    GSList *path=NULL;
    while ( current != root ) {
        path = g_slist_prepend(path, current);
        current = current->parent;
    }

    SPItem *next;
    // first, try from the current object
    next = next_item<D>(desktop, path, root, only_in_viewport, inlayer, onlyvisible, onlysensitive);
    g_slist_free(path);

    if (!next) { // if we ran out of objects, start over at the root
        next = next_item<D>(desktop, NULL, root, only_in_viewport, inlayer, onlyvisible, onlysensitive);
    }

    return next;
}

void
sp_selection_item_next(SPDesktop *desktop)
{
    g_return_if_fail(desktop != NULL);
    Inkscape::Selection *selection = desktop->getSelection();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    PrefsSelectionContext inlayer = (PrefsSelectionContext)prefs->getInt("/options/kbselection/inlayer", PREFS_SELECTION_LAYER);
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);

    SPObject *root;
    if (PREFS_SELECTION_ALL != inlayer) {
        root = selection->activeContext();
    } else {
        root = desktop->currentRoot();
    }

    SPItem *item=next_item_from_list<Forward>(desktop, selection->itemList(), root, SP_CYCLING == SP_CYCLE_VISIBLE, inlayer, onlyvisible, onlysensitive);

    if (item) {
        selection->set(item, PREFS_SELECTION_LAYER_RECURSIVE == inlayer);
        if ( SP_CYCLING == SP_CYCLE_FOCUS ) {
            scroll_to_show_item(desktop, item);
        }
    }
}

void
sp_selection_item_prev(SPDesktop *desktop)
{
    SPDocument *document = desktop->getDocument();
    g_return_if_fail(document != NULL);
    g_return_if_fail(desktop != NULL);
    Inkscape::Selection *selection = desktop->getSelection();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    PrefsSelectionContext inlayer = (PrefsSelectionContext) prefs->getInt("/options/kbselection/inlayer", PREFS_SELECTION_LAYER);
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);

    SPObject *root;
    if (PREFS_SELECTION_ALL != inlayer) {
        root = selection->activeContext();
    } else {
        root = desktop->currentRoot();
    }

    SPItem *item=next_item_from_list<ListReverse>(desktop, selection->itemList(), root, SP_CYCLING == SP_CYCLE_VISIBLE, inlayer, onlyvisible, onlysensitive);

    if (item) {
        selection->set(item, PREFS_SELECTION_LAYER_RECURSIVE == inlayer);
        if ( SP_CYCLING == SP_CYCLE_FOCUS ) {
            scroll_to_show_item(desktop, item);
        }
    }
}

void sp_selection_next_patheffect_param(SPDesktop * dt)
{
    if (!dt) return;

    Inkscape::Selection *selection = dt->getSelection();
    if ( selection && !selection->isEmpty() ) {
        SPItem *item = selection->singleItem();
        if ( SPLPEItem *lpeitem = dynamic_cast<SPLPEItem*>(item) ) {
            if (lpeitem->hasPathEffect()) {
                lpeitem->editNextParamOncanvas(dt);
            } else {
                dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("The selection has no applied path effect."));
            }
        }
    }
}

/*bool has_path_recursive(SPObject *obj)
{
    if (!obj) return false;
    if (SP_IS_PATH(obj)) {
        return true;
    }
    if (SP_IS_GROUP(obj) || SP_IS_OBJECTGROUP(obj)) {
        for (SPObject *c = obj->children; c; c = c->next) {
            if (has_path_recursive(c)) return true;
        }
    }
    return false;
}*/

void sp_selection_edit_clip_or_mask(SPDesktop * /*dt*/, bool /*clip*/)
{
    return;
    /*if (!dt) return;
    using namespace Inkscape::UI;

    Inkscape::Selection *selection = dt->getSelection();
    if (!selection || selection->isEmpty()) return;

    GSList const *items = selection->itemList();
    bool has_path = false;
    for (GSList *i = const_cast<GSList*>(items); i; i= i->next) {
        SPItem *item = SP_ITEM(i->data);
        SPObject *search = clip
            ? (item->clip_ref ? item->clip_ref->getObject() : NULL)
            : item->mask_ref ? item->mask_ref->getObject() : NULL;
        has_path |= has_path_recursive(search);
        if (has_path) break;
    }
    if (has_path) {
        if (!tools_isactive(dt, TOOLS_NODES)) {
            tools_switch(dt, TOOLS_NODES);
        }
        ink_node_tool_set_mode(INK_NODE_TOOL(dt->event_context),
            clip ? NODE_TOOL_EDIT_CLIPPING_PATHS : NODE_TOOL_EDIT_MASKS);
    } else if (clip) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE,
            _("The selection has no applied clip path."));
    } else {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE,
            _("The selection has no applied mask."));
    }*/
}





/**
 * If \a item is not entirely visible then adjust visible area to centre on the centre on of
 * \a item.
 */
void scroll_to_show_item(SPDesktop *desktop, SPItem *item)
{
    Geom::Rect dbox = desktop->get_display_area();
    Geom::OptRect sbox = item->desktopVisualBounds();

    if ( sbox && dbox.contains(*sbox) == false ) {
        Geom::Point const s_dt = sbox->midpoint();
        Geom::Point const s_w = desktop->d2w(s_dt);
        Geom::Point const d_dt = dbox.midpoint();
        Geom::Point const d_w = desktop->d2w(d_dt);
        Geom::Point const moved_w( d_w - s_w );
        gint const dx = (gint) moved_w[X];
        gint const dy = (gint) moved_w[Y];
        desktop->scroll_world(dx, dy);
    }
}


void sp_selection_clone(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection();

    Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object</b> to clone."));
        return;
    }

    std::vector<Inkscape::XML::Node*> reprs (selection->reprList());

    selection->clear();

    // sorting items from different parents sorts each parent's subset without possibly mixing them, just what we need
    sort(reprs.begin(),reprs.end(),sp_repr_compare_position_bool);

    std::vector<Inkscape::XML::Node*> newsel;

    for(std::vector<Inkscape::XML::Node*>::const_iterator i=reprs.begin();i!=reprs.end();i++){
    	Inkscape::XML::Node *sel_repr = *i;
        Inkscape::XML::Node *parent = sel_repr->parent();

        Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");
        clone->setAttribute("x", "0", false);
        clone->setAttribute("y", "0", false);
        gchar *href_str = g_strdup_printf("#%s", sel_repr->attribute("id"));
        clone->setAttribute("xlink:href", href_str, false);
        g_free(href_str);

        clone->setAttribute("inkscape:transform-center-x", sel_repr->attribute("inkscape:transform-center-x"), false);
        clone->setAttribute("inkscape:transform-center-y", sel_repr->attribute("inkscape:transform-center-y"), false);

        // add the new clone to the top of the original's parent
        parent->appendChild(clone);

        newsel.push_back(clone);
        Inkscape::GC::release(clone);
    }

    DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_CLONE,
                       C_("Action", "Clone"));

    selection->setReprList(newsel);
}

void
sp_selection_relink(SPDesktop *desktop)
{
    if (!desktop)
        return;

    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>clones</b> to relink."));
        return;
    }

    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    const gchar *newid = cm->getFirstObjectID();
    if (!newid) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Copy an <b>object</b> to clipboard to relink clones to."));
        return;
    }
    gchar *newref = g_strdup_printf("#%s", newid);

    // Get a copy of current selection.
    bool relinked = false;
    std::vector<SPItem*> items=selection->itemList();
    for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
        SPItem *item = *i;

        if (dynamic_cast<SPUse *>(item)) {
            item->getRepr()->setAttribute("xlink:href", newref);
            item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            relinked = true;
        }
    }

    g_free(newref);

    if (!relinked) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No clones to relink</b> in the selection."));
    } else {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_UNLINK_CLONE,
                           _("Relink clone"));
    }
}


void
sp_selection_unlink(SPDesktop *desktop)
{
    if (!desktop)
        return;

    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>clones</b> to unlink."));
        return;
    }

    // Get a copy of current selection.
    std::vector<SPItem*> new_select;
    bool unlinked = false;
    std::vector<SPItem*> items=selection->itemList();
    for (std::vector<SPItem*>::const_reverse_iterator i=items.rbegin();i!=items.rend();i++){
        SPItem *item = *i;

        if (dynamic_cast<SPText *>(item)) {
            SPObject *tspan = sp_tref_convert_to_tspan(item);

            if (tspan) {
                item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }

            // Set unlink to true, and fall into the next if which
            // will include this text item in the new selection
            unlinked = true;
        }

        if (!(dynamic_cast<SPUse *>(item) || dynamic_cast<SPTRef *>(item))) {
            // keep the non-use item in the new selection
            new_select.push_back(item);
            continue;
        }

        SPItem *unlink = NULL;
        SPUse *use = dynamic_cast<SPUse *>(item);
        if (use) {
            unlink = use->unlink();
            // Unable to unlink use (external or invalid href?)
            if (!unlink) {
                new_select.push_back(item);
                continue;
            }
        } else /*if (SP_IS_TREF(use))*/ {
            unlink = dynamic_cast<SPItem *>(sp_tref_convert_to_tspan(item));
            g_assert(unlink != NULL);
        }

        unlinked = true;
        // Add ungrouped items to the new selection.
        new_select.push_back(unlink);
    }

    if (!new_select.empty()) { // set new selection
        selection->clear();
        selection->setList(new_select);
    }
    if (!unlinked) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No clones to unlink</b> in the selection."));
    }

    DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_UNLINK_CLONE,
                       _("Unlink clone"));
}

void
sp_select_clone_original(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = desktop->getSelection();

    SPItem *item = selection->singleItem();

    gchar const *error = _("Select a <b>clone</b> to go to its original. Select a <b>linked offset</b> to go to its source. Select a <b>text on path</b> to go to the path. Select a <b>flowed text</b> to go to its frame.");

    // Check if other than two objects are selected

    std::vector<SPItem*> items=selection->itemList();
    if (items.size() != 1 || !item) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, error);
        return;
    }

    SPItem *original = NULL;
    SPUse *use = dynamic_cast<SPUse *>(item);
    if (use) {
        original = use->get_original();
    } else {
        SPOffset *offset = dynamic_cast<SPOffset *>(item);
        if (offset && offset->sourceHref) {
            original = sp_offset_get_source(offset);
        } else {
            SPText *text = dynamic_cast<SPText *>(item);
            SPTextPath *textpath = (text) ? dynamic_cast<SPTextPath *>(text->firstChild()) : NULL;
            if (text && textpath) {
                original = sp_textpath_get_path_item(textpath);
            } else {
                SPFlowtext *flowtext = dynamic_cast<SPFlowtext *>(item);
                if (flowtext) {
                    original = flowtext->get_frame(NULL); // first frame only
                } else {
                    SPLPEItem *lpeItem = dynamic_cast<SPLPEItem *>(item);
                    if (lpeItem) {
                        // check if the applied LPE is Clone original, if so, go to the refered path
                        Inkscape::LivePathEffect::Effect* lpe = lpeItem->getPathEffectOfType(Inkscape::LivePathEffect::CLONE_ORIGINAL);
                        if (lpe) {
                            Inkscape::LivePathEffect::Parameter *lpeparam = lpe->getParameter("linkedpath");
                            if (Inkscape::LivePathEffect::OriginalPathParam *pathparam = dynamic_cast<Inkscape::LivePathEffect::OriginalPathParam *>(lpeparam)) {
                                original = pathparam->getObject();
                            }
                        }
                    }
                }
            }
        }
    }

    if (original == NULL) { // it's an object that we don't know what to do with
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, error);
        return;
    }

    if (!original) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>Cannot find</b> the object to select (orphaned clone, offset, textpath, flowed text?)"));
        return;
    }

    for (SPObject *o = original; o && !dynamic_cast<SPRoot *>(o); o = o->parent) {
        if (dynamic_cast<SPDefs *>(o)) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("The object you're trying to select is <b>not visible</b> (it is in &lt;defs&gt;)"));
            return;
        }
    }

    if (original) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool highlight = prefs->getBool("/options/highlightoriginal/value");
        if (highlight) {
            Geom::OptRect a = item->desktopVisualBounds();
            Geom::OptRect b = original->desktopVisualBounds();
            if ( a && b ) {
                // draw a flashing line between the objects
                SPCurve *curve = new SPCurve();
                curve->moveto(a->midpoint());
                curve->lineto(b->midpoint());

                SPCanvasItem * canvasitem = sp_canvas_bpath_new(desktop->getTempGroup(), curve);
                sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(canvasitem), 0x0000ddff, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT, 5, 3);
                sp_canvas_item_show(canvasitem);
                curve->unref();
                desktop->add_temporary_canvasitem(canvasitem, 1000);
            }
        }

        selection->clear();
        selection->set(original);
        if (SP_CYCLING == SP_CYCLE_FOCUS) {
            scroll_to_show_item(desktop, original);
        }
    }
}

/**
* This creates a new path, applies the Original Path LPE, and has it refer to the selection.
*/
void sp_selection_clone_original_path_lpe(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection();

    Inkscape::SVGOStringStream os;
    SPObject * firstItem = NULL;
    std::vector<SPItem*> items=selection->itemList();
    for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
        if (SP_IS_SHAPE(*i) || SP_IS_TEXT(*i)) {
            if (firstItem) {
                os << "|";
            } else {
                firstItem = SP_ITEM(*i);
            }
            os << '#' << SP_ITEM(*i)->getId() << ",0";
        }
    }
    if (firstItem) {
        Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
        SPObject *parent = firstItem->parent;

        // create the LPE
        Inkscape::XML::Node *lpe_repr = xml_doc->createElement("inkscape:path-effect");
        {
            lpe_repr->setAttribute("effect", "fill_between_many");
            lpe_repr->setAttribute("linkedpaths", os.str());
            desktop->doc()->getDefs()->getRepr()->addChild(lpe_repr, NULL); // adds to <defs> and assigns the 'id' attribute
        }
        std::string lpe_id_href = std::string("#") + lpe_repr->attribute("id");
        Inkscape::GC::release(lpe_repr);

        // create the new path
        Inkscape::XML::Node *clone = xml_doc->createElement("svg:path");
        {
            clone->setAttribute("d", "M 0 0", false);
            // add the new clone to the top of the original's parent
            parent->appendChildRepr(clone);
            SPObject *clone_obj = desktop->doc()->getObjectById(clone->attribute("id"));
            SPLPEItem *clone_lpeitem = dynamic_cast<SPLPEItem *>(clone_obj);
            if (clone_lpeitem) {
                clone_lpeitem->addPathEffect(lpe_id_href, false);
            }
        }

        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_CLONE_ORIGINAL_PATH_LPE, _("Fill between many"));
        // select the new object:
        selection->set(clone);

        Inkscape::GC::release(clone);
    } else {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select path(s) to fill."));
    }
}

void sp_selection_to_marker(SPDesktop *desktop, bool apply)
{
    // sp_selection_tile has similar code
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to marker."));
        return;
    }

    doc->ensureUpToDate();
    Geom::OptRect r = selection->visualBounds();
    boost::optional<Geom::Point> c = selection->center();
    if ( !r || !c ) {
        return;
    }

    // FIXME: Inverted Y coodinate
    Geom::Point doc_height( 0, doc->getHeight().value("px"));

    // calculate the transform to be applied to objects to move them to 0,0
    Geom::Point corner( r->min()[Geom::X], r->max()[Geom::Y] ); // FIXME: Inverted Y coodinate  
    Geom::Point move_p = doc_height - corner;
    move_p[Geom::Y] = -move_p[Geom::Y];
    Geom::Affine move = Geom::Affine(Geom::Translate(move_p));

    Geom::Point center( *c - corner ); // As defined by rotation center
    center[Geom::Y] = -center[Geom::Y];

    std::vector<SPItem*> items(selection->itemList());

    //items = g_slist_sort(items, (GCompareFunc) sp_object_compare_position);  // Why needed?

    // bottommost object, after sorting
    SPObject *parent = items[0]->parent;

    Geom::Affine parent_transform;
    {
        SPItem *parentItem = dynamic_cast<SPItem *>(parent);
        if (parent) {
            parent_transform = parentItem->i2doc_affine();
        } else {
            g_assert_not_reached();
        }
    }

    // Create a list of duplicates, to be pasted inside marker element.
    std::vector<Inkscape::XML::Node*> repr_copies;
    for (std::vector<SPItem*>::const_reverse_iterator i=items.rbegin();i!=items.rend();i++){
        Inkscape::XML::Node *dup = (*i)->getRepr()->duplicate(xml_doc);
        repr_copies.push_back(dup);
    }

    Geom::Rect bbox(desktop->dt2doc(r->min()), desktop->dt2doc(r->max()));

    if (apply) {
        // Delete objects so that their clones don't get alerted;
        // the objects will be restored inside the marker element.
        for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
            SPObject *item = *i;
            item->deleteObject(false);
        }
    }

    // Hack: Temporarily set clone compensation to unmoved, so that we can move clone-originals
    // without disturbing clones.
    // See ActorAlign::on_button_click() in src/ui/dialog/align-and-distribute.cpp
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

    gchar const *mark_id = generate_marker(repr_copies, bbox, doc, center, parent_transform * move);
    (void)mark_id;

    // restore compensation setting
    prefs->setInt("/options/clonecompensation/value", saved_compensation);



    DocumentUndo::done(doc, SP_VERB_EDIT_SELECTION_2_MARKER,
                       _("Objects to marker"));
}

static void sp_selection_to_guides_recursive(SPItem *item, bool wholegroups) {
    SPGroup *group = dynamic_cast<SPGroup *>(item);
    if (group && !dynamic_cast<SPBox3D *>(item) && !wholegroups) {
        std::vector<SPItem*> items=sp_item_group_item_list(group);
        for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
            sp_selection_to_guides_recursive(*i, wholegroups);
        }
    } else {
        item->convert_to_guides();
    }
}

void sp_selection_to_guides(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = desktop->getDocument();
    Inkscape::Selection *selection = desktop->getSelection();
    // we need to copy the list because it gets reset when objects are deleted
    std::vector<SPItem*> items(selection->itemList());

    if (items.empty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to guides."));
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool deleteitems = !prefs->getBool("/tools/cvg_keep_objects", 0);
    bool wholegroups = prefs->getBool("/tools/cvg_convert_whole_groups", 0);

    // If an object is earlier in the selection list than its clone, and it is deleted, then the clone will have changed
    // and its entry in the selection list is invalid (crash).
    // Therefore: first convert all, then delete all.

    for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
        sp_selection_to_guides_recursive(*i, wholegroups);
    }

    if (deleteitems) {
        selection->clear();
        sp_selection_delete_impl(items);
    }

    DocumentUndo::done(doc, SP_VERB_EDIT_SELECTION_2_GUIDES, _("Objects to guides"));
}

/*
 * Convert objects to <symbol>. How that happens depends on what is selected:
 * 
 * 1) A random selection of objects will be embedded into a single <symbol> element.
 *
 * 2) Except, a single <g> will have its content directly embedded into a <symbol>; the 'id' and
 *    'style' of the <g> are transferred to the <symbol>.
 *
 * 3) Except, a single <g> with a transform that isn't a translation will keep the group when
 *    embedded into a <symbol> (with 'id' and 'style' transferred to <symbol>). This is because a
 *    <symbol> cannot have a transform. (If the transform is a pure translation, the translation
 *    is moved to the referencing <use> element that is created.)
 *
 * Possible improvements:
 *
 *   Move objects inside symbol so bbox corner at 0,0 (see marker/pattern)
 *
 *   For SVG2, set 'refX' 'refY' to object center (with compensating shift in <use>
 *   transformation).
 */
void sp_selection_symbol(SPDesktop *desktop, bool /*apply*/ )
{
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // Check if something is selected.
    if (selection->isEmpty()) {
      desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>objects</b> to convert to symbol."));
      return;
    }

    doc->ensureUpToDate();

    std::vector<SPObject*> items(selection->list());
    sort(items.begin(),items.end(),sp_object_compare_position_bool);

    // Keep track of parent, this is where <use> will be inserted.
    Inkscape::XML::Node *the_first_repr = items[0]->getRepr();
    Inkscape::XML::Node *the_parent_repr = the_first_repr->parent();

    // Find out if we have a single group
    bool single_group = false;
    SPGroup *the_group = NULL;
    Geom::Affine transform;
    if( items.size() == 1 ) {
        SPObject *object = items[0];
        the_group = dynamic_cast<SPGroup *>(object);
        if ( the_group ) {
            single_group = true;

            if( !sp_svg_transform_read( object->getAttribute("transform"), &transform ))
                transform = Geom::identity();

            if( transform.isTranslation() ) {

                // Create new list from group children.
                items = object->childList(false);

                // Hack: Temporarily set clone compensation to unmoved, so that we can move clone-originals
                // without disturbing clones.
                // See ActorAlign::on_button_click() in src/ui/dialog/align-and-distribute.cpp
                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
                prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

                // Remove transform on group, updating clones.
                the_group->doWriteTransform(object->getRepr(), Geom::identity());

                // restore compensation setting
                prefs->setInt("/options/clonecompensation/value", saved_compensation);
            }
        }
    }

    // Create new <symbol>
    Inkscape::XML::Node *defsrepr = doc->getDefs()->getRepr();
    Inkscape::XML::Node *symbol_repr = xml_doc->createElement("svg:symbol");
    defsrepr->appendChild(symbol_repr);

    // For a single group, copy relevant attributes.
    if( single_group ) {

        symbol_repr->setAttribute("style",  the_group->getAttribute("style"));
        symbol_repr->setAttribute("class",  the_group->getAttribute("class"));
        Glib::ustring id = the_group->getAttribute("id");
        the_group->setAttribute("id", id + "_transform");
        symbol_repr->setAttribute("id",     the_group->getAttribute("id")   );

        // This should eventually be replaced by 'refX' and 'refY' once SVG WG approves it.
        // It is done here for round-tripping
        symbol_repr->setAttribute("inkscape:transform-center-x",
                                  the_group->getAttribute("inkscape:transform-center-x"));
        symbol_repr->setAttribute("inkscape:transform-center-y",
                                  the_group->getAttribute("inkscape:transform-center-y"));

        the_group->setAttribute("style", NULL);

    }

    // Move selected items to new <symbol>
    for (std::vector<SPObject*>::const_reverse_iterator i=items.rbegin();i!=items.rend();i++){
      Inkscape::XML::Node *repr = (*i)->getRepr();
      repr->parent()->removeChild(repr);
      symbol_repr->addChild(repr,NULL);
    }

    if( single_group && transform.isTranslation() ) {
        the_group->deleteObject(true);
    }

    // Create <use> pointing to new symbol (to replace the moved objects).
    Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");

    clone->setAttribute("xlink:href", Glib::ustring("#")+symbol_repr->attribute("id"), false);

    the_parent_repr->appendChild(clone);

    if( single_group && transform.isTranslation() ) {
        if( !transform.isIdentity() )
            clone->setAttribute("transform", sp_svg_transform_write( transform ));
    }

    // Change selection to new <use> element.
    selection->set(clone);

    // Clean up
    Inkscape::GC::release(symbol_repr);

    DocumentUndo::done(doc, SP_VERB_EDIT_SYMBOL, _("Group to symbol"));
}

/*
 * Convert <symbol> to <g>. All <use> elements referencing symbol remain unchanged.
 */
void sp_selection_unsymbol(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // Check if something is selected.
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select a <b>symbol</b> to extract objects from."));
        return;
    }

    SPObject* symbol = selection->single();
 
    // Make sure we have only one object in selection.
    // Require that we really have a <symbol>.
    if( symbol == NULL || !dynamic_cast<SPSymbol *>( symbol ))  {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select only one <b>symbol</b> in Symbol dialog to convert to group."));
        return;
    }

    doc->ensureUpToDate();

    // Create new <g> and insert in current layer
    Inkscape::XML::Node *group = xml_doc->createElement("svg:g");
    desktop->currentLayer()->getRepr()->appendChild(group);

    // Move all children of symbol to group
    std::vector<SPObject*> children = symbol->childList(false);

    // Converting a group to a symbol inserts a group for non-translational transform.
    // In converting a symbol back to a group we strip out the inserted group (or any other
    // group that only adds a transform to the symbol content).
    if( children.size() == 1 ) {
        SPObject *object = children[0];
        if ( dynamic_cast<SPGroup *>( object ) ) {
            if( object->getAttribute("style") == NULL ||
                object->getAttribute("class") == NULL ) {

                group->setAttribute("transform", object->getAttribute("transform"));
                children = object->childList(false);
            }
        }
    }
        
    for (std::vector<SPObject*>::const_reverse_iterator i=children.rbegin();i!=children.rend();i++){
        Inkscape::XML::Node *repr = (*i)->getRepr();
        repr->parent()->removeChild(repr);
        group->addChild(repr,NULL);
    }

    // Copy relevant attributes
    group->setAttribute("style", symbol->getAttribute("style"));
    group->setAttribute("class", symbol->getAttribute("class"));
    group->setAttribute("inkscape:transform-center-x",
                        symbol->getAttribute("inkscape:transform-center-x"));
    group->setAttribute("inkscape:transform-center-y",
                        symbol->getAttribute("inkscape:transform-center-y"));


    // Need to delete <symbol>; all <use> elements that referenced <symbol> should
    // auto-magically reference <g> (if <symbol> deleted after setting <g> 'id').
    Glib::ustring id = symbol->getAttribute("id");
    group->setAttribute("id", id);
    symbol->deleteObject(true);

    // Change selection to new <g> element.
    SPItem *group_item = static_cast<SPItem *>(desktop->getDocument()->getObjectByRepr(group));
    selection->set(group_item);

    // Clean up
    Inkscape::GC::release(group);

    DocumentUndo::done(doc, SP_VERB_EDIT_UNSYMBOL, _("Group from symbol"));
}

void
sp_selection_tile(SPDesktop *desktop, bool apply)
{
    // sp_selection_to_marker has similar code
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to pattern."));
        return;
    }

    doc->ensureUpToDate();
    Geom::OptRect r = selection->visualBounds();
    if ( !r ) {
        return;
    }

    // calculate the transform to be applied to objects to move them to 0,0
    Geom::Point move_p = Geom::Point(0, doc->getHeight().value("px")) - (r->min() + Geom::Point(0, r->dimensions()[Geom::Y]));
    move_p[Geom::Y] = -move_p[Geom::Y];
    Geom::Affine move = Geom::Affine(Geom::Translate(move_p));

    std::vector<SPItem*> items (selection->itemList());

    sort(items.begin(),items.end(),sp_object_compare_position_bool);

    // bottommost object, after sorting
    SPObject *parent = items[0]->parent;
    

    Geom::Affine parent_transform;
    {
        SPItem *parentItem = dynamic_cast<SPItem *>(parent);
        if (parentItem) {
            parent_transform = parentItem->i2doc_affine();
        } else {
            g_assert_not_reached();
        }
    }

    // remember the position of the first item
    gint pos = items[0]->getRepr()->position();

    // create a list of duplicates
    std::vector<Inkscape::XML::Node*> repr_copies;
    for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
        Inkscape::XML::Node *dup = (*i)->getRepr()->duplicate(xml_doc);
        repr_copies.push_back(dup);
    }

    Geom::Rect bbox(desktop->dt2doc(r->min()), desktop->dt2doc(r->max()));

    if (apply) {
        // delete objects so that their clones don't get alerted; this object will be restored shortly
        for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
            SPObject *item = *i;
            item->deleteObject(false);
        }
    }

    // Hack: Temporarily set clone compensation to unmoved, so that we can move clone-originals
    // without disturbing clones.
    // See ActorAlign::on_button_click() in src/ui/dialog/align-and-distribute.cpp
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

    gchar const *pat_id = SPPattern::produce(repr_copies, bbox, doc,
                                       ( Geom::Affine(Geom::Translate(desktop->dt2doc(Geom::Point(r->min()[Geom::X],
                                                                                            r->max()[Geom::Y]))))
                                         * parent_transform.inverse() ),
                                       parent_transform * move);

    // restore compensation setting
    prefs->setInt("/options/clonecompensation/value", saved_compensation);

    if (apply) {
        Inkscape::XML::Node *rect = xml_doc->createElement("svg:rect");
        gchar *style_str = g_strdup_printf("stroke:none;fill:url(#%s)", pat_id);
        rect->setAttribute("style", style_str);
        g_free(style_str);

        Geom::Point min = bbox.min() * parent_transform.inverse();
        Geom::Point max = bbox.max() * parent_transform.inverse();

        sp_repr_set_svg_double(rect, "width", max[Geom::X] - min[Geom::X]);
        sp_repr_set_svg_double(rect, "height", max[Geom::Y] - min[Geom::Y]);
        sp_repr_set_svg_double(rect, "x", min[Geom::X]);
        sp_repr_set_svg_double(rect, "y", min[Geom::Y]);

        // restore parent and position
        parent->getRepr()->appendChild(rect);
        rect->setPosition(pos > 0 ? pos : 0);
        SPItem *rectangle = static_cast<SPItem *>(desktop->getDocument()->getObjectByRepr(rect));

        Inkscape::GC::release(rect);

        selection->clear();
        selection->set(rectangle);
    }


    DocumentUndo::done(doc, SP_VERB_EDIT_TILE,
                       _("Objects to pattern"));
}

void sp_selection_untile(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object with pattern fill</b> to extract objects from."));
        return;
    }

    std::vector<SPItem*> new_select;

    bool did = false;

    std::vector<SPItem*> items(selection->itemList());
    for (std::vector<SPItem*>::const_reverse_iterator i=items.rbegin();i!=items.rend();i++){
        SPItem *item = *i;

        SPStyle *style = item->style;

        if (!style || !style->fill.isPaintserver())
            continue;

        SPPaintServer *server = item->style->getFillPaintServer();

        SPPattern *basePat = dynamic_cast<SPPattern *>(server);
        if (!basePat) {
            continue;
        }

        did = true;

        SPPattern *pattern = basePat->rootPattern();

        Geom::Affine pat_transform = basePat->getTransform();
        pat_transform *= item->transform;

        for (SPObject *child = pattern->firstChild() ; child != NULL; child = child->next ) {
            if (dynamic_cast<SPItem *>(child)) {
                Inkscape::XML::Node *copy = child->getRepr()->duplicate(xml_doc);
                SPItem *i = dynamic_cast<SPItem *>(desktop->currentLayer()->appendChildRepr(copy));

               // FIXME: relink clones to the new canvas objects
               // use SPObject::setid when mental finishes it to steal ids of

                // this is needed to make sure the new item has curve (simply requestDisplayUpdate does not work)
                doc->ensureUpToDate();

                if (i) {
                    Geom::Affine transform( i->transform * pat_transform );
                    i->doWriteTransform(i->getRepr(), transform);

                    new_select.push_back(i);
                } else {
                    g_assert_not_reached();
                }
            }
        }

        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property(css, "fill", "none");
        sp_repr_css_change(item->getRepr(), css, "style");
    }

    if (!did) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No pattern fills</b> in the selection."));
    } else {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_EDIT_UNTILE,
                           _("Pattern to objects"));
        selection->setList(new_select);
    }
}

void sp_selection_get_export_hints(Inkscape::Selection *selection, Glib::ustring &filename, float *xdpi, float *ydpi)
{
    if (selection->isEmpty()) {
        return;
    }

    std::vector<Inkscape::XML::Node*> const reprlst = selection->reprList();
    bool filename_search = TRUE;
    bool xdpi_search = TRUE;
    bool ydpi_search = TRUE;

    for (std::vector<Inkscape::XML::Node*>::const_iterator i=reprlst.begin();filename_search&&xdpi_search&&ydpi_search&&i!=reprlst.end();i++){
        gchar const *dpi_string;
        Inkscape::XML::Node *repr = *i;

        if (filename_search) {
            const gchar* tmp = repr->attribute("inkscape:export-filename");
            if (tmp){
                filename = tmp;
                filename_search = FALSE;
            }
            else{
                filename.clear();
            }
        }

        if (xdpi_search) {
            dpi_string = repr->attribute("inkscape:export-xdpi");
            if (dpi_string != NULL) {
                *xdpi = atof(dpi_string);
                xdpi_search = FALSE;
            }
        }

        if (ydpi_search) {
            dpi_string = repr->attribute("inkscape:export-ydpi");
            if (dpi_string != NULL) {
                *ydpi = atof(dpi_string);
                ydpi_search = FALSE;
            }
        }
    }
}

void sp_document_get_export_hints(SPDocument *doc, Glib::ustring &filename, float *xdpi, float *ydpi)
{
    Inkscape::XML::Node * repr = doc->getReprRoot();

    const gchar* tmp = repr->attribute("inkscape:export-filename");
    if(tmp)
    {
        filename = tmp;
    }
    else
    {
        filename.clear();
    }
    gchar const *dpi_string = repr->attribute("inkscape:export-xdpi");
    if (dpi_string != NULL) {
        *xdpi = atof(dpi_string);
    }

    dpi_string = NULL;
    dpi_string = repr->attribute("inkscape:export-ydpi");
    if (dpi_string != NULL) {
        *ydpi = atof(dpi_string);
    }
}

void sp_selection_create_bitmap_copy(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    SPDocument *document = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to make a bitmap copy."));
        return;
    }

    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Rendering bitmap..."));
    // set "busy" cursor
    desktop->setWaitingCursor();

    // Get the bounding box of the selection
    document->ensureUpToDate();
    Geom::OptRect bbox = selection->visualBounds();
    if (!bbox) {
        desktop->clearWaitingCursor();
        return; // exceptional situation, so not bother with a translatable error message, just quit quietly
    }

    // List of the items to show; all others will be hidden
    std::vector<SPItem*> items(selection->itemList());

    // Sort items so that the topmost comes last
    sort(items.begin(),items.end(),sp_item_repr_compare_position_bool);

    // Generate a random value from the current time (you may create bitmap from the same object(s)
    // multiple times, and this is done so that they don't clash)
    GTimeVal cu;
    g_get_current_time(&cu);
    guint current = (int) (cu.tv_sec * 1000000 + cu.tv_usec) % 1024;

    // Create the filename.
    gchar *const basename = g_strdup_printf("%s-%s-%u.png",
                                            document->getName(),
                                            items[0]->getRepr()->attribute("id"),
                                            current);
    // Imagemagick is known not to handle spaces in filenames, so we replace anything but letters,
    // digits, and a few other chars, with "_"
    g_strcanon(basename, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.=+~$#@^&!?", '_');

    // Build the complete path by adding document base dir, if set, otherwise home dir
    gchar *directory = NULL;
    if ( document->getURI() ) {
        directory = g_path_get_dirname( document->getURI() );
    }
    if (directory == NULL) {
        directory = INKSCAPE.homedir_path(NULL);
    }
    gchar *filepath = g_build_filename(directory, basename, NULL);
    g_free(directory);

    //g_print("%s\n", filepath);

    // Remember parent and z-order of the topmost one
    gint pos = items.back()->getRepr()->position();
    SPObject *parent_object = items.back()->parent;
    Inkscape::XML::Node *parent = parent_object->getRepr();

    // Calculate resolution
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double res;
    int const prefs_res = prefs->getInt("/options/createbitmap/resolution", 0);
    int const prefs_min = prefs->getInt("/options/createbitmap/minsize", 0);
    if (0 < prefs_res) {
        // If it's given explicitly in prefs, take it
        res = prefs_res;
    } else if (0 < prefs_min) {
        // If minsize is given, look up minimum bitmap size (default 250 pixels) and calculate resolution from it
        res = Inkscape::Util::Quantity::convert(prefs_min, "in", "px") / MIN(bbox->width(), bbox->height());
    } else {
        float hint_xdpi = 0, hint_ydpi = 0;
        Glib::ustring hint_filename;
        // take resolution hint from the selected objects
        sp_selection_get_export_hints(selection, hint_filename, &hint_xdpi, &hint_ydpi);
        if (hint_xdpi != 0) {
            res = hint_xdpi;
        } else {
            // take resolution hint from the document
            sp_document_get_export_hints(document, hint_filename, &hint_xdpi, &hint_ydpi);
            if (hint_xdpi != 0) {
                res = hint_xdpi;
            } else {
                // if all else fails, take the default 96 dpi
                res = Inkscape::Util::Quantity::convert(1, "in", "px");
            }
        }
    }

    // The width and height of the bitmap in pixels
    unsigned width = (unsigned) floor(bbox->width() * Inkscape::Util::Quantity::convert(res, "px", "in"));
    unsigned height =(unsigned) floor(bbox->height() * Inkscape::Util::Quantity::convert(res, "px", "in"));

    // Find out if we have to run an external filter
    gchar const *run = NULL;
    Glib::ustring filter = prefs->getString("/options/createbitmap/filter");
    if (!filter.empty()) {
        // filter command is given;
        // see if we have a parameter to pass to it
        Glib::ustring param1 = prefs->getString("/options/createbitmap/filter_param1");
        if (!param1.empty()) {
            if (param1[param1.length() - 1] == '%') {
                // if the param string ends with %, interpret it as a percentage of the image's max dimension
                gchar p1[256];
                g_ascii_dtostr(p1, 256, ceil(g_ascii_strtod(param1.data(), NULL) * MAX(width, height) / 100));
                // the first param is always the image filename, the second is param1
                run = g_strdup_printf("%s \"%s\" %s", filter.data(), filepath, p1);
            } else {
                // otherwise pass the param1 unchanged
                run = g_strdup_printf("%s \"%s\" %s", filter.data(), filepath, param1.data());
            }
        } else {
            // run without extra parameter
            run = g_strdup_printf("%s \"%s\"", filter.data(), filepath);
        }
    }

    // Calculate the matrix that will be applied to the image so that it exactly overlaps the source objects
    Geom::Affine eek;
    {
        SPItem *parentItem = dynamic_cast<SPItem *>(parent_object);
        if (parentItem) {
            eek = parentItem->i2dt_affine();
        } else {
            g_assert_not_reached();
        }
    }
    Geom::Affine t;

    double shift_x = bbox->min()[Geom::X];
    double shift_y = bbox->max()[Geom::Y];
    if (res == Inkscape::Util::Quantity::convert(1, "in", "px")) { // for default 96 dpi, snap it to pixel grid
        shift_x = round(shift_x);
        shift_y = -round(-shift_y); // this gets correct rounding despite coordinate inversion, remove the negations when the inversion is gone
    }
    t = Geom::Scale(1, -1) * Geom::Translate(shift_x, shift_y) * eek.inverse();  /// @fixme hardcoded doc2dt transform?

    // TODO: avoid roundtrip via file
    // Do the export
    sp_export_png_file(document, filepath,
                       bbox->min()[Geom::X], bbox->min()[Geom::Y],
                       bbox->max()[Geom::X], bbox->max()[Geom::Y],
                       width, height, res, res,
                       (guint32) 0xffffff00,
                       NULL, NULL,
                       true,  /*bool force_overwrite,*/
                       items);

    // Run filter, if any
    if (run) {
        g_print("Running external filter: %s\n", run);
        int result = system(run);

        if(result == -1)
            g_warning("Could not run external filter: %s\n", run);
    }

    // Import the image back
    Inkscape::Pixbuf *pb = Inkscape::Pixbuf::create_from_file(filepath);
    if (pb) {
        // Create the repr for the image
        // TODO: avoid unnecessary roundtrip between data URI and decoded pixbuf
        Inkscape::XML::Node * repr = xml_doc->createElement("svg:image");
        sp_embed_image(repr, pb);
        if (res == Inkscape::Util::Quantity::convert(1, "in", "px")) { // for default 96 dpi, snap it to pixel grid
            sp_repr_set_svg_double(repr, "width", width);
            sp_repr_set_svg_double(repr, "height", height);
        } else {
            sp_repr_set_svg_double(repr, "width", bbox->width());
            sp_repr_set_svg_double(repr, "height", bbox->height());
        }

        // Write transform
        gchar *c=sp_svg_transform_write(t);
        repr->setAttribute("transform", c);
        g_free(c);

        // add the new repr to the parent
        parent->appendChild(repr);

        // move to the saved position
        repr->setPosition(pos > 0 ? pos + 1 : 1);

        // Set selection to the new image
        selection->clear();
        selection->add(repr);

        // Clean up
        Inkscape::GC::release(repr);
        g_object_unref(pb);

        // Complete undoable transaction
        DocumentUndo::done(document, SP_VERB_SELECTION_CREATE_BITMAP,
                           _("Create bitmap"));
    }

    desktop->clearWaitingCursor();

    g_free(basename);
    g_free(filepath);
}

/* Creates a mask or clipPath from selection.
 * What is a clip group?
 * A clip group is a tangled mess of XML that allows an object inside a group
 * to clip the entire group using a few <use>s and generally irritating me.
 */

void sp_selection_set_clipgroup(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }
    SPDocument* doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to create clippath or mask from."));
        return;
    }
        
    std::vector<Inkscape::XML::Node*> p(selection->reprList());
    
    sort(p.begin(),p.end(),sp_repr_compare_position_bool);

    selection->clear();

    int topmost = (p.back())->position();
    Inkscape::XML::Node *topmost_parent = (p.back())->parent();

    Inkscape::XML::Node *inner = xml_doc->createElement("svg:g");
    inner->setAttribute("inkscape:label", "Clip");
    
    for(std::vector<Inkscape::XML::Node*>::const_iterator i=p.begin();i!=p.end();i++){
    	Inkscape::XML::Node *current = *i;

        if (current->parent() == topmost_parent) {
            Inkscape::XML::Node *spnew = current->duplicate(xml_doc);
            sp_repr_unparent(current);
            inner->appendChild(spnew);
            Inkscape::GC::release(spnew);
            topmost --; // only reduce count for those items deleted from topmost_parent
        } else { // move it to topmost_parent first
            std::vector<Inkscape::XML::Node*> temp_clip;

            // At this point, current may already have no item, due to its being a clone whose original is already moved away
            // So we copy it artificially calculating the transform from its repr->attr("transform") and the parent transform
            gchar const *t_str = current->attribute("transform");
            Geom::Affine item_t(Geom::identity());
            if (t_str)
                sp_svg_transform_read(t_str, &item_t);
            item_t *= SP_ITEM(doc->getObjectByRepr(current->parent()))->i2doc_affine();
            // FIXME: when moving both clone and original from a transformed group (either by
            // grouping into another parent, or by cut/paste) the transform from the original's
            // parent becomes embedded into original itself, and this affects its clones. Fix
            // this by remembering the transform diffs we write to each item into an array and
            // then, if this is clone, looking up its original in that array and pre-multiplying
            // it by the inverse of that original's transform diff.

            sp_selection_copy_one(current, item_t, temp_clip, xml_doc);
            sp_repr_unparent(current);

            // paste into topmost_parent (temporarily)
            std::vector<Inkscape::XML::Node*> copied = sp_selection_paste_impl(doc, doc->getObjectByRepr(topmost_parent), temp_clip);
            if (!copied.empty()) { // if success,
                // take pasted object (now in topmost_parent)
                Inkscape::XML::Node *in_topmost = copied.back();
                // make a copy
                Inkscape::XML::Node *spnew = in_topmost->duplicate(xml_doc);
                // remove pasted
                sp_repr_unparent(in_topmost);
                // put its copy into group
                inner->appendChild(spnew);
                Inkscape::GC::release(spnew);
            }
        }
    }
    
    Inkscape::XML::Node *outer = xml_doc->createElement("svg:g");
    outer->appendChild(inner);
    topmost_parent->appendChild(outer);
    outer->setPosition(topmost + 1);
    
    Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");
    clone->setAttribute("x", "0", false);
    clone->setAttribute("y", "0", false);
    clone->setAttribute("xlink:href", g_strdup_printf("#%s", inner->attribute("id")), false);

    clone->setAttribute("inkscape:transform-center-x", inner->attribute("inkscape:transform-center-x"), false);
    clone->setAttribute("inkscape:transform-center-y", inner->attribute("inkscape:transform-center-y"), false);

    const Geom::Affine maskTransform(Geom::Affine::identity());
    std::vector<Inkscape::XML::Node*> templist;
    templist.push_back(clone);
    // add the new clone to the top of the original's parent
    gchar const *mask_id = SPClipPath::create(templist, doc, &maskTransform);
    
    
    outer->setAttribute("clip-path", g_strdup_printf("url(#%s)", mask_id));

    Inkscape::GC::release(clone);
    
    selection->set(outer);
    DocumentUndo::done(doc, SP_VERB_OBJECT_SET_CLIPPATH, _("Create Clip Group"));
}

/**
 * Creates a mask or clipPath from selection.
 * Two different modes:
 *  if applyToLayer, all selection is moved to DEFS as mask/clippath
 *       and is applied to current layer
 *  otherwise, topmost object is used as mask for other objects
 * If \a apply_clip_path parameter is true, clipPath is created, otherwise mask
 *
 */
void sp_selection_set_mask(SPDesktop *desktop, bool apply_clip_path, bool apply_to_layer)
{
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    bool is_empty = selection->isEmpty();
    if ( apply_to_layer && is_empty) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to create clippath or mask from."));
        return;
    } else if (!apply_to_layer && ( is_empty || selection->itemList().size()==1 )) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select mask object and <b>object(s)</b> to apply clippath or mask to."));
        return;
    }

    // FIXME: temporary patch to prevent crash!
    // Remove this when bboxes are fixed to not blow up on an item clipped/masked with its own clone
    bool clone_with_original = selection_contains_both_clone_and_original(selection);
    if (clone_with_original) {
        return; // in this version, you cannot clip/mask an object with its own clone
    }
    // /END FIXME

    doc->ensureUpToDate();

    std::vector<SPItem*> items(selection->itemList());

    sort(items.begin(),items.end(),sp_object_compare_position_bool);

    // See lp bug #542004
    selection->clear();

    // create a list of duplicates
    std::vector<Inkscape::XML::Node*> mask_items;
    std::vector<SPItem*> apply_to_items;
    std::vector<SPItem*> items_to_delete;
    std::vector<SPItem*> items_to_select;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool topmost = prefs->getBool("/options/maskobject/topmost", true);
    bool remove_original = prefs->getBool("/options/maskobject/remove", true);
    int grouping = prefs->getInt("/options/maskobject/grouping", PREFS_MASKOBJECT_GROUPING_NONE);

    if (apply_to_layer) {
        // all selected items are used for mask, which is applied to a layer
        apply_to_items.push_back(SP_ITEM(desktop->currentLayer()));
    }

    for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++) {
        if((!topmost && !apply_to_layer && *i == items.front())
                || (topmost && !apply_to_layer && *i == items.back())
		        || apply_to_layer){

        	Geom::Affine oldtr=(*i)->transform;
            (*i)->doWriteTransform((*i)->getRepr(), (*i)->i2doc_affine());
            Inkscape::XML::Node *dup = (*i)->getRepr()->duplicate(xml_doc);
            (*i)->doWriteTransform((*i)->getRepr(), oldtr);
            mask_items.push_back(dup);

            if (remove_original) {
                items_to_delete.push_back(*i);
            }
            else {
                items_to_select.push_back(*i);
            }
        	continue;
        }else{
            apply_to_items.push_back(*i);
            items_to_select.push_back(*i);
        }
    }

    items.clear();

    if (grouping == PREFS_MASKOBJECT_GROUPING_ALL) {
        // group all those objects into one group
        // and apply mask to that
        Inkscape::XML::Node *group = xml_doc->createElement("svg:g");

        // make a note we should ungroup this when unsetting mask
        group->setAttribute("inkscape:groupmode", "maskhelper");

        std::vector<Inkscape::XML::Node*> reprs_to_group;
        for (std::vector<SPItem*>::const_iterator i = apply_to_items.begin(); i != apply_to_items.end(); i++) {
            reprs_to_group.push_back(static_cast<SPObject*>(*i)->getRepr());
        }
        items_to_select.clear();

        sp_selection_group_impl(reprs_to_group, group, xml_doc, doc);

        // apply clip/mask only to newly created group
        apply_to_items.clear();
        apply_to_items.push_back(dynamic_cast<SPItem*>(doc->getObjectByRepr(group)));

        items_to_select.push_back((SPItem*)(doc->getObjectByRepr(group)));

        Inkscape::GC::release(group);
    }
    if (grouping == PREFS_MASKOBJECT_GROUPING_SEPARATE) {
        items_to_select.clear();
    }


    gchar const *attributeName = apply_clip_path ? "clip-path" : "mask";
    for (std::vector<SPItem*>::const_reverse_iterator i = apply_to_items.rbegin(); i != apply_to_items.rend(); i++) {
        SPItem *item = reinterpret_cast<SPItem *>(*i);
        // inverted object transform should be applied to a mask object,
        // as mask is calculated in user space (after applying transform)
        std::vector<Inkscape::XML::Node*> mask_items_dup;
        for(std::vector<Inkscape::XML::Node*>::const_iterator it=mask_items.begin();it!=mask_items.end();it++)
        	mask_items_dup.push_back((*it)->duplicate(xml_doc));
        Inkscape::XML::Node *current = SP_OBJECT(*i)->getRepr();
        // Node to apply mask to
        Inkscape::XML::Node *apply_mask_to = current;

        if (grouping == PREFS_MASKOBJECT_GROUPING_SEPARATE) {
            // enclose current node in group, and apply crop/mask on that
            Inkscape::XML::Node *group = xml_doc->createElement("svg:g");
            // make a note we should ungroup this when unsetting mask
            group->setAttribute("inkscape:groupmode", "maskhelper");

            Inkscape::XML::Node *spnew = current->duplicate(xml_doc);
            gint position = current->position();
            current->parent()->appendChild(group);
            sp_repr_unparent(current);
            group->appendChild(spnew);
            group->setPosition(position);

            // Apply clip/mask to group instead
            apply_mask_to = group;

            items_to_select.push_back(item = (SPItem*)(doc->getObjectByRepr(group)));
            Inkscape::GC::release(spnew);
            Inkscape::GC::release(group);
        }

        Geom::Affine maskTransform(item->i2doc_affine().inverse());

        gchar const *mask_id = NULL;
        if (apply_clip_path) {
            mask_id = SPClipPath::create(mask_items_dup, doc, &maskTransform);
        } else {
            mask_id = sp_mask_create(mask_items_dup, doc, &maskTransform);
        }

        apply_mask_to->setAttribute(attributeName, Glib::ustring("url(#") + mask_id + ')');

    }

    for (std::vector<SPItem*>::const_iterator i = items_to_delete.begin(); i != items_to_delete.end(); i++) {
        SPObject *item = reinterpret_cast<SPObject*>(*i);
        item->deleteObject(false);
        items_to_select.erase(remove(items_to_select.begin(), items_to_select.end(), item), items_to_select.end());
    }

    selection->addList(items_to_select);

    if (apply_clip_path) {
        DocumentUndo::done(doc, SP_VERB_OBJECT_SET_CLIPPATH, _("Set clipping path"));
    } else {
        DocumentUndo::done(doc, SP_VERB_OBJECT_SET_MASK, _("Set mask"));
    }
}

void sp_selection_unset_mask(SPDesktop *desktop, bool apply_clip_path) {
    if (desktop == NULL) {
        return;
    }

    SPDocument *doc = desktop->getDocument();
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();
    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to remove clippath or mask from."));
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool remove_original = prefs->getBool("/options/maskobject/remove", true);
    bool ungroup_masked = prefs->getBool("/options/maskobject/ungrouping", true);
    doc->ensureUpToDate();

    gchar const *attributeName = apply_clip_path ? "clip-path" : "mask";
    std::map<SPObject*,SPItem*> referenced_objects;

    std::vector<SPItem*> items(selection->itemList());
    selection->clear();

    GSList *items_to_ungroup = NULL;
    std::vector<SPItem*> items_to_select(items);


    // SPObject* refers to a group containing the clipped path or mask itself,
    // whereas SPItem* refers to the item being clipped or masked
    for (std::vector<SPItem*>::const_iterator i=items.begin();i!=items.end();i++){
        if (remove_original) {
            // remember referenced mask/clippath, so orphaned masks can be moved back to document
            SPItem *item = *i;
            Inkscape::URIReference *uri_ref = NULL;

            if (apply_clip_path) {
                uri_ref = item->clip_ref;
            } else {
                uri_ref = item->mask_ref;
            }

            // collect distinct mask object (and associate with item to apply transform)
            if ((NULL != uri_ref) && (NULL != uri_ref->getObject())) {
                referenced_objects[uri_ref->getObject()] = item;
            }
        }

        (*i)->getRepr()->setAttribute(attributeName, "none");

        SPGroup *group = dynamic_cast<SPGroup *>(*i);
        if (ungroup_masked && group) {
                // if we had previously enclosed masked object in group,
                // add it to list so we can ungroup it later

                // ungroup only groups we created when setting clip/mask
                if (group->layerMode() == SPGroup::MASK_HELPER) {
                    items_to_ungroup = g_slist_prepend(items_to_ungroup, group);
                }

        }
    }

    // restore mask objects into a document
    for ( std::map<SPObject*,SPItem*>::iterator it = referenced_objects.begin() ; it != referenced_objects.end() ; ++it) {
        SPObject *obj = (*it).first; // Group containing the clipped paths or masks
        GSList *items_to_move = NULL;
        for ( SPObject *child = obj->firstChild() ; child; child = child->getNext() ) {
            // Collect all clipped paths and masks within a single group
            Inkscape::XML::Node *copy = child->getRepr()->duplicate(xml_doc);
            if(copy->attribute("inkscape:original-d") && copy->attribute("inkscape:path-effect"))
            {
                copy->setAttribute("d", copy->attribute("inkscape:original-d"));
            }
            items_to_move = g_slist_prepend(items_to_move, copy);
        }

        if (!obj->isReferenced()) {
            // delete from defs if no other object references this mask
            obj->deleteObject(false);
        }

        // remember parent and position of the item to which the clippath/mask was applied
        Inkscape::XML::Node *parent = ((*it).second)->getRepr()->parent();
        gint pos = ((*it).second)->getRepr()->position();

        // Iterate through all clipped paths / masks
        for (GSList *i = items_to_move; NULL != i; i = i->next) {
            Inkscape::XML::Node *repr = static_cast<Inkscape::XML::Node *>(i->data);

            // insert into parent, restore pos
            parent->appendChild(repr);
            repr->setPosition((pos + 1) > 0 ? (pos + 1) : 0);

            SPItem *mask_item = static_cast<SPItem *>(desktop->getDocument()->getObjectByRepr(repr));
            items_to_select.push_back(mask_item);

            // transform mask, so it is moved the same spot where mask was applied
            Geom::Affine transform(mask_item->transform);
            transform *= (*it).second->transform;
            mask_item->doWriteTransform(mask_item->getRepr(), transform);
        }

        g_slist_free(items_to_move);
    }

    // ungroup marked groups added when setting mask
    for (GSList *i = items_to_ungroup ; NULL != i ; i = i->next) {
        SPGroup *group = dynamic_cast<SPGroup *>(static_cast<SPObject *>(i->data));
        if (group) {
            items_to_select.erase(remove(items_to_select.begin(), items_to_select.end(), group), items_to_select.end());
            std::vector<SPItem*> children;
            sp_item_group_ungroup(group, children, false);
            items_to_select.insert(items_to_select.end(),children.rbegin(),children.rend());
        } else {
            g_assert_not_reached();
        }
    }

    g_slist_free(items_to_ungroup);

    // rebuild selection
    selection->addList(items_to_select);

    if (apply_clip_path) {
        DocumentUndo::done(doc, SP_VERB_OBJECT_UNSET_CLIPPATH, _("Release clipping path"));
    } else {
        DocumentUndo::done(doc, SP_VERB_OBJECT_UNSET_MASK, _("Release mask"));
    }
}

/**
 * \param with_margins margins defined in the xml under <sodipodi:namedview>
 *                     "fit-margin-..." attributes.  See SPDocument::fitToRect.
 * \return true if an undoable change should be recorded.
 */
bool
fit_canvas_to_selection(SPDesktop *desktop, bool with_margins)
{
    g_return_val_if_fail(desktop != NULL, false);
    SPDocument *doc = desktop->getDocument();

    g_return_val_if_fail(doc != NULL, false);
    g_return_val_if_fail(desktop->selection != NULL, false);

    if (desktop->selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to fit canvas to."));
        return false;
    }
    Geom::OptRect const bbox(desktop->selection->visualBounds());
    if (bbox) {
        doc->fitToRect(*bbox, with_margins);
        return true;
    } else {
        return false;
    }
}

/**
 * Fit canvas to the bounding box of the selection, as an undoable action.
 */
void
verb_fit_canvas_to_selection(SPDesktop *const desktop)
{
    if (fit_canvas_to_selection(desktop)) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_FIT_CANVAS_TO_SELECTION,
                           _("Fit Page to Selection"));
    }
}

/**
 * \param with_margins margins defined in the xml under <sodipodi:namedview>
 *                     "fit-margin-..." attributes.  See SPDocument::fitToRect.
 */
bool
fit_canvas_to_drawing(SPDocument *doc, bool with_margins)
{
    g_return_val_if_fail(doc != NULL, false);

    doc->ensureUpToDate();
    SPItem const *const root = doc->getRoot();
    Geom::OptRect bbox = root->desktopVisualBounds();
    if (bbox) {
        doc->fitToRect(*bbox, with_margins);
        return true;
    } else {
        return false;
    }
}

void
verb_fit_canvas_to_drawing(SPDesktop *desktop)
{
    if (fit_canvas_to_drawing(desktop->getDocument())) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_FIT_CANVAS_TO_DRAWING,
                           _("Fit Page to Drawing"));
    }
}

/**
 * Fits canvas to selection or drawing with margins from <sodipodi:namedview>
 * "fit-margin-..." attributes.  See SPDocument::fitToRect and
 * ui/dialog/page-sizer.
 */
void fit_canvas_to_selection_or_drawing(SPDesktop *desktop) {
    g_return_if_fail(desktop != NULL);
    SPDocument *doc = desktop->getDocument();

    g_return_if_fail(doc != NULL);
    g_return_if_fail(desktop->selection != NULL);

    bool const changed = ( desktop->selection->isEmpty()
                           ? fit_canvas_to_drawing(doc, true)
                           : fit_canvas_to_selection(desktop, true) );
    if (changed) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING,
                           _("Fit Page to Selection or Drawing"));
    }
};

static void itemtree_map(void (*f)(SPItem *, SPDesktop *), SPObject *root, SPDesktop *desktop) {
    // don't operate on layers
    {
        SPItem *item = dynamic_cast<SPItem *>(root);
        if (item && !desktop->isLayer(item)) {
            f(item, desktop);
        }
    }
    for ( SPObject::SiblingIterator iter = root->firstChild() ; iter ; ++iter ) {
        //don't recurse into locked layers
        SPItem *item = dynamic_cast<SPItem *>(&*iter);
        if (!(item && desktop->isLayer(item) && item->isLocked())) {
            itemtree_map(f, iter, desktop);
        }
    }
}

static void unlock(SPItem *item, SPDesktop */*desktop*/) {
    if (item->isLocked()) {
        item->setLocked(FALSE);
    }
}

static void unhide(SPItem *item, SPDesktop *desktop) {
    if (desktop->itemIsHidden(item)) {
        item->setExplicitlyHidden(FALSE);
    }
}

static void process_all(void (*f)(SPItem *, SPDesktop *), SPDesktop *dt, bool layer_only) {
    if (!dt) return;

    SPObject *root;
    if (layer_only) {
        root = dt->currentLayer();
    } else {
        root = dt->currentRoot();
    }

    itemtree_map(f, root, dt);
}

void unlock_all(SPDesktop *dt) {
    process_all(&unlock, dt, true);
}

void unlock_all_in_all_layers(SPDesktop *dt) {
    process_all(&unlock, dt, false);
}

void unhide_all(SPDesktop *dt) {
    process_all(&unhide, dt, true);
}

void unhide_all_in_all_layers(SPDesktop *dt) {
    process_all(&unhide, dt, false);
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
