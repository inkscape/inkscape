/** @file
 * @brief Print Colors Preview dialog
 */
/* Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPLv2 (or later).  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_PRINT_COLORS_PREVIEW_H
#define INKSCAPE_UI_DIALOG_PRINT_COLORS_PREVIEW_H

#include "ui/widget/panel.h"
#include "verbs.h"

#include <gtkmm/box.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

class PrintColorsPreviewDialog : public UI::Widget::Panel {
public:
    PrintColorsPreviewDialog();
    ~PrintColorsPreviewDialog();

    static PrintColorsPreviewDialog &getInstance()
    { return *new PrintColorsPreviewDialog(); }

private:
    void toggle_cyan();
    void toggle_magenta();
    void toggle_yellow();
    void toggle_black();

    Gtk::ToggleButton* cyan;
    Gtk::ToggleButton* magenta;
    Gtk::ToggleButton* yellow;
    Gtk::ToggleButton* black;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //#ifndef INKSCAPE_UI_PRINT_COLORS_PREVIEW_H
