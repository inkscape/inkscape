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



import javax.swing.JPanel;
import javax.swing.JTextPane;
import java.awt.BorderLayout;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;


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

public boolean isDirty()
{
    String txt = getText();
    String hash = getHash(txt);
    if (lastHash != null && !lastHash.equals(hash))
        return true;
    return false;
}


/**
 *
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

