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


#include "description.h"

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <sstream>


#include <glibmm/i18n.h>

#include <xml/node.h>

#include <extension/extension.h>
#include <prefs-utils.h>

namespace Inkscape {
namespace Extension {


/** \brief  Initialize the object, to do that, copy the data. */
ParamDescription::ParamDescription (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext), _value(NULL)
{
    // printf("Building Description\n");
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    if (defaultval != NULL)
        _value = g_strdup(defaultval);

    return;
}

/** \brief  Create a label for the description */
Gtk::Widget *
ParamDescription::get_widget (SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * /*changeSignal*/)
{
	if (_gui_hidden) return NULL;

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_value)));
    label->set_line_wrap();
    label->show();

    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    hbox->pack_start(*label, true, true, 5);
    hbox->show();

    return hbox;
}

}  /* namespace Extension */
}  /* namespace Inkscape */
