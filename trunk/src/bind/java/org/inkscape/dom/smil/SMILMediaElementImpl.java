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
       extends SMILElementImpl
             //ElementTime
       implements org.w3c.dom.smil.SMILMediaElement
{
public SMILMediaElementImpl()
{
    imbue(_ElementTime = new ElementTimeImpl());
}

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

