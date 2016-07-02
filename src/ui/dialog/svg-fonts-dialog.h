/** @file
 * @brief SVG Fonts dialog
 */
/* Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Authors
 * Released under GNU GPLv2 (or later).  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_SVG_FONTS_H
#define INKSCAPE_UI_DIALOG_SVG_FONTS_H

#include "ui/widget/panel.h"
#include <2geom/pathvector.h>
#include "ui/widget/spinbutton.h"

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/entry.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "attributes.h"
#include "xml/helper-observer.h"

namespace Gtk {
#if WITH_GTKMM_3_0
class Scale;
#else
class HScale;
#endif
}

class SPGlyph;
class SPGlyphKerning;
class SvgFont;

class SvgFontDrawingArea : Gtk::DrawingArea{
public:
    SvgFontDrawingArea();
    void set_text(Glib::ustring);
    void set_svgfont(SvgFont*);
    void set_size(int x, int y);
    void redraw();
private:
    int _x,_y;
    SvgFont* _svgfont;
    Glib::ustring _text;
    bool on_expose_event (GdkEventExpose *event);
};

class SPFont;

namespace Inkscape {
namespace UI {
namespace Dialog {

class GlyphComboBox : public Gtk::ComboBoxText {
public:
    GlyphComboBox();
    void update(SPFont*);
};

class SvgFontsDialog : public UI::Widget::Panel {
public:
    SvgFontsDialog();
    ~SvgFontsDialog();

    static SvgFontsDialog &getInstance() { return *new SvgFontsDialog(); }

    void update_fonts();
    SvgFont* get_selected_svgfont();
    SPFont* get_selected_spfont();
    SPGlyph* get_selected_glyph();
    SPGlyphKerning* get_selected_kerning_pair();

    //TODO: these methods should be private, right?!
    void on_font_selection_changed();
    void on_kerning_pair_selection_changed();
    void on_preview_text_changed();
    void on_kerning_pair_changed();
    void on_kerning_value_changed();
    void on_setfontdata_changed();
    void add_font();
    Geom::PathVector flip_coordinate_system(Geom::PathVector pathv);

    //TODO: AttrEntry is currently unused. Should we remove it?
    class AttrEntry : public Gtk::HBox
    {
    public:
        AttrEntry(SvgFontsDialog* d, gchar* lbl, const SPAttributeEnum attr);
        void set_text(char*);
    private:
        SvgFontsDialog* dialog;
        void on_attr_changed();
        Gtk::Entry entry;
        SPAttributeEnum attr;
    };

private:
    void update_glyphs();
    void update_sensitiveness();
    void update_global_settings_tab();
    void populate_glyphs_box();
    void populate_kerning_pairs_box();
    void set_glyph_description_from_selected_path();
    void missing_glyph_description_from_selected_path();
    void reset_missing_glyph_description();
    void add_glyph();
    void glyph_unicode_edit(const Glib::ustring&, const Glib::ustring&);
    void glyph_name_edit(const Glib::ustring&, const Glib::ustring&);
    void remove_selected_glyph();
    void remove_selected_font();
    void remove_selected_kerning_pair();

    void add_kerning_pair();

    void create_glyphs_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
    void glyphs_list_button_release(GdkEventButton* event);

    void create_fonts_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
    void fonts_list_button_release(GdkEventButton* event);

    void create_kerning_pairs_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
    void kerning_pairs_list_button_release(GdkEventButton* event);

    Inkscape::XML::SignalObserver _defs_observer; //in order to update fonts
    Inkscape::XML::SignalObserver _glyphs_observer;

    Gtk::HBox* AttrCombo(gchar* lbl, const SPAttributeEnum attr);
//    Gtk::HBox* AttrSpin(gchar* lbl, const SPAttributeEnum attr);
    Gtk::VBox* global_settings_tab();
    AttrEntry* _familyname_entry;

    Gtk::VBox* kerning_tab();
    Gtk::VBox* glyphs_tab();
    Gtk::Button _add;
    Gtk::Button add_glyph_button;
    Gtk::Button glyph_from_path_button;
    Gtk::Button missing_glyph_button;
    Gtk::Button missing_glyph_reset_button;

    class Columns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Columns()
	{
            add(spfont);
            add(svgfont);
            add(label);
	}

        Gtk::TreeModelColumn<SPFont*> spfont;
        Gtk::TreeModelColumn<SvgFont*> svgfont;
        Gtk::TreeModelColumn<Glib::ustring> label;
    };
    Glib::RefPtr<Gtk::ListStore> _model;
    Columns _columns;
    Gtk::TreeView _FontsList;

    class GlyphsColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        GlyphsColumns()
	{
            add(glyph_node);
            add(glyph_name);
            add(unicode);
	}

        Gtk::TreeModelColumn<SPGlyph*> glyph_node;
        Gtk::TreeModelColumn<Glib::ustring> glyph_name;
        Gtk::TreeModelColumn<Glib::ustring> unicode;
    };
    GlyphsColumns _GlyphsListColumns;
    Glib::RefPtr<Gtk::ListStore> _GlyphsListStore;
    Gtk::TreeView _GlyphsList;
    Gtk::ScrolledWindow _GlyphsListScroller;

    class KerningPairColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
      KerningPairColumns()
	{
	  add(first_glyph);
	  add(second_glyph);
	  add(kerning_value);
	  add(spnode);
	}

      Gtk::TreeModelColumn<Glib::ustring> first_glyph;
      Gtk::TreeModelColumn<Glib::ustring> second_glyph;
      Gtk::TreeModelColumn<double> kerning_value;
      Gtk::TreeModelColumn<SPGlyphKerning*> spnode;
    };
    KerningPairColumns _KerningPairsListColumns;
    Glib::RefPtr<Gtk::ListStore> _KerningPairsListStore;
    Gtk::TreeView _KerningPairsList;
    Gtk::ScrolledWindow _KerningPairsListScroller;
    Gtk::Button add_kernpair_button;

    Gtk::VBox _font_settings;
    Gtk::VBox global_vbox;
    Gtk::VBox glyphs_vbox;
    Gtk::VBox kerning_vbox;
    Gtk::Entry _preview_entry;

    Gtk::Menu _FontsContextMenu;
    Gtk::Menu _GlyphsContextMenu;
    Gtk::Menu _KerningPairsContextMenu;

    SvgFontDrawingArea _font_da, kerning_preview;
    GlyphComboBox first_glyph, second_glyph;
    SPGlyphKerning* kerning_pair;
    Inkscape::UI::Widget::SpinButton setwidth_spin;

#if WITH_GTKMM_3_0
    Gtk::Scale* kerning_slider;
#else
    Gtk::HScale* kerning_slider;
#endif

    class EntryWidget : public Gtk::HBox
    {
    public:
        EntryWidget()
	{
            this->add(this->_label);
            this->add(this->_entry);
	}
        void set_label(const gchar* l){
            this->_label.set_text(l);
        }
    private:
        Gtk::Label _label;
        Gtk::Entry _entry;
    };
    EntryWidget _font_family, _font_variant;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifndef INKSCAPE_UI_DIALOG_SVG_FONTS_H

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
