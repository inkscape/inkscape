/**
 * \brief Print dialog
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2007 Kees Cook
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_PRINT_H
#define INKSCAPE_UI_DIALOG_PRINT_H

#include <glibmm/i18n.h>
#include <gtkmm/printoperation.h> // GtkMM printoperation
#include <gtk/gtkprintoperation.h> // Gtk

#include "desktop.h"
#include "sp-item.h"

#include "ui/widget/rendering-options.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

class Print : public Gtk::PrintOperation {
public:
    Print(SPDocument *doc, SPItem *base);

protected:
    Gtk::Widget *on_create_custom_widget ();

    void    on_begin_print(const Glib::RefPtr<Gtk::PrintContext> &context);
    bool    on_paginate(const Glib::RefPtr<Gtk::PrintContext> &context);
    void    on_draw_page (const Glib::RefPtr<Gtk::PrintContext> &context,
                          int page_nr);

private:
    SPDocument *_doc;
    SPItem     *_base;
    Inkscape::UI::Widget::RenderingOptions _tab;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_PRINT_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
