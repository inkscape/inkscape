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
long backPtr;


/**
 * Redirect stdout
 */
private native void stdOutWrite(long ptr, int b);
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
private native void stdErrWrite(long ptr, int b);
class StdErrStream extends OutputStream
{

public void write(int b)
{
    stdErrWrite(backPtr, b);
}


}


static void err(String message)
{
    JOptionPane.showMessageDialog(null, message,
               "Script Error", JOptionPane.ERROR_MESSAGE);
}



/**
 * Run a script buffer
 *
 * @param lang the scripting language to run
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public boolean run(String lang, String str)
{
    ScriptEngineManager factory = new ScriptEngineManager();
    // create JavaScript engine
    ScriptEngine engine = factory.getEngineByName(lang);
    // evaluate JavaScript code from given file - specified by first argument
    try
        {
        engine.eval(str);
        }
    catch (javax.script.ScriptException e)
        {
        err("Executing script: " + e);
        }
    return true;
}


/**
 * Run a script file
 *
 * @param lang the scripting language to run
 * @param fname the script file to execute
 * @return true if successful, else false
 */
public boolean runFile(String lang, String fname)
{
    ScriptEngineManager factory = new ScriptEngineManager();
    // create JavaScript engine
    ScriptEngine engine = factory.getEngineByName(lang);
    // evaluate JavaScript code from given file - specified by first argument
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
 * Constructor
 * @param backPtr pointer back to the C context that called this
 */
public ScriptRunner(long backPtr)
{
    this.backPtr = backPtr;
    System.setOut(new PrintStream(new StdOutStream()));
    System.setErr(new PrintStream(new StdErrStream()));
}



private static ScriptRunner _instance = null;


public static ScriptRunner getInstance(long backPtr)
{
    if (_instance == null)
        _instance = new ScriptRunner(backPtr);
    return _instance;
}


/**
 * Run a script buffer
 *
 * @param backPtr pointer back to the C context that called this
 * @param lang the scripting language to run
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public static boolean run(long ptr, String lang, String str)
{
    ScriptRunner runner = getInstance(ptr);
    return runner.run(lang, str);
}


/**
 * Run a script file
 *
 * @param backPtr pointer back to the C context that called this
 * @param lang the scripting language to run
 * @param fname the script file to execute
 * @return true if successful, else false
 */
public static boolean runFile(long ptr, String lang, String fname)
{
    ScriptRunner runner = getInstance(ptr);
    return runner.runFile(lang, fname);
}



}
