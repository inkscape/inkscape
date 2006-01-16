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
            const gchar *object_count_str = NULL;
            object_count_str = g_strdup_printf (
                                ngettext("<b>%i</b> object selected",
                                         "<b>%i</b> objects selected",
                                         object_count),
                                object_count);

            if (selection->numberOfLayers() == 1) {
                _context.setF(Inkscape::NORMAL_MESSAGE, _("%s%s. %s."), 
                              object_count_str, layer_phrase, when_selected);
            } else {
                _context.setF(Inkscape::NORMAL_MESSAGE, 
                              ngettext("%s in <b>%i</b> layer. %s.",
                                       "%s in <b>%i</b> layers. %s.", 
                                       selection->numberOfLayers()),
                              object_count_str, selection->numberOfLayers(), when_selected);
            }

            if (object_count_str)
                g_free ((gchar *) object_count_str);
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
