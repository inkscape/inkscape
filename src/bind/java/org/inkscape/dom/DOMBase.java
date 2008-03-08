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

package org.inkscape.dom;



/**
 * This is the base Java class upon which
 * all of the DOM classes are rooted
 */
public class DOMBase
{

protected long _pointer;

/**
 * @see dobinding.cpp: DOMBase_construct()
 */
protected native void construct();

/**
 * @see dobinding.cpp: DOMBase_destruct()
 */
protected native void destruct();



/**
 * Overload Object.finalize() so that we
 * can perform proper cleanup.
 */
protected void finalize()
{
    destruct();
}


public DOMBase()
{
    construct();
    _pointer = 0;
}

}
//########################################################################
//# E N D    O F    F I L E
//########################################################################

