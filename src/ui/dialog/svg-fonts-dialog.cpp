/**
 * \brief SVG Fonts dialog
 *
 * Authors:
 *   Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPLv2 (or later).  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS

#include <gtkmm/notebook.h>
#include "svg-fonts-dialog.h"
#include <glibmm/i18n.h>
#include <string.h>

SvgFontDrawingArea::SvgFontDrawingArea(){
	this->text = "";
	this->svgfont = NULL;
}

void SvgFontDrawingArea::set_svgfont(SvgFont* svgfont){
	this->svgfont = svgfont;
}

void SvgFontDrawingArea::set_text(Glib::ustring text){
	this->text = text;
}

void SvgFontDrawingArea::set_size(int x, int y){
    this->x = x;
    this->y = y;
    ((Gtk::Widget*) this)->set_size_request(x, y);
}

void SvgFontDrawingArea::redraw(){
	((Gtk::Widget*) this)->queue_draw();
}

bool SvgFontDrawingArea::on_expose_event (GdkEventExpose *event){
  if (this->svgfont){
    Glib::RefPtr<Gdk::Window> window = get_window();
    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    cr->set_font_face( Cairo::RefPtr<Cairo::FontFace>(new Cairo::FontFace(this->svgfont->get_font_face(), false /* does not have reference */)) );
    cr->set_font_size (this->y-20);
    cr->move_to (10, 10);
    cr->show_text (this->text.c_str());
  }
  return TRUE;
}

namespace Inkscape {
namespace UI {
namespace Dialog {

Gtk::HBox* SvgFontsDialog::AttrEntry(gchar* lbl){
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->add(* Gtk::manage(new Gtk::Label(lbl)) );
    hbox->add(* Gtk::manage(new Gtk::Entry()) );
    hbox->show_all();
    return hbox;
}

Gtk::HBox* SvgFontsDialog::AttrCombo(gchar* lbl){
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->add(* Gtk::manage(new Gtk::Label(lbl)) );
    hbox->add(* Gtk::manage(new Gtk::ComboBox()) );
    hbox->show_all();
    return hbox;
}

/*
Gtk::HBox* SvgFontsDialog::AttrSpin(gchar* lbl){
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->add(* Gtk::manage(new Gtk::Label(lbl)) );
    hbox->add(* Gtk::manage(new Gtk::SpinBox()) );
    hbox->show_all();
    return hbox;
}*/

/*** SvgFontsDialog ***/

GlyphComboBox::GlyphComboBox(){
}

void GlyphComboBox::update(SPFont* spfont){
    if (spfont) {
        this->clear();
        for(SPObject* node = spfont->children; node; node=node->next){
            if (SP_IS_GLYPH(node)){
                this->append_text(((SPGlyph*)node)->unicode);
            }
        }
    }
}

void SvgFontsDialog::on_kerning_changed(){
    if (this->kerning_pair){
        this->kerning_pair->k = kerning_spin.get_value();
        kerning_preview.redraw();
        _font_da.redraw();
    }
}

void SvgFontsDialog::on_glyphs_changed(){
    std::string str1(first_glyph.get_active_text());
    std::string str2(second_glyph.get_active_text());
    kerning_preview.set_text((gchar*) (str1+str2).c_str());
    kerning_preview.redraw();


    //look for this kerning pair on the currently selected font
    this->kerning_pair = NULL;
    for(SPObject* node = this->get_selected_spfont()->children; node; node=node->next){
        if (SP_IS_HKERN(node) && ((SPGlyphKerning*)node)->u1->contains((gchar) first_glyph.get_active_text().c_str()[0])
                                  && ((SPGlyphKerning*)node)->u2->contains((gchar) second_glyph.get_active_text().c_str()[0]) ){
            this->kerning_pair = (SPGlyphKerning*)node;
            continue;
        }
    }

//TODO:
    //if not found,
      //create new kern node
    if (this->kerning_pair)
        kerning_spin.set_value(this->kerning_pair->k);
}

/* Add all fonts in the document to the combobox. */
void SvgFontsDialog::update_fonts()
{
    SPDesktop* desktop = this->getDesktop();
    SPDocument* document = sp_desktop_document(desktop);
    const GSList* fonts = sp_document_get_resource_list(document, "font");

    _model->clear();
    for(const GSList *l = fonts; l; l = l->next) {
        Gtk::TreeModel::Row row = *_model->append();
        SPFont* f = (SPFont*)l->data;
        row[_columns.spfont] = f;
        row[_columns.svgfont] = new SvgFont(f);
        const gchar* lbl = f->label();
        const gchar* id = SP_OBJECT_ID(f);
        row[_columns.label] = lbl ? lbl : (id ? id : "font");
    }
}

void SvgFontsDialog::on_preview_text_changed(){
    _font_da.set_text((gchar*) _preview_entry.get_text().c_str());
    _font_da.set_text(_preview_entry.get_text());
    _font_da.redraw();
}

void SvgFontsDialog::on_font_selection_changed(){
    SPFont* spfont = this->get_selected_spfont();
    SvgFont* svgfont = this->get_selected_svgfont();
    first_glyph.update(spfont);
    second_glyph.update(spfont);
    kerning_preview.set_svgfont(svgfont);
    _font_da.set_svgfont(svgfont);
    _font_da.redraw();

    int steps = 50;
    double set_width = spfont->horiz_adv_x;
    setwidth_spin.set_value(set_width);

    kerning_spin.set_range(0,set_width);
    kerning_spin.set_increments(int(set_width/steps),2*int(set_width/steps));
    kerning_spin.set_value(0);
}

void SvgFontsDialog::on_setwidth_changed(){
    SPFont* spfont = this->get_selected_spfont();
    if (spfont){
        spfont->horiz_adv_x = setwidth_spin.get_value();
        //TODO: tell cairo that the glyphs cache has to be invalidated
    }
}

SvgFont* SvgFontsDialog::get_selected_svgfont()
{
    Gtk::TreeModel::iterator i = _font_list.get_selection()->get_selected();
    if(i)
        return (*i)[_columns.svgfont];
    return NULL;
}

SPFont* SvgFontsDialog::get_selected_spfont()
{
    Gtk::TreeModel::iterator i = _font_list.get_selection()->get_selected();
    if(i)
        return (*i)[_columns.spfont];
    return NULL;
}

Gtk::VBox* SvgFontsDialog::global_settings_tab(){
    Gtk::VBox* global_vbox = Gtk::manage(new Gtk::VBox());

    global_vbox->add(*AttrEntry((gchar*) "Family Name"));
    global_vbox->add(*AttrCombo((gchar*) "Style"));
    global_vbox->add(*AttrCombo((gchar*) "Variant"));
    global_vbox->add(*AttrCombo((gchar*) "Weight"));

//Set Width (horiz_adv_x):
    Gtk::HBox* setwidth_hbox = Gtk::manage(new Gtk::HBox());
    setwidth_hbox->add(*Gtk::manage(new Gtk::Label(_("Set width:"))));
    setwidth_hbox->add(setwidth_spin);

    setwidth_spin.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_setwidth_changed));
    setwidth_spin.set_range(0, 4096);
    setwidth_spin.set_increments(10, 100);
    global_vbox->add(*setwidth_hbox);

    return global_vbox;
}

Gtk::VBox* SvgFontsDialog::glyphs_tab(){
    Gtk::VBox* glyphs_vbox = Gtk::manage(new Gtk::VBox());
    glyphs_vbox->add(*AttrEntry((gchar*) "Glyph Name"));
    glyphs_vbox->add(*AttrEntry((gchar*) "Unicode"));
    //glyphs_vbox->add(*AttrSpin((gchar*) "Horizontal Advance"));
    //glyphs_vbox->add(*AttrCombo((gchar*) "Missing Glyph"));
    return glyphs_vbox;
}

Gtk::VBox* SvgFontsDialog::kerning_tab(){

//kerning setup:
    Gtk::VBox* kernvbox = Gtk::manage(new Gtk::VBox());

//Kerning Setup:
    kernvbox->add(*Gtk::manage(new Gtk::Label(_("Kerning Setup:"))));
    Gtk::HBox* kerning_selector = Gtk::manage(new Gtk::HBox());
    kerning_selector->add(*Gtk::manage(new Gtk::Label(_("1st Glyph:"))));
    kerning_selector->add(first_glyph);
    kerning_selector->add(*Gtk::manage(new Gtk::Label(_("2nd Glyph:"))));
    kerning_selector->add(second_glyph);
    first_glyph.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_glyphs_changed));
    second_glyph.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_glyphs_changed));
    kerning_spin.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_kerning_changed));

    kernvbox->add(*kerning_selector);
    kernvbox->add((Gtk::Widget&) kerning_preview);

    Gtk::HBox* kerning_amount_hbox = Gtk::manage(new Gtk::HBox());
    kernvbox->add(*kerning_amount_hbox);
    kerning_amount_hbox->add(*Gtk::manage(new Gtk::Label(_("Kerning value:"))));
    kerning_amount_hbox->add(kerning_spin);

    kerning_preview.set_size(300 + 20, 150 + 20);
    _font_da.set_size(150 + 20, 50 + 20);

    return kernvbox;
}

SvgFontsDialog::SvgFontsDialog()
 : UI::Widget::Panel("", "dialogs.svgfonts", SP_VERB_DIALOG_SVG_FONTS)
{
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());
    hbox->add(_font_list);
    hbox->add(_font_settings);
    _getContents()->add(*hbox);

//List of SVGFonts declared in a document:
    _model = Gtk::ListStore::create(_columns);
    _font_list.set_model(_model);
    _font_list.append_column_editable(_("_Font"), _columns.label);
    _font_list.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_font_selection_changed));

    this->update_fonts();

    Gtk::Notebook *tabs = Gtk::manage(new Gtk::Notebook());
    tabs->set_scrollable();

    tabs->append_page(*global_settings_tab(), _("_Global Settings"), true);
    tabs->append_page(*glyphs_tab(), _("_Glyphs"), true);
    tabs->append_page(*kerning_tab(), _("_Kerning"), true);

    _font_settings.add(*tabs);

//Text Preview:
    _preview_entry.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_preview_text_changed));
    _getContents()->add((Gtk::Widget&) _font_da);
    _preview_entry.set_text("Sample Text");
    _font_da.set_text("Sample Text");

    Gtk::HBox* preview_entry_hbox = Gtk::manage(new Gtk::HBox());
    _getContents()->add(*preview_entry_hbox);
    preview_entry_hbox->add(*Gtk::manage(new Gtk::Label(_("Preview Text:"))));
    preview_entry_hbox->add(_preview_entry);

    _getContents()->show_all();
}

SvgFontsDialog::~SvgFontsDialog(){}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifdef ENABLE_SVG_FONTS
