/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include "xml/quote.h"
#include "layer-model.h"
#include "selection.h"
#include "selection-describer.h"
#include "desktop.h"
#include "sp-textpath.h"
#include "sp-offset.h"
#include "sp-flowtext.h"
#include "sp-use.h"
#include "sp-symbol.h"
#include "sp-rect.h"
#include "box3d.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-anchor.h"
#include "sp-image.h"
#include "sp-path.h"
#include "sp-line.h"
#include "sp-use.h"
#include "sp-polyline.h"
#include "sp-spiral.h"

// Returns a list of terms for the items to be used in the statusbar
char* collect_terms (const std::vector<SPItem*> &items)
{
    GSList *check = NULL;
    std::stringstream ss;
    bool first = true;

    for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();++iter ) {
        SPItem *item = *iter;
        if (item) {
            const char *term = item->displayName();
            if (term != NULL && g_slist_find (check, term) == NULL) {
                check = g_slist_prepend (check, (void *) term);
                ss << (first ? "" : ", ") << "<b>" << term << "</b>";
                first = false;
            }
        }
    }
    return g_strdup(ss.str().c_str());
}

// Returns the number of terms in the list
static int count_terms (const std::vector<SPItem*> &items)
{
    GSList *check = NULL;
    int count=0;
    for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();++iter ) {
        SPItem *item = *iter;
        if (item) {
            const char *term = item->displayName();
            if (term != NULL && g_slist_find (check, term) == NULL) {
                check = g_slist_prepend (check, (void *) term);
                count++;
            }
        }
    }
    return count;
}

// Returns the number of filtered items in the list
static int count_filtered (const std::vector<SPItem*> &items)
{
    int count=0;
    for ( std::vector<SPItem*>::const_iterator iter=items.begin();iter!=items.end();++iter ) {
        SPItem *item = *iter;
        if (item) {
            count += item->isFiltered();
        }
    }
    return count;
}


namespace Inkscape {

SelectionDescriber::SelectionDescriber(Inkscape::Selection *selection, MessageStack *stack, char *when_selected, char *when_nothing)
    : _context(stack),
      _when_selected (when_selected),
      _when_nothing (when_nothing)
{
    _selection_changed_connection = new sigc::connection (
             selection->connectChanged(
                 sigc::mem_fun(*this, &SelectionDescriber::_updateMessageFromSelection)));
    _selection_modified_connection = new sigc::connection (
             selection->connectModified(
                 sigc::mem_fun(*this, &SelectionDescriber::_selectionModified)));
    _updateMessageFromSelection(selection);
}

SelectionDescriber::~SelectionDescriber()
{
    _selection_changed_connection->disconnect();
    _selection_modified_connection->disconnect();
    delete _selection_changed_connection;
    delete _selection_modified_connection;
}

void SelectionDescriber::_selectionModified(Inkscape::Selection *selection, guint /*flags*/)
{
    _updateMessageFromSelection(selection);
}

void SelectionDescriber::_updateMessageFromSelection(Inkscape::Selection *selection) {
	std::vector<SPItem*> const items = selection->itemList();

    if (items.empty()) { // no items
        _context.set(Inkscape::NORMAL_MESSAGE, _when_nothing);
    } else {
        SPItem *item = items[0];
        g_assert(item != NULL);
        SPObject *layer = selection->layers()->layerForObject(item);
        SPObject *root = selection->layers()->currentRoot();

        // Layer name
        gchar *layer_name;
        if (layer == root) {
            layer_name = g_strdup(_("root"));
        } else if(!layer) {
            layer_name = g_strdup(_("none"));
        } else {
            char const *layer_label;
            bool is_label = false;
            if (layer->label()) {
                layer_label = layer->label();
                is_label = true;
            } else {
                layer_label = layer->defaultLabel();
            }
            char *quoted_layer_label = xml_quote_strdup(layer_label);
            if (is_label) {
                layer_name = g_strdup_printf(_("layer <b>%s</b>"), quoted_layer_label);
            } else {
                layer_name = g_strdup_printf(_("layer <b><i>%s</i></b>"), quoted_layer_label);
            }
            g_free(quoted_layer_label);
        }

        // Parent name
        SPObject *parent = item->parent;
        gchar const *parent_label = parent->getId();
        gchar *parent_name = NULL;
        if (parent_label) {
            char *quoted_parent_label = xml_quote_strdup(parent_label);
            parent_name = g_strdup_printf(_("<i>%s</i>"), quoted_parent_label);
            g_free(quoted_parent_label);
        }

        gchar *in_phrase;
        guint num_layers = selection->numberOfLayers();
        guint num_parents = selection->numberOfParents();
        if (num_layers == 1) {
            if (num_parents == 1) {
                if (layer == parent)
                    in_phrase = g_strdup_printf(_(" in %s"), layer_name);
                else if (!layer)
                    in_phrase = g_strdup_printf("%s", _(" hidden in definitions"));
                else if (parent_name)
                    in_phrase = g_strdup_printf(_(" in group %s (%s)"), parent_name, layer_name);
                else
                    in_phrase = g_strdup_printf(_(" in unnamed group (%s)"), layer_name);
            } else {
                    in_phrase = g_strdup_printf(ngettext(" in <b>%i</b> parent (%s)", " in <b>%i</b> parents (%s)", num_parents), num_parents, layer_name);
            }
        } else {
            in_phrase = g_strdup_printf(ngettext(" in <b>%i</b> layer", " in <b>%i</b> layers", num_layers), num_layers);
        }
        g_free (layer_name);
        g_free (parent_name);

        if (items.size()==1) { // one item
            char *item_desc = item->detailedDescription();

            bool isUse = dynamic_cast<SPUse *>(item) != NULL;
            if (isUse && dynamic_cast<SPSymbol *>(item->firstChild())) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, in_phrase,
                              _("Convert symbol to group to edit"), _when_selected);
            } else if (dynamic_cast<SPSymbol *>(item)) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s.",
                              item_desc, in_phrase,
                              _("Remove from symbols tray to edit symbol"));
            } else {
                SPOffset *offset = (isUse) ? NULL : dynamic_cast<SPOffset *>(item);
                if (isUse || (offset && offset->sourceHref)) {
                    _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                                  item_desc, in_phrase,
                                  _("Use <b>Shift+D</b> to look up original"), _when_selected);
                } else {
                    SPText *text = dynamic_cast<SPText *>(item);
                    if (text && text->firstChild() && dynamic_cast<SPText *>(text->firstChild())) {
                        _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                                      item_desc, in_phrase,
                                      _("Use <b>Shift+D</b> to look up path"), _when_selected);
                    } else {
                        SPFlowtext *flowtext = dynamic_cast<SPFlowtext *>(item);
                        if (flowtext && !flowtext->has_internal_frame()) {
                            _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                                          item_desc, in_phrase,
                                          _("Use <b>Shift+D</b> to look up frame"), _when_selected);
                        } else {
                            _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s.",
                                          item_desc, in_phrase, _when_selected);
                        }
                    }
                }
            }

            g_free(item_desc);
        } else { // multiple items
            int objcount = items.size();
            char *terms = collect_terms (items);
            int n_terms = count_terms(items);
            
            gchar *objects_str = g_strdup_printf(ngettext(
                "<b>%1$i</b> objects selected of type %2$s",
                "<b>%1$i</b> objects selected of types %2$s", n_terms),
                 objcount, terms);

            g_free(terms);

            // indicate all, some, or none filtered
            gchar *filt_str = NULL;
            int n_filt = count_filtered(items);  //all filtered
            if (n_filt) {
                filt_str = g_strdup_printf(ngettext("; <i>%d filtered object</i> ",
                                                     "; <i>%d filtered objects</i> ", n_filt), n_filt);
            } else {
                filt_str = g_strdup("");
            }

            _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s%s. %s.", objects_str, filt_str, in_phrase, _when_selected);
            if (objects_str) {
                g_free(objects_str);
                objects_str = 0;
            }
            if (filt_str) {
                g_free(filt_str);
                filt_str = 0;
            }
        }

        g_free(in_phrase);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
