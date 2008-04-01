/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2007-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

package org.inkscape.script;

import org.inkscape.cmn.Resource;

import javax.script.*;

import javax.swing.WindowConstants;
import javax.swing.JFrame;
import javax.swing.JButton;
import javax.swing.JMenu;
import javax.swing.JLabel;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JComboBox;
import javax.swing.ButtonGroup;
import javax.swing.JOptionPane;
import javax.swing.JTabbedPane;
import javax.swing.JToolBar;
import javax.swing.Action;
import javax.swing.AbstractAction;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.BorderLayout;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;


import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.List;



/**
 *  This is the main Script Console window.   It contains
 *  a terminal-like console, and a simple script editor. 
 */
public class ScriptConsole extends JFrame
{
Terminal    terminal;
Editor      editor;

JTabbedPane tabPane;
JToolBar    toolbar;
JMenuBar    menubar;
JComboBox   engineBox;

//########################################################################
//# MESSSAGES
//########################################################################
void err(String fmt, Object... arguments)
{
    terminal.errorf("ScriptConsole err:" + fmt + "\n", arguments);
}

void msg(String fmt, Object... arguments)
{
    terminal.outputf("ScriptConsole:" + fmt, arguments);
}

void trace(String fmt, Object... arguments)
{
    terminal.outputf("ScriptConsole:" + fmt + "\n", arguments);
}






void alert(String msg)
{
    JOptionPane.showMessageDialog(this, msg);
}




//########################################################################
//# S C R I P T S
//########################################################################
ScriptEngine engine;

ArrayList<ScriptEngine> engines;


public void setEngine(ScriptEngine engine)
{
    this.engine = engine;
    this.engine.getContext().setWriter(terminal.getOutWriter());
    this.engine.getContext().setErrorWriter(terminal.getErrWriter());
    //do something to make the combobox show the current engine
}


public ScriptEngine getEngine()
{
    return engine;
}


public boolean setEngine(String langName)
{
    for (ScriptEngine engine : engines)
        {
        for(String name: engine.getFactory().getNames())
		    {
            if (langName.equalsIgnoreCase(name))
                {
                setEngine(engine);
                return true;
			    }
			}
		}
    return false;
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


public void actionPerformed(ActionEvent evt)
{
    int index = engineBox.getSelectedIndex();
    if (index<0)
        return;
    ScriptEngine engine = engines.get(index);
    setEngine(engine);
}

public ScriptEngineAction()
{
    super("SelectEngine", null);
    putValue(SHORT_DESCRIPTION, "Select a scripting engine");
}

}


private void initScripts()
{
    engines = new ArrayList<ScriptEngine>();
    Action action = new ScriptEngineAction();
    engineBox = new JComboBox();
    engineBox.setAction(action);
    engineBox.setEditable(false);
    toolbar.add(engineBox);

    ScriptEngineManager scriptEngineManager =
             new ScriptEngineManager();
    List<ScriptEngineFactory> factories =
	        scriptEngineManager.getEngineFactories();
    for (ScriptEngineFactory factory: factories)
	    {
        trace("ScriptEngineFactory Info");
        String engName     = factory.getEngineName();
        String engVersion  = factory.getEngineVersion();
        String fullEngName = engName + " (" + engVersion + ")";
        String langName    = factory.getLanguageName();
        String langVersion = factory.getLanguageVersion();
        String fullLangName = langName + " (" + langVersion + ")";
        trace("\t" + fullEngName);
        List<String> engNames = factory.getNames();
        for(String name: engNames)
		    {
            trace("\tEngine Alias: " + name);
            }
        trace("\t" + fullLangName);
        engines.add(factory.getScriptEngine());
        engineBox.addItem(fullLangName + " / " + fullEngName);
        }
    if (engineBox.getItemCount()>0)
        {
        engineBox.setSelectedIndex(0);
        setEngine(engines.get(0));
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
    editor.openFile();
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
    editor.saveFile();
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
    editor.saveAsFile();
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

    editor = new Editor(this);
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

