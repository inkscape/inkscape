/**
 *  Dialog for executing and monitoring script execution
 *  
 * Author:  
 *   Bob Jamison
 *
 * Copyright (C) 2004-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/menubar.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

#include "scriptdialog.h"
#include <extension/script/InkscapeScript.h>



namespace Inkscape
{
namespace UI
{
namespace Dialog
{



//#########################################################################
//## I M P L E M E N T A T I O N
//#########################################################################

/**
 * A script editor/executor
 */
class ScriptDialogImpl : public ScriptDialog
{

    public:


    /**
     * Constructor
     */
    ScriptDialogImpl();

    /**
     * Destructor
     */
    ~ScriptDialogImpl()
        {}


    /**
     * Clear the text
     */
    void clear();

    /**
     * Execute the script
     */
    void execute(Inkscape::Extension::Script::InkscapeScript::ScriptLanguage lang);

    /**
     * Execute a Javascript script
     */
    void executeJavascript();

    /**
     * Execute a Python script
     */
    void executePython();

    /**
     * Execute a Ruby script
     */
    void executeRuby();



    private:


    Gtk::MenuBar menuBar;

    Gtk::Menu   fileMenu;

    //## Script text
    Gtk::Frame          scriptTextFrame;
    Gtk::ScrolledWindow scriptTextScroll;
    Gtk::TextView       scriptText;

    //## Output text
    Gtk::Frame          outputTextFrame;
    Gtk::ScrolledWindow outputTextScroll;
    Gtk::TextView       outputText;

    //## Error text
    Gtk::Frame          errorTextFrame;
    Gtk::ScrolledWindow errorTextScroll;
    Gtk::TextView       errorText;



};

static const char *defaultCodeStr =
    "/**\n"
    " * This is some example Javascript.\n"
    " * Try 'Execute Javascript'\n"
    " */\n"
    "function sayHello() {\n"
    "  println('Hello, world!');\n"
    "}\n"
    "\n"
    "sayHello();\n"
    "\n";




//#########################################################################
//## E V E N T S
//#########################################################################

static void textViewClear(Gtk::TextView &view)
{
    Glib::RefPtr<Gtk::TextBuffer> buffer = view.get_buffer();
    buffer->erase(buffer->begin(), buffer->end());
}


/**
 * Also a public method.  Remove all text from the dialog
 */
void ScriptDialogImpl::clear()
{
    textViewClear(scriptText);
    textViewClear(outputText);
    textViewClear(errorText);
}

/**
 * Execute the script in the dialog
 */
void
ScriptDialogImpl::execute(Inkscape::Extension::Script::InkscapeScript::ScriptLanguage
lang)
{
    Glib::ustring script = scriptText.get_buffer()->get_text(true);
    Glib::ustring output;
    Glib::ustring error;
    Inkscape::Extension::Script::InkscapeScript engine;
    bool ok = engine.interpretScript(script, output, error, lang);
    outputText.get_buffer()->set_text(output);
    errorText.get_buffer()->set_text(error);
    if (!ok)
        {
        //do we want something here?
        }
}

/**
 * Execute the script in the dialog
 */
void ScriptDialogImpl::executeJavascript()
{
    execute(Inkscape::Extension::Script::InkscapeScript::JAVASCRIPT);
}

/**
 * Execute the script in the dialog
 */
void ScriptDialogImpl::executePython()
{
    execute(Inkscape::Extension::Script::InkscapeScript::PYTHON);
}

/**
 * Execute the script in the dialog
 */
void ScriptDialogImpl::executeRuby()
{
    execute(Inkscape::Extension::Script::InkscapeScript::RUBY);
}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
ScriptDialogImpl::ScriptDialogImpl() :
    ScriptDialog()
{
    Gtk::Box *contents = _getContents();

    //## Add a menu for clear()
    menuBar.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_File"), fileMenu) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_Clear"),
           sigc::mem_fun(*this, &ScriptDialogImpl::clear) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_Execute Javascript"),
           sigc::mem_fun(*this, &ScriptDialogImpl::executeJavascript) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_Execute Python"),
           sigc::mem_fun(*this, &ScriptDialogImpl::executePython) ) );
    fileMenu.items().push_back( Gtk::Menu_Helpers::MenuElem(_("_Execute Ruby"),
           sigc::mem_fun(*this, &ScriptDialogImpl::executeRuby) ) );
    contents->pack_start(menuBar, Gtk::PACK_SHRINK);

    //### Set up the script field
    scriptText.set_editable(true);
    scriptText.get_buffer()->set_text(defaultCodeStr);
    scriptTextScroll.add(scriptText);
    scriptTextScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    scriptTextFrame.set_label(_("Script"));
    scriptTextFrame.set_shadow_type(Gtk::SHADOW_NONE);
    scriptTextFrame.add(scriptTextScroll);
    contents->pack_start(scriptTextFrame);

    //### Set up the output field
    outputText.set_editable(true);
    outputText.get_buffer()->set_text("");
    outputTextScroll.add(outputText);
    outputTextScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    outputTextFrame.set_label(_("Output"));
    outputTextFrame.set_shadow_type(Gtk::SHADOW_NONE);
    outputTextFrame.add(outputTextScroll);
    contents->pack_start(outputTextFrame);

    //### Set up the error field
    errorText.set_editable(true);
    errorText.get_buffer()->set_text("");
    errorTextScroll.add(errorText);
    errorTextScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    errorTextFrame.set_label(_("Errors"));
    errorTextFrame.set_shadow_type(Gtk::SHADOW_NONE);
    errorTextFrame.add(errorTextScroll);
    contents->pack_start(errorTextFrame);

    // sick of this thing shrinking too much
    set_size_request(350, 400);
    show_all_children();

}

/**
 * Factory method.  Use this to create a new ScriptDialog
 */
ScriptDialog &ScriptDialog::getInstance()
{
    ScriptDialog *dialog = new ScriptDialogImpl();
    return *dialog;
}






} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



