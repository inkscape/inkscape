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



public class SMILMediaElementImpl
       extends SMILElementImpl //ElementTime
       implements org.w3c.dom.smil.SMILMediaElement
{
//from ElementTime
public native TimeList getBegin();
public native void setBegin(TimeList begin)
                 throws DOMException;


public native TimeList getEnd();
public native void setEnd(TimeList end)
                 throws DOMException;


public native float getDur();
public native void setDur(float dur)
                 throws DOMException;



public native short getRestart();
public native void setRestart(short restart)
                 throws DOMException;



public native short getFill();
public native void setFill(short fill)
                 throws DOMException;


public native float getRepeatCount();
public native void setRepeatCount(float repeatCount)
                 throws DOMException;


public native float getRepeatDur();
public native void setRepeatDur(float repeatDur)
                 throws DOMException;


public native boolean beginElement();


public native boolean endElement();


public native void pauseElement();


public native void resumeElement();


public native void seekElement(float seekTo);
//end ElementTime


public native String getAbstractAttr();
public native void setAbstractAttr(String abstractAttr)
                              throws DOMException;


public native String getAlt();
public native void setAlt(String alt)
                              throws DOMException;

public native String getAuthor();
public native void setAuthor(String author)
                              throws DOMException;

public native String getClipBegin();
public native void setClipBegin(String clipBegin)
                              throws DOMException;

public native String getClipEnd();
public native void setClipEnd(String clipEnd)
                              throws DOMException;

public native String getCopyright();
public native void setCopyright(String copyright)
                              throws DOMException;

public native String getLongdesc();
public native void setLongdesc(String longdesc)
                              throws DOMException;

public native String getPort();
public native void setPort(String port)
                              throws DOMException;

public native String getReadIndex();
public native void setReadIndex(String readIndex)
                              throws DOMException;

public native String getRtpformat();
public native void setRtpformat(String rtpformat)
                              throws DOMException;

public native String getSrc();
public native void setSrc(String src)
                              throws DOMException;

public native String getStripRepeat();
public native void setStripRepeat(String stripRepeat)
                              throws DOMException;

public native String getTitle();
public native void setTitle(String title)
                              throws DOMException;

public native String getTransport();
public native void setTransport(String transport)
                              throws DOMException;

public native String getType();
public native void setType(String type)
                              throws DOMException;

}

