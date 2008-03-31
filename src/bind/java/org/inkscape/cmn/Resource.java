

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

