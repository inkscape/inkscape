<?xml version="1.0" encoding="UTF-8"?>

<!--
Copyright (c) 2005-2007 Toine de Greef (a.degreef@chello.nl)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Version history:

20070907 Initial release
20070912 Removed NaN as viewBox values

-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:svg="http://www.w3.org/2000/svg"
xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
xmlns:msxsl="urn:schemas-microsoft-com:xslt">
<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="UTF-8"/>
<xsl:include href="xaml2svg/animation.xsl" />
<xsl:include href="xaml2svg/brushes.xsl" />
<xsl:include href="xaml2svg/canvas.xsl" />
<xsl:include href="xaml2svg/geometry.xsl" />
<xsl:include href="xaml2svg/properties.xsl" />
<xsl:include href="xaml2svg/shapes.xsl" />
<xsl:include href="xaml2svg/transform.xsl" />

<xsl:template match="/">
  <svg>
    <xsl:attribute name="overflow">visible</xsl:attribute>
    <xsl:variable name="viewBox"><xsl:apply-templates mode="boundingbox" /></xsl:variable>
    <xsl:if test="system-property('xsl:vendor') = 'Microsoft' and (msxsl:node-set($viewBox)/boundingbox/@x1 != '') and (msxsl:node-set($viewBox)/boundingbox/@x2 != '') and (msxsl:node-set($viewBox)/boundingbox/@y1 != '') and (msxsl:node-set($viewBox)/boundingbox/@y2 != '')">
      <xsl:attribute name="viewBox">
        <xsl:value-of select="concat(msxsl:node-set($viewBox)/boundingbox/@x1, ' ')" />
        <xsl:value-of select="concat(msxsl:node-set($viewBox)/boundingbox/@y1, ' ')" />
        <xsl:value-of select="concat(msxsl:node-set($viewBox)/boundingbox/@x2 - msxsl:node-set($viewBox)/boundingbox/@x1, ' ')" />
        <xsl:value-of select="msxsl:node-set($viewBox)/boundingbox/@y2 - msxsl:node-set($viewBox)/boundingbox/@y1" />
      </xsl:attribute>
    </xsl:if>  
    <xsl:apply-templates mode="svg" />
  </svg>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Image']">
  <xsl:if test="@Canvas.Left or @Canvas.Top"><xsl:value-of disable-output-escaping="yes" select="concat('&lt;svg x=&quot;', @Canvas.Left, '&quot; y=&quot;', @Canvas.Top, '&quot;&gt;')" /></xsl:if>
  <image>
    <xsl:if test="@Source"><xsl:attribute name="xlink:href"><xsl:value-of select="@Source" /></xsl:attribute></xsl:if>
    <xsl:if test="@Width"><xsl:attribute name="width"><xsl:value-of select="@Width" /></xsl:attribute></xsl:if>
    <xsl:if test="@Height"><xsl:attribute name="height"><xsl:value-of select="@Height" /></xsl:attribute></xsl:if>
    <xsl:call-template name="template_properties" />
    <xsl:call-template name="template_transform" />
    <xsl:apply-templates mode="forward" />
  </image>
  <xsl:if test="@Canvas.Left or @Canvas.Top"><xsl:value-of disable-output-escaping="yes" select="'&lt;/svg&gt;&#13;'" /></xsl:if>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'TextBlock']">
  <xsl:if test="@Canvas.Left or @Canvas.Top"><xsl:value-of disable-output-escaping="yes" select="concat('&lt;svg x=&quot;', @Canvas.Left, '&quot; y=&quot;', @Canvas.Top, '&quot;&gt;')" /></xsl:if>
  <text>
    <xsl:if test="@FontSize"><xsl:attribute name="font-size"><xsl:value-of select="@FontSize" /></xsl:attribute></xsl:if>
    <xsl:if test="@FontWeight"><xsl:attribute name="font-weight"><xsl:value-of select="@FontWeight" /></xsl:attribute></xsl:if>
    <xsl:if test="@FontFamily"><xsl:attribute name="font-family"><xsl:value-of select="@FontFamily" /></xsl:attribute></xsl:if>
    <xsl:if test="@FontStyle"><xsl:attribute name="font-style"><xsl:value-of select="@FontStyle" /></xsl:attribute></xsl:if>
    <xsl:if test="@Foreground"><xsl:attribute name="fill"><xsl:value-of select="@Foreground" /></xsl:attribute></xsl:if>
    <xsl:if test="@Width"><xsl:attribute name="width"><xsl:value-of select="@Width" /></xsl:attribute></xsl:if>
    <xsl:if test="@Height"><xsl:attribute name="height"><xsl:value-of select="@Height" /></xsl:attribute></xsl:if>
    <xsl:if test="@HorizontalAlignment">
      <xsl:attribute name="text-anchor">
        <xsl:choose>
          <xsl:when test="@HorizontalAlignment = 'Left'">start</xsl:when>
          <xsl:when test="@HorizontalAlignment = 'Center'">middle</xsl:when>
          <xsl:when test="@HorizontalAlignment = 'Right'">end</xsl:when>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:attribute name="dominant-baseline">hanging</xsl:attribute>
    <xsl:call-template name="template_properties" />
    <xsl:call-template name="template_transform" />
    <xsl:apply-templates />
  </text>
  <xsl:if test="@Canvas.Left or @Canvas.Top"><xsl:value-of disable-output-escaping="yes" select="'&lt;/svg&gt;&#13;'" /></xsl:if>
</xsl:template>

<xsl:template match="*">
<xsl:comment><xsl:value-of select="concat('Unknown tag: ', name(.))" /></xsl:comment>
</xsl:template>

</xsl:stylesheet>
