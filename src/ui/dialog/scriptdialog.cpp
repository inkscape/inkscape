/**
 * @file
 * Dialog for executing and monitoring script execution.
 */
/* Author:  
 *   Bob Jamison
 *
 * Copyright (C) 2004-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "scriptdialog.h"
#include <glibmm/i18n.h>
#include <gtkmm/menubar.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>

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
    ScriptDialogImpl();
    ~ScriptDialogImpl()
        {}


    /**
     * Remove all text from the dialog.
     */
    void clear();

    /**
     * Execute a script in the dialog.
     *
     * @param lang language in which the script is programmed
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
    "importPackage(javax.swing);\n"
    "function sayHello() {\n"
    "  JOptionPane.showMessageDialog(null, 'Hello, world!',\n"
	"     'Welcome to Inkscape', JOptionPane.WARNING_MESSAGE);\n"
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

void ScriptDialogImpl::clear()
{
    textViewClear(scriptText);
    textViewClear(outputText);
    textViewClear(errorText);
}

void ScriptDialogImpl::execute(Inkscape::Extension::Script::InkscapeScript::ScriptLanguage lang)
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

void ScriptDialogImpl::executeJavascript()
{
    execute(Inkscape::Extension::Script::InkscapeScript::JAVASCRIPT);
}

void ScriptDialogImpl::executePython()
{
    execute(Inkscape::Extension::Script::InkscapeScript::PYTHON);
}

void ScriptDialogImpl::executeRuby()
{
    execute(Inkscape::Extension::Script::InkscapeScript::RUBY);
}


//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
ScriptDialogImpl::ScriptDialogImpl() :
    ScriptDialog()
{
    Gtk::Box *contents = _getContents();

    //## Add a menu for clear()
    Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem(_("File"), true));
    item->set_submenu(fileMenu);
    menuBar.append(*item);

    item = Gtk::manage(new Gtk::MenuItem(_("_Clear"), true));
    item->signal_activate().connect(sigc::mem_fun(*this, &ScriptDialogImpl::clear));
    fileMenu.append(*item);
    
    item = Gtk::manage(new Gtk::MenuItem(_("_Execute Javascript"), true));
    item->signal_activate().connect(sigc::mem_fun(*this, &ScriptDialogImpl::executeJavascript));
    fileMenu.append(*item);
    
    item = Gtk::manage(new Gtk::MenuItem(_("_Execute Python"), true));
    item->signal_activate().connect(sigc::mem_fun(*this, &ScriptDialogImpl::executePython));
    fileMenu.append(*item);
    
    item = Gtk::manage(new Gtk::MenuItem(_("_Execute Ruby"), true));
    item->signal_activate().connect(sigc::mem_fun(*this, &ScriptDialogImpl::executeRuby));
    fileMenu.append(*item);
    
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

ScriptDialog &ScriptDialog::getInstance()
{
    ScriptDialog *dialog = new ScriptDialogImpl();
    return *dialog;
}

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
