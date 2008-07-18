/**
 * \brief SVG Fonts dialog
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPLv2 (or later).  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_SVG_FONTS_H
#define INKSCAPE_UI_DIALOG_SVG_FONTS_H

#include "ui/widget/panel.h"
#include "sp-font.h"
#include "verbs.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"

#include <gtkmm.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>

#include "display/nr-svgfonts.h"

class SvgFontDrawingArea : Gtk::DrawingArea{
public:
    SvgFontDrawingArea();
    void set_text(Glib::ustring);
    void set_svgfont(SvgFont*);
    void set_size(int x, int y);
    void redraw();
private:
    int x,y;
    SvgFont* svgfont;
    Glib::ustring text;
    bool on_expose_event (GdkEventExpose *event);
};

struct SPFont;

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

    static SvgFontsDialog &getInstance()
    { return *new SvgFontsDialog(); }

    void update_fonts();
    SvgFont* get_selected_svgfont();
    SPFont* get_selected_spfont();
    void on_font_selection_changed();
    void on_preview_text_changed();
    void on_glyphs_changed();
    void on_kerning_changed();
    void on_setwidth_changed();

private:
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
    Gtk::TreeView _font_list;
    Gtk::VBox _font_settings;
    Gtk::Entry _preview_entry;
    SvgFontDrawingArea _font_da, kerning_preview;
    GlyphComboBox first_glyph, second_glyph;
    SPGlyphKerning* kerning_pair;
    Gtk::SpinButton kerning_spin, setwidth_spin;

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
