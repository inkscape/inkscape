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

import java.util.List;
import java.io.FileReader;
import java.io.PrintStream;
import java.io.OutputStream;
import java.io.IOException;
import javax.swing.JOptionPane;

//for xml
import org.w3c.dom.Document;
import java.io.ByteArrayInputStream;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.inkscape.script.ScriptConsole;



/**
 * Provide a gateway from C to Java, to simplify adding
 * interfaces. 
 */
public class Gateway
{
/**
 * Pointer back to the BinderyImpl C++ object that launched me
 */ 
long backPtr;


//########################################################################
//# MESSSAGES
//########################################################################
static void err(String message)
{
    System.err.println("Gateway err:" + message);
}

static void msg(String message)
{
    System.out.println("Gateway:" + message);
}




//########################################################################
//# R E P R  (inkscape's xml tree)
//########################################################################

private native String documentGet(long backPtr);

public String documentGet()
{
    return documentGet(backPtr);
}

public Document documentGetXml()
{
    String xmlStr = documentGet();
    if (xmlStr == null || xmlStr.length()==0)
        return null;
    Document doc = null;
    try
        {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder parser = factory.newDocumentBuilder();
        doc = parser.parse(new ByteArrayInputStream(xmlStr.getBytes()));
        }
    catch (java.io.IOException e)
        {
        err("getReprXml:" + e);
		}
    catch (javax.xml.parsers.ParserConfigurationException e)
        {
        err("getReprXml:" + e);
		}
    catch (org.xml.sax.SAXException e)
        {
        err("getReprXml:" + e);        
		}
    return doc;
}

private native boolean documentSet(long backPtr, String xmlStr);

public boolean documentSet(String xmlStr)
{
    return documentSet(backPtr, xmlStr);
}

//########################################################################
//# LOGGING STREAM
//########################################################################

public native void logWrite(long backptr, int ch);

class LogStream extends OutputStream
{

public void write(int ch)
{
    logWrite(backPtr, ch);
}

}

PrintStream log = null;

/**
 * printf-style logging
 */ 
void log(String fmt, Object... args)
{
    log.printf("Gateway:" + fmt, args);
}


//########################################################################
//# RUN
//########################################################################


/**
 * Run a script buffer
 *
 * @param backPtr pointer back to the C context that called this
 * @param lang the scripting language to run
 * @param str the script buffer to execute
 * @return true if successful, else false
 */
public boolean scriptRun(String lang, String str)
{
    //wrap whole thing in try/catch, since this will
    //likely be called from C
    try
        {
        ScriptConsole console = ScriptConsole.getInstance();
        if (console == null)
            {
            err("ScriptConsole not initialized");
            return false;
		    }
        return console.doRun(lang, str);
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
 * @param backPtr pointer back to the C context that called this
 * @param lang the scripting language to run
 * @param fname the script file to execute
 * @return true if successful, else false
 */
public boolean scriptRunFile(String lang, String fname)
{
    //wrap whole thing in try/catch, since this will
    //likely be called from C
    try
        {
        {
        ScriptConsole console = ScriptConsole.getInstance();
        if (console == null)
            {
            err("ScriptConsole not initialized");
            return false;
		    }
        return console.doRun(lang, fname);
        }
        }
    catch (Exception e)
        {
        err("scriptRunFile :" + e);
        return false;
		}
}





//########################################################################
//# C O N S O L E
//########################################################################


public boolean showConsole()
{
    ScriptConsole.getInstance().setVisible(true);
    return true;
}


//########################################################################
//# CONSTRUCTOR
//########################################################################



   
/**
 * Constructor
 * @param backPtr pointer back to the C context that called this
 */
public Gateway(long backPtr)
{
    /**
     * Set up the logging stream
     */	     
    log = new PrintStream(new LogStream());

    //Point back to C++ object
    this.backPtr = backPtr;
    
    _instance = this;
}

private static Gateway _instance = null;

public static Gateway getInstance()
{
	return _instance;
}

}
//########################################################################
//# E N D    O F    F I L E
//########################################################################


