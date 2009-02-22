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

/**
 * A BaseInterface is not an object of its down, but is owned
 * by a parent BaseObject.  It has no mapping to a C++ object of
 * its own, but is merely the delegate that a BaseObject calls.
 * This is how we provide some semblance of multiple inheritance,
 * and keep the native method count down.
 */      
public class BaseInterface extends BaseObject
{
BaseObject parent;


public void setParent(BaseObject par)
{
    parent = par;
}

/**
 * Overloaded.  getPointer() means that -any- java instance rooted on
 * either BaseObject or BaseInterface can call getPointer() to get the
 * handle to the associated C++ object pointer.  The difference is that
 * BaseObject holds the actual pointer, while BaseInterface refers to 
 * its owner BaseObject.
 */
protected long getPointer()
{
    if (parent == null)
        return 0L;
    else
        return parent.getPointer();
}

/**
 * Since this is an interface, construct() 
 * means nothing.  Nothing must happen
 */
protected void construct()
{
}

/**
 * Since this is an interface, destruct() 
 * means nothing.  Nothing must happen
 */
protected void destruct()
{
}


/**
 * Instances of this "interface"  can only exist parasitically attached
 * to a BaseObject
 */  
public BaseInterface()
{
     setParent(null);
}

}
