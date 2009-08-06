/** @file
 * @brief Print dialog
 */
/* Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2007 Kees Cook
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_PRINT_H
#define INKSCAPE_UI_DIALOG_PRINT_H

#include <glibmm/i18n.h>
#include <gtkmm/printoperation.h> // GtkMM
#include <gtk/gtkprintoperation.h> // Gtk

#include "desktop.h"
#include "sp-item.h"

#include "ui/widget/rendering-options.h"

/*
 * gtk 2.12.0 has a bug (http://bugzilla.gnome.org/show_bug.cgi?id=482089)
 * where it fails to correctly deal with gtkmm signal management.  As a result
 * we have call gtk directly instead of doing a much cleaner version of
 * this printing dialog, using full gtkmmification.  (The bug was fixed
 * in 2.12.1, so when the Inkscape gtk minimum version is bumped there,
 * we can revert Inkscape commit 16865.
 */
struct workaround_gtkmm
{
    SPDocument *_doc;
    SPItem     *_base;
    Inkscape::UI::Widget::RenderingOptions *_tab;
};

namespace Inkscape {
namespace UI {
namespace Dialog {

class Print {
public:
    Print(SPDocument *doc, SPItem *base);
    Gtk::PrintOperationResult run(Gtk::PrintOperationAction, Gtk::Window &parent_window);

protected:

private:
    GtkPrintOperation *_printop;
    SPDocument *_doc;
    SPItem     *_base;
    Inkscape::UI::Widget::RenderingOptions _tab;

    struct workaround_gtkmm _workaround;
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
