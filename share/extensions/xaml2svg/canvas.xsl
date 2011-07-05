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
-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:svg="http://www.w3.org/2000/svg"
xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="ISO-8859-1"/>

<xsl:template mode="forward" match="*[name(.) = 'Canvas' or name(.) = 'Window' or name(.) = 'StackPanel']">
  <svg>
    <xsl:choose>
    <!--
      <xsl:when test="@ID"><xsl:attribute name="id"><xsl:value-of select="@ID" /></xsl:attribute></xsl:when>
    -->
      <xsl:when test="@x:Key"><xsl:attribute name="id"><xsl:value-of select="@x:Key" /></xsl:attribute></xsl:when>
      <xsl:when test="@Name"><xsl:attribute name="id"><xsl:value-of select="@Name" /></xsl:attribute></xsl:when>
    </xsl:choose>
    <xsl:if test="@Width"><xsl:attribute name="width"><xsl:value-of select="@Width" /></xsl:attribute></xsl:if>
    <xsl:if test="@Height"><xsl:attribute name="height"><xsl:value-of select="@Height" /></xsl:attribute></xsl:if>
    <xsl:if test="@Canvas.Left"><xsl:attribute name="x"><xsl:value-of select="@Canvas.Left" /></xsl:attribute></xsl:if>
    <xsl:if test="@Canvas.Top"><xsl:attribute name="y"><xsl:value-of select="@Canvas.Top" /></xsl:attribute></xsl:if>
    <xsl:call-template name="template_properties" />
    <xsl:choose>
      <xsl:when test="@Transform or *[name(.) = 'UIElement.RenderTransform' or name(.) = 'Shape.RenderTransform' or name(.) = concat(name(..), '.RenderTransform')]">
        <g>
          <xsl:call-template name="template_transform" />
          <xsl:apply-templates select="*[name(.) = 'UIElement.RenderTransform' or name(.) = 'Shape.RenderTransform' or name(.) = concat(name(..), '.RenderTransform')]" />
          <xsl:apply-templates mode="svg" />
        </g>
      </xsl:when>
      <xsl:otherwise><xsl:apply-templates mode="svg" /></xsl:otherwise>
    </xsl:choose>
  </svg>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Border']">
  <rect>
    <xsl:if test="@Width"><xsl:attribute name="width"><xsl:value-of select="@Width" /></xsl:attribute></xsl:if>
    <xsl:if test="@Height"><xsl:attribute name="height"><xsl:value-of select="@Height" /></xsl:attribute></xsl:if>
    <xsl:if test="@BorderThickness"><xsl:attribute name="stroke-width"><xsl:value-of select="@BorderThickness" /></xsl:attribute></xsl:if>
    <xsl:if test="@Background">
      <xsl:attribute name="fill"><xsl:call-template name="template_color"><xsl:with-param name="colorspec" select="@Background" /></xsl:call-template></xsl:attribute>
      <xsl:variable name="test_opacity"><xsl:call-template name="template_opacity"><xsl:with-param name="colorspec" select="@Background" /></xsl:call-template></xsl:variable>
      <xsl:if test="string-length($test_opacity) &gt; 0"><xsl:attribute name="fill-opacity"><xsl:value-of select="$test_opacity" /></xsl:attribute></xsl:if>
    </xsl:if>
    <xsl:if test="@BorderBrush">
      <xsl:attribute name="stroke"><xsl:call-template name="template_color"><xsl:with-param name="colorspec" select="@BorderBrush" /></xsl:call-template></xsl:attribute>
      <xsl:variable name="test_opacity"><xsl:call-template name="template_opacity"><xsl:with-param name="colorspec" select="@BorderBrush" /></xsl:call-template></xsl:variable>
      <xsl:if test="string-length($test_opacity) &gt; 0"><xsl:attribute name="stroke-opacity"><xsl:value-of select="$test_opacity" /></xsl:attribute></xsl:if>
    </xsl:if>
    <xsl:call-template name="template_properties" />
  </rect>
  <xsl:apply-templates mode="forward" />
</xsl:template>

</xsl:stylesheet>
