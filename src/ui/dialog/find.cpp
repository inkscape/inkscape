/*
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "find.h"

#include <gtkmm/entry.h>
#include <gtkmm/widget.h>

#include "verbs.h"

#include "message-stack.h"
#include "helper/window.h"
#include "macros.h"
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "selection.h"
#include "desktop-handles.h"

#include "ui/dialog-events.h"
#include "verbs.h"
#include "ui/interface.h"
#include "preferences.h"
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
#include "sp-root.h"
#include "xml/repr.h"
#include "xml/node-iterators.h"
#include "xml/attribute-record.h"

#include <glibmm/i18n.h>
#include <glibmm/regex.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

Find::Find()
    : UI::Widget::Panel("", "/dialogs/find", SP_VERB_DIALOG_FIND),

      entry_find(_("F_ind:"), _("Find objects by their content or properties (exact or partial match)")),
      entry_replace(_("R_eplace:"), _("Replace match with this value")),

      check_scope_all(_("_All"), _("Search in all layers")),
      check_scope_layer(_("Current _layer"), _("Limit search to the current layer")),
      check_scope_selection(_("Sele_ction"), _("Limit search to the current selection")),
      check_searchin_text(_("_Text"), _("Search in text objects")),
      check_searchin_property(_("_Properties"), _("Search in object properties, styles, attributes and IDs")),
      vbox_searchin(0, false),
      frame_searchin(_("Search in")),
      frame_scope(_("Scope")),

      check_case_sensitive(_("Case sensiti_ve"), _("Match upper/lower case"), false),
      check_exact_match(_("E_xact match"), _("Match whole objects only"), false),
      check_include_hidden(_("Include _hidden"), _("Include hidden objects in search"), false),
      check_include_locked(_("Include loc_ked"), _("Include locked objects in search"), false),
      expander_options(_("Options")),
      frame_options(_("General")),

      check_ids(_("_ID"), _("Search id name"), true),
      check_attributename(_("Attribute _name"), _("Search attribute name"), false),
      check_attributevalue(_("Attri_bute value"), _("Search attribute value"), true),
      check_style(_("_Style"), _("Search style"), true),
      check_font(_("F_ont"), _("Search fonts"), false),
      frame_properties(_("Properties")),

      check_alltypes(_("All types"), _("Search all object types"), true),
      check_rects(_("Rectangles"), _("Search rectangles"), false),
      check_ellipses(_("Ellipses"), _("Search ellipses, arcs, circles"), false),
      check_stars(_("Stars"), _("Search stars and polygons"), false),
      check_spirals(_("Spirals"), _("Search spirals"), false),
      check_paths(_("Paths"), _("Search paths, lines, polylines"), false),
      check_texts(_("Texts"), _("Search text objects"), false),
      check_groups(_("Groups"), _("Search groups"), false),
      check_clones(
                    //TRANSLATORS: "Clones" is a noun indicating type of object to find
                    C_("Find dialog", "Clones"), _("Search clones"), false),

      check_images(_("Images"), _("Search images"), false),
      check_offsets(_("Offsets"), _("Search offset objects"), false),
      frame_types(_("Object types")),

      status(""),
      button_find(_("_Find"), _("Select all objects matching the selection criteria")),
      button_replace(_("_Replace All"), _("Replace all matches")),
      _action_replace(false),
      blocked(false),
      desktop(NULL),
      deskTrack()

{
    entry_find.getEntry()->set_width_chars(25);
    entry_replace.getEntry()->set_width_chars(25);

    Gtk::RadioButtonGroup grp_searchin = check_searchin_text.get_group();
    check_searchin_property.set_group(grp_searchin);
    vbox_searchin.pack_start(check_searchin_text, false, false);
    vbox_searchin.pack_start(check_searchin_property, false, false);
    frame_searchin.add(vbox_searchin);

    Gtk::RadioButtonGroup grp_scope = check_scope_all.get_group();
    check_scope_layer.set_group(grp_scope);
    check_scope_selection.set_group(grp_scope);
    vbox_scope.pack_start(check_scope_all, true, true);
    vbox_scope.pack_start(check_scope_layer, true, true);
    vbox_scope.pack_start(check_scope_selection, true, true);
    frame_scope.add(vbox_scope);

    hbox_searchin.set_spacing(4);
    hbox_searchin.pack_start(frame_searchin, true, true);
    hbox_searchin.pack_start(frame_scope, true, true);

    vbox_options1.pack_start(check_case_sensitive, true, true);
    vbox_options1.pack_start(check_include_hidden, true, true);
    vbox_options2.pack_start(check_exact_match, true, true);
    vbox_options2.pack_start(check_include_locked, true, true);
    hbox_options.pack_start(vbox_options1, true, true, 4);
    hbox_options.pack_start(vbox_options2, true, true, 4);
    frame_options.add(hbox_options);

    hbox_properties1.set_homogeneous(false);
    hbox_properties1.pack_start(check_ids, false, false, 4 );
    hbox_properties1.pack_start(check_style, false, false, 8);
    hbox_properties1.pack_start(check_font, false, false, 8);
    hbox_properties2.set_homogeneous(false);
    hbox_properties2.pack_start(check_attributevalue, false, false, 4);
    hbox_properties2.pack_start(check_attributename, false, false, 4);
    vbox_properties.pack_start(hbox_properties1, true, true, 0);
    vbox_properties.pack_start(hbox_properties2, true, true, 2);
    frame_properties.add(vbox_properties);

    vbox_types1.pack_start(check_alltypes, true, true);
    vbox_types1.pack_start(check_paths, true, true);
    vbox_types1.pack_start(check_texts, true, true);
    vbox_types1.pack_start(check_groups, true, true);
    vbox_types1.pack_start(check_clones, true, true);
    vbox_types1.pack_start(check_images, true, true);
    vbox_types2.pack_start(check_offsets, true, true);
    vbox_types2.pack_start(check_rects, true, true);
    vbox_types2.pack_start(check_ellipses, true, true);
    vbox_types2.pack_start(check_stars, true, true);
    vbox_types2.pack_start(check_spirals, true, true);
    hbox_types.pack_start(vbox_types1, true, true, 4);
    hbox_types.pack_start(vbox_types2, true, true, 4);
    frame_types.add(hbox_types);

    vbox_expander.pack_start(frame_options, true, true, 4);
    vbox_expander.pack_start(frame_properties, true, true, 4);
    vbox_expander.pack_start(frame_types, true, true, 4);

    expander_options.set_use_underline();
    expander_options.add(vbox_expander);

    box_buttons.set_layout(Gtk::BUTTONBOX_END);
    box_buttons.set_spacing(4);
    box_buttons.pack_start(button_find, true, true, 6);
    box_buttons.pack_start(button_replace, true, true, 6);
    hboxbutton_row.pack_start(status, true, true, 6);
    hboxbutton_row.pack_end(box_buttons, true, true);

    Gtk::Box *contents = _getContents();
    contents->set_spacing(6);
    contents->pack_start(entry_find, false, false);
    contents->pack_start(entry_replace, false, false);
    contents->pack_start(hbox_searchin, false, false);
    contents->pack_start(expander_options, false, false);
    contents->pack_end(hboxbutton_row, false, false);

    checkProperties.push_back(&check_ids);
    checkProperties.push_back(&check_style);
    checkProperties.push_back(&check_font);
    checkProperties.push_back(&check_attributevalue);
    checkProperties.push_back(&check_attributename);

    checkTypes.push_back(&check_paths);
    checkTypes.push_back(&check_texts);
    checkTypes.push_back(&check_groups);
    checkTypes.push_back(&check_clones);
    checkTypes.push_back(&check_images);
    checkTypes.push_back(&check_offsets);
    checkTypes.push_back(&check_rects);
    checkTypes.push_back(&check_ellipses);
    checkTypes.push_back(&check_stars);
    checkTypes.push_back(&check_spirals);
    checkTypes.push_back(&check_offsets);

    // set signals to handle clicks
    expander_options.property_expanded().signal_changed().connect(sigc::mem_fun(*this, &Find::onExpander));
    button_find.signal_clicked().connect(sigc::mem_fun(*this, &Find::onFind));
    button_replace.signal_clicked().connect(sigc::mem_fun(*this, &Find::onReplace));
    check_searchin_text.signal_clicked().connect(sigc::mem_fun(*this, &Find::onSearchinText));
    check_searchin_property.signal_clicked().connect(sigc::mem_fun(*this, &Find::onSearchinProperty));
    check_alltypes.signal_clicked().connect(sigc::mem_fun(*this, &Find::onToggleAlltypes));

    for(size_t i = 0; i < checkProperties.size(); i++) {
        checkProperties[i]->signal_clicked().connect(sigc::mem_fun(*this, &Find::onToggleCheck));
    }

    for(size_t i = 0; i < checkTypes.size(); i++) {
        checkTypes[i]->signal_clicked().connect(sigc::mem_fun(*this, &Find::onToggleCheck));
    }

    onSearchinText();
    onToggleAlltypes();

    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &Find::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children();

    Inkscape::Selection *selection = sp_desktop_selection (SP_ACTIVE_DESKTOP);
    SPItem *item = selection->singleItem();
    if (item) {
        if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
            gchar *str;
            str = sp_te_get_string_multiline (item);
            entry_find.getEntry()->set_text(str);
        }
    }

    button_find.set_can_default();
    //button_find.grab_default(); // activatable by Enter
    entry_find.getEntry()->grab_focus();
}

Find::~Find()
{
    desktopChangeConn.disconnect();
    selectChangedConn.disconnect();
    deskTrack.disconnect();
}

void Find::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void Find::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (this->desktop) {
            selectChangedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &Find::onSelectionChange)));
        }
    }
}

void Find::onSelectionChange(void)
{
    if (!blocked) {
        status.set_text("");
    }
}

/*########################################################################
# FIND helper functions
########################################################################*/

Glib::ustring Find::find_replace(const gchar *str, const gchar *find, const gchar *replace, bool exact, bool casematch, bool replaceall)
{
    Glib::ustring ustr = str;
    Glib::ustring ufind = find;
    if (!casematch) {
        ufind = ufind.lowercase();
    }
    gsize n = find_strcmp_pos(ustr.c_str(), ufind.c_str(), exact, casematch);
    while (n != std::string::npos) {
        ustr.replace(n, ufind.length(), replace);
        if (!replaceall) {
            return ustr;
        }
        // Start the next search after the last replace character to avoid infinite loops (replace "a" with "aaa" etc)
        n = find_strcmp_pos(ustr.c_str(), ufind.c_str(), exact, casematch, n + strlen(replace) + 1);
    }
    return ustr;
}

gsize Find::find_strcmp_pos(const gchar *str, const gchar *find, bool exact, bool casematch, gsize start/*=0*/)
{
    Glib::ustring ustr = str;
    Glib::ustring ufind = find;

    if (!casematch) {
        ustr = ustr.lowercase();
        ufind = ufind.lowercase();
    }

    gsize pos = std::string::npos;
    if (exact) {
        if (ustr == ufind) {
            pos = 0;
        }
    } else {
        pos = ustr.find(ufind, start);
    }

    return pos;
}


bool Find::find_strcmp(const gchar *str, const gchar *find, bool exact, bool casematch)
{
    return (std::string::npos != find_strcmp_pos(str, find, exact, casematch));
}

bool Find::item_text_match (SPItem *item, const gchar *find, bool exact, bool casematch, bool replace/*=false*/)
{
    if (item->getRepr() == NULL) {
        return false;
    }

    if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
        const gchar *item_text = sp_te_get_string_multiline (item);
        if (item_text == NULL) {
            return false;
        }
        bool found = find_strcmp(item_text, find, exact, casematch);

        if (found && replace) {
            Glib::ustring ufind = find;
            if (!casematch) {
                ufind = ufind.lowercase();
            }

            Inkscape::Text::Layout const *layout = te_get_layout (item);
            if (!layout) {
                return found;
            }

            gchar* replace_text  = g_strdup(entry_replace.getEntry()->get_text().c_str());
            gsize n = find_strcmp_pos(item_text, ufind.c_str(), exact, casematch);
            static Inkscape::Text::Layout::iterator _begin_w;
            static Inkscape::Text::Layout::iterator _end_w;
            while (n != std::string::npos) {
                _begin_w = layout->charIndexToIterator(n);
                _end_w = layout->charIndexToIterator(n + strlen(find));
                sp_te_replace(item, _begin_w, _end_w, replace_text);
                item_text = sp_te_get_string_multiline (item);
                n = find_strcmp_pos(item_text, ufind.c_str(), exact, casematch, n + strlen(replace_text) + 1);
            }

            g_free(replace_text);
        }

        return found;
    }
    return false;
}


bool Find::item_id_match (SPItem *item, const gchar *id, bool exact, bool casematch, bool replace/*=false*/)
{
    if (item->getRepr() == NULL) {
        return false;
    }

    if (SP_IS_STRING(item)) { // SPStrings have "on demand" ids which are useless for searching
        return false;
    }

    const gchar *item_id = item->getRepr()->attribute("id");
    if (item_id == NULL) {
        return false;
    }

    bool found = find_strcmp(item_id, id, exact, casematch);

    if (found && replace) {
        gchar * replace_text  = g_strdup(entry_replace.getEntry()->get_text().c_str());
        Glib::ustring new_item_style = find_replace(item_id, id, replace_text , exact, casematch, true);
        if (new_item_style != item_id) {
            item->getRepr()->setAttribute("id", new_item_style.data());
        }
        g_free(replace_text);
    }

    return found;
}

bool Find::item_style_match (SPItem *item, const gchar *text, bool exact, bool casematch, bool replace/*=false*/)
{
    if (item->getRepr() == NULL) {
        return false;
    }

    gchar *item_style = g_strdup(item->getRepr()->attribute("style"));
    if (item_style == NULL) {
        return false;
    }

    bool found = find_strcmp(item_style, text, exact, casematch);

    if (found && replace) {
        gchar * replace_text  = g_strdup(entry_replace.getEntry()->get_text().c_str());
        Glib::ustring new_item_style = find_replace(item_style, text, replace_text , exact, casematch, true);
        if (new_item_style != item_style) {
            item->getRepr()->setAttribute("style", new_item_style.data());
        }
        g_free(replace_text);
    }

    g_free(item_style);
    return found;
}

bool Find::item_attr_match(SPItem *item, const gchar *text, bool exact, bool /*casematch*/, bool replace/*=false*/)
{
    bool found = false;

    if (item->getRepr() == NULL) {
        return false;
    }

    gchar *attr_value = g_strdup(item->getRepr()->attribute(text));
    if (exact) {
        found =  (attr_value != NULL);
    } else {
        found = item->getRepr()->matchAttributeName(text);
    }
    g_free(attr_value);

    // TODO - Rename attribute name ?
    if (found && replace) {
        found = false;
    }

    return found;
}

bool Find::item_attrvalue_match(SPItem *item, const gchar *text, bool exact, bool casematch, bool replace/*=false*/)
{
    bool ret = false;

    if (item->getRepr() == NULL) {
        return false;
    }

    Inkscape::Util::List<Inkscape::XML::AttributeRecord const> iter = item->getRepr()->attributeList();
    for (; iter; ++iter) {
        const gchar* key = g_quark_to_string(iter->key);
        gchar *attr_value = g_strdup(item->getRepr()->attribute(key));
        bool found = find_strcmp(attr_value, text, exact, casematch);
        if (found) {
            ret = true;
        }

        if (found && replace) {
            gchar * replace_text  = g_strdup(entry_replace.getEntry()->get_text().c_str());
            Glib::ustring new_item_style = find_replace(attr_value, text, replace_text , exact, casematch, true);
            if (new_item_style != attr_value) {
                item->getRepr()->setAttribute(key, new_item_style.data());
            }
        }

        g_free(attr_value);
    }

    return ret;
}


bool Find::item_font_match(SPItem *item, const gchar *text, bool exact, bool casematch, bool /*replace*/ /*=false*/)
{
    bool ret = false;

    if (item->getRepr() == NULL) {
        return false;
    }

    const gchar *item_style = item->getRepr()->attribute("style");
    if (item_style == NULL) {
        return false;
    }

    std::vector<Glib::ustring> vFontTokenNames;
    vFontTokenNames.push_back("font-family:");
    vFontTokenNames.push_back("-inkscape-font-specification:");

    std::vector<Glib::ustring> vStyleTokens = Glib::Regex::split_simple(";", item_style);
    for(size_t i=0; i<vStyleTokens.size(); i++) {
        Glib::ustring token = vStyleTokens[i];
        for(size_t j=0; j<vFontTokenNames.size(); j++) {
            if ( token.find(vFontTokenNames[j]) != std::string::npos) {
                Glib::ustring font1 = Glib::ustring(vFontTokenNames[j]).append(text);
                bool found = find_strcmp(token.c_str(), font1.c_str(), exact, casematch);
                if (found) {
                    ret = true;
                    if (_action_replace) {
                        gchar *replace_text  = g_strdup(entry_replace.getEntry()->get_text().c_str());
                        gchar *orig_str = g_strdup(token.c_str());
                        // Exact match fails since the "font-family:" is in the token, since the find was exact it still works with false below
                        Glib::ustring new_item_style = find_replace(orig_str, text, replace_text , false /*exact*/, casematch, true);
                        if (new_item_style != orig_str) {
                            vStyleTokens.at(i) = new_item_style;
                        }
                        g_free(orig_str);
                        g_free(replace_text);
                    }
                }
            }
        }
    }

    if (ret && _action_replace) {
        Glib::ustring new_item_style;
        for(size_t i=0; i<vStyleTokens.size(); i++) {
            new_item_style.append(vStyleTokens.at(i)).append(";");
        }
        new_item_style.erase(new_item_style.size()-1);
        item->getRepr()->setAttribute("style", new_item_style.data());
    }

    return ret;
}


GSList *Find::filter_fields (GSList *l, bool exact, bool casematch)
{
    Glib::ustring tmp = entry_find.getEntry()->get_text();
    if (tmp.empty()) {
        return l;
    }
    gchar* text = g_strdup(tmp.c_str());

    GSList *in = l;
    GSList *out = NULL;

    if (check_searchin_text.get_active()) {
        for (GSList *i = in; i != NULL; i = i->next) {
            if (item_text_match (SP_ITEM(i->data), text, exact, casematch)) {
                if (!g_slist_find(out, i->data)) {
                    out = g_slist_prepend (out, i->data);
                    if (_action_replace) {
                        item_text_match (SP_ITEM(i->data), text, exact, casematch, _action_replace);
                    }
                }
            }
        }
    }
    else if (check_searchin_property.get_active()) {

        bool ids = check_ids.get_active();
        bool style = check_style.get_active();
        bool font = check_font.get_active();
        bool attrname  = check_attributename.get_active();
        bool attrvalue = check_attributevalue.get_active();

        if (ids) {
            for (GSList *i = in; i != NULL; i = i->next) {
                if (item_id_match (SP_ITEM(i->data), text, exact, casematch)) {
                    if (!g_slist_find(out, i->data)) {
                        out = g_slist_prepend (out, i->data);
                        if (_action_replace) {
                            item_id_match (SP_ITEM(i->data), text, exact, casematch, _action_replace);
                        }
                    }
                }
            }
        }


        if (style) {
            for (GSList *i = in; i != NULL; i = i->next) {
                if (item_style_match (SP_ITEM(i->data), text, exact, casematch)) {
                    if (!g_slist_find(out, i->data))
                        if (!g_slist_find(out, i->data)) {
                            out = g_slist_prepend (out, i->data);
                            if (_action_replace) {
                                item_style_match (SP_ITEM(i->data), text, exact, casematch, _action_replace);
                            }
                        }
                }
            }
        }


        if (attrname) {
            for (GSList *i = in; i != NULL; i = i->next) {
                if (item_attr_match (SP_ITEM(i->data), text, exact, casematch)) {
                    if (!g_slist_find(out, i->data)) {
                        out = g_slist_prepend (out, i->data);
                        if (_action_replace) {
                            item_attr_match (SP_ITEM(i->data), text, exact, casematch, _action_replace);
                        }
                    }
                }
            }
        }


        if (attrvalue) {
            for (GSList *i = in; i != NULL; i = i->next) {
                if (item_attrvalue_match (SP_ITEM(i->data), text, exact, casematch)) {
                    if (!g_slist_find(out, i->data)) {
                        out = g_slist_prepend (out, i->data);
                        if (_action_replace) {
                            item_attrvalue_match (SP_ITEM(i->data), text, exact, casematch, _action_replace);
                        }
                    }
                }
            }
        }


        if (font) {
            for (GSList *i = in; i != NULL; i = i->next) {
                if (item_font_match (SP_ITEM(i->data), text, exact, casematch)) {
                    if (!g_slist_find(out, i->data)) {
                        out = g_slist_prepend (out, i->data);
                        if (_action_replace) {
                            item_font_match (SP_ITEM(i->data), text, exact, casematch, _action_replace);
                        }
                    }
                }
            }
        }

    }

    g_free(text);

    return out;
}


bool Find::item_type_match (SPItem *item)
{
    bool all  =check_alltypes.get_active();

    if ( SP_IS_RECT(item)) {
        return ( all ||check_rects.get_active());

    } else if (SP_IS_GENERICELLIPSE(item)) {
        return ( all ||  check_ellipses.get_active());

    } else if (SP_IS_STAR(item) || SP_IS_POLYGON(item)) {
        return ( all || check_stars.get_active());

    } else if (SP_IS_SPIRAL(item)) {
        return ( all || check_spirals.get_active());

    } else if (SP_IS_PATH(item) || SP_IS_LINE(item) || SP_IS_POLYLINE(item)) {
        return (all || check_paths.get_active());

    } else if (SP_IS_TEXT(item) || SP_IS_TSPAN(item) || SP_IS_TREF(item) || SP_IS_STRING(item)) {
        return (all || check_texts.get_active());

    } else if (SP_IS_GROUP(item) && !desktop->isLayer(item) ) { // never select layers!
        return (all || check_groups.get_active());

    } else if (SP_IS_USE(item)) {
        return (all || check_clones.get_active());

    } else if (SP_IS_IMAGE(item)) {
        return (all || check_images.get_active());

    } else if (SP_IS_OFFSET(item)) {
        return (all || check_offsets.get_active());
    }

    return false;
}

GSList *Find::filter_types (GSList *l)
{
    GSList *n = NULL;
    for (GSList *i = l; i != NULL; i = i->next) {
        if (item_type_match (SP_ITEM(i->data))) {
            n = g_slist_prepend (n, i->data);
        }
    }
    return n;
}


GSList *Find::filter_list (GSList *l, bool exact, bool casematch)
{
    l = filter_types (l);
    l = filter_fields (l, exact, casematch);
    return l;
}

GSList *Find::all_items (SPObject *r, GSList *l, bool hidden, bool locked)
{
    if (SP_IS_DEFS(r)) {
        return l; // we're not interested in items in defs
    }

    if (!strcmp(r->getRepr()->name(), "svg:metadata")) {
        return l; // we're not interested in metadata
    }

    for (SPObject *child = r->firstChild(); child; child = child->getNext()) {
        if (SP_IS_ITEM(child) && !child->cloned && !desktop->isLayer(SP_ITEM(child))) {
            SPItem *item = reinterpret_cast<SPItem *>(child);
            if ((hidden || !desktop->itemIsHidden(item)) && (locked || !item->isLocked())) {
                l = g_slist_prepend (l, child);
            }
        }
        l = all_items (child, l, hidden, locked);
    }
    return l;
}

GSList *Find::all_selection_items (Inkscape::Selection *s, GSList *l, SPObject *ancestor, bool hidden, bool locked)
{
    for (GSList *i = (GSList *) s->itemList(); i != NULL; i = i->next) {
        if (SP_IS_ITEM (i->data) && !reinterpret_cast<SPItem *>(i->data)->cloned && !desktop->isLayer(SP_ITEM(i->data))) {
            SPItem *item = reinterpret_cast<SPItem *>(i->data);
            if (!ancestor || ancestor->isAncestorOf(item)) {
                if ((hidden || !desktop->itemIsHidden(item)) && (locked || !item->isLocked())) {
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

void Find::onFind()
{
    _action_replace = false;
    onAction();

    // Return focus to the find entry
    entry_find.getEntry()->grab_focus();
}

void Find::onReplace()
{
    if (entry_find.getEntry()->get_text().length() < 1) {
        status.set_text(_("Nothing to replace"));
        return;
    }
    _action_replace = true;
    onAction();

    // Return focus to the find entry
    entry_find.getEntry()->grab_focus();
}

void Find::onAction()
{

    bool hidden = check_include_hidden.get_active();
    bool locked = check_include_locked.get_active();
    bool exact = check_exact_match.get_active();
    bool casematch = check_case_sensitive.get_active();
    blocked = true;

    GSList *l = NULL;
    if (check_scope_selection.get_active()) {
        if (check_scope_layer.get_active()) {
            l = all_selection_items (desktop->selection, l, desktop->currentLayer(), hidden, locked);
        } else {
            l = all_selection_items (desktop->selection, l, NULL, hidden, locked);
        }
    } else {
        if (check_scope_layer.get_active()) {
            l = all_items (desktop->currentLayer(), l, hidden, locked);
        } else {
            l = all_items(sp_desktop_document(desktop)->getRoot(), l, hidden, locked);
        }
    }
    guint all = g_slist_length (l);

    GSList *n = filter_list (l, exact, casematch);

    if (n != NULL) {
        int count = g_slist_length (n);
        desktop->messageStack()->flashF(Inkscape::NORMAL_MESSAGE,
                                        // TRANSLATORS: "%s" is replaced with "exact" or "partial" when this string is displayed
                                        ngettext("<b>%d</b> object found (out of <b>%d</b>), %s match.",
                                                 "<b>%d</b> objects found (out of <b>%d</b>), %s match.",
                                                 count),
                                        count, all, exact? _("exact") : _("partial"));
        if (_action_replace){
            // TRANSLATORS: "%1" is replaced with the number of matches
            status.set_text(Glib::ustring::compose(ngettext("%1 match replaced","%1 matches replaced",count), count));
        }
        else {
            // TRANSLATORS: "%1" is replaced with the number of matches
            status.set_text(Glib::ustring::compose(ngettext("%1 object found","%1 objects found",count), count));
            bool attributenameyok = !check_attributename.get_active();
            button_replace.set_sensitive(attributenameyok);
        }

        Inkscape::Selection *selection = sp_desktop_selection (desktop);
        selection->clear();
        selection->setList(n);
        scroll_to_show_item (desktop, SP_ITEM(n->data));

        if (_action_replace) {
            DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_TEXT, _("Replace text or property"));
        }

    } else {
        status.set_text(_("Nothing found"));
        if (!check_scope_selection.get_active()) {
            Inkscape::Selection *selection = sp_desktop_selection (desktop);
            selection->clear();
        }
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No objects found"));
    }
    blocked = false;

}

void Find::onToggleCheck ()
{
    bool objectok = false;
    status.set_text("");

    if (check_alltypes.get_active()) {
        objectok = true;
    }
    for(int i = 0; i < 11; i++) {
        if (checkTypes[i]->get_active()) {
            objectok = true;
        }
    }

    if (!objectok) {
        status.set_text(_("Select an object type"));
    }


    bool propertyok = false;

    if (!check_searchin_property.get_active()) {
        propertyok = true;
    } else {

        for(size_t i = 0; i < checkProperties.size(); i++) {
            if (checkProperties[i]->get_active()) {
                propertyok = true;
            }
        }
    }

    if (!propertyok) {
        status.set_text(_("Select a property"));
    }

    // Can't replace attribute names
    // bool attributenameyok = !check_attributename.get_active();

    button_find.set_sensitive(objectok && propertyok);
    // button_replace.set_sensitive(objectok && propertyok && attributenameyok);
    button_replace.set_sensitive(false);
}

void Find::onToggleAlltypes ()
{
     bool all  =check_alltypes.get_active();
     for(size_t i = 0; i < checkTypes.size(); i++) {
         checkTypes[i]->set_sensitive(!all);
     }

     onToggleCheck();
}

void Find::onSearchinText ()
{
    searchinToggle(false);
    onToggleCheck();
}

void Find::onSearchinProperty ()
{
    searchinToggle(true);
    onToggleCheck();
}

void Find::searchinToggle(bool on)
{
    for(size_t i = 0; i < checkProperties.size(); i++) {
        checkProperties[i]->set_sensitive(on);
    }
}

void Find::onExpander ()
{
    if (!expander_options.get_expanded())
        squeeze_window();
}

/*########################################################################
# UTILITY
########################################################################*/

void Find::squeeze_window()
{
    // TODO: resize dialog window when the expander is closed
    // set_size_request(-1, -1);
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape


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
