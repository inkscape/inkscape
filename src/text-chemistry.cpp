#define __SP_TEXT_CHEMISTRY_C__

/*
 * Text commands
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2004 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cstring>
#include <string>
#include <glibmm/i18n.h>

#include "libnr/nr-matrix-fns.h"
#include "xml/repr.h"
#include "sp-rect.h"
#include "sp-textpath.h"
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "message-stack.h"
#include "selection.h"
#include "style.h"
#include "desktop-handles.h"
#include "text-editing.h"
#include "text-chemistry.h"
#include "sp-flowtext.h"
#include "sp-flowregion.h"
#include "sp-flowdiv.h"
#include "sp-tspan.h"


SPItem *
text_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_TEXT(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

SPItem *
flowtext_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_FLOWTEXT(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

SPItem *
text_or_flowtext_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_TEXT(items->data) || SP_IS_FLOWTEXT(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

SPItem *
shape_in_selection(Inkscape::Selection *selection)
{
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_SHAPE(items->data))
            return ((SPItem *) items->data);
    }
    return NULL;
}

void
text_put_on_path()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    SPItem *text = text_or_flowtext_in_selection(selection);
    SPItem *shape = shape_in_selection(selection);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());

    if (!text || !shape || g_slist_length((GSList *) selection->itemList()) != 2) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text and a path</b> to put text on path."));
        return;
    }

    if (SP_IS_TEXT_TEXTPATH(text)) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("This text object is <b>already put on a path</b>. Remove it from the path first. Use <b>Shift+D</b> to look up its path."));
        return;
    }

    if (SP_IS_RECT(shape)) {
        // rect is the only SPShape which is not <path> yet, and thus SVG forbids us from putting text on it
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("You cannot put text on a rectangle in this version. Convert rectangle to path first."));
        return;
    }

    // if a flowed text is selected, convert it to a regular text object
    if (SP_IS_FLOWTEXT(text)) {

        if (!SP_FLOWTEXT(text)->layout.outputExists()) {
            sp_desktop_message_stack(desktop)->
                flash(Inkscape::WARNING_MESSAGE, 
                      _("The flowed text(s) must be <b>visible</b> in order to be put on a path."));
        }

        Inkscape::XML::Node *repr = SP_FLOWTEXT(text)->getAsText();

        if (!repr) return;

        Inkscape::XML::Node *parent = SP_OBJECT_REPR(text)->parent();
        parent->appendChild(repr);

        SPItem *new_item = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);
        sp_item_write_transform(new_item, repr, text->transform);
        SP_OBJECT(new_item)->updateRepr();

        Inkscape::GC::release(repr);
        text->deleteObject(); // delete the orignal flowtext

        sp_document_ensure_up_to_date(sp_desktop_document(desktop));

        selection->clear();

        text = new_item; // point to the new text
    }

    Inkscape::Text::Layout const *layout = te_get_layout(text);
    Inkscape::Text::Layout::Alignment text_alignment = layout->paragraphAlignment(layout->begin());

    // remove transform from text, but recursively scale text's fontsize by the expansion
    SP_TEXT(text)->_adjustFontsizeRecursive (text, NR::expansion(SP_ITEM(text)->transform));
    SP_OBJECT_REPR(text)->setAttribute("transform", NULL);

    // make a list of text children
    GSList *text_reprs = NULL;
    for (SPObject *o = SP_OBJECT(text)->children; o != NULL; o = o->next) {
        text_reprs = g_slist_prepend(text_reprs, SP_OBJECT_REPR(o));
    }

    // create textPath and put it into the text
    Inkscape::XML::Node *textpath = xml_doc->createElement("svg:textPath");
    // reference the shape
    textpath->setAttribute("xlink:href", g_strdup_printf("#%s", SP_OBJECT_REPR(shape)->attribute("id")));
    if (text_alignment == Inkscape::Text::Layout::RIGHT)
        textpath->setAttribute("startOffset", "100%");
    else if (text_alignment == Inkscape::Text::Layout::CENTER)
        textpath->setAttribute("startOffset", "50%");
    SP_OBJECT_REPR(text)->addChild(textpath, NULL);

    for ( GSList *i = text_reprs ; i ; i = i->next ) {
        // Make a copy of each text child
        Inkscape::XML::Node *copy = ((Inkscape::XML::Node *) i->data)->duplicate(xml_doc);
        // We cannot have multiline in textpath, so remove line attrs from tspans
        if (!strcmp(copy->name(), "svg:tspan")) {
            copy->setAttribute("sodipodi:role", NULL);
            copy->setAttribute("x", NULL);
            copy->setAttribute("y", NULL);
        }
        // remove the old repr from under text
        SP_OBJECT_REPR(text)->removeChild((Inkscape::XML::Node *) i->data);
        // put its copy into under textPath
        textpath->addChild(copy, NULL); // fixme: copy id
    }

    // x/y are useless with textpath, and confuse Batik 1.5
    SP_OBJECT_REPR(text)->setAttribute("x", NULL);
    SP_OBJECT_REPR(text)->setAttribute("y", NULL);

    sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, 
                     _("Put text on path"));
    g_slist_free(text_reprs);
}

void
text_remove_from_path()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text on path</b> to remove it from path."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        if (!SP_IS_TEXT_TEXTPATH(SP_OBJECT(items->data))) {
            continue;
        }

        SPObject *tp = sp_object_first_child(SP_OBJECT(items->data));

        did = true;

        sp_textpath_to_text(tp);
    }

    if (!did) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No texts-on-paths</b> in the selection."));
    } else {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, 
                         _("Remove text from path"));
        selection->setList(g_slist_copy((GSList *) selection->itemList())); // reselect to update statusbar description
    }
}

void
text_remove_all_kerns_recursively(SPObject *o)
{
    SP_OBJECT_REPR(o)->setAttribute("dx", NULL);
    SP_OBJECT_REPR(o)->setAttribute("dy", NULL);
    SP_OBJECT_REPR(o)->setAttribute("rotate", NULL);

    // if x contains a list, leave only the first value
    gchar *x = (gchar *) SP_OBJECT_REPR(o)->attribute("x");
    if (x) {
        gchar **xa_space = g_strsplit(x, " ", 0);
        gchar **xa_comma = g_strsplit(x, ",", 0);
        if (xa_space && *xa_space && *(xa_space + 1)) {
            SP_OBJECT_REPR(o)->setAttribute("x", g_strdup(*xa_space));
        } else if (xa_comma && *xa_comma && *(xa_comma + 1)) {
            SP_OBJECT_REPR(o)->setAttribute("x", g_strdup(*xa_comma));
        }
        g_strfreev(xa_space);
        g_strfreev(xa_comma);
    }

    for (SPObject *i = sp_object_first_child(o); i != NULL; i = SP_OBJECT_NEXT(i)) {
        text_remove_all_kerns_recursively(i);
    }
}

//FIXME: must work with text selection
void
text_remove_all_kerns()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>text(s)</b> to remove kerns from."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {
        SPObject *obj = SP_OBJECT(items->data);

        if (!SP_IS_TEXT(obj) && !SP_IS_TSPAN(obj) && !SP_IS_FLOWTEXT(obj)) {
            continue;
        }

        text_remove_all_kerns_recursively(obj);
        obj->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        did = true;
    }

    if (!did) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("Select <b>text(s)</b> to remove kerns from."));
    } else {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, 
                         _("Remove manual kerns"));
    }
}

void
text_flow_into_shape()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    SPDocument *doc = sp_desktop_document (desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    SPItem *text = text_or_flowtext_in_selection(selection);
    SPItem *shape = shape_in_selection(selection);

    if (!text || !shape || g_slist_length((GSList *) selection->itemList()) < 2) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a text</b> and one or more <b>paths or shapes</b> to flow text into frame."));
        return;
    }

    if (SP_IS_TEXT(text)) {
      // remove transform from text, but recursively scale text's fontsize by the expansion
      SP_TEXT(text)->_adjustFontsizeRecursive(text, NR::expansion(SP_ITEM(text)->transform));
      SP_OBJECT_REPR(text)->setAttribute("transform", NULL);
    }

    Inkscape::XML::Node *root_repr = xml_doc->createElement("svg:flowRoot");
    root_repr->setAttribute("xml:space", "preserve"); // we preserve spaces in the text objects we create
    root_repr->setAttribute("style", SP_OBJECT_REPR(text)->attribute("style")); // fixme: transfer style attrs too
    SP_OBJECT_REPR(SP_OBJECT_PARENT(shape))->appendChild(root_repr);
    SPObject *root_object = doc->getObjectByRepr(root_repr);
    g_return_if_fail(SP_IS_FLOWTEXT(root_object));

    Inkscape::XML::Node *region_repr = xml_doc->createElement("svg:flowRegion");
    root_repr->appendChild(region_repr);
    SPObject *object = doc->getObjectByRepr(region_repr);
    g_return_if_fail(SP_IS_FLOWREGION(object));

    /* Add clones */
    for (GSList *items = (GSList *) selection->itemList();
         items != NULL;
         items = items->next) {
        SPItem *item = SP_ITEM(items->data);
        if (SP_IS_SHAPE(item)){
            Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");
            clone->setAttribute("x", "0");
            clone->setAttribute("y", "0");
            clone->setAttribute("xlink:href", g_strdup_printf("#%s", SP_OBJECT_REPR(item)->attribute("id")));

            // add the new clone to the region
            region_repr->appendChild(clone);
        }
    }

    if (SP_IS_TEXT(text)) { // flow from text, as string
        Inkscape::XML::Node *para_repr = xml_doc->createElement("svg:flowPara");
        root_repr->appendChild(para_repr);
        object = doc->getObjectByRepr(para_repr);
        g_return_if_fail(SP_IS_FLOWPARA(object));

        Inkscape::Text::Layout const *layout = te_get_layout(text);
        Glib::ustring text_ustring = sp_te_get_string_multiline(text, layout->begin(), layout->end());

        Inkscape::XML::Node *text_repr = xml_doc->createTextNode(text_ustring.c_str()); // FIXME: transfer all formatting! and convert newlines into flowParas!
        para_repr->appendChild(text_repr);

        Inkscape::GC::release(para_repr);
        Inkscape::GC::release(text_repr);

    } else { // reflow an already flowed text, preserving paras
        for (SPObject *o = SP_OBJECT(text)->children; o != NULL; o = o->next) {
            if (SP_IS_FLOWPARA(o)) {
                Inkscape::XML::Node *para_repr = SP_OBJECT_REPR(o)->duplicate(xml_doc);
                root_repr->appendChild(para_repr);
                object = doc->getObjectByRepr(para_repr);
                g_return_if_fail(SP_IS_FLOWPARA(object));
                Inkscape::GC::release(para_repr);
            }
        }
    }

    SP_OBJECT(text)->deleteObject (true);

    sp_document_done(doc, SP_VERB_CONTEXT_TEXT,
                     _("Flow text into shape"));

    sp_desktop_selection(desktop)->set(SP_ITEM(root_object));

    Inkscape::GC::release(root_repr);
    Inkscape::GC::release(region_repr);
}

void
text_unflow ()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop)
        return;

    SPDocument *doc = sp_desktop_document (desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);


    if (!flowtext_in_selection(selection) || g_slist_length((GSList *) selection->itemList()) < 1) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>a flowed text</b> to unflow it."));
        return;
    }

    GSList *new_objs = NULL;
    GSList *old_objs = NULL;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        if (!SP_IS_FLOWTEXT(SP_OBJECT(items->data))) {
            continue;
        }

        SPItem *flowtext = SP_ITEM(items->data);

        // we discard transform when unflowing, but we must preserve expansion which is visible as
        // font size multiplier
        double ex = (flowtext->transform).descrim();

        if (sp_te_get_string_multiline(flowtext) == NULL) { // flowtext is empty
            continue;
        }

        /* Create <text> */
        Inkscape::XML::Node *rtext = xml_doc->createElement("svg:text");
        rtext->setAttribute("xml:space", "preserve"); // we preserve spaces in the text objects we create

        /* Set style */
        rtext->setAttribute("style", SP_OBJECT_REPR(flowtext)->attribute("style")); // fixme: transfer style attrs too; and from descendants

        NRRect bbox;
        sp_item_invoke_bbox(SP_ITEM(flowtext), &bbox, sp_item_i2doc_affine(SP_ITEM(flowtext)), TRUE);
        Geom::Point xy(bbox.x0, bbox.y0);
        if (xy[Geom::X] != 1e18 && xy[Geom::Y] != 1e18) {
            sp_repr_set_svg_double(rtext, "x", xy[Geom::X]);
            sp_repr_set_svg_double(rtext, "y", xy[Geom::Y]);
        }

        /* Create <tspan> */
        Inkscape::XML::Node *rtspan = xml_doc->createElement("svg:tspan");
        rtspan->setAttribute("sodipodi:role", "line"); // otherwise, why bother creating the tspan?
        rtext->addChild(rtspan, NULL);

        gchar *text_string = sp_te_get_string_multiline(flowtext);
        Inkscape::XML::Node *text_repr = xml_doc->createTextNode(text_string); // FIXME: transfer all formatting!!!
        free(text_string);
        rtspan->appendChild(text_repr);

        SP_OBJECT_REPR(SP_OBJECT_PARENT(flowtext))->appendChild(rtext);
        SPObject *text_object = doc->getObjectByRepr(rtext);

        // restore the font size multiplier from the flowtext's transform
        SP_TEXT(text_object)->_adjustFontsizeRecursive(SP_ITEM(text_object), ex);

        new_objs = g_slist_prepend (new_objs, text_object);
        old_objs = g_slist_prepend (old_objs, flowtext);

        Inkscape::GC::release(rtext);
        Inkscape::GC::release(rtspan);
        Inkscape::GC::release(text_repr);
    }

    selection->clear();
    selection->setList(new_objs);
    for (GSList *i = old_objs; i; i = i->next) {
        SP_OBJECT(i->data)->deleteObject (true);
    }

    g_slist_free (old_objs);
    g_slist_free (new_objs);

    sp_document_done(doc, SP_VERB_CONTEXT_TEXT, 
                     _("Unflow flowed text"));
}

void
flowtext_to_text()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, 
                                                 _("Select <b>flowed text(s)</b> to convert."));
        return;
    }

    bool did = false;

    GSList *reprs = NULL;
    GSList *items = g_slist_copy((GSList *) selection->itemList());
    for (; items != NULL; items = items->next) {
        
        SPItem *item = (SPItem *) items->data;

        if (!SP_IS_FLOWTEXT(item))
            continue;

        if (!SP_FLOWTEXT(item)->layout.outputExists()) {
            sp_desktop_message_stack(desktop)->
                flash(Inkscape::WARNING_MESSAGE, 
                      _("The flowed text(s) must be <b>visible</b> in order to be converted."));
            return;
        }

        Inkscape::XML::Node *repr = SP_FLOWTEXT(item)->getAsText();

        if (!repr) break;

        did = true;

        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        parent->addChild(repr, SP_OBJECT_REPR(item));

        SPItem *new_item = (SPItem *) sp_desktop_document(desktop)->getObjectByRepr(repr);
        sp_item_write_transform(new_item, repr, item->transform);
        SP_OBJECT(new_item)->updateRepr();
    
        Inkscape::GC::release(repr);
        item->deleteObject();

        reprs = g_slist_prepend(reprs, repr);
    }

    g_slist_free(items);

    if (did) {
        sp_document_done(sp_desktop_document(desktop), 
                         SP_VERB_OBJECT_FLOWTEXT_TO_TEXT,
                         _("Convert flowed text to text"));
        selection->setReprList(reprs);        
    } else {
        sp_desktop_message_stack(desktop)->
            flash(Inkscape::ERROR_MESSAGE,
                  _("<b>No flowed text(s)</b> to convert in the selection."));
    }

    g_slist_free(reprs);
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
