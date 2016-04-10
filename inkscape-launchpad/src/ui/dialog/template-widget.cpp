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
#include "desktop.h"

#include "document.h"
#include "document-undo.h"
#include "file.h"
#include "sp-namedview.h"
#include "extension/implementation/implementation.h"
#include "inkscape.h"


namespace Inkscape {
namespace UI {
    

TemplateWidget::TemplateWidget()
    : _more_info_button(_("More info"))
    , _short_description_label(" ")
    , _template_name_label(_("no template selected"))
    , _effect_prefs(NULL)
{
    pack_start(_template_name_label, Gtk::PACK_SHRINK, 10);
    pack_start(_preview_box, Gtk::PACK_SHRINK, 0);
    
    _preview_box.pack_start(_preview_image, Gtk::PACK_EXPAND_PADDING, 15);
    _preview_box.pack_start(_preview_render, Gtk::PACK_EXPAND_PADDING, 10);
    
    _short_description_label.set_line_wrap(true);

    Gtk::Alignment *align;
    align = Gtk::manage(new Gtk::Alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER, 0.0, 0.0));
    pack_end(*align, Gtk::PACK_SHRINK);
    align->add(_more_info_button);
    
    pack_end(_short_description_label, Gtk::PACK_SHRINK, 5);
    
    _more_info_button.signal_clicked().connect(
    sigc::mem_fun(*this, &TemplateWidget::_displayTemplateDetails));
    _more_info_button.set_sensitive(false);
}


void TemplateWidget::create()
{
    if (_current_template.display_name == "")
        return;
    
    if (_current_template.is_procedural){
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        SPDesktop *desc = sp_file_new_default();
        _current_template.tpl_effect->effect(desc);
        DocumentUndo::clearUndo(desc->getDocument());
        desc->getDocument()->setModifiedSinceSave(false);

	// Apply cx,cy etc. from document
	sp_namedview_window_from_document( desc );

        if (desktop)
            desktop->clearWaitingCursor();
    }
    else {
        sp_file_new(_current_template.path);
    }
}


void TemplateWidget::display(TemplateLoadTab::TemplateData data)
{
    clear();
    _current_template = data;

    _template_name_label.set_text(_current_template.display_name);
    _short_description_label.set_text(_current_template.short_description);

    std::string imagePath = Glib::build_filename(Glib::path_get_dirname(_current_template.path),  _current_template.preview_name);
    if (data.preview_name != ""){
        _preview_image.set(imagePath);
        _preview_image.show();
    }
    else if (!data.is_procedural){
        Glib::ustring gPath = data.path.c_str();
        _preview_render.showImage(gPath);
        _preview_render.show();
    }

    if (data.is_procedural){
        _effect_prefs = data.tpl_effect->get_imp()->prefs_effect(data.tpl_effect, SP_ACTIVE_DESKTOP, NULL, NULL); 
        pack_start(*_effect_prefs);
    }
    _more_info_button.set_sensitive(true);
}

void TemplateWidget::clear()
{
    _template_name_label.set_text("");
    _short_description_label.set_text("");
    _preview_render.hide();
    _preview_image.hide();
    if (_effect_prefs != NULL){
        remove (*_effect_prefs);
        _effect_prefs = NULL;
    }
    _more_info_button.set_sensitive(false);
}

void TemplateWidget::_displayTemplateDetails()
{    
    Glib::ustring message = _current_template.display_name + "\n\n";
    
    if (_current_template.path != "")
        message += _("Path: ") + _current_template.path + "\n\n";
    
    if (_current_template.long_description != "")
        message += _("Description: ") + _current_template.long_description + "\n\n";
    if (!_current_template.keywords.empty()){
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
