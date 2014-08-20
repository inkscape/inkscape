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
xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
exclude-result-prefixes="x">
<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="ISO-8859-1"/>

<xsl:template mode="forward" match="*[name(.) = 'LinearGradientBrush']">
  <linearGradient>
    <xsl:attribute name="id">
      <xsl:choose>
        <xsl:when test="@x:Key"><xsl:value-of select="@x:Key" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="concat('id_', generate-id(..))" /></xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <xsl:attribute name="gradientUnits">
      <xsl:choose>
        <xsl:when test="@MappingMode = 'RelativeToBoundingBox' or not(@StartPoint and @EndPoint)"><xsl:value-of select="'boundingBox'" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="'userSpaceOnUse'" /></xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <xsl:if test="@SpreadMethod">
      <xsl:attribute name="spreadMethod">
        <xsl:value-of select="translate(@SpreadMethod, 'PR', 'pr')" />
      </xsl:attribute>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@MappingMode = 'Absolute' and @StartPoint and @EndPoint">
        <xsl:attribute name="x1"><xsl:value-of select="substring-before(@StartPoint,',')" /></xsl:attribute>
        <xsl:attribute name="x1"><xsl:value-of select="substring-before(@StartPoint,',')" /></xsl:attribute>
        <xsl:attribute name="y1"><xsl:value-of select="substring-after(@StartPoint,',')" /></xsl:attribute>
        <xsl:attribute name="x2"><xsl:value-of select="substring-before(@EndPoint,',')" /></xsl:attribute>
        <xsl:attribute name="y2"><xsl:value-of select="substring-after(@EndPoint,',')" /></xsl:attribute>
      </xsl:when>
      <xsl:when test="@StartPoint and @EndPoint">
        <xsl:attribute name="x1"><xsl:value-of select="concat(100 * number(substring-before(@StartPoint,',')), '%')" /></xsl:attribute>
        <xsl:attribute name="y1"><xsl:value-of select="concat(100 * number(substring-after(@StartPoint,',')), '%')" /></xsl:attribute>
        <xsl:attribute name="x2"><xsl:value-of select="concat(100 * number(substring-before(@EndPoint,',')), '%')" /></xsl:attribute>
        <xsl:attribute name="y2"><xsl:value-of select="concat(100 * number(substring-after(@EndPoint,',')), '%')" /></xsl:attribute>
      </xsl:when>
      <xsl:otherwise>
        <xsl:attribute name="x1"><xsl:value-of select="0" /></xsl:attribute>
        <xsl:attribute name="y1"><xsl:value-of select="0" /></xsl:attribute>
        <xsl:attribute name="x2"><xsl:value-of select="'100%'" /></xsl:attribute>
        <xsl:attribute name="y2"><xsl:value-of select="'100%'" /></xsl:attribute>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:call-template name="template_gradienttransform" />
    <xsl:apply-templates select="*[name(.) != 'Brush.Transform' and name(.) != concat(name(..), '.Transform') and name(.) != 'Brush.RelativeTransform' and name(.) != concat(name(..), '.RelativeTransform')]" />
  </linearGradient>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'RadialGradientBrush']">
  <radialGradient>
    <xsl:attribute name="id">
      <xsl:choose>
        <xsl:when test="@x:Key"><xsl:value-of select="@x:Key" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="concat('id_', generate-id(..))" /></xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <xsl:attribute name="gradientUnits">
      <xsl:choose>
        <xsl:when test="@MappingMode = 'RelativeToBoundingBox' or not(@StartPoint and @EndPoint)"><xsl:value-of select="'boundingBox'" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="'userSpaceOnUse'" /></xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <xsl:if test="@SpreadMethod">
      <xsl:attribute name="spreadMethod">
        <xsl:value-of select="translate(@SpreadMethod, 'PR', 'pr')" />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@Center">
      <xsl:attribute name="cx"><xsl:value-of select="substring-before(@Center, ',')" /></xsl:attribute>
    <xsl:attribute name="cy"><xsl:value-of select="substring-after(@Center, ',')" /></xsl:attribute>
    </xsl:if>
    <xsl:if test="@GradientOrigin">
      <xsl:attribute name="fx"><xsl:value-of select="substring-before(@GradientOrigin, ',')" /></xsl:attribute>
      <xsl:attribute name="fy"><xsl:value-of select="substring-after(@GradientOrigin, ',')" /></xsl:attribute>
    </xsl:if>
    <!-- Xamlon uses Focus -->
    <xsl:if test="@Focus">
      <xsl:attribute name="fx"><xsl:value-of select="substring-before(@Focus, ',')" /></xsl:attribute>
      <xsl:attribute name="fy"><xsl:value-of select="substring-after(@Focus, ',')" /></xsl:attribute>
    </xsl:if>
    <xsl:attribute name="r"><xsl:value-of select="@RadiusX" /></xsl:attribute>
    <xsl:call-template name="template_gradienttransform" />
    <xsl:apply-templates select="*[name(.) != 'Brush.Transform' and name(.) != concat(name(..), '.Transform') and name(.) != 'Brush.RelativeTransform' and name(.) != concat(name(..), '.RelativeTransform')]" />
  </radialGradient>
</xsl:template>

<xsl:template match="*[name(.) = 'GradientStopCollection' or name(.) = 'GradientBrush.GradientStops' or name(.) = concat(name(..), '.GradientStops')]">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'GradientStop']">
  <stop>
    <xsl:if test="@Offset"><xsl:attribute name="offset"><xsl:value-of select="@Offset" /></xsl:attribute></xsl:if>
    <xsl:if test="@Color">
      <xsl:attribute name="stop-color"><xsl:call-template name="template_color"><xsl:with-param name="colorspec" select="@Color" /></xsl:call-template></xsl:attribute>
      <xsl:variable name="test_opacity"><xsl:call-template name="template_opacity"><xsl:with-param name="colorspec" select="@Color" /></xsl:call-template></xsl:variable>
      <xsl:if test="string-length($test_opacity) &gt; 0"><xsl:attribute name="stop-opacity"><xsl:value-of select="$test_opacity" /></xsl:attribute></xsl:if>
    </xsl:if>
  </stop>
</xsl:template>

<xsl:template match="*[name(.) = 'SolidColorBrush']">
  <xsl:call-template name="template_properties" />
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'ImageBrush']">
  <defs>
    <pattern>
      <xsl:choose>
        <xsl:when test="@TileMode != 'none' and @Viewport and @ViewportUnits = 'Absolute'">
          <xsl:attribute name="patternUnits">userSpaceOnUse</xsl:attribute>
          <xsl:attribute name="x"><xsl:value-of select="substring-before(@Viewport, ',')" /></xsl:attribute>
          <xsl:attribute name="y"><xsl:value-of select="substring-before(substring-after(@Viewport, ','), ',')" /></xsl:attribute>
          <xsl:attribute name="width"><xsl:value-of select="substring-before(substring-after(substring-after(@Viewport, ','), ','), ',')" /></xsl:attribute>
          <xsl:attribute name="height"><xsl:value-of select="substring-after(substring-after(substring-after(@Viewport, ','), ','), ',')" /></xsl:attribute>
        </xsl:when>
        <xsl:when test="@TileMode != 'none' and @Viewport">
          <xsl:attribute name="patternUnits">boundingBox</xsl:attribute>
          <xsl:attribute name="x"><xsl:value-of select="concat(100 * number(substring-before(@Viewport, ',')), '%')" /></xsl:attribute>
          <xsl:attribute name="y"><xsl:value-of select="concat(100 * number(substring-before(substring-after(@Viewport, ','), ',')), '%')" /></xsl:attribute>
          <xsl:attribute name="width"><xsl:value-of select="concat(100 * number(substring-before(substring-after(substring-after(@Viewport, ','), ','), ',')), '%')" /></xsl:attribute>
          <xsl:attribute name="height"><xsl:value-of select="concat(100 * number(substring-after(substring-after(substring-after(@Viewport, ','), ','), ',')), '%')" /></xsl:attribute>
        </xsl:when>
        <xsl:when test="@Viewport and ../../@Width and ../../@Height">
          <xsl:attribute name="patternUnits">userSpaceOnUse</xsl:attribute>
          <xsl:attribute name="x">0</xsl:attribute>
          <xsl:attribute name="y">0</xsl:attribute>
          <xsl:attribute name="width"><xsl:value-of select="../../@Width" /></xsl:attribute>
          <xsl:attribute name="height"><xsl:value-of select="../../@Height" /></xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="patternUnits">boundingBox </xsl:attribute>
          <xsl:attribute name="x">0</xsl:attribute>
          <xsl:attribute name="y">0</xsl:attribute>
          <xsl:attribute name="width">100%</xsl:attribute>
          <xsl:attribute name="height">100%</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:attribute name="id"><xsl:value-of select="concat('id_', generate-id(..))" /></xsl:attribute>
      <image>
        <xsl:attribute name="xlink:href"><xsl:value-of select="@ImageSource" /></xsl:attribute>
        <xsl:choose>
          <xsl:when test="@Viewport and @ViewportUnits = 'Absolute'">
            <xsl:attribute name="patternUnits">userSpaceOnUse</xsl:attribute>
            <xsl:attribute name="x"><xsl:value-of select="0" /></xsl:attribute>
            <xsl:attribute name="y"><xsl:value-of select="0" /></xsl:attribute>
            <xsl:attribute name="width"><xsl:value-of select="substring-before(substring-after(substring-after(@Viewport, ','), ','), ',')" /></xsl:attribute>
            <xsl:attribute name="height"><xsl:value-of select="substring-after(substring-after(substring-after(@Viewport, ','), ','), ',')" /></xsl:attribute>
          </xsl:when>
          <xsl:when test="@Viewport">
            <xsl:attribute name="patternUnits">boundingBox</xsl:attribute>
            <xsl:attribute name="x"><xsl:value-of select="0" /></xsl:attribute>
            <xsl:attribute name="y"><xsl:value-of select="0" /></xsl:attribute>
            <xsl:attribute name="width"><xsl:value-of select="concat(100 * number(substring-before(substring-after(substring-after(@Viewport, ','), ','), ',')), '%')" /></xsl:attribute>
            <xsl:attribute name="height"><xsl:value-of select="concat(100 * number(substring-after(substring-after(substring-after(@Viewport, ','), ','), ',')), '%')" /></xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="patternUnits">boundingBox</xsl:attribute>
            <xsl:attribute name="x">0</xsl:attribute>
            <xsl:attribute name="y">0</xsl:attribute>
            <xsl:attribute name="width">100%</xsl:attribute>
            <xsl:attribute name="height">100%</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:attribute name="style">opacity:1</xsl:attribute>
        <xsl:attribute name="image-rendering">optimizeSpeed</xsl:attribute>
      </image>
    </pattern>
  </defs>
</xsl:template>

<xsl:template match="*[name(.) = 'DrawingBrush']">
    <pattern>
      <xsl:choose>
        <xsl:when test="@TileMode != 'none' and @Viewport and @ViewportUnits = 'Absolute'">
          <xsl:attribute name="patternUnits">userSpaceOnUse</xsl:attribute>
          <xsl:attribute name="x"><xsl:value-of select="substring-before(@Viewport, ',')" /></xsl:attribute>
          <xsl:attribute name="y"><xsl:value-of select="substring-before(substring-after(@Viewport, ','), ',')" /></xsl:attribute>
          <xsl:attribute name="width"><xsl:value-of select="substring-before(substring-after(substring-after(@Viewport, ','), ','), ',')" /></xsl:attribute>
          <xsl:attribute name="height"><xsl:value-of select="substring-after(substring-after(substring-after(@Viewport, ','), ','), ',')" /></xsl:attribute>
        </xsl:when>
        <xsl:when test="@TileMode != 'none' and @Viewport">
          <xsl:attribute name="patternUnits">boundingBox</xsl:attribute>
          <xsl:attribute name="x"><xsl:value-of select="concat(100 * number(substring-before(@Viewport, ',')), '%')" /></xsl:attribute>
          <xsl:attribute name="y"><xsl:value-of select="concat(100 * number(substring-before(substring-after(@Viewport, ','), ',')), '%')" /></xsl:attribute>
          <xsl:attribute name="width"><xsl:value-of select="concat(100 * number(substring-before(substring-after(substring-after(@Viewport, ','), ','), ',')), '%')" /></xsl:attribute>
          <xsl:attribute name="height"><xsl:value-of select="concat(100 * number(substring-after(substring-after(substring-after(@Viewport, ','), ','), ',')), '%')" /></xsl:attribute>
        </xsl:when>
        <xsl:when test="@Viewport and ../../@Width and ../../@Height">
          <xsl:attribute name="patternUnits">userSpaceOnUse</xsl:attribute>
          <xsl:attribute name="x">0</xsl:attribute>
          <xsl:attribute name="y">0</xsl:attribute>
          <xsl:attribute name="width"><xsl:value-of select="../../@Width" /></xsl:attribute>
          <xsl:attribute name="height"><xsl:value-of select="../../@Height" /></xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="patternUnits">boundingBox</xsl:attribute>
          <xsl:attribute name="x">0</xsl:attribute>
          <xsl:attribute name="y">0</xsl:attribute>
          <xsl:attribute name="width">100%</xsl:attribute>
          <xsl:attribute name="height">100%</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:attribute name="id"><xsl:value-of select="concat('id_', generate-id(..))" /></xsl:attribute>
      <xsl:apply-templates mode="forward" />
    </pattern>
</xsl:template>

<xsl:template match="*[name(.) = 'DrawingBrush.Drawing']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

</xsl:stylesheet>
