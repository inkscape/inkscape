/*
 * A very simple dialog for displaying Inkscape messages. Messages
 * sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "messages.h"


#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Also a public method.  Remove all text from the dialog
 */
void Messages::clear()
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = messageText.get_buffer();
    buffer->erase(buffer->begin(), buffer->end());
}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
Messages::Messages()
    : UI::Widget::Panel("", "dialogs.messages", SP_VERB_DIALOG_DEBUG)
{
    Gtk::Box *contents = _getContents();

    //## Add a menu for clear()
    menuBar.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_File"), fileMenu) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_Clear"),
           sigc::mem_fun(*this, &Messages::clear) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("Capture log messages"),
           sigc::mem_fun(*this, &Messages::captureLogMessages) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("Release log messages"),
           sigc::mem_fun(*this, &Messages::releaseLogMessages) ) );
    contents->pack_start(menuBar, Gtk::PACK_SHRINK);
    

    //### Set up the text widget
    messageText.set_editable(false);
    textScroll.add(messageText);
    textScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    contents->pack_start(textScroll);

    // sick of this thing shrinking too much
    set_size_request(400, 300);
    
    show_all_children();

    message(_("Ready."));
    message(_("Enable log display by setting dialogs.debug 'redirect' attribute to 1 in preferences.xml"));

    handlerDefault = 0;
    handlerGlibmm  = 0;
    handlerAtkmm   = 0;
    handlerPangomm = 0;
    handlerGdkmm   = 0;
    handlerGtkmm   = 0;
}

Messages::~Messages()
{
}


//#########################################################################
//## M E T H O D S
//#########################################################################

void Messages::message(char *msg)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = messageText.get_buffer();
    Glib::ustring uMsg = msg;
    if (uMsg[uMsg.length()-1] != '\n')
        uMsg += '\n';
    buffer->insert (buffer->end(), uMsg);
}


void dialogLoggingFunction(const gchar */*log_domain*/,
                           GLogLevelFlags /*log_level*/,
                           const gchar *messageText,
                           gpointer user_data)
{
    Messages *dlg = (Messages *)user_data;

    dlg->message((char *)messageText);

}


void Messages::captureLogMessages()
{
    /*
    This might likely need more code, to capture Gtkmm
    and Glibmm warnings, or maybe just simply grab stdout/stderr
    */
   GLogLevelFlags flags = (GLogLevelFlags) (G_LOG_LEVEL_ERROR   | G_LOG_LEVEL_CRITICAL |
                             G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE  |
                             G_LOG_LEVEL_INFO    | G_LOG_LEVEL_DEBUG);
    if ( !handlerDefault ) {
        handlerDefault = g_log_set_handler(NULL, flags,
              dialogLoggingFunction, (gpointer)this);
    }
    if ( !handlerGlibmm ) {
        handlerGlibmm = g_log_set_handler("glibmm", flags,
              dialogLoggingFunction, (gpointer)this);
    }
    if ( !handlerAtkmm ) {
        handlerAtkmm = g_log_set_handler("atkmm", flags,
              dialogLoggingFunction, (gpointer)this);
    }
    if ( !handlerPangomm ) {
        handlerPangomm = g_log_set_handler("pangomm", flags,
              dialogLoggingFunction, (gpointer)this);
    }
    if ( !handlerGdkmm ) {
        handlerGdkmm = g_log_set_handler("gdkmm", flags,
              dialogLoggingFunction, (gpointer)this);
    }
    if ( !handlerGtkmm ) {
        handlerGtkmm = g_log_set_handler("gtkmm", flags,
              dialogLoggingFunction, (gpointer)this);
    }
    message((char*)"log capture started");
}

void Messages::releaseLogMessages()
{
    if ( handlerDefault ) {
        g_log_remove_handler(NULL, handlerDefault);
        handlerDefault = 0;
    }
    if ( handlerGlibmm ) {
        g_log_remove_handler("glibmm", handlerGlibmm);
        handlerGlibmm = 0;
    }
    if ( handlerAtkmm ) {
        g_log_remove_handler("atkmm", handlerAtkmm);
        handlerAtkmm = 0;
    }
    if ( handlerPangomm ) {
        g_log_remove_handler("pangomm", handlerPangomm);
        handlerPangomm = 0;
    }
    if ( handlerGdkmm ) {
        g_log_remove_handler("gdkmm", handlerGdkmm);
        handlerGdkmm = 0;
    }
    if ( handlerGtkmm ) {
        g_log_remove_handler("gtkmm", handlerGtkmm);
        handlerGtkmm = 0;
    }
    message((char*)"log capture discontinued");
}

} //namespace Dialog
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################
