/** @file
 * @brief New From Template abstract tab implementation
 */
/* Authors:
 *   Jan Darowski <jan.darowski@gmail.com>, supervised by Krzysztof Kosiński   
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "template-load-tab.h"
#include "template-widget.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>
#include <glibmm/i18n.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <glibmm/stringutils.h>

#include "interface.h"
#include "file.h"
#include "path-prefix.h"
#include "preferences.h"
#include "inkscape.h"
#include "xml/repr.h"
#include "xml/document.h"
#include "xml/node.h"


namespace Inkscape {
namespace UI {
    

TemplateLoadTab::TemplateLoadTab()
    : _current_keyword("")
    , _keywords_combo(true)
    , _current_search_type(ALL)
{
    set_border_width(10);

    _info_widget = manage(new TemplateWidget());
    
    Gtk::Label *title;
    title = manage(new Gtk::Label(_("Search:")));
    _search_box.pack_start(*title, Gtk::PACK_SHRINK);
    _search_box.pack_start(_keywords_combo, Gtk::PACK_SHRINK, 5);
    
    _tlist_box.pack_start(_search_box, Gtk::PACK_SHRINK, 10);
    
    pack_start(_tlist_box, Gtk::PACK_SHRINK);
    pack_start(*_info_widget, Gtk::PACK_EXPAND_WIDGET, 5);
    
    Gtk::ScrolledWindow *scrolled;
    scrolled = manage(new Gtk::ScrolledWindow());
    scrolled->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scrolled->add(_tlist_view);
    _tlist_box.pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET, 5);
    
    _keywords_combo.signal_changed().connect(
    sigc::mem_fun(*this, &TemplateLoadTab::_keywordSelected));
    this->show_all();
    
    _loading_path = "";
    _loadTemplates();
    _initLists();
}


TemplateLoadTab::~TemplateLoadTab()
{
}


void TemplateLoadTab::createTemplate()
{
    _info_widget->create();
}


void TemplateLoadTab::_displayTemplateInfo()
{
    Glib::RefPtr<Gtk::TreeSelection> templateSelectionRef = _tlist_view.get_selection();
    if (templateSelectionRef->get_selected()) {
        _current_template = (*templateSelectionRef->get_selected())[_columns.textValue];

       _info_widget->display(_tdata[_current_template]);
    }
     
}


void TemplateLoadTab::_initKeywordsList()
{
    _keywords_combo.append(_("All"));
    
    for (std::set<Glib::ustring>::iterator it = _keywords.begin() ; it != _keywords.end() ; ++it){
        _keywords_combo.append(*it);
    }
}


void TemplateLoadTab::_initLists()
{
    _tlist_store = Gtk::ListStore::create(_columns);
    _tlist_view.set_model(_tlist_store);
    _tlist_view.append_column("", _columns.textValue);
    _tlist_view.set_headers_visible(false);
    
    _initKeywordsList();
    _refreshTemplatesList();
   
    Glib::RefPtr<Gtk::TreeSelection> templateSelectionRef =
    _tlist_view.get_selection(); 
    templateSelectionRef->signal_changed().connect(
    sigc::mem_fun(*this, &TemplateLoadTab::_displayTemplateInfo));
}


void TemplateLoadTab::_keywordSelected()
{
    _current_keyword = _keywords_combo.get_active_text();
    if (_current_keyword == ""){
        _current_keyword = _keywords_combo.get_entry_text();
        _current_search_type = USER_SPECIFIED;
    }
    else
        _current_search_type = LIST_KEYWORD;
    
    if (_current_keyword == "" || _current_keyword == _("All"))
        _current_search_type = ALL;
    
    _refreshTemplatesList();
}


void TemplateLoadTab::_refreshTemplatesList()
{
     _tlist_store->clear();
    
    switch (_current_search_type){
    case ALL :{
            
        for (std::map<Glib::ustring, TemplateData>::iterator it = _tdata.begin() ; it != _tdata.end() ; ++it) {
            Gtk::TreeModel::iterator iter = _tlist_store->append();
            Gtk::TreeModel::Row row = *iter;
            row[_columns.textValue]  = it->first;
        }
        break;
    }
    
    case LIST_KEYWORD: {
        for (std::map<Glib::ustring, TemplateData>::iterator it = _tdata.begin() ; it != _tdata.end() ; ++it) {
            if (it->second.keywords.count(_current_keyword) != 0){
                Gtk::TreeModel::iterator iter = _tlist_store->append();
                Gtk::TreeModel::Row row = *iter;
                row[_columns.textValue]  = it->first;
            }
        }
        break;
    }
    
    case USER_SPECIFIED : {
        for (std::map<Glib::ustring, TemplateData>::iterator it = _tdata.begin() ; it != _tdata.end() ; ++it) {
            if (it->second.keywords.count(_current_keyword) != 0 || 
                it->second.display_name.find(_current_keyword) != Glib::ustring::npos ||
                it->second.author.find(_current_keyword) != Glib::ustring::npos ||
                it->second.short_description.find(_current_keyword) != Glib::ustring::npos ||
                it->second.long_description.find(_current_keyword) != Glib::ustring::npos )
            {
                Gtk::TreeModel::iterator iter = _tlist_store->append();
                Gtk::TreeModel::Row row = *iter;
                row[_columns.textValue]  = it->first;
            }
        }
        break;
    }
    }
} 


void TemplateLoadTab::_loadTemplates()
{
    // user's local dir
    _getTemplatesFromDir(profile_path("templates") + _loading_path);

    // system templates dir
    _getTemplatesFromDir(INKSCAPE_TEMPLATESDIR + _loading_path);
}


TemplateLoadTab::TemplateData TemplateLoadTab::_processTemplateFile(const Glib::ustring &path)
{
    TemplateData result;
    result.path = path;
    result.is_procedural = false;
    result.preview_name = "";
    
    // convert path into valid template name
    result.display_name = Glib::path_get_basename(path);
    gsize n = 0;
    while ((n = result.display_name.find_first_of("_", 0)) < Glib::ustring::npos){
        result.display_name.replace(n, 1, 1, ' ');
    }   
    n =  result.display_name.rfind(".svg");
    result.display_name.replace(n, 4, 1, ' ');
    
    Inkscape::XML::Document *rdoc;
    rdoc = sp_repr_read_file(path.data(), SP_SVG_NS_URI);
    Inkscape::XML::Node *myRoot;
    Inkscape::XML::Node *dataNode;

    if (rdoc){
        myRoot = rdoc->root();
        if (strcmp(myRoot->name(), "svg:svg") != 0){     // Wrong file format
            return result;
        }
        
        myRoot = sp_repr_lookup_name(myRoot, "inkscape:_templateinfo");
        
        if (myRoot == NULL)    // No template info
            return result;

        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:_name")) != NULL)
            result.display_name = dgettext("Document template name", dataNode->firstChild()->content());
        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:author")) != NULL)
            result.author = dataNode->firstChild()->content();
        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:_short")) != NULL)
            result.short_description = dgettext("Document template short description", dataNode->firstChild()->content());
        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:_long") )!= NULL)
            result.long_description = dgettext("Document template long description", dataNode->firstChild()->content());
        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:preview")) != NULL)
            result.preview_name = dataNode->firstChild()->content();
        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:date")) != NULL){
            result.creation_date = dataNode->firstChild()->content();
        }
        
        if ((dataNode = sp_repr_lookup_name(myRoot, "inkscape:_keywords")) != NULL){
            Glib::ustring data = dataNode->firstChild()->content();
            while (!data.empty()){
                std::size_t pos = data.find_first_of(" ");
                if (pos == Glib::ustring::npos)
                    pos = data.size();
                
                Glib::ustring keyword = dgettext("Document template keyword", data.substr(0, pos).data());
                result.keywords.insert(keyword);
                _keywords.insert(keyword);
                
                if (pos == data.size())
                    break;
                data.erase(0, pos+1);
            }
        }
    }
    
    return result;
}


void TemplateLoadTab::_getTemplatesFromDir(const Glib::ustring &path)
{
    if ( !Glib::file_test(path, Glib::FILE_TEST_EXISTS) ||
         !Glib::file_test(path, Glib::FILE_TEST_IS_DIR))
        return;
    
    Glib::Dir dir(path);

    Glib::ustring file = Glib::build_filename(path, dir.read_name());
    while (file != path){
        if (Glib::str_has_suffix(file, ".svg") && !Glib::str_has_prefix(Glib::path_get_basename(file), "default")){
            TemplateData tmp = _processTemplateFile(file);
            if (tmp.display_name != "")
                _tdata[tmp.display_name] = tmp;
        }
        file = Glib::build_filename(path, dir.read_name());
    }
}

}
}
