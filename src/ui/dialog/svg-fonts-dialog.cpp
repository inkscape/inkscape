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
        const gchar* lbl = f->label();
        const gchar* id = SP_OBJECT_ID(f);
        row[_columns.label] = lbl ? lbl : (id ? id : "font");
    }
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
    this->update_fonts();

//Settings for the selected SVGFont:
    _font_family.set_label("font-family");
    _font_variant.set_label("font-variant");

    _font_settings.add(_font_family);
    _font_settings.add(_font_variant);
}

SvgFontsDialog::~SvgFontsDialog(){}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifdef ENABLE_SVG_FONTS
