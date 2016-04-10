/** @file
 * @brief New From Template main dialog
 */
/* Authors:
 *   Jan Darowski <jan.darowski@gmail.com>, supervised by Krzysztof Kosiński    
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_SEEN_UI_DIALOG_NEW_FROM_TEMPLATE_H
#define INKSCAPE_SEEN_UI_DIALOG_NEW_FROM_TEMPLATE_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>

#include "template-load-tab.h"


namespace Inkscape {
namespace UI {
    

class NewFromTemplate : public Gtk::Dialog
{

friend class TemplateLoadTab;
public:
    static void load_new_from_template();
    void setCreateButtonSensitive(bool value);
    virtual ~NewFromTemplate();

private:
    NewFromTemplate();
    Gtk::Button _create_template_button;
    TemplateLoadTab* _main_widget;
    
    void _createFromTemplate();
    void _onClose();
};

}
}
#endif
