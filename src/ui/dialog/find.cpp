/**
 * \brief Find dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/widget.h>
#include "find.h"
#include "verbs.h"

#include "message-stack.h"
#include "helper/window.h"
#include "macros.h"
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"

#include "dialogs/dialog-events.h"
#include "verbs.h"
#include "interface.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "selection-chemistry.h"
#include "sp-defs.h"
#include "sp-rect.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-spiral.h"
#include "sp-path.h"
#include "sp-line.h"
#include "sp-polyline.h"
#include "sp-item-group.h"
#include "sp-use.h"
#include "sp-image.h"
#include "sp-offset.h"
#include "xml/repr.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

Find::Find()
    : UI::Widget::Panel("", "/dialogs/find", SP_VERB_DIALOG_FIND),
      _entry_text(_("_Text: "), _("Find objects by their text content (exact or partial match)")),
      _entry_id(_("_ID: "), _("Find objects by the value of the id attribute (exact or partial match)")),
      _entry_style(_("_Style: "), _("Find objects by the value of the style attribute (exact or partial match)")),
      _entry_attribute(_("_Attribute: "), _("Find objects by the name of an attribute (exact or partial match)")),
      _check_search_selection(_("Search in s_election"), _("Limit search to the current selection")),
      _check_search_layer(_("Search in current _layer"), _("Limit search to the current layer")),
      _check_include_hidden(_("Include _hidden"), _("Include hidden objects in search")),
      _check_include_locked(_("Include l_ocked"), _("Include locked objects in search")),

      _check_all(_("All types"), _("Search in all object types")),
      _check_all_shapes(_("All shapes"), _("Search all shapes")),
      _check_rects(_("Rectangles"), _("Search rectangles")),
      _check_ellipses(_("Ellipses"), _("Search ellipses, arcs, circles")),
      _check_stars(_("Stars"), _("Search stars and polygons")),
      _check_spirals(_("Spirals"), _("Search spirals")),
      _check_paths(_("Paths"), _("Search paths, lines, polylines")),
      _check_texts(_("Texts"), _("Search text objects")),
      _check_groups(_("Groups"), _("Search groups")),
      _check_clones(
                  //TRANSLATORS: only translate "string" in "context|string".
                  // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                  // "Clones" is a noun indicating type of object to find
    		  Q_("find|Clones"), _("Search clones")),
      _check_images(_("Images"), _("Search images")),
      _check_offsets(_("Offsets"), _("Search offset objects")),
    
      _button_clear(_("_Clear"), _("Clear values")),
      _button_find(_("_Find"), _("Select objects matching all of the fields you filled in"))
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);
    
    contents->pack_start(_entry_text, true, true);
    contents->pack_start(_entry_id, true, true);
    contents->pack_start(_entry_style, true, true);
    contents->pack_start(_entry_attribute, true, true);

    contents->pack_start(_check_all, true, true);
    contents->pack_start(_check_all_shapes, true, true);
    contents->pack_start(_check_rects, true, true);
    contents->pack_start(_check_ellipses, true, true);
    contents->pack_start(_check_stars, true, true);
    contents->pack_start(_check_spirals, true, true);
    contents->pack_start(_check_paths, true, true);
    contents->pack_start(_check_texts, true, true);
    contents->pack_start(_check_groups, true, true);
    contents->pack_start(_check_clones, true, true);
    contents->pack_start(_check_images, true, true);
    contents->pack_start(_check_offsets, true, true);

    contents->pack_start(_check_search_selection, true, true);
    contents->pack_start(_check_search_layer, true, true);
    contents->pack_start(_check_include_hidden, true, true);
    contents->pack_start(_check_include_locked, true, true);

    contents->pack_start(_button_clear, true, true);
    contents->pack_start(_button_find, true, true);

    // set signals to handle clicks
    _check_all.signal_clicked().connect(sigc::mem_fun(*this, &Find::onToggleAlltypes));
    _check_all_shapes.signal_clicked().connect(sigc::mem_fun(*this, &Find::onToggleShapes));
    _button_clear.signal_clicked().connect(sigc::mem_fun(*this, &Find::onClear));
    _button_find.signal_clicked().connect(sigc::mem_fun(*this, &Find::onFind));

    _button_find.set_flags(Gtk::CAN_DEFAULT);
    // set_default (_button_find); // activatable by Enter
    _entry_text.getEntry()->grab_focus();

    show_all_children();
    onClear();
}

Find::~Find() 
{
}


/*########################################################################
# FIND helper functions
########################################################################*/

                   
bool
Find::item_id_match (SPItem *item, const gchar *id, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    if (SP_IS_STRING(item)) // SPStrings have "on demand" ids which are useless for searching
        return false;

    const gchar *item_id = (SP_OBJECT_REPR (item))->attribute("id");
    if (item_id == NULL)
        return false;

    if (exact) {
        return ((bool) !strcmp(item_id, id));
    } else {
//        g_print ("strstr: %s %s: %s\n", item_id, id, strstr(item_id, id) != NULL? "yes":"no");
        return ((bool) (strstr(item_id, id) != NULL));
    }
}

bool
Find::item_text_match (SPItem *item, const gchar *text, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
        const gchar *item_text = sp_te_get_string_multiline (item);
        if (item_text == NULL)
            return false;
        bool ret;
        if (exact) {
            ret = ((bool) !strcasecmp(item_text, text));
        } else {
            //FIXME: strcasestr
            ret = ((bool) (strstr(item_text, text) != NULL));
        }
        g_free ((void*) item_text);
        return ret;
    }
    return false;
}

bool
Find::item_style_match (SPItem *item, const gchar *text, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    const gchar *item_text = (SP_OBJECT_REPR (item))->attribute("style");
    if (item_text == NULL)
        return false;

    if (exact) {
        return ((bool) !strcmp(item_text, text));
    } else {
        return ((bool) (strstr(item_text, text) != NULL));
    }
}

bool
Find::item_attr_match (SPItem *item, const gchar *name, bool exact)
{
    if (SP_OBJECT_REPR (item) == NULL)
        return false;

    if (exact) {
        const gchar *attr_value = (SP_OBJECT_REPR (item))->attribute(name);
        return ((bool) (attr_value != NULL));
    } else {
        return SP_OBJECT_REPR (item)->matchAttributeName(name);
    }
}


GSList *
Find::filter_fields (GSList *l, bool exact)
{
    const gchar* text = _entry_text.getEntry()->get_text().c_str();
    const gchar* id = _entry_id.getEntry()->get_text().c_str();
    const gchar* style = _entry_style.getEntry()->get_text().c_str();
    const gchar* attr = _entry_attribute.getEntry()->get_text().c_str();
    
    GSList *in = l;
    GSList *out = NULL;
    if (strlen (text) != 0) {
        for (GSList *i = in; i != NULL; i = i->next) {
            if (item_text_match (SP_ITEM(i->data), text, exact)) {
                out = g_slist_prepend (out, i->data);
            }
        }
    } else {
        out = in;
    }
    
    in = out;
    out = NULL;
    if (strlen (id) != 0) {
        for (GSList *i = in; i != NULL; i = i->next) {
            if (item_id_match (SP_ITEM(i->data), id, exact)) {
                out = g_slist_prepend (out, i->data);
            }
        }
    } else {
        out = in;
    }

    in = out;
    out = NULL;
    if (strlen (style) != 0) {
        for (GSList *i = in; i != NULL; i = i->next) {
            if (item_style_match (SP_ITEM(i->data), style, exact)) {
                out = g_slist_prepend (out, i->data);
            }
        }
    } else {
        out = in;
    }

    in = out;
    out = NULL;
    if (strlen (attr) != 0) {
        for (GSList *i = in; i != NULL; i = i->next) {
            if (item_attr_match (SP_ITEM(i->data), attr, exact)) {
                out = g_slist_prepend (out, i->data);
            }
        }
    } else {
        out = in;
    }

    return out;
}


bool
Find::item_type_match (SPItem *item)
{
    SPDesktop *desktop = getDesktop();

    if (SP_IS_RECT(item)) {
        return (_check_all_shapes.get_active() || _check_rects.get_active());

    } else if (SP_IS_GENERICELLIPSE(item) || SP_IS_ELLIPSE(item) || SP_IS_ARC(item) || SP_IS_CIRCLE(item)) {
        return (_check_all_shapes.get_active() || _check_ellipses.get_active());

    } else if (SP_IS_STAR(item) || SP_IS_POLYGON(item)) {
        return (_check_all_shapes.get_active() || _check_stars.get_active());

    } else if (SP_IS_SPIRAL(item)) {
        return (_check_all_shapes.get_active() || _check_spirals.get_active());

    } else if (SP_IS_PATH(item) || SP_IS_LINE(item) || SP_IS_POLYLINE(item)) {
        return (_check_paths.get_active());

    } else if (SP_IS_TEXT(item) || SP_IS_TSPAN(item) || SP_IS_TREF(item) || SP_IS_STRING(item)) {
        return (_check_texts.get_active());

    } else if (SP_IS_GROUP(item) && !desktop->isLayer(item) ) { // never select layers!
        return (_check_groups.get_active());

    } else if (SP_IS_USE(item)) {
        return (_check_clones.get_active());

    } else if (SP_IS_IMAGE(item)) {
        return (_check_images.get_active());

    } else if (SP_IS_OFFSET(item)) {
        return (_check_offsets.get_active());
    }

    return false;
}

GSList *
Find::filter_types (GSList *l)
{
    if (_check_all.get_active()) return l;

    GSList *n = NULL;
    for (GSList *i = l; i != NULL; i = i->next) {
        if (item_type_match (SP_ITEM(i->data))) {
            n = g_slist_prepend (n, i->data);
        }
    }
    return n;
}


GSList *
Find::filter_list (GSList *l, bool exact)
{
    l = filter_fields (l, exact);
    l = filter_types (l);
    return l;
}

GSList *
Find::all_items (SPObject *r, GSList *l, bool hidden, bool locked)
{
    SPDesktop *desktop = getDesktop();

    if (SP_IS_DEFS(r))
        return l; // we're not interested in items in defs

    if (!strcmp (SP_OBJECT_REPR (r)->name(), "svg:metadata"))
        return l; // we're not interested in metadata

    for (SPObject *child = sp_object_first_child(r); child; child = SP_OBJECT_NEXT (child)) {
        if (SP_IS_ITEM (child) && !SP_OBJECT_IS_CLONED (child) && !desktop->isLayer(SP_ITEM(child))) {
                if ((hidden || !desktop->itemIsHidden(SP_ITEM(child))) && (locked || !SP_ITEM(child)->isLocked())) {
                    l = g_slist_prepend (l, child);
                }
        }
        l = all_items (child, l, hidden, locked);
    }
    return l;
}

GSList *
Find::all_selection_items (Inkscape::Selection *s, GSList *l, SPObject *ancestor, bool hidden, bool locked)
{
    SPDesktop *desktop = getDesktop();

    for (GSList *i = (GSList *) s->itemList(); i != NULL; i = i->next) {
        if (SP_IS_ITEM (i->data) && !SP_OBJECT_IS_CLONED (i->data) && !desktop->isLayer(SP_ITEM(i->data))) {
            if (!ancestor || ancestor->isAncestorOf(SP_OBJECT (i->data))) {
                if ((hidden || !desktop->itemIsHidden(SP_ITEM(i->data))) && (locked || !SP_ITEM(i->data)->isLocked())) {
                    l = g_slist_prepend (l, i->data);
                }
            }
        }
        if (!ancestor || ancestor->isAncestorOf(SP_OBJECT (i->data))) {
            l = all_items (SP_OBJECT (i->data), l, hidden, locked);
        }
    }
    return l;
}
                   
                   

/*########################################################################
# BUTTON CLICK HANDLERS    (callbacks)
########################################################################*/

void
Find::onClear()
{         
    _entry_text.getEntry()->set_text(Glib::ustring(""));
    _entry_id.getEntry()->set_text(Glib::ustring(""));
    _entry_style.getEntry()->set_text(Glib::ustring(""));
    _entry_attribute.getEntry()->set_text(Glib::ustring(""));

    _check_all.set_active();
}
    
    

void
Find::onFind()
{   
    SPDesktop *desktop = getDesktop();

    bool hidden = _check_include_hidden.get_active();
    bool locked = _check_include_locked.get_active();

    GSList *l = NULL;
    if (_check_search_selection.get_active()) {
        if (_check_search_layer.get_active()) {
            l = all_selection_items (desktop->selection, l, desktop->currentLayer(), hidden, locked);
        } else {
            l = all_selection_items (desktop->selection, l, NULL, hidden, locked);
        }
    } else {
        if (_check_search_layer.get_active()) {
            l = all_items (desktop->currentLayer(), l, hidden, locked);
        } else {
            l = all_items (SP_DOCUMENT_ROOT (sp_desktop_document (desktop)), l, hidden, locked);
        }
    }
    guint all = g_slist_length (l);

    bool exact = true;
    GSList *n = NULL;
    n = filter_list (l, exact);
    if (n == NULL) {
        exact = false;
        n = filter_list (l, exact);
    }

    if (n != NULL) {
        int count = g_slist_length (n);
        desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE,
                                        // TRANSLATORS: "%s" is replaced with "exact" or "partial" when this string is displayed
                                        ngettext("<b>%d</b> object found (out of <b>%d</b>), %s match.",
                                                 "<b>%d</b> objects found (out of <b>%d</b>), %s match.",
                                                 count),
                                        count, all, exact? _("exact") : _("partial"));

        Inkscape::Selection *selection = sp_desktop_selection (desktop);
        selection->clear();
        selection->setList(n);
        scroll_to_show_item (desktop, SP_ITEM(n->data));
    } else {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No objects found"));
    }
}

void
Find::onToggleAlltypes ()
{
    if (_check_all.get_active()) {
        // explicit toggle to make sure its handler gets called, no matter what was the original state
        _check_all_shapes.toggled();
        _check_all_shapes.set_active();
        _check_all_shapes.hide();
        _check_paths.hide();
        _check_texts.hide();
        _check_groups.hide();
        _check_clones.hide();
        _check_images.hide();
        _check_offsets.hide();
    } else {
        // explicit toggle to make sure its handler gets called, no matter what was the original state
        _check_all_shapes.toggled();
        _check_all_shapes.set_active();
        _check_all_shapes.show();

        _check_paths.set_active();
        _check_paths.show();
        _check_texts.set_active();
        _check_texts.show();
        _check_groups.set_active();
        _check_groups.show();
        _check_clones.set_active();
        _check_clones.show();
        _check_images.set_active();
        _check_images.show();
        _check_offsets.set_active();
        _check_offsets.show();
    }
    squeeze_window();
}

void
Find::onToggleShapes ()
{
    if (_check_all_shapes.get_active()) {
        _check_rects.hide();
        _check_ellipses.hide();
        _check_stars.hide();
        _check_spirals.hide();
    } else {
        _check_rects.set_active();
        _check_rects.show();
        _check_ellipses.set_active();
        _check_ellipses.show();
        _check_stars.set_active();
        _check_stars.show();
        _check_spirals.set_active();
        _check_spirals.show();
    }
    squeeze_window();
}


/*########################################################################
# UTILITY
########################################################################*/



void
Find::squeeze_window()
{
    // TO DO: make window as small as possible
}



} // namespace Dialog
} // namespace UI
} // namespace Inkscape

