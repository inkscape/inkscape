/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include "xml/quote.h"
#include "selection.h"
#include "selection-describer.h"
#include "desktop.h"
#include "sp-textpath.h"
#include "sp-offset.h"
#include "sp-flowtext.h"
#include "sp-use.h"
#include "sp-rect.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-anchor.h"
#include "sp-image.h"
#include "sp-path.h"
#include "sp-line.h"
#include "sp-use.h"
#include "sp-polyline.h"
#include "sp-spiral.h"

const gchar *
type2term(GType type)
{
    if (type == SP_TYPE_ANCHOR)
        { return _("Link"); }
    if (type == SP_TYPE_CIRCLE)
        { return _("Circle"); }
    if (type == SP_TYPE_ELLIPSE)
        { return _("Ellipse"); }
    if (type == SP_TYPE_FLOWTEXT)
        { return _("Flowed text"); }
    if (type == SP_TYPE_GROUP)
        { return _("Group"); }
    if (type == SP_TYPE_IMAGE)
        { return _("Image"); }
    if (type == SP_TYPE_LINE)
        { return _("Line"); }
    if (type == SP_TYPE_PATH)
        { return _("Path"); }
    if (type == SP_TYPE_POLYGON)
        { return _("Polygon"); }
    if (type == SP_TYPE_POLYLINE)
        { return _("Polyline"); }
    if (type == SP_TYPE_RECT)
        { return _("Rectangle"); }
    if (type == SP_TYPE_TEXT)
        { return _("Text"); }
    if (type == SP_TYPE_USE)
        { return _("Clone"); }
    if (type == SP_TYPE_ARC)
        { return _("Ellipse"); }
    if (type == SP_TYPE_OFFSET)
        { return _("Offset path"); }
    if (type == SP_TYPE_SPIRAL)
        { return _("Spiral"); }
    if (type == SP_TYPE_STAR)
        { return _("Star"); }
    return NULL;
}

GSList *collect_terms (GSList *items)
{
    GSList *r = NULL;
    for (GSList *i = items; i != NULL; i = i->next) {
        const gchar *term = type2term (G_OBJECT_TYPE(i->data));
        if (term != NULL && g_slist_find (r, term) == NULL)
            r = g_slist_prepend (r, (void *) term);
    }
    return r;
}


namespace Inkscape {

SelectionDescriber::SelectionDescriber(Inkscape::Selection *selection, MessageStack *stack)
: _context(stack)
{
    selection->connectChanged(sigc::mem_fun(*this, &SelectionDescriber::_updateMessageFromSelection));
    _updateMessageFromSelection(selection);
}

void SelectionDescriber::_updateMessageFromSelection(Inkscape::Selection *selection) {
    GSList const *items = selection->itemList();

    char const *when_selected = _("Click selection to toggle scale/rotation handles");
    if (!items) { // no items
        _context.set(Inkscape::NORMAL_MESSAGE, _("No objects selected. Click, Shift+click, or drag around objects to select."));
    } else {
        SPItem *item = SP_ITEM(items->data);
        SPObject *layer = selection->desktop()->layerForObject (SP_OBJECT (item));
        SPObject *root = selection->desktop()->currentRoot();
        gchar *layer_phrase;
        if (layer == root) {
            layer_phrase = g_strdup("");  // for simplicity
        } else {
            char const *name, *fmt;
            if (layer && layer->label()) {
                name = layer->label();
                fmt = _(" in layer <b>%s</b>");
            } else {
                name = layer->defaultLabel();
                fmt = _(" in layer <b><i>%s</i></b>");
            }
            char *quoted_name = xml_quote_strdup(name);
            layer_phrase = g_strdup_printf(fmt, quoted_name);
            g_free(quoted_name);
        }

        if (!items->next) { // one item
            char *item_desc = sp_item_description(item);
            if (SP_IS_USE(item) || (SP_IS_OFFSET(item) && SP_OFFSET (item)->sourceHref)) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, layer_phrase,
                              _("Use <b>Shift+D</b> to look up original"), when_selected);
            } else if (SP_IS_TEXT_TEXTPATH(item)) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, layer_phrase,
                              _("Use <b>Shift+D</b> to look up path"), when_selected);
            } else if (SP_IS_FLOWTEXT(item) && !SP_FLOWTEXT(item)->has_internal_frame()) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, layer_phrase,
                              _("Use <b>Shift+D</b> to look up frame"), when_selected);
            } else {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s.",
                              item_desc, layer_phrase, when_selected);
            }
            g_free(item_desc);
        } else { // multiple items
            int object_count = g_slist_length((GSList *)items);

            const gchar *objects_str = NULL;
            GSList *terms = collect_terms ((GSList *)items);
            int n_terms = g_slist_length(terms);
            if (n_terms == 0) {
                objects_str = g_strdup_printf (_("<b>%i</b> objects selected"), object_count);
            } else if (n_terms == 1) {
                objects_str = g_strdup_printf (_("<b>%i</b> objects of type <b>%s</b>"), object_count, (gchar *) terms->data);
            } else if (n_terms == 2) {
                objects_str = g_strdup_printf (_("<b>%i</b> objects of types <b>%s</b>, <b>%s</b>"), object_count, (gchar *) terms->data, (gchar *) terms->next->data);
            } else if (n_terms == 3) {
                objects_str = g_strdup_printf (_("<b>%i</b> objects of types <b>%s</b>, <b>%s</b>, <b>%s</b>"), object_count, (gchar *) terms->data, (gchar *) terms->next->data, (gchar *) terms->next->next->data);
            } else {
                objects_str = g_strdup_printf (_("<b>%i</b> objects of <b>%i</b> types"), object_count, n_terms);
            }
            g_slist_free (terms);

            if (selection->numberOfLayers() == 1) {
                _context.setF(Inkscape::NORMAL_MESSAGE, _("%s%s. %s."),
                              objects_str, layer_phrase, when_selected);
            } else {
                _context.setF(Inkscape::NORMAL_MESSAGE,
                              ngettext("%s in <b>%i</b> layer. %s.",
                                       "%s in <b>%i</b> layers. %s.",
                                       selection->numberOfLayers()),
                              objects_str, selection->numberOfLayers(), when_selected);
            }

            if (objects_str)
                g_free ((gchar *) objects_str);
        }

        g_free(layer_phrase);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
