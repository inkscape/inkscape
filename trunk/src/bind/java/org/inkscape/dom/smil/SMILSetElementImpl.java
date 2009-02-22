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

import org.w3c.dom.smil.TimeList;



public class SMILSetElementImpl
       extends SMILElementImpl
               //ElementTimeControl,
               //ElementTime,
               //ElementTargetAttributes
       implements org.w3c.dom.smil.SMILSetElement
{

public SMILSetElementImpl()
{
    imbue(_ElementTimeControl = new ElementTimeControlImpl());
    imbue(_ElementTime = new ElementTimeImpl());
    imbue(_ElementTargetAttributes = new ElementTargetAttributesImpl());
}


//from ElementTimeControl
ElementTimeControlImpl _ElementTimeControl;
public boolean beginElementAt(float offset) throws DOMException
    { return _ElementTimeControl.beginElementAt(offset); }
public boolean endElementAt(float offset) throws DOMException
    { return _ElementTimeControl.endElementAt(offset); }
//end ElementTimeControl

//from ElementTime
ElementTimeImpl _ElementTime;
public TimeList getBegin()
    { return _ElementTime.getBegin(); }
public void setBegin(TimeList begin) throws DOMException
    { _ElementTime.setBegin(begin); }
public TimeList getEnd()
    { return _ElementTime.getEnd(); }
public void setEnd(TimeList end) throws DOMException
    { _ElementTime.setEnd(end); }
public float getDur()
    { return _ElementTime.getDur(); }
public void setDur(float dur) throws DOMException
    { _ElementTime.setDur(dur); }
public short getRestart()
    { return _ElementTime.getRestart(); }
public void setRestart(short restart) throws DOMException
    { _ElementTime.setRestart(restart); }
public short getFill()
    { return _ElementTime.getFill(); }
public void setFill(short fill) throws DOMException
    { _ElementTime.setFill(fill); }
public float getRepeatCount()
    { return _ElementTime.getRepeatCount(); }
public void setRepeatCount(float repeatCount) throws DOMException
    { _ElementTime.setRepeatCount(repeatCount); }
public float getRepeatDur()
    { return _ElementTime.getRepeatDur(); }
public void setRepeatDur(float repeatDur) throws DOMException
    { _ElementTime.setRepeatDur(repeatDur); }
public boolean beginElement()
    { return _ElementTime.beginElement(); }
public boolean endElement()
    { return _ElementTime.endElement(); }
public void pauseElement()
    { _ElementTime.pauseElement(); }
public void resumeElement()
    { _ElementTime.resumeElement(); }
public void seekElement(float seekTo)
    { _ElementTime.seekElement(seekTo); }
//end ElementTime

//from ElementTargetAttributes
ElementTargetAttributesImpl _ElementTargetAttributes;
public String getAttributeName()
    { return _ElementTargetAttributes.getAttributeName(); }
public void setAttributeName(String attributeName)
    { _ElementTargetAttributes.setAttributeName(attributeName); }
public short getAttributeType()
    { return _ElementTargetAttributes.getAttributeType(); }
public void setAttributeType(short attributeType)
    { _ElementTargetAttributes.setAttributeType(attributeType); }
//end ElementTargetAttributes




public native String getTo();
public native void setTo(String to);

}

