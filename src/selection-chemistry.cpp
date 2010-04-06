/** @file
 * @brief Miscellanous operations on selected items
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Andrius R. <knutux@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Martin Sucha <martin.sucha-inkscape@jts-sro.sk>
 *
 * Copyright (C) 1999-2010 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "selection-chemistry.h"

// TOOD fixme: This should be moved into preference repr
SPCycleType SP_CYCLING = SP_CYCLE_FOCUS;


#include <gtkmm/clipboard.h>

#include "svg/svg.h"
#include "desktop.h"
#include "desktop-style.h"
#include "dir-util.h"
#include "selection.h"
#include "tools-switch.h"
#include "desktop-handles.h"
#include "message-stack.h"
#include "sp-item-transform.h"
#include "marker.h"
#include "sp-use.h"
#include "sp-textpath.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "sp-flowtext.h"
#include "sp-flowregion.h"
#include "sp-image.h"
#include "text-editing.h"
#include "text-context.h"
#include "connector-context.h"
#include "sp-path.h"
#include "sp-conn-end.h"
#include "dropper-context.h"
#include <glibmm/i18n.h>
#include "libnr/nr-matrix-rotate-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-ops.h"
#include <libnr/nr-matrix-ops.h>
#include <2geom/transforms.h>
#include "xml/repr.h"
#include "xml/rebase-hrefs.h"
#include "style.h"
#include "document-private.h"
#include "sp-gradient.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient-fns.h"
#include "sp-pattern.h"
#include "sp-radial-gradient-fns.h"
#include "gradient-context.h"
#include "sp-namedview.h"
#include "preferences.h"
#include "sp-offset.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "file.h"
#include "helper/png-write.h"
#include "layer-fns.h"
#include "context-fns.h"
#include <map>
#include <cstring>
#include <string>
#include "helper/units.h"
#include "sp-item.h"
#include "box3d.h"
#include "persp3d.h"
#include "unit-constants.h"
#include "xml/simple-document.h"
#include "sp-filter-reference.h"
#include "gradient-drag.h"
#include "uri-references.h"
#include "libnr/nr-convert2geom.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "inkscape-private.h"
#include "path-chemistry.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/multi-path-manipulator.h"

#include "enums.h"
#include "sp-item-group.h"

// For clippath editing
#include "tools-switch.h"
#include "ui/tool/node-tool.h"

#include "ui/clipboard.h"

using Geom::X;
using Geom::Y;

/* The clipboard handling is in ui/clipboard.cpp now. There are some legacy functions left here,
because the layer manipulation code uses them. It should be rewritten specifically
for that purpose. */



namespace Inkscape {

void SelectionHelper::selectAll(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
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
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_selected_nodes->selectAll();
    } else {
        sp_edit_select_all_in_all_layers(dt);
    }
}

void SelectionHelper::selectNone(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_selected_nodes->clear();
    } else {
        sp_desktop_selection(dt)->clear();
    }
}

void SelectionHelper::invert(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_multipath->invertSelectionInSubpaths();
    } else {
        sp_edit_invert(dt);
    }
}

void SelectionHelper::invertAllInAll(SPDesktop *dt)
{
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_selected_nodes->invertSelection();
    } else {
        sp_edit_invert_in_all_layers(dt);
    }
}

void SelectionHelper::reverse(SPDesktop *dt)
{
    // TODO make this a virtual method of event context!
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_multipath->reverseSubpaths();
    } else {
        sp_selected_path_reverse(dt);
    }
}

void SelectionHelper::selectNext(SPDesktop *dt)
{
    SPEventContext *ec = dt->event_context;
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_multipath->shiftSelection(1);
    } else if (tools_isactive(dt, TOOLS_GRADIENT)
               && ec->_grdrag->isNonEmpty()) {
        sp_gradient_context_select_next(ec);
    } else {
        sp_selection_item_next(dt);
    }
}

void SelectionHelper::selectPrev(SPDesktop *dt)
{
    SPEventContext *ec = dt->event_context;
    if (tools_isactive(dt, TOOLS_NODES)) {
        InkNodeTool *nt = static_cast<InkNodeTool*>(dt->event_context);
        nt->_multipath->shiftSelection(-1);
    } else if (tools_isactive(dt, TOOLS_GRADIENT)
               && ec->_grdrag->isNonEmpty()) {
        sp_gradient_context_select_prev(ec);
    } else {
        sp_selection_item_prev(dt);
    }
}

} // namespace Inkscape


/**
 * Copies repr and its inherited css style elements, along with the accumulated transform 'full_t',
 * then prepends the copy to 'clip'.
 */
void sp_selection_copy_one(Inkscape::XML::Node *repr, Geom::Matrix full_t, GSList **clip, Inkscape::XML::Document* xml_doc)
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

    *clip = g_slist_prepend(*clip, copy);
}

void sp_selection_copy_impl(GSList const *items, GSList **clip, Inkscape::XML::Document* xml_doc)
{
    // Sort items:
    GSList *sorted_items = g_slist_copy((GSList *) items);
    sorted_items = g_slist_sort((GSList *) sorted_items, (GCompareFunc) sp_object_compare_position);

    // Copy item reprs:
    for (GSList *i = (GSList *) sorted_items; i != NULL; i = i->next) {
        sp_selection_copy_one(SP_OBJECT_REPR(i->data), sp_item_i2doc_affine(SP_ITEM(i->data)), clip, xml_doc);
    }

    *clip = g_slist_reverse(*clip);
    g_slist_free((GSList *) sorted_items);
}

GSList *sp_selection_paste_impl(SPDocument *doc, SPObject *parent, GSList **clip)
{
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    GSList *copied = NULL;
    // add objects to document
    for (GSList *l = *clip; l != NULL; l = l->next) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) l->data;
        Inkscape::XML::Node *copy = repr->duplicate(xml_doc);

        // premultiply the item transform by the accumulated parent transform in the paste layer
        Geom::Matrix local(sp_item_i2doc_affine(SP_ITEM(parent)));
        if (!local.isIdentity()) {
            gchar const *t_str = copy->attribute("transform");
            Geom::Matrix item_t(Geom::identity());
            if (t_str)
                sp_svg_transform_read(t_str, &item_t);
            item_t *= local.inverse();
            // (we're dealing with unattached repr, so we write to its attr instead of using sp_item_set_transform)
            gchar *affinestr=sp_svg_transform_write(item_t);
            copy->setAttribute("transform", affinestr);
            g_free(affinestr);
        }

        parent->appendChildRepr(copy);
        copied = g_slist_prepend(copied, copy);
        Inkscape::GC::release(copy);
    }
    return copied;
}

void sp_selection_delete_impl(GSList const *items, bool propagate = true, bool propagate_descendants = true)
{
    for (GSList const *i = items ; i ; i = i->next ) {
        sp_object_ref((SPObject *)i->data, NULL);
    }
    for (GSList const *i = items; i != NULL; i = i->next) {
        SPItem *item = (SPItem *) i->data;
        SP_OBJECT(item)->deleteObject(propagate, propagate_descendants);
        sp_object_unref((SPObject *)item, NULL);
    }
}


void sp_selection_delete(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return;
    }

    if (tools_isactive(desktop, TOOLS_TEXT))
        if (sp_text_delete_selection(desktop->event_context)) {
            sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT,
                             _("Delete text"));
            return;
        }

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("<b>Nothing</b> was deleted."));
        return;
    }

    GSList const *selected = g_slist_copy(const_cast<GSList *>(selection->itemList()));
    selection->clear();
    sp_selection_delete_impl(selected);
    g_slist_free((GSList *) selected);

    /* a tool may have set up private information in it's selection context
     * that depends on desktop items.  I think the only sane way to deal with
     * this currently is to reset the current tool, which will reset it's
     * associated selection context.  For example: deleting an object
     * while moving it around the canvas.
     */
    tools_switch( desktop, tools_active( desktop ) );

    sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_DELETE,
                     _("Delete"));
}

void add_ids_recursive(std::vector<const gchar *> &ids, SPObject *obj)
{
    if (!obj)
        return;

    ids.push_back(obj->getId());

    if (SP_IS_GROUP(obj)) {
        for (SPObject *child = sp_object_first_child(obj) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            add_ids_recursive(ids, child);
        }
    }
}

void sp_selection_duplicate(SPDesktop *desktop, bool suppressDone)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = desktop->doc();
    Inkscape::XML::Document* xml_doc = sp_document_repr_doc(doc);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to duplicate."));
        return;
    }

    GSList *reprs = g_slist_copy((GSList *) selection->reprList());

    selection->clear();

    // sorting items from different parents sorts each parent's subset without possibly mixing
    // them, just what we need
    reprs = g_slist_sort(reprs, (GCompareFunc) sp_repr_compare_position);

    GSList *newsel = NULL;

    std::vector<const gchar *> old_ids;
    std::vector<const gchar *> new_ids;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool relink_clones = prefs->getBool("/options/relinkclonesonduplicate/value");

    while (reprs) {
        Inkscape::XML::Node *old_repr = (Inkscape::XML::Node *) reprs->data;
        Inkscape::XML::Node *parent = old_repr->parent();
        Inkscape::XML::Node *copy = old_repr->duplicate(xml_doc);

        parent->appendChild(copy);

        if (relink_clones) {
            SPObject *old_obj = doc->getObjectByRepr(old_repr);
            SPObject *new_obj = doc->getObjectByRepr(copy);
            add_ids_recursive(old_ids, old_obj);
            add_ids_recursive(new_ids, new_obj);
        }

        newsel = g_slist_prepend(newsel, copy);
        reprs = g_slist_remove(reprs, reprs->data);
        Inkscape::GC::release(copy);
    }

    if (relink_clones) {

        g_assert(old_ids.size() == new_ids.size());

        for (unsigned int i = 0; i < old_ids.size(); i++) {
            const gchar *id = old_ids[i];
            SPObject *old_clone = doc->getObjectById(id);
            if (SP_IS_USE(old_clone)) {
                SPItem *orig = sp_use_get_original(SP_USE(old_clone));
                if (!orig) // orphaned
                    continue;
                for (unsigned int j = 0; j < old_ids.size(); j++) {
                    if (!strcmp(orig->getId(), old_ids[j])) {
                        // we have both orig and clone in selection, relink
                        // std::cout << id  << " old, its ori: " << SP_OBJECT_ID(orig) << "; will relink:" << new_ids[i] << " to " << new_ids[j] << "\n";
                        gchar *newref = g_strdup_printf("#%s", new_ids[j]);
                        SPObject *new_clone = doc->getObjectById(new_ids[i]);
                        SP_OBJECT_REPR(new_clone)->setAttribute("xlink:href", newref);
                        new_clone->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                        g_free(newref);
                    }
                }
            }
        }
    }


    if ( !suppressDone ) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_DUPLICATE,
                         _("Duplicate"));
    }

    selection->setReprList(newsel);

    g_slist_free(newsel);
}

void sp_edit_clear_all(SPDesktop *dt)
{
    if (!dt)
        return;

    SPDocument *doc = sp_desktop_document(dt);
    sp_desktop_selection(dt)->clear();

    g_return_if_fail(SP_IS_GROUP(dt->currentLayer()));
    GSList *items = sp_item_group_item_list(SP_GROUP(dt->currentLayer()));

    while (items) {
        SP_OBJECT(items->data)->deleteObject();
        items = g_slist_remove(items, items->data);
    }

    sp_document_done(doc, SP_VERB_EDIT_CLEAR_ALL,
                     _("Delete all"));
}

GSList *
get_all_items(GSList *list, SPObject *from, SPDesktop *desktop, bool onlyvisible, bool onlysensitive, GSList const *exclude)
{
    for (SPObject *child = sp_object_first_child(SP_OBJECT(from)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_ITEM(child) &&
            !desktop->isLayer(SP_ITEM(child)) &&
            (!onlysensitive || !SP_ITEM(child)->isLocked()) &&
            (!onlyvisible || !desktop->itemIsHidden(SP_ITEM(child))) &&
            (!exclude || !g_slist_find((GSList *) exclude, child))
            )
        {
            list = g_slist_prepend(list, SP_ITEM(child));
        }

        if (SP_IS_ITEM(child) && desktop->isLayer(SP_ITEM(child))) {
            list = get_all_items(list, child, desktop, onlyvisible, onlysensitive, exclude);
        }
    }

    return list;
}

void sp_edit_select_all_full(SPDesktop *dt, bool force_all_layers, bool invert)
{
    if (!dt)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(dt);

    g_return_if_fail(SP_IS_GROUP(dt->currentLayer()));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    PrefsSelectionContext inlayer = (PrefsSelectionContext) prefs->getInt("/options/kbselection/inlayer", PREFS_SELECTION_LAYER);
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);

    GSList *items = NULL;

    GSList const *exclude = NULL;
    if (invert) {
        exclude = selection->itemList();
    }

    if (force_all_layers)
        inlayer = PREFS_SELECTION_ALL;

    switch (inlayer) {
        case PREFS_SELECTION_LAYER: {
        if ( (onlysensitive && SP_ITEM(dt->currentLayer())->isLocked()) ||
             (onlyvisible && dt->itemIsHidden(SP_ITEM(dt->currentLayer()))) )
        return;

        GSList *all_items = sp_item_group_item_list(SP_GROUP(dt->currentLayer()));

        for (GSList *i = all_items; i; i = i->next) {
            SPItem *item = SP_ITEM(i->data);

            if (item && (!onlysensitive || !item->isLocked())) {
                if (!onlyvisible || !dt->itemIsHidden(item)) {
                    if (!dt->isLayer(item)) {
                        if (!invert || !g_slist_find((GSList *) exclude, item)) {
                            items = g_slist_prepend(items, item); // leave it in the list
                        }
                    }
                }
            }
        }

        g_slist_free(all_items);
            break;
        }
        case PREFS_SELECTION_LAYER_RECURSIVE: {
            items = get_all_items(NULL, dt->currentLayer(), dt, onlyvisible, onlysensitive, exclude);
            break;
        }
        default: {
        items = get_all_items(NULL, dt->currentRoot(), dt, onlyvisible, onlysensitive, exclude);
            break;
    }
    }

    selection->setList(items);

    if (items) {
        g_slist_free(items);
    }
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

void sp_selection_group_impl(GSList *p, Inkscape::XML::Node *group, Inkscape::XML::Document *xml_doc, SPDocument *doc) {
    
    p = g_slist_sort(p, (GCompareFunc) sp_repr_compare_position);

    // Remember the position and parent of the topmost object.
    gint topmost = ((Inkscape::XML::Node *) g_slist_last(p)->data)->position();
    Inkscape::XML::Node *topmost_parent = ((Inkscape::XML::Node *) g_slist_last(p)->data)->parent();

    while (p) {
        Inkscape::XML::Node *current = (Inkscape::XML::Node *) p->data;

        if (current->parent() == topmost_parent) {
            Inkscape::XML::Node *spnew = current->duplicate(xml_doc);
            sp_repr_unparent(current);
            group->appendChild(spnew);
            Inkscape::GC::release(spnew);
            topmost --; // only reduce count for those items deleted from topmost_parent
        } else { // move it to topmost_parent first
            GSList *temp_clip = NULL;

            // At this point, current may already have no item, due to its being a clone whose original is already moved away
            // So we copy it artificially calculating the transform from its repr->attr("transform") and the parent transform
            gchar const *t_str = current->attribute("transform");
            Geom::Matrix item_t(Geom::identity());
            if (t_str)
                sp_svg_transform_read(t_str, &item_t);
            item_t *= sp_item_i2doc_affine(SP_ITEM(doc->getObjectByRepr(current->parent())));
            // FIXME: when moving both clone and original from a transformed group (either by
            // grouping into another parent, or by cut/paste) the transform from the original's
            // parent becomes embedded into original itself, and this affects its clones. Fix
            // this by remembering the transform diffs we write to each item into an array and
            // then, if this is clone, looking up its original in that array and pre-multiplying
            // it by the inverse of that original's transform diff.

            sp_selection_copy_one(current, item_t, &temp_clip, xml_doc);
            sp_repr_unparent(current);

            // paste into topmost_parent (temporarily)
            GSList *copied = sp_selection_paste_impl(doc, doc->getObjectByRepr(topmost_parent), &temp_clip);
            if (temp_clip) g_slist_free(temp_clip);
            if (copied) { // if success,
                // take pasted object (now in topmost_parent)
                Inkscape::XML::Node *in_topmost = (Inkscape::XML::Node *) copied->data;
                // make a copy
                Inkscape::XML::Node *spnew = in_topmost->duplicate(xml_doc);
                // remove pasted
                sp_repr_unparent(in_topmost);
                // put its copy into group
                group->appendChild(spnew);
                Inkscape::GC::release(spnew);
                g_slist_free(copied);
            }
        }
        p = g_slist_remove(p, current);
    }

    // Add the new group to the topmost members' parent
    topmost_parent->appendChild(group);

    // Move to the position of the topmost, reduced by the number of items deleted from topmost_parent
    group->setPosition(topmost + 1);
}

void sp_selection_group(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // Check if something is selected.
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>some objects</b> to group."));
        return;
    }

    GSList const *l = (GSList *) selection->reprList();
    
    GSList *p = g_slist_copy((GSList *) l);
    
    selection->clear();

    Inkscape::XML::Node *group = xml_doc->createElement("svg:g");

    sp_selection_group_impl(p, group, xml_doc, doc);

    sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_GROUP,
                     _("Group"));

    selection->set(group);
    Inkscape::GC::release(group);
}

void sp_selection_ungroup(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select a <b>group</b> to ungroup."));
        return;
    }

    GSList *items = g_slist_copy((GSList *) selection->itemList());
    selection->clear();

    // Get a copy of current selection.
    GSList *new_select = NULL;
    bool ungrouped = false;
    for (GSList *i = items;
         i != NULL;
         i = i->next)
    {
        SPItem *group = (SPItem *) i->data;

        // when ungrouping cloned groups with their originals, some objects that were selected may no more exist due to unlinking
        if (!SP_IS_OBJECT(group)) {
            continue;
        }

        /* We do not allow ungrouping <svg> etc. (lauris) */
        if (strcmp(SP_OBJECT_REPR(group)->name(), "svg:g") && strcmp(SP_OBJECT_REPR(group)->name(), "svg:switch")) {
            // keep the non-group item in the new selection
            new_select = g_slist_append(new_select, group);
            continue;
        }

        GSList *children = NULL;
        /* This is not strictly required, but is nicer to rely on group ::destroy (lauris) */
        sp_item_group_ungroup(SP_GROUP(group), &children, false);
        ungrouped = true;
        // Add ungrouped items to the new selection.
        new_select = g_slist_concat(new_select, children);
    }

    if (new_select) { // Set new selection.
        selection->addList(new_select);
        g_slist_free(new_select);
    }
    if (!ungrouped) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No groups</b> to ungroup in the selection."));
    }

    g_slist_free(items);

    sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_UNGROUP,
                     _("Ungroup"));
}

/** Replace all groups in the list with their member objects, recursively; returns a new list, frees old */
GSList *
sp_degroup_list(GSList *items)
{
    GSList *out = NULL;
    bool has_groups = false;
    for (GSList *item = items; item; item = item->next) {
        if (!SP_IS_GROUP(item->data)) {
            out = g_slist_prepend(out, item->data);
        } else {
            has_groups = true;
            GSList *members = sp_item_group_item_list(SP_GROUP(item->data));
            for (GSList *member = members; member; member = member->next) {
                out = g_slist_prepend(out, member->data);
            }
            g_slist_free(members);
        }
    }
    out = g_slist_reverse(out);
    g_slist_free(items);

    if (has_groups) { // recurse if we unwrapped a group - it may have contained others
        out = sp_degroup_list(out);
    }

    return out;
}


/** If items in the list have a common parent, return it, otherwise return NULL */
static SPGroup *
sp_item_list_common_parent_group(GSList const *items)
{
    if (!items) {
        return NULL;
    }
    SPObject *parent = SP_OBJECT_PARENT(items->data);
    /* Strictly speaking this CAN happen, if user selects <svg> from Inkscape::XML editor */
    if (!SP_IS_GROUP(parent)) {
        return NULL;
    }
    for (items = items->next; items; items = items->next) {
        if (SP_OBJECT_PARENT(items->data) != parent) {
            return NULL;
        }
    }

    return SP_GROUP(parent);
}

/** Finds out the minimum common bbox of the selected items. */
static Geom::OptRect
enclose_items(GSList const *items)
{
    g_assert(items != NULL);

    Geom::OptRect r;
    for (GSList const *i = items; i; i = i->next) {
        r = Geom::unify(r, sp_item_bbox_desktop((SPItem *) i->data));
    }
    return r;
}

SPObject *
prev_sibling(SPObject *child)
{
    SPObject *parent = SP_OBJECT_PARENT(child);
    if (!SP_IS_GROUP(parent)) {
        return NULL;
    }
    for ( SPObject *i = sp_object_first_child(parent) ; i; i = SP_OBJECT_NEXT(i) ) {
        if (i->next == child)
            return i;
    }
    return NULL;
}

void
sp_selection_raise(SPDesktop *desktop)
{
    if (!desktop)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    GSList const *items = (GSList *) selection->itemList();
    if (!items) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to raise."));
        return;
    }

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    Inkscape::XML::Node *grepr = SP_OBJECT_REPR(group);

    /* Construct reverse-ordered list of selected children. */
    GSList *rev = g_slist_copy((GSList *) items);
    rev = g_slist_sort(rev, (GCompareFunc) sp_item_repr_compare_position);

    // Determine the common bbox of the selected items.
    Geom::OptRect selected = enclose_items(items);

    // Iterate over all objects in the selection (starting from top).
    if (selected) {
        while (rev) {
            SPObject *child = SP_OBJECT(rev->data);
            // for each selected object, find the next sibling
            for (SPObject *newref = child->next; newref; newref = newref->next) {
                // if the sibling is an item AND overlaps our selection,
                if (SP_IS_ITEM(newref)) {
                    Geom::OptRect newref_bbox = sp_item_bbox_desktop(SP_ITEM(newref));
                    if ( newref_bbox && selected->intersects(*newref_bbox) ) {
                        // AND if it's not one of our selected objects,
                        if (!g_slist_find((GSList *) items, newref)) {
                            // move the selected object after that sibling
                            grepr->changeOrder(SP_OBJECT_REPR(child), SP_OBJECT_REPR(newref));
                        }
                        break;
                    }
                }
            }
            rev = g_slist_remove(rev, child);
        }
    } else {
        g_slist_free(rev);
    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_RAISE,
                     //TRANSLATORS: only translate "string" in "context|string".
                     // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                     // "Raise" means "to raise an object" in the undo history
                     Q_("undo action|Raise"));
}

void sp_selection_raise_to_top(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *document = sp_desktop_document(desktop);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to raise to top."));
        return;
    }

    GSList const *items = (GSList *) selection->itemList();

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    GSList *rl = g_slist_copy((GSList *) selection->reprList());
    rl = g_slist_sort(rl, (GCompareFunc) sp_repr_compare_position);

    for (GSList *l = rl; l != NULL; l = l->next) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) l->data;
        repr->setPosition(-1);
    }

    g_slist_free(rl);

    sp_document_done(document, SP_VERB_SELECTION_TO_FRONT,
                     _("Raise to top"));
}

void
sp_selection_lower(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    GSList const *items = (GSList *) selection->itemList();
    if (!items) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to lower."));
        return;
    }

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    Inkscape::XML::Node *grepr = SP_OBJECT_REPR(group);

    // Determine the common bbox of the selected items.
    Geom::OptRect selected = enclose_items(items);

    /* Construct direct-ordered list of selected children. */
    GSList *rev = g_slist_copy((GSList *) items);
    rev = g_slist_sort(rev, (GCompareFunc) sp_item_repr_compare_position);
    rev = g_slist_reverse(rev);

    // Iterate over all objects in the selection (starting from top).
    if (selected) {
        while (rev) {
            SPObject *child = SP_OBJECT(rev->data);
            // for each selected object, find the prev sibling
            for (SPObject *newref = prev_sibling(child); newref; newref = prev_sibling(newref)) {
                // if the sibling is an item AND overlaps our selection,
                if (SP_IS_ITEM(newref)) {
                    Geom::OptRect ref_bbox = sp_item_bbox_desktop(SP_ITEM(newref));
                    if ( ref_bbox && selected->intersects(*ref_bbox) ) {
                        // AND if it's not one of our selected objects,
                        if (!g_slist_find((GSList *) items, newref)) {
                            // move the selected object before that sibling
                            SPObject *put_after = prev_sibling(newref);
                            if (put_after)
                                grepr->changeOrder(SP_OBJECT_REPR(child), SP_OBJECT_REPR(put_after));
                            else
                                SP_OBJECT_REPR(child)->setPosition(0);
                        }
                        break;
                    }
                }
            }
            rev = g_slist_remove(rev, child);
        }
    } else {
        g_slist_free(rev);
    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_LOWER,
                     _("Lower"));
}

void sp_selection_lower_to_bottom(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *document = sp_desktop_document(desktop);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to lower to bottom."));
        return;
    }

    GSList const *items = (GSList *) selection->itemList();

    SPGroup const *group = sp_item_list_common_parent_group(items);
    if (!group) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("You cannot raise/lower objects from <b>different groups</b> or <b>layers</b>."));
        return;
    }

    GSList *rl;
    rl = g_slist_copy((GSList *) selection->reprList());
    rl = g_slist_sort(rl, (GCompareFunc) sp_repr_compare_position);
    rl = g_slist_reverse(rl);

    for (GSList *l = rl; l != NULL; l = l->next) {
        gint minpos;
        SPObject *pp, *pc;
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) l->data;
        pp = document->getObjectByRepr(sp_repr_parent(repr));
        minpos = 0;
        g_assert(SP_IS_GROUP(pp));
        pc = sp_object_first_child(pp);
        while (!SP_IS_ITEM(pc)) {
            minpos += 1;
            pc = pc->next;
        }
        repr->setPosition(minpos);
    }

    g_slist_free(rl);

    sp_document_done(document, SP_VERB_SELECTION_TO_BACK,
                     _("Lower to bottom"));
}

void
sp_undo(SPDesktop *desktop, SPDocument *)
{
        if (!sp_document_undo(sp_desktop_document(desktop)))
            desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Nothing to undo."));
}

void
sp_redo(SPDesktop *desktop, SPDocument *)
{
        if (!sp_document_redo(sp_desktop_document(desktop)))
            desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Nothing to redo."));
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
take_style_from_item(SPItem *item)
{
    // write the complete cascaded style, context-free
    SPCSSAttr *css = sp_css_attr_from_object(SP_OBJECT(item), SP_STYLE_FLAG_ALWAYS);
    if (css == NULL)
        return NULL;

    if ((SP_IS_GROUP(item) && SP_OBJECT(item)->children) ||
        (SP_IS_TEXT(item) && SP_OBJECT(item)->children && SP_OBJECT(item)->children->next == NULL)) {
        // if this is a text with exactly one tspan child, merge the style of that tspan as well
        // If this is a group, merge the style of its topmost (last) child with style
        for (SPObject *last_element = item->lastChild(); last_element != NULL; last_element = SP_OBJECT_PREV(last_element)) {
            if (SP_OBJECT_STYLE(last_element) != NULL) {
                SPCSSAttr *temp = sp_css_attr_from_object(last_element, SP_STYLE_FLAG_IFSET);
                if (temp) {
                    sp_repr_css_merge(css, temp);
                    sp_repr_css_attr_unref(temp);
                }
                break;
            }
        }
    }
    if (!(SP_IS_TEXT(item) || SP_IS_TSPAN(item) || SP_IS_TREF(item) || SP_IS_STRING(item))) {
        // do not copy text properties from non-text objects, it's confusing
        css = sp_css_attr_unset_text(css);
    }

    // FIXME: also transform gradient/pattern fills, by forking? NO, this must be nondestructive
    double ex = to_2geom(sp_item_i2doc_affine(item)).descrim();
    if (ex != 1.0) {
        css = sp_css_attr_scale(css, ex);
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
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_PASTE, _("Paste"));
    }
}

void sp_selection_paste_style(SPDesktop *desktop)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pasteStyle(desktop)) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_PASTE_STYLE, _("Paste style"));
    }
}


void sp_selection_paste_livepatheffect(SPDesktop *desktop)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pastePathEffect(desktop)) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_PASTE_LIVEPATHEFFECT,
                         _("Paste live path effect"));
    }
}


void sp_selection_remove_livepatheffect_impl(SPItem *item)
{
    if ( item && SP_IS_LPE_ITEM(item) &&
         sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {
        sp_lpe_item_remove_all_path_effects(SP_LPE_ITEM(item), false);
    }
}

void sp_selection_remove_livepatheffect(SPDesktop *desktop)
{
    if (desktop == NULL) return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to remove live path effects from."));
        return;
    }

    for ( GSList const *itemlist = selection->itemList(); itemlist != NULL; itemlist = g_slist_next(itemlist) ) {
        SPItem *item = reinterpret_cast<SPItem*>(itemlist->data);

        sp_selection_remove_livepatheffect_impl(item);

    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_REMOVE_LIVEPATHEFFECT,
                     _("Remove live path effect"));
}

void sp_selection_remove_filter(SPDesktop *desktop)
{
    if (desktop == NULL) return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to remove filters from."));
        return;
    }

    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_unset_property(css, "filter");
    sp_desktop_set_style(desktop, css);
    sp_repr_css_attr_unref(css);

    sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_REMOVE_FILTER,
                     _("Remove filter"));
}


void sp_selection_paste_size(SPDesktop *desktop, bool apply_x, bool apply_y)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pasteSize(desktop, false, apply_x, apply_y)) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_PASTE_SIZE,
                         _("Paste size"));
    }
}

void sp_selection_paste_size_separately(SPDesktop *desktop, bool apply_x, bool apply_y)
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    if (cm->pasteSize(desktop, true, apply_x, apply_y)) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_PASTE_SIZE_SEPARATELY,
                         _("Paste size separately"));
    }
}

void sp_selection_to_next_layer(SPDesktop *dt, bool suppressDone)
{
    Inkscape::Selection *selection = sp_desktop_selection(dt);

    // check if something is selected
    if (selection->isEmpty()) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to move to the layer above."));
        return;
    }

    GSList const *items = g_slist_copy((GSList *) selection->itemList());

    bool no_more = false; // Set to true, if no more layers above
    SPObject *next=Inkscape::next_layer(dt->currentRoot(), dt->currentLayer());
    if (next) {
        GSList *temp_clip = NULL;
        sp_selection_copy_impl(items, &temp_clip, sp_document_repr_doc(dt->doc()));
        sp_selection_delete_impl(items, false, false);
        next=Inkscape::next_layer(dt->currentRoot(), dt->currentLayer()); // Fixes bug 1482973: crash while moving layers
        GSList *copied;
        if (next) {
            copied = sp_selection_paste_impl(sp_desktop_document(dt), next, &temp_clip);
        } else {
            copied = sp_selection_paste_impl(sp_desktop_document(dt), dt->currentLayer(), &temp_clip);
            no_more = true;
        }
        selection->setReprList((GSList const *) copied);
        g_slist_free(copied);
        if (temp_clip) g_slist_free(temp_clip);
        if (next) dt->setCurrentLayer(next);
        if ( !suppressDone ) {
            sp_document_done(sp_desktop_document(dt), SP_VERB_LAYER_MOVE_TO_NEXT,
                             _("Raise to next layer"));
        }
    } else {
        no_more = true;
    }

    if (no_more) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No more layers above."));
    }

    g_slist_free((GSList *) items);
}

void sp_selection_to_prev_layer(SPDesktop *dt, bool suppressDone)
{
    Inkscape::Selection *selection = sp_desktop_selection(dt);

    // check if something is selected
    if (selection->isEmpty()) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to move to the layer below."));
        return;
    }

    GSList const *items = g_slist_copy((GSList *) selection->itemList());

    bool no_more = false; // Set to true, if no more layers below
    SPObject *next=Inkscape::previous_layer(dt->currentRoot(), dt->currentLayer());
    if (next) {
        GSList *temp_clip = NULL;
        sp_selection_copy_impl(items, &temp_clip, sp_document_repr_doc(dt->doc())); // we're in the same doc, so no need to copy defs
        sp_selection_delete_impl(items, false, false);
        next=Inkscape::previous_layer(dt->currentRoot(), dt->currentLayer()); // Fixes bug 1482973: crash while moving layers
        GSList *copied;
        if (next) {
            copied = sp_selection_paste_impl(sp_desktop_document(dt), next, &temp_clip);
        } else {
            copied = sp_selection_paste_impl(sp_desktop_document(dt), dt->currentLayer(), &temp_clip);
            no_more = true;
        }
        selection->setReprList((GSList const *) copied);
        g_slist_free(copied);
        if (temp_clip) g_slist_free(temp_clip);
        if (next) dt->setCurrentLayer(next);
        if ( !suppressDone ) {
            sp_document_done(sp_desktop_document(dt), SP_VERB_LAYER_MOVE_TO_PREV,
                             _("Lower to previous layer"));
        }
    } else {
        no_more = true;
    }

    if (no_more) {
        dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No more layers below."));
    }

    g_slist_free((GSList *) items);
}

bool
selection_contains_original(SPItem *item, Inkscape::Selection *selection)
{
    bool contains_original = false;

    bool is_use = SP_IS_USE(item);
    SPItem *item_use = item;
    SPItem *item_use_first = item;
    while (is_use && item_use && !contains_original)
    {
        item_use = sp_use_get_original(SP_USE(item_use));
        contains_original |= selection->includes(item_use);
        if (item_use == item_use_first)
            break;
        is_use = SP_IS_USE(item_use);
    }

    // If it's a tref, check whether the object containing the character
    // data is part of the selection
    if (!contains_original && SP_IS_TREF(item)) {
        contains_original = selection->includes(SP_TREF(item)->getObjectReferredTo());
    }

    return contains_original;
}


bool
selection_contains_both_clone_and_original(Inkscape::Selection *selection)
{
    bool clone_with_original = false;
    for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
        SPItem *item = SP_ITEM(l->data);
        clone_with_original |= selection_contains_original(item, selection);
        if (clone_with_original)
            break;
    }
    return clone_with_original;
}

/** Apply matrix to the selection.  \a set_i2d is normally true, which means objects are in the
original transform, synced with their reprs, and need to jump to the new transform in one go. A
value of set_i2d==false is only used by seltrans when it's dragging objects live (not outlines); in
that case, items are already in the new position, but the repr is in the old, and this function
then simply updates the repr from item->transform.
 */
void sp_selection_apply_affine(Inkscape::Selection *selection, Geom::Matrix const &affine, bool set_i2d, bool compensate)
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
            transf_persp = persp3d_create_xml_element (SP_OBJECT_DOCUMENT(persp), persp->perspective_impl);

            for (std::list<SPBox3D *>::iterator b = selboxes.begin(); b != selboxes.end(); ++b)
                box3d_switch_perspectives(*b, persp, transf_persp);
        } else {
            transf_persp = persp;
        }

        persp3d_apply_affine_transformation(transf_persp, affine);
    }

    for (GSList const *l = selection->itemList(); l != NULL; l = l->next) {
        SPItem *item = SP_ITEM(l->data);

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
        bool transform_textpath_with_path = (SP_IS_TEXT_TEXTPATH(item) && selection->includes( sp_textpath_get_path_item(SP_TEXTPATH(sp_object_first_child(SP_OBJECT(item)))) ));
        // ...both a flowtext and its frame?
        bool transform_flowtext_with_frame = (SP_IS_FLOWTEXT(item) && selection->includes( SP_FLOWTEXT(item)->get_frame(NULL))); // (only the first frame is checked so far)
        // ...both an offset and its source?
        bool transform_offset_with_source = (SP_IS_OFFSET(item) && SP_OFFSET(item)->sourceHref) && selection->includes( sp_offset_get_source(SP_OFFSET(item)) );

        // If we're moving a connector, we want to detach it
        // from shapes that aren't part of the selection, but
        // leave it attached if they are
        if (cc_item_is_connector(item)) {
            SPItem *attItem[2];
            SP_PATH(item)->connEndPair.getAttachedItems(attItem);

            for (int n = 0; n < 2; ++n) {
                if (!selection->includes(attItem[n])) {
                    sp_conn_end_detach(item, n);
                }
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
        if (transform_textpath_with_path || transform_offset_with_source) {
            // Restore item->transform field from the repr, in case it was changed by seltrans.
            sp_object_read_attr(SP_OBJECT(item), "transform");
        } else if (transform_flowtext_with_frame) {
            // apply the inverse of the region's transform to the <use> so that the flow remains
            // the same (even though the output itself gets transformed)
            for (SPObject *region = item->firstChild() ; region ; region = SP_OBJECT_NEXT(region)) {
                if (!SP_IS_FLOWREGION(region) && !SP_IS_FLOWREGIONEXCLUDE(region))
                    continue;
                for (SPObject *use = region->firstChild() ; use ; use = SP_OBJECT_NEXT(use)) {
                    if (!SP_IS_USE(use)) continue;
                    sp_item_write_transform(SP_USE(use), SP_OBJECT_REPR(use), item->transform.inverse(), NULL, compensate);
                }
            }
        } else if (transform_clone_with_original) {
            // We are transforming a clone along with its original. The below matrix juggling is
            // necessary to ensure that they transform as a whole, i.e. the clone's induced
            // transform and its move compensation are both cancelled out.

            // restore item->transform field from the repr, in case it was changed by seltrans
            sp_object_read_attr(SP_OBJECT(item), "transform");

            // calculate the matrix we need to apply to the clone to cancel its induced transform from its original
            Geom::Matrix parent2dt = sp_item_i2d_affine(SP_ITEM(SP_OBJECT_PARENT(item)));
            Geom::Matrix t = parent2dt * affine * parent2dt.inverse();
            Geom::Matrix t_inv = t.inverse();
            Geom::Matrix result = t_inv * item->transform * t;

            if ((prefs_parallel || prefs_unmoved) && affine.isTranslation()) {
                // we need to cancel out the move compensation, too

                // find out the clone move, same as in sp_use_move_compensate
                Geom::Matrix parent = sp_use_get_parent_transform(SP_USE(item));
                Geom::Matrix clone_move = parent.inverse() * t * parent;

                if (prefs_parallel) {
                    Geom::Matrix move = result * clone_move * t_inv;
                    sp_item_write_transform(item, SP_OBJECT_REPR(item), move, &move, compensate);

                } else if (prefs_unmoved) {
                    //if (SP_IS_USE(sp_use_get_original(SP_USE(item))))
                    //    clone_move = Geom::identity();
                    Geom::Matrix move = result * clone_move;
                    sp_item_write_transform(item, SP_OBJECT_REPR(item), move, &t, compensate);
                }

            } else {
                // just apply the result
                sp_item_write_transform(item, SP_OBJECT_REPR(item), result, &t, compensate);
            }

        } else {
            if (set_i2d) {
                sp_item_set_i2d_affine(item, sp_item_i2d_affine(item) * (Geom::Matrix)affine);
            }
            sp_item_write_transform(item, SP_OBJECT_REPR(item), item->transform, NULL, compensate);
        }

        // if we're moving the actual object, not just updating the repr, we can transform the
        // center by the same matrix (only necessary for non-translations)
        if (set_i2d && item->isCenterSet() && !(affine.isTranslation() || affine.isIdentity())) {
            item->setCenter(old_center * affine);
            SP_OBJECT(item)->updateRepr();
        }
    }
}

void sp_selection_remove_transform(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    GSList const *l = (GSList *) selection->reprList();
    while (l != NULL) {
        ((Inkscape::XML::Node*)l->data)->setAttribute("transform", NULL, false);
        l = l->next;
    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_OBJECT_FLATTEN,
                     _("Remove transform"));
}

void
sp_selection_scale_absolute(Inkscape::Selection *selection,
                            double const x0, double const x1,
                            double const y0, double const y1)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect const bbox(selection->bounds());
    if ( !bbox ) {
        return;
    }

    Geom::Translate const p2o(-bbox->min());

    Geom::Scale const newSize(x1 - x0,
                              y1 - y0);
    Geom::Scale const scale( newSize * Geom::Scale(bbox->dimensions()).inverse() );
    Geom::Translate const o2n(x0, y0);
    Geom::Matrix const final( p2o * scale * o2n );

    sp_selection_apply_affine(selection, final);
}


void sp_selection_scale_relative(Inkscape::Selection *selection, Geom::Point const &align, Geom::Scale const &scale)
{
    if (selection->isEmpty())
        return;

    Geom::OptRect const bbox(selection->bounds());

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
    Geom::Matrix const final( n2d * scale * d2n );
    sp_selection_apply_affine(selection, final);
}

void
sp_selection_rotate_relative(Inkscape::Selection *selection, Geom::Point const &center, gdouble const angle_degrees)
{
    Geom::Translate const d2n(center);
    Geom::Translate const n2d(-center);
    Geom::Rotate const rotate(Geom::Rotate::from_degrees(angle_degrees));
    Geom::Matrix const final( Geom::Matrix(n2d) * rotate * d2n );
    sp_selection_apply_affine(selection, final);
}

void
sp_selection_skew_relative(Inkscape::Selection *selection, Geom::Point const &align, double dx, double dy)
{
    Geom::Translate const d2n(align);
    Geom::Translate const n2d(-align);
    Geom::Matrix const skew(1, dy,
                            dx, 1,
                            0, 0);
    Geom::Matrix const final( n2d * skew * d2n );
    sp_selection_apply_affine(selection, final);
}

void sp_selection_move_relative(Inkscape::Selection *selection, Geom::Point const &move, bool compensate)
{
    sp_selection_apply_affine(selection, Geom::Matrix(Geom::Translate(move)), true, compensate);
}

void sp_selection_move_relative(Inkscape::Selection *selection, double dx, double dy)
{
    sp_selection_apply_affine(selection, Geom::Matrix(Geom::Translate(dx, dy)));
}

/**
 * @brief Rotates selected objects 90 degrees, either clock-wise or counter-clockwise, depending on the value of ccw
 */
void sp_selection_rotate_90(SPDesktop *desktop, bool ccw)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty())
        return;

    GSList const *l = selection->itemList();
    Geom::Rotate const rot_90(Geom::Point(0, ccw ? 1 : -1)); // pos. or neg. rotation, depending on the value of ccw
    for (GSList const *l2 = l ; l2 != NULL ; l2 = l2->next) {
        SPItem *item = SP_ITEM(l2->data);
        sp_item_rotate_rel(item, rot_90);
    }

    sp_document_done(sp_desktop_document(desktop),
                     ccw ? SP_VERB_OBJECT_ROTATE_90_CCW : SP_VERB_OBJECT_ROTATE_90_CW,
                     ccw ? _("Rotate 90&#176; CCW") : _("Rotate 90&#176; CW"));
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

    sp_document_maybe_done(sp_desktop_document(selection->desktop()),
                           ( ( angle_degrees > 0 )
                             ? "selector:rotate:ccw"
                             : "selector:rotate:cw" ),
                           SP_VERB_CONTEXT_SELECT,
                           _("Rotate"));
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

    Geom::OptRect const bbox(selection->bounds());
    boost::optional<Geom::Point> center = selection->center();

    if ( !bbox || !center ) {
        return;
    }

    gdouble const zoom = selection->desktop()->current_zoom();
    gdouble const zmove = angle / zoom;
    gdouble const r = Geom::L2(cornerFarthestFrom(*bbox, *center) - *center);

    gdouble const zangle = 180 * atan2(zmove, r) / M_PI;

    sp_selection_rotate_relative(selection, *center, zangle);

    sp_document_maybe_done(sp_desktop_document(selection->desktop()),
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

    Geom::OptRect const bbox(selection->bounds());
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

    sp_document_maybe_done(sp_desktop_document(selection->desktop()),
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

    Geom::OptRect sel_bbox = selection->bounds();

    if (!sel_bbox) {
        return;
    }

    Geom::Point const center(sel_bbox->midpoint());
    sp_selection_scale_relative(selection, center, Geom::Scale(times, times));
    sp_document_done(sp_desktop_document(selection->desktop()), SP_VERB_CONTEXT_SELECT,
                     _("Scale by whole factor"));
}

void
sp_selection_move(SPDesktop *desktop, gdouble dx, gdouble dy)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    if (selection->isEmpty()) {
        return;
    }

    sp_selection_move_relative(selection, dx, dy);

    if (dx == 0) {
        sp_document_maybe_done(sp_desktop_document(desktop), "selector:move:vertical", SP_VERB_CONTEXT_SELECT,
                               _("Move vertically"));
    } else if (dy == 0) {
        sp_document_maybe_done(sp_desktop_document(desktop), "selector:move:horizontal", SP_VERB_CONTEXT_SELECT,
                               _("Move horizontally"));
    } else {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_SELECT,
                         _("Move"));
    }
}

void
sp_selection_move_screen(SPDesktop *desktop, gdouble dx, gdouble dy)
{
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    if (selection->isEmpty()) {
        return;
    }

    // same as sp_selection_move but divide deltas by zoom factor
    gdouble const zoom = desktop->current_zoom();
    gdouble const zdx = dx / zoom;
    gdouble const zdy = dy / zoom;
    sp_selection_move_relative(selection, zdx, zdy);

    if (dx == 0) {
        sp_document_maybe_done(sp_desktop_document(desktop), "selector:move:vertical", SP_VERB_CONTEXT_SELECT,
                               _("Move vertically by pixels"));
    } else if (dy == 0) {
        sp_document_maybe_done(sp_desktop_document(desktop), "selector:move:horizontal", SP_VERB_CONTEXT_SELECT,
                               _("Move horizontally by pixels"));
    } else {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_SELECT,
                         _("Move"));
    }
}

namespace {

template <typename D>
SPItem *next_item(SPDesktop *desktop, GSList *path, SPObject *root,
                  bool only_in_viewport, PrefsSelectionContext inlayer, bool onlyvisible, bool onlysensitive);

template <typename D>
SPItem *next_item_from_list(SPDesktop *desktop, GSList const *items, SPObject *root,
                  bool only_in_viewport, PrefsSelectionContext inlayer, bool onlyvisible, bool onlysensitive);

struct Forward {
    typedef SPObject *Iterator;

    static Iterator children(SPObject *o) { return sp_object_first_child(o); }
    static Iterator siblings_after(SPObject *o) { return SP_OBJECT_NEXT(o); }
    static void dispose(Iterator /*i*/) {}

    static SPObject *object(Iterator i) { return i; }
    static Iterator next(Iterator i) { return SP_OBJECT_NEXT(i); }
};

struct Reverse {
    typedef GSList *Iterator;

    static Iterator children(SPObject *o) {
        return make_list(o->firstChild(), NULL);
    }
    static Iterator siblings_after(SPObject *o) {
        return make_list(SP_OBJECT_PARENT(o)->firstChild(), o);
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
        GSList *list=NULL;
        while ( object != limit ) {
            list = g_slist_prepend(list, object);
            object = SP_OBJECT_NEXT(object);
        }
        return list;
    }
};

}

void
sp_selection_item_next(SPDesktop *desktop)
{
    g_return_if_fail(desktop != NULL);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

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
    SPDocument *document = sp_desktop_document(desktop);
    g_return_if_fail(document != NULL);
    g_return_if_fail(desktop != NULL);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

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

    SPItem *item=next_item_from_list<Reverse>(desktop, selection->itemList(), root, SP_CYCLING == SP_CYCLE_VISIBLE, inlayer, onlyvisible, onlysensitive);

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

    Inkscape::Selection *selection = sp_desktop_selection(dt);
    if ( selection && !selection->isEmpty() ) {
        SPItem *item = selection->singleItem();
        if ( item && SP_IS_SHAPE(item)) {
            if (sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {
                sp_lpe_item_edit_next_param_oncanvas(SP_LPE_ITEM(item), dt);
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

    Inkscape::Selection *selection = sp_desktop_selection(dt);
    if (!selection || selection->isEmpty()) return;

    GSList const *items = selection->itemList();
    bool has_path = false;
    for (GSList *i = const_cast<GSList*>(items); i; i= i->next) {
        SPItem *item = SP_ITEM(i->data);
        SPObject *search = clip
            ? SP_OBJECT(item->clip_ref ? item->clip_ref->getObject() : NULL)
            : SP_OBJECT(item->mask_ref ? item->mask_ref->getObject() : NULL);
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


namespace {

template <typename D>
SPItem *next_item_from_list(SPDesktop *desktop, GSList const *items,
                            SPObject *root, bool only_in_viewport, PrefsSelectionContext inlayer, bool onlyvisible, bool onlysensitive)
{
    SPObject *current=root;
    while (items) {
        SPItem *item=SP_ITEM(items->data);
        if ( root->isAncestorOf(item) &&
             ( !only_in_viewport || desktop->isWithinViewport(item) ) )
        {
            current = item;
            break;
        }
        items = items->next;
    }

    GSList *path=NULL;
    while ( current != root ) {
        path = g_slist_prepend(path, current);
        current = SP_OBJECT_PARENT(current);
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

template <typename D>
SPItem *next_item(SPDesktop *desktop, GSList *path, SPObject *root,
                  bool only_in_viewport, PrefsSelectionContext inlayer, bool onlyvisible, bool onlysensitive)
{
    typename D::Iterator children;
    typename D::Iterator iter;

    SPItem *found=NULL;

    if (path) {
        SPObject *object=reinterpret_cast<SPObject *>(path->data);
        g_assert(SP_OBJECT_PARENT(object) == root);
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
        } else if ( SP_IS_ITEM(object) &&
                    ( !only_in_viewport || desktop->isWithinViewport(SP_ITEM(object)) ) &&
                    ( !onlyvisible || !desktop->itemIsHidden(SP_ITEM(object))) &&
                    ( !onlysensitive || !SP_ITEM(object)->isLocked()) &&
                    !desktop->isLayer(SP_ITEM(object)) )
        {
            found = SP_ITEM(object);
        }
        iter = D::next(iter);
    }

    D::dispose(children);

    return found;
}

}

/**
 * If \a item is not entirely visible then adjust visible area to centre on the centre on of
 * \a item.
 */
void scroll_to_show_item(SPDesktop *desktop, SPItem *item)
{
    Geom::Rect dbox = desktop->get_display_area();
    Geom::OptRect sbox = sp_item_bbox_desktop(item);

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


void
sp_selection_clone(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object</b> to clone."));
        return;
    }

    GSList *reprs = g_slist_copy((GSList *) selection->reprList());

    selection->clear();

    // sorting items from different parents sorts each parent's subset without possibly mixing them, just what we need
    reprs = g_slist_sort(reprs, (GCompareFunc) sp_repr_compare_position);

    GSList *newsel = NULL;

    while (reprs) {
        Inkscape::XML::Node *sel_repr = (Inkscape::XML::Node *) reprs->data;
        Inkscape::XML::Node *parent = sp_repr_parent(sel_repr);

        Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");
        clone->setAttribute("x", "0", false);
        clone->setAttribute("y", "0", false);
        clone->setAttribute("xlink:href", g_strdup_printf("#%s", sel_repr->attribute("id")), false);

        clone->setAttribute("inkscape:transform-center-x", sel_repr->attribute("inkscape:transform-center-x"), false);
        clone->setAttribute("inkscape:transform-center-y", sel_repr->attribute("inkscape:transform-center-y"), false);

        // add the new clone to the top of the original's parent
        parent->appendChild(clone);

        newsel = g_slist_prepend(newsel, clone);
        reprs = g_slist_remove(reprs, sel_repr);
        Inkscape::GC::release(clone);
    }

    // TRANSLATORS: only translate "string" in "context|string".
    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
    sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_CLONE,
                     Q_("action|Clone"));

    selection->setReprList(newsel);

    g_slist_free(newsel);
}

void
sp_selection_relink(SPDesktop *desktop)
{
    if (!desktop)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

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
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next)
    {
        SPItem *item = (SPItem *) items->data;

        if (!SP_IS_USE(item))
            continue;

        SP_OBJECT_REPR(item)->setAttribute("xlink:href", newref);
        SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        relinked = true;
    }

    g_free(newref);

    if (!relinked) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No clones to relink</b> in the selection."));
    } else {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_UNLINK_CLONE,
                         _("Relink clone"));
    }
}


void
sp_selection_unlink(SPDesktop *desktop)
{
    if (!desktop)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>clones</b> to unlink."));
        return;
    }

    // Get a copy of current selection.
    GSList *new_select = NULL;
    bool unlinked = false;
    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next)
    {
        SPItem *item = (SPItem *) items->data;

        if (SP_IS_TEXT(item)) {
            SPObject *tspan = sp_tref_convert_to_tspan(SP_OBJECT(item));

            if (tspan) {
                SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }

            // Set unlink to true, and fall into the next if which
            // will include this text item in the new selection
            unlinked = true;
        }

        if (!(SP_IS_USE(item) || SP_IS_TREF(item))) {
            // keep the non-use item in the new selection
            new_select = g_slist_prepend(new_select, item);
            continue;
        }

        SPItem *unlink;
        if (SP_IS_USE(item)) {
            unlink = sp_use_unlink(SP_USE(item));
        } else /*if (SP_IS_TREF(use))*/ {
            unlink = SP_ITEM(sp_tref_convert_to_tspan(SP_OBJECT(item)));
        }

        unlinked = true;
        // Add ungrouped items to the new selection.
        new_select = g_slist_prepend(new_select, unlink);
    }

    if (new_select) { // set new selection
        selection->clear();
        selection->setList(new_select);
        g_slist_free(new_select);
    }
    if (!unlinked) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No clones to unlink</b> in the selection."));
    }

    sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_UNLINK_CLONE,
                     _("Unlink clone"));
}

void
sp_select_clone_original(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    SPItem *item = selection->singleItem();

    gchar const *error = _("Select a <b>clone</b> to go to its original. Select a <b>linked offset</b> to go to its source. Select a <b>text on path</b> to go to the path. Select a <b>flowed text</b> to go to its frame.");

    // Check if other than two objects are selected
    if (g_slist_length((GSList *) selection->itemList()) != 1 || !item) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, error);
        return;
    }

    SPItem *original = NULL;
    if (SP_IS_USE(item)) {
        original = sp_use_get_original(SP_USE(item));
    } else if (SP_IS_OFFSET(item) && SP_OFFSET(item)->sourceHref) {
        original = sp_offset_get_source(SP_OFFSET(item));
    } else if (SP_IS_TEXT_TEXTPATH(item)) {
        original = sp_textpath_get_path_item(SP_TEXTPATH(sp_object_first_child(SP_OBJECT(item))));
    } else if (SP_IS_FLOWTEXT(item)) {
        original = SP_FLOWTEXT(item)->get_frame(NULL); // first frame only
    } else { // it's an object that we don't know what to do with
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, error);
        return;
    }

    if (!original) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>Cannot find</b> the object to select (orphaned clone, offset, textpath, flowed text?)"));
        return;
    }

    for (SPObject *o = original; o && !SP_IS_ROOT(o); o = SP_OBJECT_PARENT(o)) {
        if (SP_IS_DEFS(o)) {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("The object you're trying to select is <b>not visible</b> (it is in &lt;defs&gt;)"));
            return;
        }
    }

    if (original) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool highlight = prefs->getBool("/options/highlightoriginal/value");
        if (highlight) {
            Geom::OptRect a = item->getBounds(sp_item_i2d_affine(item));
            Geom::OptRect b = original->getBounds(sp_item_i2d_affine(original));
            if ( a && b ) {
                // draw a flashing line between the objects
                SPCurve *curve = new SPCurve();
                curve->moveto(a->midpoint());
                curve->lineto(b->midpoint());

                SPCanvasItem * canvasitem = sp_canvas_bpath_new(sp_desktop_tempgroup(desktop), curve);
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


void sp_selection_to_marker(SPDesktop *desktop, bool apply)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to marker."));
        return;
    }

    sp_document_ensure_up_to_date(doc);
    Geom::OptRect r = selection->bounds();
    boost::optional<Geom::Point> c = selection->center();
    if ( !r || !c ) {
        return;
    }

    // calculate the transform to be applied to objects to move them to 0,0
    Geom::Point move_p = Geom::Point(0, sp_document_height(doc)) - *c;
    move_p[Geom::Y] = -move_p[Geom::Y];
    Geom::Matrix move = Geom::Matrix(Geom::Translate(move_p));

    GSList *items = g_slist_copy((GSList *) selection->itemList());

    items = g_slist_sort(items, (GCompareFunc) sp_object_compare_position);

    // bottommost object, after sorting
    SPObject *parent = SP_OBJECT_PARENT(items->data);

    Geom::Matrix parent_transform(sp_item_i2doc_affine(SP_ITEM(parent)));

    // remember the position of the first item
    gint pos = SP_OBJECT_REPR(items->data)->position();
    (void)pos; // TODO check why this was remembered

    // create a list of duplicates
    GSList *repr_copies = NULL;
    for (GSList *i = items; i != NULL; i = i->next) {
        Inkscape::XML::Node *dup = (SP_OBJECT_REPR(i->data))->duplicate(xml_doc);
        repr_copies = g_slist_prepend(repr_copies, dup);
    }

    Geom::Rect bounds(desktop->dt2doc(r->min()), desktop->dt2doc(r->max()));

    if (apply) {
        // delete objects so that their clones don't get alerted; this object will be restored shortly
        for (GSList *i = items; i != NULL; i = i->next) {
            SPObject *item = SP_OBJECT(i->data);
            item->deleteObject(false);
        }
    }

    // Hack: Temporarily set clone compensation to unmoved, so that we can move clone-originals
    // without disturbing clones.
    // See ActorAlign::on_button_click() in src/ui/dialog/align-and-distribute.cpp
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

    gchar const *mark_id = generate_marker(repr_copies, bounds, doc,
                                           ( Geom::Matrix(Geom::Translate(desktop->dt2doc(
                                                                              Geom::Point(r->min()[Geom::X],
                                                                                          r->max()[Geom::Y]))))
                                             * parent_transform.inverse() ),
                                           parent_transform * move);
    (void)mark_id;

    // restore compensation setting
    prefs->setInt("/options/clonecompensation/value", saved_compensation);


    g_slist_free(items);

    sp_document_done(doc, SP_VERB_EDIT_SELECTION_2_MARKER,
                     _("Objects to marker"));
}

static void sp_selection_to_guides_recursive(SPItem *item, bool deleteitem, bool wholegroups) {
    if (SP_IS_GROUP(item) && !SP_IS_BOX3D(item) && !wholegroups) {
        for (GSList *i = sp_item_group_item_list(SP_GROUP(item)); i != NULL; i = i->next) {
            sp_selection_to_guides_recursive(SP_ITEM(i->data), deleteitem, wholegroups);
        }
    } else {
        sp_item_convert_item_to_guides(item);

        if (deleteitem) {
            SP_OBJECT(item)->deleteObject(true);
        }
    }
}

void sp_selection_to_guides(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    // we need to copy the list because it gets reset when objects are deleted
    GSList *items = g_slist_copy((GSList *) selection->itemList());

    if (!items) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to guides."));
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool deleteitem = !prefs->getBool("/tools/cvg_keep_objects", 0);
    bool wholegroups = prefs->getBool("/tools/cvg_convert_whole_groups", 0);

    for (GSList const *i = items; i != NULL; i = i->next) {
        sp_selection_to_guides_recursive(SP_ITEM(i->data), deleteitem, wholegroups);
    }

    sp_document_done(doc, SP_VERB_EDIT_SELECTION_2_GUIDES, _("Objects to guides"));
}

void
sp_selection_tile(SPDesktop *desktop, bool apply)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to pattern."));
        return;
    }

    sp_document_ensure_up_to_date(doc);
    Geom::OptRect r = selection->bounds();
    if ( !r ) {
        return;
    }

    // calculate the transform to be applied to objects to move them to 0,0
    Geom::Point move_p = Geom::Point(0, sp_document_height(doc)) - (r->min() + Geom::Point(0, r->dimensions()[Geom::Y]));
    move_p[Geom::Y] = -move_p[Geom::Y];
    Geom::Matrix move = Geom::Matrix(Geom::Translate(move_p));

    GSList *items = g_slist_copy((GSList *) selection->itemList());

    items = g_slist_sort(items, (GCompareFunc) sp_object_compare_position);

    // bottommost object, after sorting
    SPObject *parent = SP_OBJECT_PARENT(items->data);

    Geom::Matrix parent_transform(sp_item_i2doc_affine(SP_ITEM(parent)));

    // remember the position of the first item
    gint pos = SP_OBJECT_REPR(items->data)->position();

    // create a list of duplicates
    GSList *repr_copies = NULL;
    for (GSList *i = items; i != NULL; i = i->next) {
        Inkscape::XML::Node *dup = (SP_OBJECT_REPR(i->data))->duplicate(xml_doc);
        repr_copies = g_slist_prepend(repr_copies, dup);
    }
    // restore the z-order after prepends
    repr_copies = g_slist_reverse(repr_copies);

    Geom::Rect bounds(desktop->dt2doc(r->min()), desktop->dt2doc(r->max()));

    if (apply) {
        // delete objects so that their clones don't get alerted; this object will be restored shortly
        for (GSList *i = items; i != NULL; i = i->next) {
            SPObject *item = SP_OBJECT(i->data);
            item->deleteObject(false);
        }
    }

    // Hack: Temporarily set clone compensation to unmoved, so that we can move clone-originals
    // without disturbing clones.
    // See ActorAlign::on_button_click() in src/ui/dialog/align-and-distribute.cpp
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
    prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

    gchar const *pat_id = pattern_tile(repr_copies, bounds, doc,
                                       ( Geom::Matrix(Geom::Translate(desktop->dt2doc(Geom::Point(r->min()[Geom::X],
                                                                                            r->max()[Geom::Y]))))
                                         * to_2geom(parent_transform.inverse()) ),
                                       parent_transform * move);

    // restore compensation setting
    prefs->setInt("/options/clonecompensation/value", saved_compensation);

    if (apply) {
        Inkscape::XML::Node *rect = xml_doc->createElement("svg:rect");
        rect->setAttribute("style", g_strdup_printf("stroke:none;fill:url(#%s)", pat_id));

        Geom::Point min = bounds.min() * to_2geom(parent_transform.inverse());
        Geom::Point max = bounds.max() * to_2geom(parent_transform.inverse());

        sp_repr_set_svg_double(rect, "width", max[Geom::X] - min[Geom::X]);
        sp_repr_set_svg_double(rect, "height", max[Geom::Y] - min[Geom::Y]);
        sp_repr_set_svg_double(rect, "x", min[Geom::X]);
        sp_repr_set_svg_double(rect, "y", min[Geom::Y]);

        // restore parent and position
        SP_OBJECT_REPR(parent)->appendChild(rect);
        rect->setPosition(pos > 0 ? pos : 0);
        SPItem *rectangle = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(rect);

        Inkscape::GC::release(rect);

        selection->clear();
        selection->set(rectangle);
    }

    g_slist_free(items);

    sp_document_done(doc, SP_VERB_EDIT_TILE,
                     _("Objects to pattern"));
}

void
sp_selection_untile(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object with pattern fill</b> to extract objects from."));
        return;
    }

    GSList *new_select = NULL;

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        SPStyle *style = SP_OBJECT_STYLE(item);

        if (!style || !style->fill.isPaintserver())
            continue;

        SPObject *server = SP_OBJECT_STYLE_FILL_SERVER(item);

        if (!SP_IS_PATTERN(server))
            continue;

        did = true;

        SPPattern *pattern = pattern_getroot(SP_PATTERN(server));

        Geom::Matrix pat_transform = to_2geom(pattern_patternTransform(SP_PATTERN(server)));
        pat_transform *= item->transform;

        for (SPObject *child = sp_object_first_child(SP_OBJECT(pattern)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            Inkscape::XML::Node *copy = SP_OBJECT_REPR(child)->duplicate(xml_doc);
            SPItem *i = SP_ITEM(desktop->currentLayer()->appendChildRepr(copy));

           // FIXME: relink clones to the new canvas objects
           // use SPObject::setid when mental finishes it to steal ids of

            // this is needed to make sure the new item has curve (simply requestDisplayUpdate does not work)
            sp_document_ensure_up_to_date(doc);

            Geom::Matrix transform( i->transform * pat_transform );
            sp_item_write_transform(i, SP_OBJECT_REPR(i), transform);

            new_select = g_slist_prepend(new_select, i);
        }

        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property(css, "fill", "none");
        sp_repr_css_change(SP_OBJECT_REPR(item), css, "style");
    }

    if (!did) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No pattern fills</b> in the selection."));
    } else {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_EDIT_UNTILE,
                         _("Pattern to objects"));
        selection->setList(new_select);
    }
}

void
sp_selection_get_export_hints(Inkscape::Selection *selection, char const **filename, float *xdpi, float *ydpi)
{
    if (selection->isEmpty()) {
        return;
    }

    GSList const *reprlst = selection->reprList();
    bool filename_search = TRUE;
    bool xdpi_search = TRUE;
    bool ydpi_search = TRUE;

    for (; reprlst != NULL &&
            filename_search &&
            xdpi_search &&
            ydpi_search;
        reprlst = reprlst->next) {
        gchar const *dpi_string;
        Inkscape::XML::Node * repr = (Inkscape::XML::Node *)reprlst->data;

        if (filename_search) {
            *filename = repr->attribute("inkscape:export-filename");
            if (*filename != NULL)
                filename_search = FALSE;
        }

        if (xdpi_search) {
            dpi_string = NULL;
            dpi_string = repr->attribute("inkscape:export-xdpi");
            if (dpi_string != NULL) {
                *xdpi = atof(dpi_string);
                xdpi_search = FALSE;
            }
        }

        if (ydpi_search) {
            dpi_string = NULL;
            dpi_string = repr->attribute("inkscape:export-ydpi");
            if (dpi_string != NULL) {
                *ydpi = atof(dpi_string);
                ydpi_search = FALSE;
            }
        }
    }
}

void
sp_document_get_export_hints(SPDocument *doc, char const **filename, float *xdpi, float *ydpi)
{
    Inkscape::XML::Node * repr = sp_document_repr_root(doc);
    gchar const *dpi_string;

    *filename = repr->attribute("inkscape:export-filename");

    dpi_string = NULL;
    dpi_string = repr->attribute("inkscape:export-xdpi");
    if (dpi_string != NULL) {
        *xdpi = atof(dpi_string);
    }

    dpi_string = NULL;
    dpi_string = repr->attribute("inkscape:export-ydpi");
    if (dpi_string != NULL) {
        *ydpi = atof(dpi_string);
    }
}

void
sp_selection_create_bitmap_copy(SPDesktop *desktop)
{
    if (desktop == NULL)
        return;

    SPDocument *document = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to make a bitmap copy."));
        return;
    }

    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Rendering bitmap..."));
    // set "busy" cursor
    desktop->setWaitingCursor();

    // Get the bounding box of the selection
    NRRect bbox;
    sp_document_ensure_up_to_date(document);
    selection->bounds(&bbox);
    if (NR_RECT_DFLS_TEST_EMPTY(&bbox)) {
        desktop->clearWaitingCursor();
        return; // exceptional situation, so not bother with a translatable error message, just quit quietly
    }

    // List of the items to show; all others will be hidden
    GSList *items = g_slist_copy((GSList *) selection->itemList());

    // Sort items so that the topmost comes last
    items = g_slist_sort(items, (GCompareFunc) sp_item_repr_compare_position);

    // Generate a random value from the current time (you may create bitmap from the same object(s)
    // multiple times, and this is done so that they don't clash)
    GTimeVal cu;
    g_get_current_time(&cu);
    guint current = (int) (cu.tv_sec * 1000000 + cu.tv_usec) % 1024;

    // Create the filename.
    gchar *const basename = g_strdup_printf("%s-%s-%u.png",
                                            document->name,
                                            SP_OBJECT_REPR(items->data)->attribute("id"),
                                            current);
    // Imagemagick is known not to handle spaces in filenames, so we replace anything but letters,
    // digits, and a few other chars, with "_"
    g_strcanon(basename, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.=+~$#@^&!?", '_');

    // Build the complete path by adding document base dir, if set, otherwise home dir
    gchar * directory = NULL;
    if (SP_DOCUMENT_URI(document)) {
        directory = g_dirname(SP_DOCUMENT_URI(document));
    }
    if (directory == NULL) {
        directory = homedir_path(NULL);
    }
    gchar *filepath = g_build_filename(directory, basename, NULL);

    //g_print("%s\n", filepath);

    // Remember parent and z-order of the topmost one
    gint pos = SP_OBJECT_REPR(g_slist_last(items)->data)->position();
    SPObject *parent_object = SP_OBJECT_PARENT(g_slist_last(items)->data);
    Inkscape::XML::Node *parent = SP_OBJECT_REPR(parent_object);

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
        res = PX_PER_IN * prefs_min / MIN((bbox.x1 - bbox.x0), (bbox.y1 - bbox.y0));
    } else {
        float hint_xdpi = 0, hint_ydpi = 0;
        char const *hint_filename;
        // take resolution hint from the selected objects
        sp_selection_get_export_hints(selection, &hint_filename, &hint_xdpi, &hint_ydpi);
        if (hint_xdpi != 0) {
            res = hint_xdpi;
        } else {
            // take resolution hint from the document
            sp_document_get_export_hints(document, &hint_filename, &hint_xdpi, &hint_ydpi);
            if (hint_xdpi != 0) {
                res = hint_xdpi;
            } else {
                // if all else fails, take the default 90 dpi
                res = PX_PER_IN;
            }
        }
    }

    // The width and height of the bitmap in pixels
    unsigned width = (unsigned) floor((bbox.x1 - bbox.x0) * res / PX_PER_IN);
    unsigned height =(unsigned) floor((bbox.y1 - bbox.y0) * res / PX_PER_IN);

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
    Geom::Matrix eek(sp_item_i2d_affine(SP_ITEM(parent_object)));
    Geom::Matrix t;

    double shift_x = bbox.x0;
    double shift_y = bbox.y1;
    if (res == PX_PER_IN) { // for default 90 dpi, snap it to pixel grid
        shift_x = round(shift_x);
        shift_y = -round(-shift_y); // this gets correct rounding despite coordinate inversion, remove the negations when the inversion is gone
    }
    t = Geom::Scale(1, -1) * Geom::Translate(shift_x, shift_y) * eek.inverse();

    // Do the export
    sp_export_png_file(document, filepath,
                       bbox.x0, bbox.y0, bbox.x1, bbox.y1,
                       width, height, res, res,
                       (guint32) 0xffffff00,
                       NULL, NULL,
                       true,  /*bool force_overwrite,*/
                       items);

    g_slist_free(items);

    // Run filter, if any
    if (run) {
        g_print("Running external filter: %s\n", run);
        int retval;
        retval = system(run);
    }

    // Import the image back
    GdkPixbuf *pb = gdk_pixbuf_new_from_file(filepath, NULL);
    if (pb) {
        // Create the repr for the image
        Inkscape::XML::Node * repr = xml_doc->createElement("svg:image");
        sp_embed_image(repr, pb, "image/png");
        if (res == PX_PER_IN) { // for default 90 dpi, snap it to pixel grid
            sp_repr_set_svg_double(repr, "width", width);
            sp_repr_set_svg_double(repr, "height", height);
        } else {
            sp_repr_set_svg_double(repr, "width", (bbox.x1 - bbox.x0));
            sp_repr_set_svg_double(repr, "height", (bbox.y1 - bbox.y0));
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
        gdk_pixbuf_unref(pb);

        // Complete undoable transaction
        sp_document_done(document, SP_VERB_SELECTION_CREATE_BITMAP,
                         _("Create bitmap"));
    }

    desktop->clearWaitingCursor();

    g_free(basename);
    g_free(filepath);
}

/**
 * \brief Creates a mask or clipPath from selection
 * Two different modes:
 *  if applyToLayer, all selection is moved to DEFS as mask/clippath
 *       and is applied to current layer
 *  otherwise, topmost object is used as mask for other objects
 * If \a apply_clip_path parameter is true, clipPath is created, otherwise mask
 *
 */
void
sp_selection_set_mask(SPDesktop *desktop, bool apply_clip_path, bool apply_to_layer)
{
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    bool is_empty = selection->isEmpty();
    if ( apply_to_layer && is_empty) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to create clippath or mask from."));
        return;
    } else if (!apply_to_layer && ( is_empty || NULL == selection->itemList()->next )) {
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

    sp_document_ensure_up_to_date(doc);

    GSList *items = g_slist_copy((GSList *) selection->itemList());

    items = g_slist_sort(items, (GCompareFunc) sp_object_compare_position);
    
    // See lp bug #542004
    selection->clear();

    // create a list of duplicates
    GSList *mask_items = NULL;
    GSList *apply_to_items = NULL;
    GSList *items_to_delete = NULL;
    GSList *items_to_select = NULL;
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool topmost = prefs->getBool("/options/maskobject/topmost", true);
    bool remove_original = prefs->getBool("/options/maskobject/remove", true);
    int grouping = prefs->getInt("/options/maskobject/grouping", PREFS_MASKOBJECT_GROUPING_NONE);

    if (apply_to_layer) {
        // all selected items are used for mask, which is applied to a layer
        apply_to_items = g_slist_prepend(apply_to_items, desktop->currentLayer());
        
        for (GSList *i = items; i != NULL; i = i->next) {
            Inkscape::XML::Node *dup = (SP_OBJECT_REPR(i->data))->duplicate(xml_doc);
            mask_items = g_slist_prepend(mask_items, dup);

            SPObject *item = SP_OBJECT(i->data);
            if (remove_original) {                
                items_to_delete = g_slist_prepend(items_to_delete, item);
            }
            else {
                items_to_select = g_slist_prepend(items_to_select, item);
            }
        }
    } else if (!topmost) {
        // topmost item is used as a mask, which is applied to other items in a selection
        GSList *i = items;
        Inkscape::XML::Node *dup = (SP_OBJECT_REPR(i->data))->duplicate(xml_doc);
        mask_items = g_slist_prepend(mask_items, dup);

        if (remove_original) {
            SPObject *item = SP_OBJECT(i->data);
            items_to_delete = g_slist_prepend(items_to_delete, item);
        }

        for (i = i->next; i != NULL; i = i->next) {
            apply_to_items = g_slist_prepend(apply_to_items, i->data);
            items_to_select = g_slist_prepend(items_to_select, i->data);
        }
    } else {
        GSList *i = NULL;
        for (i = items; NULL != i->next; i = i->next) {
            apply_to_items = g_slist_prepend(apply_to_items, i->data);
            items_to_select = g_slist_prepend(items_to_select, i->data);
        }

        Inkscape::XML::Node *dup = (SP_OBJECT_REPR(i->data))->duplicate(xml_doc);
        mask_items = g_slist_prepend(mask_items, dup);

        if (remove_original) {
            SPObject *item = SP_OBJECT(i->data);
            items_to_delete = g_slist_prepend(items_to_delete, item);
        }
    }

    g_slist_free(items);
    items = NULL;

    if (apply_to_items && grouping == PREFS_MASKOBJECT_GROUPING_ALL) {
        // group all those objects into one group
        // and apply mask to that
        Inkscape::XML::Node *group = xml_doc->createElement("svg:g");

        // make a note we should ungroup this when unsetting mask
        group->setAttribute("inkscape:groupmode", "maskhelper");

        GSList *reprs_to_group = NULL;

        for (GSList *i = apply_to_items ; NULL != i ; i = i->next) {
                reprs_to_group = g_slist_prepend(reprs_to_group, SP_OBJECT_REPR(i->data));
                items_to_select = g_slist_remove(items_to_select, i->data);
        }
        reprs_to_group = g_slist_reverse(reprs_to_group);

        sp_selection_group_impl(reprs_to_group, group, xml_doc, doc);
        
        reprs_to_group = NULL;

        // apply clip/mask only to newly created group
        g_slist_free(apply_to_items);
        apply_to_items = NULL;
        apply_to_items = g_slist_prepend(apply_to_items, doc->getObjectByRepr(group));

        items_to_select = g_slist_prepend(items_to_select, doc->getObjectByRepr(group));

        Inkscape::GC::release(group);
    }

    gchar const *attributeName = apply_clip_path ? "clip-path" : "mask";
    for (GSList *i = apply_to_items; NULL != i; i = i->next) {
        SPItem *item = reinterpret_cast<SPItem *>(i->data);
        // inverted object transform should be applied to a mask object,
        // as mask is calculated in user space (after applying transform)
        Geom::Matrix maskTransform(item->transform.inverse());

        GSList *mask_items_dup = NULL;
        for (GSList *mask_item = mask_items; NULL != mask_item; mask_item = mask_item->next) {
            Inkscape::XML::Node *dup = reinterpret_cast<Inkscape::XML::Node *>(mask_item->data)->duplicate(xml_doc);
            mask_items_dup = g_slist_prepend(mask_items_dup, dup);
        }

        gchar const *mask_id = NULL;
        if (apply_clip_path) {
            mask_id = sp_clippath_create(mask_items_dup, doc, &maskTransform);
        } else {
            mask_id = sp_mask_create(mask_items_dup, doc, &maskTransform);
        }

        g_slist_free(mask_items_dup);
        mask_items_dup = NULL;

        Inkscape::XML::Node *current = SP_OBJECT_REPR(i->data);
        // Node to apply mask to
        Inkscape::XML::Node *apply_mask_to = current;

        if (grouping == PREFS_MASKOBJECT_GROUPING_SEPARATE) {
            // enclose current node in group, and apply crop/mask on that
            Inkscape::XML::Node *group = xml_doc->createElement("svg:g");
            // make a note we should ungroup this when unsetting mask
            group->setAttribute("inkscape:groupmode", "maskhelper");

            Inkscape::XML::Node *spnew = current->duplicate(xml_doc);
            gint position = current->position();
            items_to_select = g_slist_remove(items_to_select, item);
            current->parent()->appendChild(group);
            sp_repr_unparent(current);
            group->appendChild(spnew);
            group->setPosition(position);

            // Apply clip/mask to group instead
            apply_mask_to = group;

            items_to_select = g_slist_prepend(items_to_select, doc->getObjectByRepr(group));
            Inkscape::GC::release(spnew);
            Inkscape::GC::release(group);
        }

        apply_mask_to->setAttribute(attributeName, g_strdup_printf("url(#%s)", mask_id));

    }

    g_slist_free(mask_items);
    g_slist_free(apply_to_items);

    for (GSList *i = items_to_delete; NULL != i; i = i->next) {
        SPObject *item = SP_OBJECT(i->data);
        item->deleteObject(false);
        items_to_select = g_slist_remove(items_to_select, item);
    }
    g_slist_free(items_to_delete);
    
    items_to_select = g_slist_reverse(items_to_select);
    
    selection->addList(items_to_select);
    g_slist_free(items_to_select);

    if (apply_clip_path)
        sp_document_done(doc, SP_VERB_OBJECT_SET_CLIPPATH, _("Set clipping path"));
    else
        sp_document_done(doc, SP_VERB_OBJECT_SET_MASK, _("Set mask"));
}

void sp_selection_unset_mask(SPDesktop *desktop, bool apply_clip_path) {
    if (desktop == NULL)
        return;

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to remove clippath or mask from."));
        return;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool remove_original = prefs->getBool("/options/maskobject/remove", true);
    bool ungroup_masked = prefs->getBool("/options/maskobject/ungrouping", true);
    sp_document_ensure_up_to_date(doc);

    gchar const *attributeName = apply_clip_path ? "clip-path" : "mask";
    std::map<SPObject*,SPItem*> referenced_objects;

    GSList *items = g_slist_copy((GSList *) selection->itemList());
    selection->clear();
    
    GSList *items_to_ungroup = NULL;
    GSList *items_to_select = g_slist_copy(items);
    items_to_select = g_slist_reverse(items_to_select);    
        

    // SPObject* refers to a group containing the clipped path or mask itself,
    // whereas SPItem* refers to the item being clipped or masked
    for (GSList const *i = items; NULL != i; i = i->next) {
        if (remove_original) {
            // remember referenced mask/clippath, so orphaned masks can be moved back to document
            SPItem *item = reinterpret_cast<SPItem *>(i->data);
            Inkscape::URIReference *uri_ref = NULL;

            if (apply_clip_path) {
                uri_ref = item->clip_ref;
            } else {
                uri_ref = item->mask_ref;
            }

            // collect distinct mask object (and associate with item to apply transform)
            if (NULL != uri_ref && NULL != uri_ref->getObject()) {
                referenced_objects[uri_ref->getObject()] = item;
            }
        }

        SP_OBJECT_REPR(i->data)->setAttribute(attributeName, "none");

        if (ungroup_masked && SP_IS_GROUP(i->data)) {
                // if we had previously enclosed masked object in group,
                // add it to list so we can ungroup it later
                SPGroup *item = SP_GROUP(i->data);

                // ungroup only groups we created when setting clip/mask
                if (item->layerMode() == SPGroup::MASK_HELPER) {
                    items_to_ungroup = g_slist_prepend(items_to_ungroup, item);
                }

        }
    }
    g_slist_free(items);

    // restore mask objects into a document
    for ( std::map<SPObject*,SPItem*>::iterator it = referenced_objects.begin() ; it != referenced_objects.end() ; ++it) {
        SPObject *obj = (*it).first; // Group containing the clipped paths or masks
        GSList *items_to_move = NULL;
        for (SPObject *child = sp_object_first_child(obj) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
            // Collect all clipped paths and masks within a single group
            Inkscape::XML::Node *copy = SP_OBJECT_REPR(child)->duplicate(xml_doc);
            items_to_move = g_slist_prepend(items_to_move, copy);
        }

        if (!obj->isReferenced()) {
            // delete from defs if no other object references this mask
            obj->deleteObject(false);
        }

        // remember parent and position of the item to which the clippath/mask was applied
        Inkscape::XML::Node *parent = SP_OBJECT_REPR((*it).second)->parent();
        gint pos = SP_OBJECT_REPR((*it).second)->position();

        // Iterate through all clipped paths / masks
        for (GSList *i = items_to_move; NULL != i; i = i->next) {
            Inkscape::XML::Node *repr = (Inkscape::XML::Node *)i->data;

            // insert into parent, restore pos
            parent->appendChild(repr);
            repr->setPosition((pos + 1) > 0 ? (pos + 1) : 0);

            SPItem *mask_item = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);
            items_to_select = g_slist_prepend(items_to_select, mask_item);

            // transform mask, so it is moved the same spot where mask was applied
            Geom::Matrix transform(mask_item->transform);
            transform *= (*it).second->transform;
            sp_item_write_transform(mask_item, SP_OBJECT_REPR(mask_item), transform);
        }

        g_slist_free(items_to_move);
    }

    // ungroup marked groups added when setting mask
    for (GSList *i = items_to_ungroup ; NULL != i ; i = i->next) {
        items_to_select = g_slist_remove(items_to_select, SP_GROUP(i->data));
        GSList *children = NULL;
        sp_item_group_ungroup(SP_GROUP(i->data), &children, false);
        items_to_select = g_slist_concat(children, items_to_select);
    }

    g_slist_free(items_to_ungroup);
    
    // rebuild selection
    items_to_select = g_slist_reverse(items_to_select);
    selection->addList(items_to_select);
    g_slist_free(items_to_select);

    if (apply_clip_path)
        sp_document_done(doc, SP_VERB_OBJECT_UNSET_CLIPPATH, _("Release clipping path"));
    else
        sp_document_done(doc, SP_VERB_OBJECT_UNSET_MASK, _("Release mask"));
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
    SPDocument *doc = sp_desktop_document(desktop);

    g_return_val_if_fail(doc != NULL, false);
    g_return_val_if_fail(desktop->selection != NULL, false);

    if (desktop->selection->isEmpty()) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to fit canvas to."));
        return false;
    }
    Geom::OptRect const bbox(desktop->selection->bounds());
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
        sp_document_done(sp_desktop_document(desktop), SP_VERB_FIT_CANVAS_TO_SELECTION,
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

    sp_document_ensure_up_to_date(doc);
    SPItem const *const root = SP_ITEM(doc->root);
    Geom::OptRect const bbox(root->getBounds(sp_item_i2d_affine(root)));
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
    if (fit_canvas_to_drawing(sp_desktop_document(desktop))) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_FIT_CANVAS_TO_DRAWING,
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
    SPDocument *doc = sp_desktop_document(desktop);

    g_return_if_fail(doc != NULL);
    g_return_if_fail(desktop->selection != NULL);

    bool const changed = ( desktop->selection->isEmpty()
                           ? fit_canvas_to_drawing(doc, true)
                           : fit_canvas_to_selection(desktop, true) );
    if (changed) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING,
                         _("Fit Page to Selection or Drawing"));
    }
};

static void itemtree_map(void (*f)(SPItem *, SPDesktop *), SPObject *root, SPDesktop *desktop) {
    // don't operate on layers
    if (SP_IS_ITEM(root) && !desktop->isLayer(SP_ITEM(root))) {
        f(SP_ITEM(root), desktop);
    }
    for ( SPObject::SiblingIterator iter = root->firstChild() ; iter ; ++iter ) {
        //don't recurse into locked layers
        if (!(SP_IS_ITEM(&*iter) && desktop->isLayer(SP_ITEM(&*iter)) && SP_ITEM(&*iter)->isLocked())) {
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
