/*
 * Here are handlers for modifying selections, specific to paths
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2008 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <cstring>
#include <string>
#include "xml/repr.h"
#include "svg/svg.h"
#include "display/curve.h"
#include "color.h"
#include <glib.h>
#include <glibmm/i18n.h>
#include "sp-path.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
#include "style.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "message-stack.h"
#include "selection.h"

#include "box3d.h"
#include <2geom/pathvector.h>
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "verbs.h"

using Inkscape::DocumentUndo;


inline bool less_than_items(SPItem const *first, SPItem const *second)
{
    return sp_repr_compare_position(first->getRepr(),
                                    second->getRepr())<0;
}

void
sp_selected_path_combine(SPDesktop *desktop)
{
    Inkscape::Selection *selection = desktop->getSelection();
    SPDocument *doc = desktop->getDocument();

    std::vector<SPItem*> items(selection->itemList());
    
    if (items.size() < 1) {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to combine."));
        return;
    }

    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Combining paths..."));
    // set "busy" cursor
    desktop->setWaitingCursor();

    items = sp_degroup_list (items); // descend into any groups in selection

    std::vector<SPItem*> to_paths;
    for (std::vector<SPItem*>::const_reverse_iterator i = items.rbegin(); i != items.rend(); i++) {
        if (!dynamic_cast<SPPath *>(*i) && !dynamic_cast<SPGroup *>(*i)) {
            to_paths.push_back(*i);
        }
    }
    std::vector<Inkscape::XML::Node*> converted;
    bool did = sp_item_list_to_curves(to_paths, items, converted);
    for (std::vector<Inkscape::XML::Node*>::const_iterator i = converted.begin(); i != converted.end(); i++)
        items.push_back((SPItem*)doc->getObjectByRepr(*i));

    items = sp_degroup_list (items); // converting to path may have added more groups, descend again

    sort(items.begin(),items.end(),less_than_items);
    assert(!items.empty()); // cannot be NULL because of list length check at top of function

    // remember the position, id, transform and style of the topmost path, they will be assigned to the combined one
    gint position = 0;
    char const *id = NULL;
    char const *transform = NULL;
    char const *style = NULL;
    char const *path_effect = NULL;

    SPCurve* curve = NULL;
    SPItem *first = NULL;
    Inkscape::XML::Node *parent = NULL; 

    if (did) {
        selection->clear();
    }

    for (std::vector<SPItem*>::const_reverse_iterator i = items.rbegin(); i != items.rend(); i++){

        SPItem *item = *i;
        SPPath *path = dynamic_cast<SPPath *>(item);
        if (!path) {
            continue;
        }

        if (!did) {
            selection->clear();
            did = true;
        }

        SPCurve *c = path->get_curve_for_edit();
        if (first == NULL) {  // this is the topmost path
            first = item;
            parent = first->getRepr()->parent();
            position = first->getRepr()->position();
            id = first->getRepr()->attribute("id");
            transform = first->getRepr()->attribute("transform");
            // FIXME: merge styles of combined objects instead of using the first one's style
            style = first->getRepr()->attribute("style");
            path_effect = first->getRepr()->attribute("inkscape:path-effect");
            //c->transform(item->transform);
            curve = c;
        } else {
            c->transform(item->getRelativeTransform(first));
            curve->append(c, false);
            c->unref();

            // reduce position only if the same parent
            if (item->getRepr()->parent() == parent) {
                position--;
            }
            // delete the object for real, so that its clones can take appropriate action
            item->deleteObject();
        }
    }


    if (did) {
        first->deleteObject(false);
        // delete the topmost.

        Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

        // restore id, transform, path effect, and style
        repr->setAttribute("id", id);
        if (transform) {
            repr->setAttribute("transform", transform);
        }
        repr->setAttribute("style", style);

        repr->setAttribute("inkscape:path-effect", path_effect);

        // set path data corresponding to new curve
        gchar *dstring = sp_svg_write_path(curve->get_pathvector());
        curve->unref();
        if (path_effect) {
            repr->setAttribute("inkscape:original-d", dstring);
        } else {
            repr->setAttribute("d", dstring);
        }
        g_free(dstring);

        // add the new group to the parent of the topmost
        parent->appendChild(repr);

        // move to the position of the topmost, reduced by the number of deleted items
        repr->setPosition(position > 0 ? position : 0);

        DocumentUndo::done(desktop->getDocument(), SP_VERB_SELECTION_COMBINE, 
                           _("Combine"));

        selection->set(repr);

        Inkscape::GC::release(repr);

    } else {
        desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No path(s)</b> to combine in the selection."));
    }

    desktop->clearWaitingCursor();
}

void
sp_selected_path_break_apart(SPDesktop *desktop)
{
    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty()) {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>path(s)</b> to break apart."));
        return;
    }

    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Breaking apart paths..."));
    // set "busy" cursor
    desktop->setWaitingCursor();

    bool did = false;

    std::vector<SPItem*> itemlist(selection->itemList());
    for (std::vector<SPItem*>::const_iterator i = itemlist.begin(); i != itemlist.end(); i++){

        SPItem *item = *i;

        SPPath *path = dynamic_cast<SPPath *>(item);
        if (!path) {
            continue;
        }

        SPCurve *curve = path->get_curve_for_edit();
        if (curve == NULL) {
            continue;
        }

        did = true;

        Inkscape::XML::Node *parent = item->getRepr()->parent();
        gint pos = item->getRepr()->position();
        char const *id = item->getRepr()->attribute("id");

        // XML Tree being used directly here while it shouldn't be...
        gchar *style = g_strdup(item->getRepr()->attribute("style"));
        // XML Tree being used directly here while it shouldn't be...
        gchar *path_effect = g_strdup(item->getRepr()->attribute("inkscape:path-effect"));

        Geom::PathVector apv = curve->get_pathvector() * path->transform;

        curve->unref();

        // it's going to resurrect as one of the pieces, so we delete without advertisement
        item->deleteObject(false);

        curve = new SPCurve(apv);
        g_assert(curve != NULL);

        GSList *list = curve->split();

        curve->unref();

        std::vector<Inkscape::XML::Node*> reprs;
        for (GSList *l = list; l != NULL; l = l->next) {
            curve = (SPCurve *) l->data;

            Inkscape::XML::Node *repr = parent->document()->createElement("svg:path");
            repr->setAttribute("style", style);

            repr->setAttribute("inkscape:path-effect", path_effect);

            gchar *str = sp_svg_write_path(curve->get_pathvector());
            if (path_effect)
                repr->setAttribute("inkscape:original-d", str);
            else
                repr->setAttribute("d", str);
            g_free(str);

            // add the new repr to the parent
            parent->appendChild(repr);

            // move to the saved position
            repr->setPosition(pos > 0 ? pos : 0);

            // if it's the first one, restore id
            if (l == list)
                repr->setAttribute("id", id);

            reprs.push_back(repr);

            Inkscape::GC::release(repr);
        }
        selection->setReprList(reprs);

        g_slist_free(list);
        g_free(style);
        g_free(path_effect);
    }

    desktop->clearWaitingCursor();

    if (did) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_SELECTION_BREAK_APART, 
                           _("Break apart"));
    } else {
        desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No path(s)</b> to break apart in the selection."));
    }
}

/* This function is an entry point from GUI */
void
sp_selected_path_to_curves(Inkscape::Selection *selection, SPDesktop *desktop, bool interactive)
{
    if (selection->isEmpty()) {
        if (interactive && desktop)
            desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to path."));
        return;
    }

    bool did = false;
    if (interactive && desktop) {
        desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Converting objects to paths..."));
        // set "busy" cursor
        desktop->setWaitingCursor();
    }

    std::vector<SPItem*> selected(selection->itemList());
    std::vector<Inkscape::XML::Node*> to_select;
    selection->clear();
    std::vector<SPItem*> items(selected);

    did = sp_item_list_to_curves(items, selected, to_select);

    selection->setReprList(to_select);
    selection->addList(selected);

    if (interactive && desktop) {
        desktop->clearWaitingCursor();
        if (did) {
            DocumentUndo::done(desktop->getDocument(), SP_VERB_OBJECT_TO_CURVE, 
                               _("Object to path"));
        } else {
            desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No objects</b> to convert to path in the selection."));
            return;
        }
    }
}

/** Converts the selected items to LPEItems if they are not already so; e.g. SPRects) */
void sp_selected_to_lpeitems(SPDesktop *desktop)
{
    Inkscape::Selection *selection = desktop->getSelection();

    if (selection->isEmpty()) {
        return;
    }

    std::vector<SPItem*> selected(selection->itemList());
    std::vector<Inkscape::XML::Node*> to_select;
    selection->clear();
    std::vector<SPItem*> items(selected);


    sp_item_list_to_curves(items, selected, to_select, true);

    selection->setReprList(to_select);
    selection->addList(selected);
}

bool
sp_item_list_to_curves(const std::vector<SPItem*> &items, std::vector<SPItem*>& selected, std::vector<Inkscape::XML::Node*> &to_select, bool skip_all_lpeitems)
{
    bool did = false;
    for (std::vector<SPItem*>::const_iterator i = items.begin(); i != items.end(); i++){
        SPItem *item = *i;
        g_assert(item != NULL);
        SPDocument *document = item->document;

        SPGroup *group = dynamic_cast<SPGroup *>(item);
        if ( skip_all_lpeitems &&
             dynamic_cast<SPLPEItem *>(item) && 
             !group ) // also convert objects in an SPGroup when skip_all_lpeitems is set.
        { 
            continue;
        }

        SPPath *path = dynamic_cast<SPPath *>(item);
        if (path && !path->_curve_before_lpe) {
            // remove connector attributes
            if (item->getAttribute("inkscape:connector-type") != NULL) {
                item->removeAttribute("inkscape:connection-start");
                item->removeAttribute("inkscape:connection-end");
                item->removeAttribute("inkscape:connector-type");
                item->removeAttribute("inkscape:connector-curvature");
                did = true;
            }
            continue; // already a path, and no path effect
        }

        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box) {
            // convert 3D box to ordinary group of paths; replace the old element in 'selected' with the new group
            Inkscape::XML::Node *repr = box3d_convert_to_group(box)->getRepr();
            
            if (repr) {
                to_select.insert(to_select.begin(),repr);
                did = true;
                std::vector<SPItem*>::iterator element=find(selected.begin(),selected.end(),item);
                if(element != selected.end())
                    selected.erase(find(selected.begin(),selected.end(),item));
            }

            continue;
        }
        
        if (group) {
            group->removeAllPathEffects(true);
            std::vector<SPItem*> item_list = sp_item_group_item_list(group);
            
            std::vector<Inkscape::XML::Node*> item_to_select;
            std::vector<SPItem*> item_selected;
            
            if (sp_item_list_to_curves(item_list, item_selected, item_to_select))
                did = true;


            continue;
        }

        Inkscape::XML::Node *repr = sp_selected_item_to_curved_repr(item, 0);
        if (!repr)
            continue;

        did = true;
        std::vector<SPItem*>::iterator element=find(selected.begin(),selected.end(),item);
        if(element != selected.end())
            selected.erase(element);

        // remember the position of the item
        gint pos = item->getRepr()->position();
        // remember parent
        Inkscape::XML::Node *parent = item->getRepr()->parent();
        // remember id
        char const *id = item->getRepr()->attribute("id");
        // remember title
        gchar *title = item->title();
        // remember description
        gchar *desc = item->desc();
        // remember highlight color
        guint32 highlight_color = 0;
        if (item->isHighlightSet())
            highlight_color = item->highlight_color();

        // It's going to resurrect, so we delete without notifying listeners.
        item->deleteObject(false);

        // restore id
        repr->setAttribute("id", id);
        // add the new repr to the parent
        parent->appendChild(repr);
        SPObject* newObj = document->getObjectByRepr(repr);
        if (title && newObj) {
            newObj->setTitle(title);
            g_free(title);
        }
        if (desc && newObj) {
            newObj->setDesc(desc);
            g_free(desc);
        }
        if (highlight_color && newObj) {
                SP_ITEM(newObj)->setHighlightColor( highlight_color );
        }

        // move to the saved position
        repr->setPosition(pos > 0 ? pos : 0);

        /* Buglet: We don't re-add the (new version of the) object to the selection of any other
         * desktops where it was previously selected. */
        to_select.insert(to_select.begin(),repr);
        Inkscape::GC::release(repr);
    }
    
    return did;
}

Inkscape::XML::Node *
sp_selected_item_to_curved_repr(SPItem *item, guint32 /*text_grouping_policy*/)
{
    if (!item)
        return NULL;

    Inkscape::XML::Document *xml_doc = item->getRepr()->document();

    if (dynamic_cast<SPText *>(item) || dynamic_cast<SPFlowtext *>(item)) {
        // Special treatment for text: convert each glyph to separate path, then group the paths
        Inkscape::XML::Node *g_repr = xml_doc->createElement("svg:g");

        Glib::ustring original_text; // To save original text of accessibility.

        g_repr->setAttribute("transform", item->getRepr()->attribute("transform"));
        /* Mask */
        gchar *mask_str = (gchar *) item->getRepr()->attribute("mask");
        if ( mask_str )
            g_repr->setAttribute("mask", mask_str);
        /* Clip path */
        gchar *clip_path_str = (gchar *) item->getRepr()->attribute("clip-path");
        if ( clip_path_str )
            g_repr->setAttribute("clip-path", clip_path_str);
        /* Rotation center */
        g_repr->setAttribute("inkscape:transform-center-x", item->getRepr()->attribute("inkscape:transform-center-x"), false);
        g_repr->setAttribute("inkscape:transform-center-y", item->getRepr()->attribute("inkscape:transform-center-y"), false);

        /* Whole text's style */
        Glib::ustring style_str =
            item->style->write( SP_STYLE_FLAG_IFDIFF, item->parent ? item->parent->style : NULL); // TODO investigate posibility
        g_repr->setAttribute("style", style_str.c_str());

        Inkscape::Text::Layout::iterator iter = te_get_layout(item)->begin(); 
        do {
            original_text += (gunichar)te_get_layout(item)->characterAt( iter );

            Inkscape::Text::Layout::iterator iter_next = iter;
            iter_next.nextGlyph(); // iter_next is one glyph ahead from iter
            if (iter == iter_next)
                break;

            /* This glyph's style */
            SPObject const *pos_obj = 0;
            void *rawptr = 0;
            te_get_layout(item)->getSourceOfCharacter(iter, &rawptr);
            if (!rawptr || !SP_IS_OBJECT(rawptr)) // no source for glyph, abort
                break;
            pos_obj = SP_OBJECT(rawptr);
            while (dynamic_cast<SPString const *>(pos_obj) && pos_obj->parent) {
               pos_obj = pos_obj->parent;   // SPStrings don't have style
            }
            Glib::ustring style_str =
                pos_obj->style->write( SP_STYLE_FLAG_IFDIFF, pos_obj->parent ? pos_obj->parent->style : NULL); // TODO investigate posibility

            // get path from iter to iter_next:
            SPCurve *curve = te_get_layout(item)->convertToCurves(iter, iter_next);
            iter = iter_next; // shift to next glyph
            if (!curve) { // error converting this glyph
                continue;
            }
            if (curve->is_empty()) { // whitespace glyph?
                curve->unref();
                continue;
            }

            Inkscape::XML::Node *p_repr = xml_doc->createElement("svg:path");

            gchar *def_str = sp_svg_write_path(curve->get_pathvector());
            p_repr->setAttribute("d", def_str);
            g_free(def_str);
            curve->unref();

            p_repr->setAttribute("style", style_str.c_str());

            g_repr->appendChild(p_repr);

            // For accessibility, store original string
            if( original_text.size() > 0 ) {
                g_repr->setAttribute("aria-label", original_text.c_str() );
            }
            Inkscape::GC::release(p_repr);

            if (iter == te_get_layout(item)->end())
                break;

        } while (true);

        return g_repr;
    }

    SPCurve *curve = NULL;
    {
        SPShape *shape = dynamic_cast<SPShape *>(item);
        if (shape) {
            curve = shape->getCurve();
        }
    }

    if (!curve)
        return NULL;

    // Prevent empty paths from being added to the document
    // otherwise we end up with zomby markup in the SVG file
    if(curve->is_empty())
    {
        curve->unref();
        return NULL;
    }

    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
    /* Transformation */
    repr->setAttribute("transform", item->getRepr()->attribute("transform"));

    /* Style */
    Glib::ustring style_str =
        item->style->write( SP_STYLE_FLAG_IFDIFF, item->parent ? item->parent->style : NULL); // TODO investigate posibility
    repr->setAttribute("style", style_str.c_str());

    /* Mask */
    gchar *mask_str = (gchar *) item->getRepr()->attribute("mask");
    if ( mask_str )
        repr->setAttribute("mask", mask_str);

    /* Clip path */
    gchar *clip_path_str = (gchar *) item->getRepr()->attribute("clip-path");
    if ( clip_path_str )
        repr->setAttribute("clip-path", clip_path_str);

    /* Rotation center */
    repr->setAttribute("inkscape:transform-center-x", item->getRepr()->attribute("inkscape:transform-center-x"), false);
    repr->setAttribute("inkscape:transform-center-y", item->getRepr()->attribute("inkscape:transform-center-y"), false);

    /* Definition */
    gchar *def_str = sp_svg_write_path(curve->get_pathvector());
    repr->setAttribute("d", def_str);
    g_free(def_str);
    curve->unref();
    return repr;
}


void
sp_selected_path_reverse(SPDesktop *desktop)
{
    Inkscape::Selection *selection = desktop->getSelection();
    std::vector<SPItem*> items = selection->itemList();

    if (items.empty()) {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>path(s)</b> to reverse."));
        return;
    }


    // set "busy" cursor
    desktop->setWaitingCursor();

    bool did = false;
    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Reversing paths..."));

    for (std::vector<SPItem*>::const_iterator i = items.begin(); i != items.end(); i++){

        SPPath *path = dynamic_cast<SPPath *>(*i);
        if (!path) {
            continue;
        }

        did = true;

        SPCurve *rcurve = path->get_curve_reference()->create_reverse();

        gchar *str = sp_svg_write_path(rcurve->get_pathvector());
        if ( path->hasPathEffectRecursive() ) {
            path->getRepr()->setAttribute("inkscape:original-d", str);
        } else {
            path->getRepr()->setAttribute("d", str);
        }
        g_free(str);

        rcurve->unref();

        // reverse nodetypes order (Bug #179866)
        gchar *nodetypes = g_strdup(path->getRepr()->attribute("sodipodi:nodetypes"));
        if ( nodetypes ) {
            path->getRepr()->setAttribute("sodipodi:nodetypes", g_strreverse(nodetypes));
            g_free(nodetypes);
        }
    }

    desktop->clearWaitingCursor();

    if (did) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_SELECTION_REVERSE,
                           _("Reverse path"));
    } else {
        desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE, _("<b>No paths</b> to reverse in the selection."));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
