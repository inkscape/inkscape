/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
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
#include "selection.h"
#include "selection-describer.h"
#include "desktop.h"
#include "sp-textpath.h"
#include "sp-offset.h"
#include "sp-flowtext.h"
#include "sp-use.h"
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

const gchar *
type2term(GType type)
{
    if (type == SP_TYPE_ANCHOR)
        //TRANSLATORS: only translate "string" in "context|string".
        // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
	// "Link" means internet link (anchor)
        { return Q_("web|Link"); }
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
    if (type == SP_TYPE_BOX3D)
        { return _("3D Box"); }
    if (type == SP_TYPE_TEXT)
        { return _("Text"); }
    if (type == SP_TYPE_USE)
        // TRANSLATORS: only translate "string" in "context|string".
        // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
        // "Clone" is a noun, type of object
        { return Q_("object|Clone"); }
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
    GSList const *items = selection->itemList();

    if (!items) { // no items
        _context.set(Inkscape::NORMAL_MESSAGE, _when_nothing);
    } else {
        SPItem *item = SP_ITEM(items->data);
        SPObject *layer = selection->desktop()->layerForObject (SP_OBJECT (item));
        SPObject *root = selection->desktop()->currentRoot();

        // Layer name
        gchar *layer_name;
        if (layer == root) {
            layer_name = g_strdup(_("root"));
        } else {
            char const *layer_label;
            bool is_label = false;
            if (layer && layer->label()) {
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
        SPObject *parent = SP_OBJECT_PARENT (item);
        gchar *parent_label = SP_OBJECT_ID(parent);
        char *quoted_parent_label = xml_quote_strdup(parent_label);
        gchar *parent_name = g_strdup_printf(_("<i>%s</i>"), quoted_parent_label);
        g_free(quoted_parent_label);

        gchar *in_phrase;
        guint num_layers = selection->numberOfLayers();
        guint num_parents = selection->numberOfParents();
        if (num_layers == 1) {
            if (num_parents == 1) {
                if (layer == parent)
                    in_phrase = g_strdup_printf(_(" in %s"), layer_name);
                else 
                    in_phrase = g_strdup_printf(_(" in group %s (%s)"), parent_name, layer_name);
            } else {
                    in_phrase = g_strdup_printf(ngettext(" in <b>%i</b> parents (%s)", " in <b>%i</b> parents (%s)", num_parents), num_parents, layer_name);
            }
        } else {
            in_phrase = g_strdup_printf(ngettext(" in <b>%i</b> layers", " in <b>%i</b> layers", num_layers), num_layers);
        }
        g_free (layer_name);
        g_free (parent_name);

        if (!items->next) { // one item
            char *item_desc = sp_item_description(item);
            if (SP_IS_USE(item) || (SP_IS_OFFSET(item) && SP_OFFSET (item)->sourceHref)) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, in_phrase,
                              _("Use <b>Shift+D</b> to look up original"), _when_selected);
            } else if (SP_IS_TEXT_TEXTPATH(item)) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, in_phrase,
                              _("Use <b>Shift+D</b> to look up path"), _when_selected);
            } else if (SP_IS_FLOWTEXT(item) && !SP_FLOWTEXT(item)->has_internal_frame()) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.",
                              item_desc, in_phrase,
                              _("Use <b>Shift+D</b> to look up frame"), _when_selected);
            } else {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s.",
                              item_desc, in_phrase, _when_selected);
            }
            g_free(item_desc);
        } else { // multiple items
            int object_count = g_slist_length((GSList *)items);

            const gchar *objects_str = NULL;
            GSList *terms = collect_terms ((GSList *)items);
            int n_terms = g_slist_length(terms);
            if (n_terms == 0) {
                objects_str = g_strdup_printf (
                    // this is only used with 2 or more objects
                    ngettext("<b>%i</b> object selected", "<b>%i</b> objects selected", object_count), 
                    object_count);
            } else if (n_terms == 1) {
                objects_str = g_strdup_printf (
                    // this is only used with 2 or more objects
                    ngettext("<b>%i</b> object of type <b>%s</b>", "<b>%i</b> objects of type <b>%s</b>", object_count),
                    object_count, (gchar *) terms->data);
            } else if (n_terms == 2) {
                objects_str = g_strdup_printf (
                    // this is only used with 2 or more objects
                    ngettext("<b>%i</b> object of types <b>%s</b>, <b>%s</b>", "<b>%i</b> objects of types <b>%s</b>, <b>%s</b>", object_count), 
                    object_count, (gchar *) terms->data, (gchar *) terms->next->data);
            } else if (n_terms == 3) {
                objects_str = g_strdup_printf (
                    // this is only used with 2 or more objects
                    ngettext("<b>%i</b> object of types <b>%s</b>, <b>%s</b>, <b>%s</b>", "<b>%i</b> objects of types <b>%s</b>, <b>%s</b>, <b>%s</b>", object_count), 
                    object_count, (gchar *) terms->data, (gchar *) terms->next->data, (gchar *) terms->next->next->data);
            } else {
                objects_str = g_strdup_printf (
                    // this is only used with 2 or more objects
                    ngettext("<b>%i</b> object of <b>%i</b> types", "<b>%i</b> objects of <b>%i</b> types", object_count), 
                    object_count, n_terms);
            }
            g_slist_free (terms);

            _context.setF(Inkscape::NORMAL_MESSAGE, _("%s%s. %s."), objects_str, in_phrase, _when_selected);

            if (objects_str)
                g_free ((gchar *) objects_str);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
