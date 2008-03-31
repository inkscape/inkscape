

package org.inkscape.script;

import org.inkscape.cmn.Resource;

import javax.script.*;

import javax.swing.WindowConstants;
import javax.swing.JFrame;
import javax.swing.JButton;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.ButtonGroup;
import javax.swing.JOptionPane;
import javax.swing.JFileChooser;
import javax.swing.JTabbedPane;
import javax.swing.JToolBar;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.Action;
import javax.swing.AbstractAction;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.BorderLayout;

import java.io.File;
import java.io.FileReader;

import java.util.List;
import java.util.HashMap;



/**
 *
 */
public class ScriptConsole extends JFrame
{
Terminal    terminal;
Editor      editor;

JTabbedPane tabPane;
JToolBar    toolbar;
JMenuBar    menubar;


//########################################################################
//# MESSSAGES
//########################################################################
void err(String fmt, Object... arguments)
{
    terminal.errorf("ScriptConsole err:" + fmt, arguments);
}

void msg(String fmt, Object... arguments)
{
    terminal.outputf("ScriptConsole:" + fmt, arguments);
}

void trace(String fmt, Object... arguments)
{
    terminal.outputf("ScriptConsole:" + fmt, arguments);
}






void alert(String msg)
{
    JOptionPane.showMessageDialog(this, msg);
}


//########################################################################
//# U T I L I T Y
//########################################################################

private JFileChooser _chooser;

JFileChooser getChooser()
{
    if (_chooser == null)
        {
        _chooser = new JFileChooser();
        _chooser.setAcceptAllFileFilterUsed(false);
        FileNameExtensionFilter filter = new FileNameExtensionFilter(
              "Script Files", "js", "py", "r");
        _chooser.setFileFilter(filter);
        }
    return _chooser;
}



//########################################################################
//# S C R I P T S
//########################################################################
ScriptEngine engine;

HashMap<String, ScriptEngineAction> scriptEngineActions;

public void setEngine(ScriptEngine engine)
{
    this.engine = engine;
    this.engine.getContext().setWriter(terminal.getOutWriter());
    this.engine.getContext().setErrorWriter(terminal.getErrWriter());
}


public ScriptEngine getEngine()
{
    return engine;
}


public boolean setEngine(String name)
{
    ScriptEngineAction action = scriptEngineActions.get(name);
    if (action == null)
        return false;
    action.setEnabled(true);
    setEngine(action.factory.getScriptEngine());
    return true;
}


/**
 * Run a script buffer
 *
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public boolean doRunCmd(String str)
{
    if (engine == null)
        {
        err("No engine set");
        return false;
        }

    //execute script from buffer
    try
        {
        getEngine().eval(str);
        }
    catch (javax.script.ScriptException e)
        {
        err("Executing script: " + e);
        //e.printStackTrace();
        }
    terminal.output("\nscript> ");
    return true;
}

/**
 * Run a script buffer
 *
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public boolean doRun(String str)
{
    if (engine == null)
        {
        err("No engine set");
        return false;
        }

    //execute script from buffer
    try
        {
        getEngine().eval(str);
        }
    catch (javax.script.ScriptException e)
        {
        err("Executing script: " + e);
        //e.printStackTrace();
        }
    return true;
}


/**
 * Run a script buffer
 *
 * @param lang the scripting language to run
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public boolean doRun(String lang, String str)
{
    // find script engine
    if (!setEngine(lang))
        {
        err("doRun: cannot find script engine '" + lang + "'");
        return false;
		}
    return doRun(str);
}


/**
 * Run a script file
 *
 * @param fname the script file to execute
 * @return true if successful, else false
 */
public boolean doRunFile(String fname)
{
    if (engine == null)
        {
        err("No engine set");
        return false;
        }

    //try opening file and feeding into engine
    FileReader in = null;
    boolean ret = true;
    try
        {
        in = new FileReader(fname);
        }
    catch (java.io.IOException e)
        {
        err("Executing file: " + e);
        return false;
        }
    try
        {
        engine.eval(in);
        }
    catch (javax.script.ScriptException e)
        {
        err("Executing file: " + e);
        ret = false;
        }
    try
        {
        in.close();
        }
    catch (java.io.IOException e)
        {
        err("Executing file: " + e);
        return false;
        }
    return ret;
}


/**
 * Run a script file
 *
 * @param lang the scripting language to run
 * @param fname the script file to execute
 * @return true if successful, else false
 */
public boolean doRunFile(String lang, String fname)
{
    // find script engine
    if (!setEngine(lang))
        {
        err("doRunFile: cannot find script engine '" + lang + "'");
        return false;
		}
    return doRunFile(fname);
}


class ScriptEngineAction extends AbstractAction
{
ScriptEngineFactory factory;


public void actionPerformed(ActionEvent evt)
{
    setEngine(factory.getScriptEngine());
}

public ScriptEngineAction(ScriptEngineFactory factory)
{
    super(factory.getEngineName(), null);
    putValue(SHORT_DESCRIPTION, factory.getLanguageName());
    this.factory = factory;
}

}


private void initScripts()
{
    JMenu menu = new JMenu("Language");
    ButtonGroup group = new ButtonGroup();
    menubar.add(menu);

    ScriptEngineManager scriptEngineManager =
             new ScriptEngineManager();
    List<ScriptEngineFactory> factories =
          scriptEngineManager.getEngineFactories();
    for (ScriptEngineFactory factory: factories)
	    {
        trace("ScriptEngineFactory Info");
        String engName     = factory.getEngineName();
        String engVersion  = factory.getEngineVersion();
        String langName    = factory.getLanguageName();
        String langVersion = factory.getLanguageVersion();
        trace("\tScript Engine: %s (%s)\n", engName, engVersion);
        List<String> engNames = factory.getNames();
        for(String name: engNames)
		    {
            trace("\tEngine Alias: %s\n", name);
            }
        trace("\tLanguage: %s (%s)\n", langName, langVersion);
        ScriptEngineAction action = new ScriptEngineAction(factory);
        JRadioButtonMenuItem item = new JRadioButtonMenuItem(action);
        group.add(item);
        menu.add(item);
        }
    if (menu.getItemCount()>0)
        {
        JMenuItem item = menu.getItem(0);
        group.setSelected(item.getModel(), true);
        ScriptEngineAction action = (ScriptEngineAction)item.getAction();
        setEngine(action.factory.getScriptEngine());
        }
}


static final String defaultCodeStr =
    "/**\n" +
    " * This is some example Javascript.\n" +
    " * Try executing\n" +
    " */\n" +
    "importPackage(javax.swing);\n" +
    "function sayHello() {\n" +
    "  JOptionPane.showMessageDialog(null, 'Hello, world!',\n" +
	"     'Welcome to Inkscape', JOptionPane.WARNING_MESSAGE);\n" +
    "}\n" +
    "\n" +
    "sayHello();\n" +
    "\n";


//########################################################################
//# A C T I O N S
//########################################################################
Action newAction;
Action openAction;
Action quitAction;
Action runAction;
Action saveAction;
Action saveAsAction;
Action stopAction;



class NewAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    //
}

public NewAction()
{
    super("New", Resource.getIcon("document-new.png"));
    putValue(SHORT_DESCRIPTION, "Create a new script file");
}

}



class OpenAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    JFileChooser chooser = getChooser();
    int ret = chooser.showOpenDialog(ScriptConsole.this);
    if (ret != JFileChooser.APPROVE_OPTION)
        return;
    File f = chooser.getSelectedFile();
    String fname = f.getName();
    alert("You selected : " + fname);
}

public OpenAction()
{
    super("Open", Resource.getIcon("document-open.png"));
    putValue(SHORT_DESCRIPTION, "Open a script file");
}

}



class QuitAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    setVisible(false);
}

public QuitAction()
{
    super("Quit", Resource.getIcon("system-log-out.png"));
    putValue(SHORT_DESCRIPTION, "Quit this script console");
}

}



class RunAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    String txt = editor.getText();
    doRun(txt);
}

public RunAction()
{
    super("Run", Resource.getIcon("go-next.png"));
    putValue(SHORT_DESCRIPTION, "Run the script in the editor");
}

}



class SaveAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    JFileChooser chooser = getChooser();
    int ret = chooser.showSaveDialog(ScriptConsole.this);
    if (ret != JFileChooser.APPROVE_OPTION)
        return;
    File f = chooser.getSelectedFile();
    String fname = f.getName();
    alert("You selected : " + fname);
}

public SaveAction()
{
    super("Save", Resource.getIcon("document-save.png"));
    putValue(SHORT_DESCRIPTION, "Save file");
}

}



class SaveAsAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    JFileChooser chooser = getChooser();
    int ret = chooser.showSaveDialog(ScriptConsole.this);
    if (ret != JFileChooser.APPROVE_OPTION)
        return;
    File f = chooser.getSelectedFile();
    String fname = f.getName();
    alert("You selected : " + fname);
}

public SaveAsAction()
{
    super("SaveAs", Resource.getIcon("document-save-as.png"));
    putValue(SHORT_DESCRIPTION, "Save under a new file name");
}

}



class StopAction extends AbstractAction
{

public void actionPerformed(ActionEvent evt)
{
    //#
}

public StopAction()
{
    super("Stop", Resource.getIcon("process-stop.png"));
    putValue(SHORT_DESCRIPTION, "Stop the running script");
}

}



HashMap<String, Action> actions;
void setupActions()
{
    actions = new HashMap<String, Action>();
    actions.put("New",    newAction    = new NewAction());
    actions.put("Open",   openAction   = new OpenAction());
    actions.put("Quit",   quitAction   = new QuitAction());
    actions.put("Run",    runAction    = new RunAction());
    actions.put("Save",   saveAction   = new SaveAction());
    actions.put("SaveAs", saveAsAction = new SaveAsAction());
    actions.put("Stop",   stopAction   = new StopAction());
}


public void enableAction(String name)
{
    Action action = actions.get(name);
    if (action == null)
        return;
    action.setEnabled(true);
}

public void disableAction(String name)
{
    Action action = actions.get(name);
    if (action == null)
        return;
    action.setEnabled(false);
}



//########################################################################
//# S E T U P
//########################################################################

JButton toolbarButton(Action action)
{
    JButton btn = new JButton(action);
    btn.setText("");
    btn.setToolTipText((String)action.getValue(Action.SHORT_DESCRIPTION));
    return btn;
}


private boolean setup()
{
    setTitle("Inkscape Script Console");
    setSize(600, 400);
    setIconImage(Resource.getImage("inkscape.png"));
    setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);

    //######################################################
    //#  A C T I O N S
    //######################################################
    setupActions();

    //######################################################
    //# M E N U
    //######################################################
    menubar = new JMenuBar();
    setJMenuBar(menubar);

    JMenu menu = new JMenu("File");
    menubar.add(menu);
    menu.add(new JMenuItem(openAction));
    menu.add(new JMenuItem(saveAction));
    menu.add(new JMenuItem(saveAsAction));
    menu.add(new JMenuItem(quitAction));
        
    menu = new JMenu("Run");
    menubar.add(menu);
    menu.add(new JMenuItem(runAction));
    menu.add(new JMenuItem(stopAction));

    //######################################################
    //# T O O L B A R
    //######################################################
    toolbar = new JToolBar();
    getContentPane().add(toolbar, BorderLayout.NORTH);
    toolbar.add(toolbarButton(openAction));
    toolbar.add(toolbarButton(saveAction));
    toolbar.add(toolbarButton(runAction));
    toolbar.add(toolbarButton(stopAction));

    //######################################################
    //# C O N T E N T
    //######################################################
    tabPane = new JTabbedPane();
    getContentPane().add(tabPane, BorderLayout.CENTER);
    
    terminal = new Terminal();
    tabPane.addTab("Console",
                   Resource.getIcon("utilities-terminal.png"),
                   terminal);
    terminal.output("\nscript> ");

    editor = new Editor();
    tabPane.addTab("Script",
                   Resource.getIcon("accessories-text-editor.png"),
                   editor);

    editor.setText(defaultCodeStr);
    
    //######################################################
    //#  E N G I N E
    //######################################################
    initScripts();

    return true;
}






public ScriptConsole()
{
    setup();
}


private static ScriptConsole _instance = null;
public static ScriptConsole getInstance()
{
    if (_instance == null)
        _instance = new ScriptConsole();
    return _instance;
}


public static void main(String argv[])
{
    ScriptConsole sc = getInstance();
    sc.setVisible(true);
}


}
//########################################################################
//# E N D    O F    F I L E
//########################################################################

