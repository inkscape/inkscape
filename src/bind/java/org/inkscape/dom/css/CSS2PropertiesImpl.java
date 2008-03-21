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
 *  Note that these files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/DOM-Level-2-Style
 */


package org.inkscape.dom.css;

import org.w3c.dom.DOMException;


public class CSS2PropertiesImpl
       implements org.w3c.dom.css.CSS2Properties
{

public native String getAzimuth();
public native void setAzimuth(String azimuth)
                                         throws DOMException;

public native String getBackground();
public native void setBackground(String background)
                                         throws DOMException;

public native String getBackgroundAttachment();
public native void setBackgroundAttachment(String backgroundAttachment)
                                         throws DOMException;

public native String getBackgroundColor();
public native void setBackgroundColor(String backgroundColor)
                                         throws DOMException;

public native String getBackgroundImage();
public native void setBackgroundImage(String backgroundImage)
                                         throws DOMException;

public native String getBackgroundPosition();
public native void setBackgroundPosition(String backgroundPosition)
                                         throws DOMException;

public native String getBackgroundRepeat();
public native void setBackgroundRepeat(String backgroundRepeat)
                                         throws DOMException;

public native String getBorder();
public native void setBorder(String border)
                                         throws DOMException;

public native String getBorderCollapse();
public native void setBorderCollapse(String borderCollapse)
                                         throws DOMException;

public native String getBorderColor();
public native void setBorderColor(String borderColor)
                                         throws DOMException;

public native String getBorderSpacing();
public native void setBorderSpacing(String borderSpacing)
                                         throws DOMException;

public native String getBorderStyle();
public native void setBorderStyle(String borderStyle)
                                         throws DOMException;

public native String getBorderTop();
public native void setBorderTop(String borderTop)
                                         throws DOMException;

public native String getBorderRight();
public native void setBorderRight(String borderRight)
                                         throws DOMException;

public native String getBorderBottom();
public native void setBorderBottom(String borderBottom)
                                         throws DOMException;

public native String getBorderLeft();
public native void setBorderLeft(String borderLeft)
                                         throws DOMException;

public native String getBorderTopColor();
public native void setBorderTopColor(String borderTopColor)
                                         throws DOMException;

public native String getBorderRightColor();
public native void setBorderRightColor(String borderRightColor)
                                         throws DOMException;

public native String getBorderBottomColor();
public native void setBorderBottomColor(String borderBottomColor)
                                         throws DOMException;

public native String getBorderLeftColor();
public native void setBorderLeftColor(String borderLeftColor)
                                         throws DOMException;

public native String getBorderTopStyle();
public native void setBorderTopStyle(String borderTopStyle)
                                         throws DOMException;

public native String getBorderRightStyle();
public native void setBorderRightStyle(String borderRightStyle)
                                         throws DOMException;

public native String getBorderBottomStyle();
public native void setBorderBottomStyle(String borderBottomStyle)
                                         throws DOMException;

public native String getBorderLeftStyle();
public native void setBorderLeftStyle(String borderLeftStyle)
                                         throws DOMException;

public native String getBorderTopWidth();
public native void setBorderTopWidth(String borderTopWidth)
                                         throws DOMException;

public native String getBorderRightWidth();
public native void setBorderRightWidth(String borderRightWidth)
                                         throws DOMException;

public native String getBorderBottomWidth();
public native void setBorderBottomWidth(String borderBottomWidth)
                                         throws DOMException;

public native String getBorderLeftWidth();
public native void setBorderLeftWidth(String borderLeftWidth)
                                         throws DOMException;

public native String getBorderWidth();
public native void setBorderWidth(String borderWidth)
                                         throws DOMException;

public native String getBottom();
public native void setBottom(String bottom)
                                         throws DOMException;

public native String getCaptionSide();
public native void setCaptionSide(String captionSide)
                                         throws DOMException;

public native String getClear();
public native void setClear(String clear)
                                         throws DOMException;

public native String getClip();
public native void setClip(String clip)
                                         throws DOMException;

public native String getColor();
public native void setColor(String color)
                                         throws DOMException;

public native String getContent();
public native void setContent(String content)
                                         throws DOMException;

public native String getCounterIncrement();
public native void setCounterIncrement(String counterIncrement)
                                         throws DOMException;

public native String getCounterReset();
public native void setCounterReset(String counterReset)
                                         throws DOMException;

public native String getCue();
public native void setCue(String cue)
                                         throws DOMException;

public native String getCueAfter();
public native void setCueAfter(String cueAfter)
                                         throws DOMException;

public native String getCueBefore();
public native void setCueBefore(String cueBefore)
                                         throws DOMException;

public native String getCursor();
public native void setCursor(String cursor)
                                         throws DOMException;

public native String getDirection();
public native void setDirection(String direction)
                                         throws DOMException;

public native String getDisplay();
public native void setDisplay(String display)
                                         throws DOMException;

public native String getElevation();
public native void setElevation(String elevation)
                                         throws DOMException;

public native String getEmptyCells();
public native void setEmptyCells(String emptyCells)
                                         throws DOMException;

public native String getCssFloat();
public native void setCssFloat(String cssFloat)
                                         throws DOMException;

public native String getFont();
public native void setFont(String font)
                                         throws DOMException;

public native String getFontFamily();
public native void setFontFamily(String fontFamily)
                                         throws DOMException;

public native String getFontSize();
public native void setFontSize(String fontSize)
                                         throws DOMException;

public native String getFontSizeAdjust();
public native void setFontSizeAdjust(String fontSizeAdjust)
                                         throws DOMException;

public native String getFontStretch();
public native void setFontStretch(String fontStretch)
                                         throws DOMException;

public native String getFontStyle();
public native void setFontStyle(String fontStyle)
                                         throws DOMException;

public native String getFontVariant();
public native void setFontVariant(String fontVariant)
                                         throws DOMException;

public native String getFontWeight();
public native void setFontWeight(String fontWeight)
                                         throws DOMException;

public native String getHeight();
public native void setHeight(String height)
                                         throws DOMException;

public native String getLeft();
public native void setLeft(String left)
                                         throws DOMException;

public native String getLetterSpacing();
public native void setLetterSpacing(String letterSpacing)
                                         throws DOMException;

public native String getLineHeight();
public native void setLineHeight(String lineHeight)
                                         throws DOMException;

public native String getListStyle();
public native void setListStyle(String listStyle)
                                         throws DOMException;

public native String getListStyleImage();
public native void setListStyleImage(String listStyleImage)
                                         throws DOMException;

public native String getListStylePosition();
public native void setListStylePosition(String listStylePosition)
                                         throws DOMException;

public native String getListStyleType();
public native void setListStyleType(String listStyleType)
                                         throws DOMException;

public native String getMargin();
public native void setMargin(String margin)
                                         throws DOMException;

public native String getMarginTop();
public native void setMarginTop(String marginTop)
                                         throws DOMException;

public native String getMarginRight();
public native void setMarginRight(String marginRight)
                                         throws DOMException;

public native String getMarginBottom();
public native void setMarginBottom(String marginBottom)
                                         throws DOMException;

public native String getMarginLeft();
public native void setMarginLeft(String marginLeft)
                                         throws DOMException;

public native String getMarkerOffset();
public native void setMarkerOffset(String markerOffset)
                                         throws DOMException;

public native String getMarks();
public native void setMarks(String marks)
                                         throws DOMException;

public native String getMaxHeight();
public native void setMaxHeight(String maxHeight)
                                         throws DOMException;

public native String getMaxWidth();
public native void setMaxWidth(String maxWidth)
                                         throws DOMException;

public native String getMinHeight();
public native void setMinHeight(String minHeight)
                                         throws DOMException;

public native String getMinWidth();
public native void setMinWidth(String minWidth)
                                         throws DOMException;

public native String getOrphans();
public native void setOrphans(String orphans)
                                         throws DOMException;

public native String getOutline();
public native void setOutline(String outline)
                                         throws DOMException;

public native String getOutlineColor();
public native void setOutlineColor(String outlineColor)
                                         throws DOMException;

public native String getOutlineStyle();
public native void setOutlineStyle(String outlineStyle)
                                         throws DOMException;

public native String getOutlineWidth();
public native void setOutlineWidth(String outlineWidth)
                                         throws DOMException;

public native String getOverflow();
public native void setOverflow(String overflow)
                                         throws DOMException;

public native String getPadding();
public native void setPadding(String padding)
                                         throws DOMException;

public native String getPaddingTop();
public native void setPaddingTop(String paddingTop)
                                         throws DOMException;

public native String getPaddingRight();
public native void setPaddingRight(String paddingRight)
                                         throws DOMException;

public native String getPaddingBottom();
public native void setPaddingBottom(String paddingBottom)
                                         throws DOMException;

public native String getPaddingLeft();
public native void setPaddingLeft(String paddingLeft)
                                         throws DOMException;

public native String getPage();
public native void setPage(String page)
                                         throws DOMException;

public native String getPageBreakAfter();
public native void setPageBreakAfter(String pageBreakAfter)
                                         throws DOMException;

public native String getPageBreakBefore();
public native void setPageBreakBefore(String pageBreakBefore)
                                         throws DOMException;

public native String getPageBreakInside();
public native void setPageBreakInside(String pageBreakInside)
                                         throws DOMException;

public native String getPause();
public native void setPause(String pause)
                                         throws DOMException;

public native String getPauseAfter();
public native void setPauseAfter(String pauseAfter)
                                         throws DOMException;

public native String getPauseBefore();
public native void setPauseBefore(String pauseBefore)
                                         throws DOMException;

public native String getPitch();
public native void setPitch(String pitch)
                                         throws DOMException;

public native String getPitchRange();
public native void setPitchRange(String pitchRange)
                                         throws DOMException;

public native String getPlayDuring();
public native void setPlayDuring(String playDuring)
                                         throws DOMException;

public native String getPosition();
public native void setPosition(String position)
                                         throws DOMException;

public native String getQuotes();
public native void setQuotes(String quotes)
                                         throws DOMException;

public native String getRichness();
public native void setRichness(String richness)
                                         throws DOMException;

public native String getRight();
public native void setRight(String right)
                                         throws DOMException;

public native String getSize();
public native void setSize(String size)
                                         throws DOMException;

public native String getSpeak();
public native void setSpeak(String speak)
                                         throws DOMException;

public native String getSpeakHeader();
public native void setSpeakHeader(String speakHeader)
                                         throws DOMException;

public native String getSpeakNumeral();
public native void setSpeakNumeral(String speakNumeral)
                                         throws DOMException;

public native String getSpeakPunctuation();
public native void setSpeakPunctuation(String speakPunctuation)
                                         throws DOMException;

public native String getSpeechRate();
public native void setSpeechRate(String speechRate)
                                         throws DOMException;

public native String getStress();
public native void setStress(String stress)
                                         throws DOMException;

public native String getTableLayout();
public native void setTableLayout(String tableLayout)
                                         throws DOMException;

public native String getTextAlign();
public native void setTextAlign(String textAlign)
                                         throws DOMException;

public native String getTextDecoration();
public native void setTextDecoration(String textDecoration)
                                         throws DOMException;

public native String getTextIndent();
public native void setTextIndent(String textIndent)
                                         throws DOMException;

public native String getTextShadow();
public native void setTextShadow(String textShadow)
                                         throws DOMException;

public native String getTextTransform();
public native void setTextTransform(String textTransform)
                                         throws DOMException;

public native String getTop();
public native void setTop(String top)
                                         throws DOMException;

public native String getUnicodeBidi();
public native void setUnicodeBidi(String unicodeBidi)
                                         throws DOMException;

public native String getVerticalAlign();
public native void setVerticalAlign(String verticalAlign)
                                         throws DOMException;

public native String getVisibility();
public native void setVisibility(String visibility)
                                         throws DOMException;

public native String getVoiceFamily();
public native void setVoiceFamily(String voiceFamily)
                                         throws DOMException;

public native String getVolume();
public native void setVolume(String volume)
                                         throws DOMException;

public native String getWhiteSpace();
public native void setWhiteSpace(String whiteSpace)
                                         throws DOMException;

public native String getWidows();
public native void setWidows(String widows)
                                         throws DOMException;

public native String getWidth();
public native void setWidth(String width)
                                         throws DOMException;

public native String getWordSpacing();
public native void setWordSpacing(String wordSpacing)
                                         throws DOMException;

public native String getZIndex();
public native void setZIndex(String zIndex)
                                         throws DOMException;

}
