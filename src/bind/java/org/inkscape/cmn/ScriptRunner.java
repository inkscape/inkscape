/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2007 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
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
import java.io.IOException;
import javax.swing.JOptionPane;




public class ScriptRunner
{



static void err(String message)
{
    JOptionPane.showMessageDialog(null, message,
               "Script Error", JOptionPane.ERROR_MESSAGE);
}



public static boolean run(String lang, String str)
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

public static boolean runFile(String lang, String fname)
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



}
