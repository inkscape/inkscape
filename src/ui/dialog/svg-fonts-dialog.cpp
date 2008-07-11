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

#include "svg-fonts-dialog.h"

SvgFontDrawingArea::SvgFontDrawingArea(){
	this->text = "Sample Text";
	this->svgfont = NULL;
        ((Gtk::Widget*) this)->set_size_request(150, 50);
}

void SvgFontDrawingArea::set_svgfont(SvgFont* svgfont){
	this->svgfont = svgfont;
}

void SvgFontDrawingArea::set_text(Glib::ustring text){
	this->text = text;
}

void SvgFontDrawingArea::redraw(){
	((Gtk::Widget*) this)->queue_draw();
}

bool SvgFontDrawingArea::on_expose_event (GdkEventExpose *event){
  if (this->svgfont){
    Glib::RefPtr<Gdk::Window> window = get_window();
    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    cr->set_font_face( Cairo::RefPtr<Cairo::FontFace>(new Cairo::FontFace(this->svgfont->get_font_face(), false /* does not have reference */)) );
    cr->set_font_size (20);
    cr->move_to (20, 20);
    cr->show_text (this->text.c_str());
  }
  return TRUE;
}

namespace Inkscape {
namespace UI {
namespace Dialog {

/*** SvgFontsDialog ***/

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
        row[_columns.font] = f;
        row[_columns.svgfont] = new SvgFont(f);
        const gchar* lbl = f->label();
        const gchar* id = SP_OBJECT_ID(f);
        row[_columns.label] = lbl ? lbl : (id ? id : "font");
    }
}

void SvgFontsDialog::on_preview_text_changed(){
//    _font_da.set_text((gchar*) _preview_entry.get_text().c_str());
    _font_da.set_text(_preview_entry.get_text());
    _font_da.redraw();
}

void SvgFontsDialog::on_font_selection_changed(){
    _font_da.set_svgfont(this->get_selected_svgfont());
    _font_da.redraw();
}

SvgFont* SvgFontsDialog::get_selected_svgfont()
{
    Gtk::TreeModel::iterator i = _font_list.get_selection()->get_selected();
    if(i)
        return (*i)[_columns.svgfont];
    return NULL;
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
    _font_list.append_column_editable("_Font", _columns.label);
    _font_list.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_font_selection_changed));

    this->update_fonts();

    _preview_entry.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_preview_text_changed));

    _getContents()->add((Gtk::Widget&) _font_da);
    _getContents()->add(_preview_entry);

/*    Gtk::HBox* preview_box = Gtk::manage(new Gtk::HBox());
    _preview_entry.signal_changed().connect(sigc::mem_fun(*this, &SvgFontsDialog::on_preview_text_changed));
    preview_box->add(_preview_entry);
    preview_box->add((Gtk::Widget&) _font_da);

    _getContents()->add(*preview_box);*/
    _getContents()->show_all();

//Settings for the selected SVGFont:
//    _font_family.set_label("font-family");
//    _font_variant.set_label("font-variant");

//    _font_settings.add(_font_family);
//    _font_settings.add(_font_variant);
}

SvgFontsDialog::~SvgFontsDialog(){}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifdef ENABLE_SVG_FONTS
