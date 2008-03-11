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

package org.inkscape.cmn;

import javax.script.*;
import java.util.List;
import java.io.FileReader;
import java.io.PrintStream;
import java.io.OutputStream;
import java.io.IOException;
import javax.swing.JOptionPane;


/**
 * Runs scripts
 */
public class ScriptRunner
{
/**
 * Pointer back to the BinderyImpl C++ object that launched me
 */ 
long backPtr;

/**
 * The script engine manager that we want to use
 */ 
ScriptEngineManager scriptEngineManager;


//########################################################################
//# MESSSAGES
//########################################################################
static void err(String message)
{
    System.err.println("ScriptRunner err:" + message);
}

static void msg(String message)
{
    System.out.println("ScriptRunner:" + message);
}

static void trace(String message)
{
    log.println("ScriptRunner:" + message);
}



//########################################################################
//# REDIRECT STDERR / STDOUT
//########################################################################
/**
 * Redirect stdout
 */
public native void stdOutWrite(long ptr, int b);
class StdOutStream extends OutputStream
{

public void write(int b)
{
    stdOutWrite(backPtr, b);
}

}


/**
 * Redirect stderr
 */
public native void stdErrWrite(long ptr, int b);
class StdErrStream extends OutputStream
{

public void write(int b)
{
    stdErrWrite(backPtr, b);
}


}

/**
 * A logging stream
 */
static PrintStream log;
public native void logWrite(long ptr, int b);
class LogStream extends OutputStream
{

public void write(int b)
{
    logWrite(backPtr, b);
}


}


//########################################################################
//# RUN
//########################################################################


/**
 * Run a script buffer
 *
 * @param lang the scripting language to run
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public boolean doRun(String lang, String str)
{
    // create JavaScript engine
    ScriptEngine engine = scriptEngineManager.getEngineByName(lang);
    if (engine == null)
        {
        err("doRun: cannot find script engine '" + lang + "'");
        return false;
		}
    //execute script from buffer
    try
        {
        engine.eval(str);
        }
    catch (javax.script.ScriptException e)
        {
        err("Executing script: " + e);
        e.printStackTrace();
        }
    return true;
}

/**
 * Run a script buffer
 *
 * @param backPtr pointer back to the C context that called this
 * @param lang the scripting language to run
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public static boolean run(String lang, String str)
{
    //wrap whole thing in try/catch, since this will
    //likely be called from C
    try
        {
        ScriptRunner runner = getInstance();
        if (runner == null)
            {
            err("ScriptRunner not initialized");
            return false;
		    }
        return runner.doRun(lang, str);
        }
    catch (Exception e)
        {
        err("run :" + e);
        e.printStackTrace();
        return false;
		}
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
    // create JavaScript engine
    ScriptEngine engine = scriptEngineManager.getEngineByName(lang);
    if (engine == null)
        {
        err("doRunFile: cannot find script engine '" + lang + "'");
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
 * @param backPtr pointer back to the C context that called this
 * @param lang the scripting language to run
 * @param fname the script file to execute
 * @return true if successful, else false
 */
public static boolean runFile(String lang, String fname)
{
    //wrap whole thing in try/catch, since this will
    //likely be called from C
    try
        {
        ScriptRunner runner = getInstance();
        if (runner == null)
            {
            err("ScriptRunner not initialized");
            return false;
		    }
        return runner.doRunFile(lang, fname);
        }
    catch (Exception e)
        {
        err("run :" + e);
        return false;
		}
}



//########################################################################
//# CONSTRUCTOR
//########################################################################



private static ScriptRunner _instance = null;
public static ScriptRunner getInstance()
{
    return _instance;
}

private void listFactories()
{
    List<ScriptEngineFactory> factories = 
          scriptEngineManager.getEngineFactories();
    for (ScriptEngineFactory factory: factories)
	    {
        log.println("ScriptEngineFactory Info");
        String engName     = factory.getEngineName();
        String engVersion  = factory.getEngineVersion();
        String langName    = factory.getLanguageName();
        String langVersion = factory.getLanguageVersion();
        log.printf("\tScript Engine: %s (%s)\n", 
               engName, engVersion);
        List<String> engNames = factory.getNames();
        for(String name: engNames)
		    {
            log.printf("\tEngine Alias: %s\n", name);
            }
        log.printf("\tLanguage: %s (%s)\n", 
               langName, langVersion);
        }
}


   
/**
 * Constructor
 * @param backPtr pointer back to the C context that called this
 */
public ScriptRunner(long backPtr)
{
    /**
     * Set up the output, error, and logging stream
     */	     
    System.setOut(new PrintStream(new StdOutStream()));
    System.setErr(new PrintStream(new StdErrStream()));
    log = new PrintStream(new LogStream());

    //Point back to C++ object
    this.backPtr = backPtr;
    
    //Start up the factory
    scriptEngineManager  = new ScriptEngineManager();
    listFactories();
    _instance = this;
}


static
{

}

}
//########################################################################
//# E N D    O F    F I L E
//########################################################################


