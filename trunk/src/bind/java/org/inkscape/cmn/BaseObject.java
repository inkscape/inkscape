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
 * This is the base of all classes which map to a corresponding
 * C++ object.  The _pointer value is storage for the C++ object's
 * pointer.  Construct() and destruct() are called for when things
 * need to be setup or cleaned up on the C++ side during creation or
 * destruction of this object.
 * 
 * @see BaseInterface for how we add multiple inheritance to classes
 * which are rooted on this class.   
 */     
public class BaseObject
{

private long _pointer;

/**
 * getPointer() means that -any- java instance rooted on
 * either BaseObject or BaseInterface can call getPointer() to get the
 * handle to the associated C++ object pointer.  The difference is that
 * BaseObject holds the actual pointer, while BaseInterface refers to 
 * its owner BaseObject.
 */
protected long getPointer()
{
    return _pointer;
}

/**
 * sets the pointer to the associated C++ object to a new value
 */
protected void setPointer(long val)
{
    _pointer = val;
}

protected native void construct();

protected native void destruct();

protected BaseInterface imbue(BaseInterface intf)
{
    intf.setParent(this);
    return intf;
}


/**
 * Simple constructor
 */ 
public BaseObject()
{
    construct();
}

}
