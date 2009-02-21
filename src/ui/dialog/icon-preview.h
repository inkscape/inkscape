/** @file
 * @brief A simple dialog for previewing icon representation.
 */
/* Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004,2005 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_ICON_PREVIEW_H
#define SEEN_ICON_PREVIEW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/image.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/toggletoolbutton.h>

#include "ui/widget/panel.h"

struct SPObject;

namespace Inkscape {
namespace UI {
namespace Dialogs {


/**
 * A panel that displays an icon preview
 */
class IconPreviewPanel : public UI::Widget::Panel
{
public:
    IconPreviewPanel();
    //IconPreviewPanel(Glib::ustring const &label);

    static IconPreviewPanel& getInstance();

    void refreshPreview();
    void modeToggled();

private:
    IconPreviewPanel(IconPreviewPanel const &); // no copy
    IconPreviewPanel &operator=(IconPreviewPanel const &); // no assign


    void on_button_clicked(int which);
    void renderPreview( SPObject* obj );
    void updateMagnify();

    Gtk::Tooltips   tips;

    Gtk::VBox       iconBox;
    Gtk::HPaned     splitter;

    int hot;
    int numEntries;
    int* sizes;

    Gtk::Image      magnified;
    Gtk::Label      magLabel;

    Gtk::Button           *refreshButton;
    Gtk::ToggleButton     *selectionButton;

    guchar** pixMem;
    Gtk::Image** images;
    Glib::ustring** labels;
    Gtk::ToggleToolButton** buttons;
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_ICON_PREVIEW_H
