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

import java.awt.Image;
import javax.swing.ImageIcon;
import java.net.URL;



/**
 * This class will hold various functions for getting a
 * resource from the classpath or jarfile
 */
public class Resource
{


public static ImageIcon getIcon(String name)
{

    String path = "/data/icons/" + name;
    URL imgurl = Resource.class.getResource(path);
    if (imgurl == null)
        {
        System.err.println("Icon '" + path + "' not found");
        return null;
        }
    ImageIcon icon = new ImageIcon(imgurl);
    return icon;
}

public static Image getImage(String name)
{

    ImageIcon icon = getIcon(name);
    if (icon == null)
        return null;
    return icon.getImage();
}


}

