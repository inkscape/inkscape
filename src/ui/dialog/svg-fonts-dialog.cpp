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
g_warning("update_fonts");
    SPDesktop* desktop = this->getDesktop();
    SPDocument* document = sp_desktop_document(desktop);
    const GSList* fonts = sp_document_get_resource_list(document, "font");

    _model->clear();
g_warning("after _model->clear()");
    for(const GSList *l = fonts; l; l = l->next) {
        Gtk::TreeModel::Row row = *_model->append();
        SPFont* f = (SPFont*)l->data;
        row[_columns.font] = f;
        const gchar* lbl = f->label();
g_warning("label: %s", lbl);
        const gchar* id = SP_OBJECT_ID(f);
        row[_columns.label] = lbl ? lbl : (id ? id : "font");
    }

//    update_selection(desktop->selection);
//    _dialog.update_filter_general_settings_view();
}

SvgFontsDialog::SvgFontsDialog()
 : UI::Widget::Panel("", "dialogs.svgfonts", SP_VERB_DIALOG_SVG_FONTS)
{
    //Gtk::Label* label = Gtk::manage(new Gtk::Label("Here we will have settings for the SVGFonts used in the document."));
    _getContents()->add(_list);
g_warning("a");
    _model = Gtk::ListStore::create(_columns);
g_warning("b");
    _list.set_model(_model);
    _list.append_column_editable("_Font", _columns.label);
g_warning("c");
    this->update_fonts();
g_warning("d");
}

SvgFontsDialog::~SvgFontsDialog(){}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifdef ENABLE_SVG_FONTS
