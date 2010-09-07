/*
 * Copyright (C) 2005-2010 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 *   Nicolas Dufour <nicoduf@yahoo.fr>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef linux  // does the dollar sign need escaping when passed as string parameter?
# define ESCAPE_DOLLAR_COMMANDLINE
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "groupheader.h"

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <sstream>
#include <glibmm/i18n.h>

#include "xml/node.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {


/** \brief  Initialize the object, to do that, copy the data. */
ParamGroupHeader::ParamGroupHeader (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext), _value(NULL)
{
    // printf("Building GroupHeader\n");
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    if (defaultval != NULL)
        _value = g_strdup(defaultval);

    return;
}

/** \brief  Create a label for the GroupHeader */
Gtk::Widget *
ParamGroupHeader::get_widget (SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * /*changeSignal*/)
{
	if (_gui_hidden) return NULL;

    Gtk::Label * label = Gtk::manage(new Gtk::Label(Glib::ustring("<b>") + _(_value) + Glib::ustring("</b>"), Gtk::ALIGN_LEFT));
    label->set_line_wrap();
    label->set_padding(0,5);
    label->set_use_markup(true);
    label->show();

    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    hbox->pack_start(*label, true, true);
    hbox->show();

    return hbox;
}

}  /* namespace Extension */
}  /* namespace Inkscape */
