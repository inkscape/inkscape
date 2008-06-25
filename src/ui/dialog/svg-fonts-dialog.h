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

#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class SvgFontsDialog : public UI::Widget::Panel {
public:
    SvgFontsDialog();
    ~SvgFontsDialog();

    static SvgFontsDialog &getInstance()
    { return *new SvgFontsDialog(); }

    void update_fonts();

private:
    class Columns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            Columns()
            {
                add(font);
                add(label);
                //add(sel);
            }

            Gtk::TreeModelColumn<SPFont*> font;
            Gtk::TreeModelColumn<Glib::ustring> label;
            //Gtk::TreeModelColumn<int> sel;
    };
    Glib::RefPtr<Gtk::ListStore> _model;
    Columns _columns;
    Gtk::TreeView _list;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifndef INKSCAPE_UI_DIALOG_SVG_FONTS_H
