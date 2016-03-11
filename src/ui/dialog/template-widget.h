/** @file
 * @brief New From Template - template widget
 */
/* Authors:
 *   Jan Darowski <jan.darowski@gmail.com>, supervised by Krzysztof Kosiński  
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_SEEN_UI_DIALOG_TEMPLATE_WIDGET_H
#define INKSCAPE_SEEN_UI_DIALOG_TEMPLATE_WIDGET_H

#include "filedialogimpl-gtkmm.h"

#include <gtkmm/box.h>

#include "template-load-tab.h"


namespace Inkscape {
namespace UI {
    
    
class TemplateWidget : public Gtk::VBox
{
public:
    TemplateWidget ();
    void create();
    void display(TemplateLoadTab::TemplateData);
    void clear();
    
private:
    TemplateLoadTab::TemplateData _current_template;
    
    Gtk::Button _more_info_button;
    Gtk::HBox _preview_box;
    Gtk::Image _preview_image;
    Dialog::SVGPreview _preview_render;
    Gtk::Label _short_description_label;
    Gtk::Label _template_name_label;
    Gtk::Widget *_effect_prefs;
    
    void _displayTemplateDetails();
};

}
}

#endif
