#ifndef FONT_LISTER_H
#define FONT_LISTER_H

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2013 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treepath.h>
#include <glibmm/ustring.h>
#include "nr-type-primitives.h"

class SPObject;
class SPDocument;
class SPCSSAttr;
class SPStyle;

namespace Inkscape {

/**
 *  This class enumerates fonts using libnrtype into reusable data stores and
 *  allows for random access to the font-family list and the font-style list.
 *  Setting the font-family updates the font-style list. "Style" in this case
 *  refers to everything but family and size (e.g. italic/oblique, weight).
 *
 *  This class handles font-family lists and fonts that are not on the system,
 *  where there is not an entry in the fontInstanceMap.
 *
 *  This class uses the idea of "font_spec". This is a plain text string as used by
 *  Pango. It is similar to the CSS font shorthand except that font-family comes
 *  first and in this class the font-size is not used.
 *
 *  This class uses the FontFactory class to get a list of system fonts
 *  and to find best matches via Pango. The Pango interface is only setup
 *  to deal with fonts that are on the system so care must be taken. For
 *  example, best matches should only be done with the first font-family
 *  in a font-family list. If the first font-family is not on the system
 *  then a generic font-family should be used (sans-serif -> Sans).
 *
 *  This class is used by the UI interface (text-toolbar, font-select, etc.).
 *
 *  "Font" includes family and style. It should not be used when one
 *  means font-family.
 */

class FontLister {
public:
    enum Exceptions {
        FAMILY_NOT_FOUND,
        STYLE_NOT_FOUND
    };

    virtual ~FontLister();

    /**
     * GtkTreeModelColumnRecord for the font-family list Gtk::ListStore
     */
    struct FontListClass : public Gtk::TreeModelColumnRecord {
        /**
         * Column containing the family name
         */
        Gtk::TreeModelColumn<Glib::ustring> family;

        /**
         * Column containing the styles for each family name.
         */
        Gtk::TreeModelColumn<GList *> styles;

        /**
         * Column containing flag if font is on system
         */
        Gtk::TreeModelColumn<bool> onSystem;
        
        /**
         * Not actually a column.
         * Necessary for quick initialization of FontLister,
         * we initially store the pango family and if the
         * font style is actually used we'll cache it in
         * %styles.
         */
        Gtk::TreeModelColumn<PangoFontFamily *> pango_family;
        
        FontListClass()
        {
            add(family);
            add(styles);
            add(onSystem);
            add(pango_family);
        }
    };

    FontListClass FontList;

    struct FontStyleListClass : public Gtk::TreeModelColumnRecord {
        /**
         * Column containing the styles as Font designer used.
         */
        Gtk::TreeModelColumn<Glib::ustring> displayStyle;

        /**
         * Column containing the styles in CSS/Pango format.
         */
        Gtk::TreeModelColumn<Glib::ustring> cssStyle;

        FontStyleListClass()
        {
            add(cssStyle);
            add(displayStyle);
        }
    };

    FontStyleListClass FontStyleList;

    /** 
     * @return the ListStore with the family names
     *
     * The return is const and the function is declared as const.
     * The ListStore is ready to be used after class instantiation
     * and should not be modified.
     */
    const Glib::RefPtr<Gtk::ListStore> get_font_list() const;

    /**
     * @return the ListStore with the styles
     */
    const Glib::RefPtr<Gtk::ListStore> get_style_list() const;

    /** 
     * Inserts a font family or font-fallback list (for use when not
     *  already in document or on system).
     */
    void insert_font_family(Glib::ustring new_family);

    /**
     * Updates font list to include fonts in document.
     */
    void update_font_list(SPDocument *document);

public:
    static Inkscape::FontLister *get_instance();

    /**
     * Takes a hand written font spec and returns a Pango generated one in
     *  standard form.
     */
    Glib::ustring canonize_fontspec(Glib::ustring fontspec);

    /**
     * Find closest system font to given font.
     */
    Glib::ustring system_fontspec(Glib::ustring fontspec);

    /**
     * Gets font-family and style from fontspec.
     *  font-family and style returned.
     */
    std::pair<Glib::ustring, Glib::ustring> ui_from_fontspec(Glib::ustring fontspec);

    /**
     * Sets font-family and style after a selection change.
     *  New font-family and style returned.
     */
    std::pair<Glib::ustring, Glib::ustring> selection_update();

    /** 
     * Sets current_fontspec, etc. If check is false, won't
     *  try to find best style match (assumes style in fontspec
     *  valid for given font-family).
     */
    void set_fontspec(Glib::ustring fontspec, bool check = true);

    Glib::ustring get_fontspec()
    {
        return current_fontspec;
    }

    /**
     * Changes font-family, updating style list and attempting to find
     *  closest style to current_style style (if check_style is true).
     *  New font-family and style returned.
     *  Does NOT update current_family and current_style.
     *  (For potential use in font-selector which doesn't update until
     *  "Apply" button clicked.)
     */
    std::pair<Glib::ustring, Glib::ustring> new_font_family(Glib::ustring family, bool check_style = true);

    /** 
     * Sets font-family, updating style list and attempting
     *  to find closest style to old current_style.
     *  New font-family and style returned.
     *  Updates current_family and current_style.
     *  Calls new_font_family().
     *  (For use in text-toolbar where update is immediate.)
     */
    std::pair<Glib::ustring, Glib::ustring> set_font_family(Glib::ustring family, bool check_style = true);

    /**
     * Sets font-family from row in list store.
     *  The row can be used to determine if we are in the
     *  document or system part of the font-family list.
     *  This is needed to handle scrolling through the
     *  font-family list correctly.
     *  Calls set_font_family().
     */
    std::pair<Glib::ustring, Glib::ustring> set_font_family(int row, bool check_style = true);

    Glib::ustring get_font_family()
    {
        return current_family;
    }

    int get_font_family_row()
    {
        return current_family_row;
    }

    /**
     * Sets style. Does not validate style for family.
     */
    void set_font_style(Glib::ustring style);

    Glib::ustring get_font_style()
    {
        return current_style;
    }

    Glib::ustring fontspec_from_style(SPStyle *style);

    /**
     * Fill css using current_fontspec.
     */
    void fill_css(SPCSSAttr *css, Glib::ustring fontspec = "");

    Gtk::TreeModel::Row get_row_for_font(Glib::ustring family);

    Gtk::TreePath get_path_for_font(Glib::ustring family);

    Gtk::TreeModel::Row get_row_for_style(Glib::ustring style);

    Gtk::TreePath get_path_for_style(Glib::ustring style);

    std::pair<Gtk::TreePath, Gtk::TreePath> get_paths(Glib::ustring family, Glib::ustring style);

    /**
     * Return best style match for new font given style for old font.
     */
    Glib::ustring get_best_style_match(Glib::ustring family, Glib::ustring style);
    
    void ensureRowStyles(GtkTreeModel* model, GtkTreeIter const* iter);

private:
    FontLister();

    void update_font_list_recursive(SPObject *r, std::list<Glib::ustring> *l);

    Glib::RefPtr<Gtk::ListStore> font_list_store;
    Glib::RefPtr<Gtk::ListStore> style_list_store;

    /**
     * Info for currently selected font (what is shown in the UI).
     *  May include font-family lists and fonts not on system.
     */
    int current_family_row;
    Glib::ustring current_family;
    Glib::ustring current_style;
    Glib::ustring current_fontspec;

    /**
     * fontspec of system font closest to current_fontspec.
     *  (What the system will use to display current_fontspec.)
     */
    Glib::ustring current_fontspec_system;

    /**
     * If a font-family is not on system, this list of styles is used.
     */
    GList *default_styles;
};

} // namespace Inkscape

// Helper functions
gboolean font_lister_separator_func(GtkTreeModel *model,
                                    GtkTreeIter *iter,
                                    gpointer /*data*/);

void font_lister_cell_data_func(GtkCellLayout * /*cell_layout*/,
                                GtkCellRenderer *cell,
                                GtkTreeModel *model,
                                GtkTreeIter *iter,
                                gpointer /*data*/);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
