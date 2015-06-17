/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef linux  // does the dollar sign need escaping when passed as string parameter?
# define ESCAPE_DOLLAR_COMMANDLINE
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <glibmm/i18n.h>

#include "xml/node.h"
#include "extension/extension.h"
#include "description.h"

namespace Inkscape {
namespace Extension {


/** \brief  Initialize the object, to do that, copy the data. */
ParamDescription::ParamDescription (const gchar * name,
                                    const gchar * guitext,
                                    const gchar * desc,
                                    const Parameter::_scope_t scope,
                                    bool gui_hidden,
                                    const gchar * gui_tip,
                                    Inkscape::Extension::Extension * ext,
                                    Inkscape::XML::Node * xml,
                                    AppearanceMode mode) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext),
              _value(NULL), _mode(mode), _indent(0)
{
    // printf("Building Description\n");
    const char * defaultval = NULL;
    if (xml->firstChild() != NULL) {
        defaultval = xml->firstChild()->content();
    }

    if (defaultval != NULL) {
        _value = g_strdup(defaultval);
    }

    _context = xml->attribute("msgctxt");

    const char * indent = xml->attribute("indent");
    if (indent != NULL) {
        _indent = atoi(indent) * 12;
    }

    return;
}

/** \brief  Create a label for the description */
Gtk::Widget *
ParamDescription::get_widget (SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * /*changeSignal*/)
{
    if (_gui_hidden) {
        return NULL;
    }
    if (_value == NULL) {
        return NULL;
    }

    Glib::ustring newguitext;

    if (_context != NULL) {
        newguitext = g_dpgettext2(NULL, _context, _value);
    } else {
        newguitext = _(_value);
    }
    
    Gtk::Label * label;
    int padding = 12 + _indent;
    if (_mode == HEADER) {
        label = Gtk::manage(new Gtk::Label(Glib::ustring("<b>") +newguitext + Glib::ustring("</b>"), Gtk::ALIGN_START));
        label->set_padding(0,5);
        label->set_use_markup(true);
        padding = _indent;
    } else {
        label = Gtk::manage(new Gtk::Label(newguitext, Gtk::ALIGN_START));
    }
    label->set_line_wrap();
    label->show();

    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    hbox->pack_start(*label, true, true, padding);
    hbox->show();

    return hbox;
}

}  /* namespace Extension */
}  /* namespace Inkscape */
