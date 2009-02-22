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


import java.awt.BorderLayout;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextPane;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;



/**
 * A simple script editor for quick fixes.
 */
public class Editor extends JPanel
{
ScriptConsole parent;
JTextPane textPane;

//########################################################################
//# MESSSAGES
//########################################################################
void err(String fmt, Object... arguments)
{
    parent.err("Editor err:" + fmt, arguments);
}

void msg(String fmt, Object... arguments)
{
    parent.msg("Editor:" + fmt, arguments);
}

void trace(String fmt, Object... arguments)
{
    parent.trace("Editor:" + fmt, arguments);
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
        _chooser.setCurrentDirectory(new File("."));
        FileNameExtensionFilter filter = new FileNameExtensionFilter(
              "Script Files", "js", "py", "r");
        _chooser.setFileFilter(filter);
        }
    return _chooser;
}


/**
 * Returns the current text contained in this editor
 */
public String getText()
{
    return textPane.getText();
}


String lastHash = null;

/**
 *  Sets the text of this editor
 */
public void setText(String txt)
{
    textPane.setText(txt);
    lastHash = getHash(txt);
    trace("hash:" + lastHash);
}

MessageDigest md = null;

final String hex = "0123456789abcdef";

String toHex(byte arr[])
{
    StringBuffer buf = new StringBuffer();
    for (byte b : arr)
        {
        buf.append(hex.charAt((b>>4) & 15));
        buf.append(hex.charAt((b   ) & 15));
		}
	return buf.toString();
}

String getHash(String text)
{
    if (md == null)
        {
        try
            {
            md = MessageDigest.getInstance("MD5");
			}
		catch (NoSuchAlgorithmException e)
		    {
		    err("getHash: " + e);
			return "";
			}
		}
    byte hash[] = md.digest(text.getBytes());
    return toHex(hash);
}


//########################################################################
//# L O A D    /     S A V E
//########################################################################
String fileName = "";

/**
 *  Gets the name of the current file in the editor
 */
public String getFileName()
{
    return fileName;
}

/**
 *  Sets the name of the current file in the editor
 */
public void setFileName(String val)
{
    fileName = val;
}

/**
 *  Selects and opens a file, loading into the editor
 */
public boolean openFile()
{
    JFileChooser chooser = getChooser();
    int ret = chooser.showOpenDialog(this);
    if (ret != JFileChooser.APPROVE_OPTION)
        return false;
    File f = chooser.getSelectedFile();
    String fname = f.getName();
    try
	    {
		FileReader in = new FileReader(fname);
        StringBuffer buf = new StringBuffer();
        while (true)
            {
            int ch = in.read();
            if (ch < 0)
                break;
            buf.append((char)ch);
            }
        in.close();
        setText(buf.toString());
        }
    catch (IOException e)
        {
        err("save file:" + e);
        return false;
		}
    return true;
}


/**
 *  Saves the file currently in the editor.  Uses the Save
 *  selector if there is not current file name. 
 */ 
public boolean saveFile()
{
    if (!isDirty())
        return true;

    String fname = getFileName();
    if (fname == null || fname.length()==0)
        {
        JFileChooser chooser = getChooser();
        int ret = chooser.showSaveDialog(this);
        if (ret != JFileChooser.APPROVE_OPTION)
            return false;
        File f = chooser.getSelectedFile();
        fname = f.getName();
        }
    try
	    {
		FileWriter out = new FileWriter(fname);
        out.write(getText());
        out.close();
        setFileName(fname);
        resetDirty();
        }
    catch (IOException e)
        {
        err("save file:" + e);
        return false;
		}
    return true;
}


/**
 *  Saves the file currently in the editor under a new name.
 *  Get the new name from the chooser, and see if it already exists. 
 */ 
public boolean saveAsFile()
{
    JFileChooser chooser = getChooser();
    int ret = chooser.showSaveDialog(this);
    if (ret != JFileChooser.APPROVE_OPTION)
        return false;
    File f = chooser.getSelectedFile();
    String fname = f.getName();
    if (f.exists())
        {
        ret = JOptionPane.showConfirmDialog(this,
		              "File '" + fname + "' already exists.  Overwrite?");
		if (ret != JOptionPane.YES_OPTION)
		    return false;
		}
    try
	    {
		FileWriter out = new FileWriter(fname);
        out.write(getText());
        out.close();
        setFileName(fname);
        resetDirty();
        }
    catch (IOException e)
        {
        err("saveAs file:" + e);
        return false;
		}
    return true;
}


/**
 * State that the editor is now 'unedited'
 */
public void resetDirty()
{
    lastHash = getHash(getText());
}

/**
 * Determines if the editor has been edited since the last open/save
 */
public boolean isDirty()
{
    String txt = getText();
    String hash = getHash(txt);
    if ( (lastHash == null && txt.length()>0) ||
	     (lastHash != null && !lastHash.equals(hash)) )
        return true;
    return false;
}


/**
 * Creates the editor for the ScriptConsole
 */
public Editor(ScriptConsole par)
{
    super();
    parent = par;
    setLayout(new BorderLayout());
    textPane = new JTextPane();
    add(textPane, BorderLayout.CENTER);
}



}

