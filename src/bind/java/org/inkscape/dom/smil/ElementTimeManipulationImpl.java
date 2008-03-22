/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (c) 2007-2008 Inkscape.org
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
 *
 *  Note that the SMIL files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/smil-boston-dom/java-binding.html
 */

package org.inkscape.dom.smil;

import org.w3c.dom.DOMException;


public class ElementTimeManipulationImpl
       extends
             org.inkscape.cmn.BaseInterface
       implements org.w3c.dom.smil.ElementTimeManipulation
{

public native float getSpeed();
public native void setSpeed(float speed)
                        throws DOMException;


public native float getAccelerate();
public native void setAccelerate(float accelerate)
                        throws DOMException;


public native float getDecelerate();
public native void setDecelerate(float decelerate)
                        throws DOMException;


public native boolean getAutoReverse();
public native void setAutoReverse(boolean autoReverse)
                        throws DOMException;

}

