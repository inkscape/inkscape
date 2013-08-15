/** @file
 * @brief New From Template - templates widget - implementation
 */
/* Authors:
 *   Jan Darowski <jan.darowski@gmail.com>, supervised by Krzysztof Kosiński   
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "template-widget.h"

#include <gtkmm/alignment.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>

#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>

#include "template-load-tab.h"
#include "file.h"

namespace Inkscape {
namespace UI {
    

TemplateWidget::TemplateWidget()
    : _more_info_button(_("More info"))
    , _short_description_label(_(" "))
    , _template_author_label(_(" "))
    , _template_name_label(_("no template selected"))
{
    pack_start(_template_name_label, Gtk::PACK_SHRINK, 10);
    pack_start(_template_author_label, Gtk::PACK_SHRINK, 0);
    pack_start(_preview_box, Gtk::PACK_SHRINK, 0);
    
    _preview_box.pack_start(_preview_image, Gtk::PACK_EXPAND_PADDING, 15);
    _preview_box.pack_start(_preview_render, Gtk::PACK_EXPAND_PADDING, 10);
    
    _short_description_label.set_line_wrap(true);
    //_short_description_label.set_size_request(200);

    Gtk::Alignment *align;
    align = manage(new Gtk::Alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0.0, 0.0));
    pack_end(*align, Gtk::PACK_SHRINK);
    align->add(_more_info_button);
    
    pack_end(_short_description_label, Gtk::PACK_SHRINK, 5);
    
    _more_info_button.signal_pressed().connect(
    sigc::mem_fun(*this, &TemplateWidget::_displayTemplateDetails));
}


void TemplateWidget::create()
{
    if (_current_template.path == "")
        return;
    
    if (_current_template.is_procedural) {}
    else {
        sp_file_new(_current_template.path);
    }
}


void TemplateWidget::display(TemplateLoadTab::TemplateData data)
{
    _current_template = data;
    if (data.is_procedural){}
    else{
        _template_name_label.set_text(_current_template.display_name);
        _template_author_label.set_text(_current_template.author);
        _short_description_label.set_text(_current_template.short_description);
        
        Glib::ustring imagePath = Glib::build_filename(Glib::path_get_dirname(_current_template.path),  _current_template.preview_name);
        if (data.preview_name != ""){
            _preview_image.set(imagePath);
            _preview_image.show();
            _preview_render.hide();
        }
        else{
            _preview_render.showImage(data.path);
            _preview_render.show();
            _preview_image.hide();
        }
    }
}


void TemplateWidget::_displayTemplateDetails()
{    
    if (_current_template.path == "")
        return;
    
    Glib::ustring message = _current_template.display_name + "\n\n" +
                            _("Path: ") + _current_template.path + "\n\n";
    
    if (_current_template.long_description != "")
        message += _("Description: ") + _current_template.long_description + "\n\n";
    if (_current_template.keywords.size() > 0){
        message += _("Keywords: ");
        for (std::set<Glib::ustring>::iterator it = _current_template.keywords.begin(); it != _current_template.keywords.end(); ++it)
            message += *it + " ";
        message += "\n\n";
    }
    
    if (_current_template.author != "")
        message += _("By: ") + _current_template.author + " " + _current_template.creation_date + "\n\n";
    
    Gtk::MessageDialog dl(message, false, Gtk::MESSAGE_OTHER);
    dl.run();
}

}
}
