/*
 * A very simple dialog for displaying Inkscape messages. Messages
 * sent to g_log(), g_warning(), g_message(), ets, are routed here,
 * in order to avoid messing with the startup console.
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>
#include <gtkmm/menubar.h>
#include <gtkmm/scrolledwindow.h>

#include "debugdialog.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {


//#########################################################################
//## I M P L E M E N T A T I O N
//#########################################################################

/**
 * A dialog that displays log messages
 */
class DebugDialogImpl : public DebugDialog, public Gtk::Dialog
{

    public:
    

    /**
     * Constructor
     */
    DebugDialogImpl();

    /**
     * Destructor
     */
    ~DebugDialogImpl();


    /**
     * Show the dialog
     */
    void show();

    /**
     * Do not show the dialog
     */
    void hide();

    /**
     * Clear all information from the dialog
     */
    void clear();

    /**
     * Display a message
     */
    void message(char const *msg);
    
    /**
     * Redirect g_log() messages to this widget
     */
    void captureLogMessages();

    /**
     * Return g_log() messages to normal handling
     */
    void releaseLogMessages();



    private:


    Gtk::MenuBar menuBar;

    Gtk::Menu   fileMenu;

    Gtk::ScrolledWindow textScroll;

    Gtk::TextView messageText;

    //Handler ID's
    guint handlerDefault;
    guint handlerGlibmm;
    guint handlerAtkmm;
    guint handlerPangomm;
    guint handlerGdkmm;
    guint handlerGtkmm;

};




//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Also a public method.  Remove all text from the dialog
 */
void DebugDialogImpl::clear()
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
DebugDialogImpl::DebugDialogImpl()
{
    set_title(_("Messages"));
    set_size_request(300, 400);

    Gtk::VBox *mainVBox = get_vbox();

    //## Add a menu for clear()
    menuBar.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_File"), fileMenu) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_Clear"),
           sigc::mem_fun(*this, &DebugDialogImpl::clear) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("Capture log messages"),
           sigc::mem_fun(*this, &DebugDialogImpl::captureLogMessages) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("Release log messages"),
           sigc::mem_fun(*this, &DebugDialogImpl::releaseLogMessages) ) );
    mainVBox->pack_start(menuBar, Gtk::PACK_SHRINK);
    

    //### Set up the text widget
    messageText.set_editable(false);
    textScroll.add(messageText);
    textScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    mainVBox->pack_start(textScroll);

    show_all_children();

    message("ready.");
    message("enable log display by setting ");
    message("dialogs.debug 'redirect' attribute to 1 in preferences.xml");

    handlerDefault = 0;
    handlerGlibmm  = 0;
    handlerAtkmm   = 0;
    handlerPangomm = 0;
    handlerGdkmm   = 0;
    handlerGtkmm   = 0;
}

/**
 * Factory method.  Use this to create a new DebugDialog
 */
DebugDialog *DebugDialog::create()
{
    DebugDialog *dialog = new DebugDialogImpl();
    return dialog;
}


/**
 * Constructor
 */
DebugDialogImpl::~DebugDialogImpl()
{


}


//#########################################################################
//## M E T H O D S
//#########################################################################

void DebugDialogImpl::show()
{
    //call super()
    Gtk::Dialog::show();
    //sp_transientize((GtkWidget *)gobj());  //Make transient
    raise();
    Gtk::Dialog::present();
}



void DebugDialogImpl::hide()
{
    //call super()
    Gtk::Dialog::hide();
}



void DebugDialogImpl::message(char const *msg)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = messageText.get_buffer();
    Glib::ustring uMsg = msg;
    if (uMsg[uMsg.length()-1] != '\n')
        uMsg += '\n';
    buffer->insert (buffer->end(), uMsg);
}


/* static instance, to reduce dependencies */
static DebugDialog *debugDialogInstance = NULL;

DebugDialog *DebugDialog::getInstance()
{
    if ( !debugDialogInstance )
        {
        debugDialogInstance = new DebugDialogImpl();
        }
    return debugDialogInstance;
}



void DebugDialog::showInstance()
{
    DebugDialog *debugDialog = getInstance();
    debugDialog->show();
}




/*##### THIS IS THE IMPORTANT PART ##### */
void dialogLoggingFunction(const gchar */*log_domain*/,
                           GLogLevelFlags /*log_level*/,
                           const gchar *messageText,
                           gpointer user_data)
{
    DebugDialogImpl *dlg = (DebugDialogImpl *)user_data;

    dlg->message(messageText);

}


void DebugDialogImpl::captureLogMessages()
{
    /*
    This might likely need more code, to capture Gtkmm
    and Glibmm warnings, or maybe just simply grab stdout/stderr
    */
   GLogLevelFlags flags = (GLogLevelFlags) (G_LOG_LEVEL_ERROR   | G_LOG_LEVEL_CRITICAL |
                             G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE  |
                             G_LOG_LEVEL_INFO    | G_LOG_LEVEL_DEBUG);
    if ( !handlerDefault )
        {
        handlerDefault = g_log_set_handler(NULL, flags,
              dialogLoggingFunction, (gpointer)this);
        }
    if ( !handlerGlibmm )
        {
        handlerGlibmm = g_log_set_handler("glibmm", flags,
              dialogLoggingFunction, (gpointer)this);
        }
    if ( !handlerAtkmm )
        {
        handlerAtkmm = g_log_set_handler("atkmm", flags,
              dialogLoggingFunction, (gpointer)this);
        }
    if ( !handlerPangomm )
        {
        handlerPangomm = g_log_set_handler("pangomm", flags,
              dialogLoggingFunction, (gpointer)this);
        }
    if ( !handlerGdkmm )
        {
        handlerGdkmm = g_log_set_handler("gdkmm", flags,
              dialogLoggingFunction, (gpointer)this);
        }
    if ( !handlerGtkmm )
        {
        handlerGtkmm = g_log_set_handler("gtkmm", flags,
              dialogLoggingFunction, (gpointer)this);
        }
    message("log capture started");
}

void DebugDialogImpl::releaseLogMessages()
{
    if ( handlerDefault )
        {
        g_log_remove_handler(NULL, handlerDefault);
        handlerDefault = 0;
        }
    if ( handlerGlibmm )
        {
        g_log_remove_handler("glibmm", handlerGlibmm);
        handlerGlibmm = 0;
        }
    if ( handlerAtkmm )
        {
        g_log_remove_handler("atkmm", handlerAtkmm);
        handlerAtkmm = 0;
        }
    if ( handlerPangomm )
        {
        g_log_remove_handler("pangomm", handlerPangomm);
        handlerPangomm = 0;
        }
    if ( handlerGdkmm )
        {
        g_log_remove_handler("gdkmm", handlerGdkmm);
        handlerGdkmm = 0;
        }
    if ( handlerGtkmm )
        {
        g_log_remove_handler("gtkmm", handlerGtkmm);
        handlerGtkmm = 0;
        }
    message("log capture discontinued");
}



} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



