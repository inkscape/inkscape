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

//####for xml
//read
import org.w3c.dom.Document;
import java.io.ByteArrayInputStream;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
//write
import java.io.ByteArrayOutputStream;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

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
void err(String message)
{
    ScriptConsole console = ScriptConsole.getInstance();
    if (console != null)
        console.err("Gateway err:" + message);
    else
        log("Gateway err:" + message);
}

void msg(String message)
{
    ScriptConsole console = ScriptConsole.getInstance();
    if (console != null)
        console.msg("Gateway err:" + message);
    else
        log("Gateway:" + message);
}

void trace(String message)
{
    ScriptConsole console = ScriptConsole.getInstance();
    if (console != null)
        console.trace("Gateway:" + message);
    else
        log("Gateway:" + message);
}


//########################################################################
//# U T I L I T Y
//########################################################################

/**
 * Parse a String to an XML Document
 */ 
public Document stringToDoc(String xmlStr)
{
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
        err("stringToDoc:" + e);
        return null;
		}
    catch (javax.xml.parsers.ParserConfigurationException e)
        {
        err("stringToDoc:" + e);
        return null;
		}
    catch (org.xml.sax.SAXException e)
        {
        err("stringToDoc:" + e);        
        return null;
		}
    return doc;
}



/**
 * Serialize an XML Document to a string
 */ 
String docToString(Document doc)
{
    if (doc == null)
        return "";
    String buf = "";
    try
        {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        TransformerFactory factory = TransformerFactory.newInstance();
        Transformer tf = factory.newTransformer();
        tf.transform(new DOMSource(doc), new StreamResult(baos));
        baos.close();
        buf = baos.toString();
        }
    catch (java.io.IOException e)
        {
        err("docToString:" + e);
        return null;
		}
    catch (javax.xml.transform.TransformerConfigurationException e)
        {
        err("docToString:" + e);
        return null;
		}
    catch (javax.xml.transform.TransformerException e)
        {
        err("docToString:" + e);        
        return null;
		}
    return buf;
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
    return stringToDoc(xmlStr);
}

private native boolean documentSet(long backPtr, String xmlStr);

public boolean documentSet(String xmlStr)
{
    return documentSet(backPtr, xmlStr);
}

public boolean documentSetXml(Document doc)
{
    String xmlStr = docToString(doc);
    return documentSet(xmlStr);
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


