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
xmlns:def="Definition"
exclude-result-prefixes="def">
<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="ISO-8859-1"/>

<xsl:template mode="forward" match="*[name(.) = 'ClipGeometry']">
<!-- -->
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'GlyphRunDrawing']">
<!-- -->
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'ImageDrawing']">
  <image>
    <xsl:if test="@ImageSource"><xsl:attribute name="xlink:href"><xsl:value-of select="@ImageSource" /></xsl:attribute></xsl:if>
    <xsl:if test="@Rect">
      <xsl:attribute name="x"><xsl:value-of select="substring-before(@Rect, ',')" /></xsl:attribute>
      <xsl:attribute name="y"><xsl:value-of select="substring-before(substring-after(@Rect, ','), ',')" /></xsl:attribute>
      <xsl:attribute name="width"><xsl:value-of select="substring-before(substring-after(substring-after(@Rect, ','), ','), ',')" /></xsl:attribute>
      <xsl:attribute name="height"><xsl:value-of select="substring-after(substring-after(substring-after(@Rect, ','), ','), ',')" /></xsl:attribute>
    </xsl:if>  
  </image>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'DrawingGroup']">
  <xsl:call-template name="template_properties" />
  <xsl:call-template name="template_transform" />
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'EllipseGeometry']">
  <xsl:variable name="cx" select="substring-before(@Center, ',')" />
  <xsl:variable name="cy" select="substring-after(@Center, ',')" />
  <xsl:value-of select="concat('M ', $cx + @RadiusX, ',', $cy, ' ')" />
  <xsl:value-of select="concat('A ', @RadiusX, ',', @RadiusY, ' 0 0 1 ', $cx, ',', $cy + @RadiusY, ' ')" />
  <xsl:value-of select="concat('A ', @RadiusX, ',', @RadiusY, ' 0 0 1 ', $cx - @RadiusX, ',', $cy, ' ')" />
  <xsl:value-of select="concat('A ', @RadiusX, ',', @RadiusY, ' 0 0 1 ', $cx, ',', $cy - @RadiusY, ' ')" />
  <xsl:value-of select="concat('A ', @RadiusX, ',', @RadiusY, ' 0 0 1 ', $cx + @RadiusX, ',', $cy, ' ')" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'LineGeometry']">
  <xsl:value-of select="concat('M ', @StartPoint, ' ')" />
  <xsl:value-of select="concat('L ', @EndPoint, ' ')" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PathGeometry']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PathGeometry.Figures']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PathFigureCollection']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PathFigure']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PathFigure.Segments']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PathSegmentCollection']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'StartSegment']">
  <xsl:value-of select="concat('M ', @Point, ' ')" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'CloseSegment']">
  <xsl:value-of select="'z '" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'LineSegment']">
  <xsl:value-of select="concat('L ', @Point, ' ')" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'ArcSegment']">
  <xsl:value-of select="concat('A ', substring-before(@Size, ','), ',', substring-after(@Size, ','), ' ', @XRotation, ' ', number(@LargeArc = 'True'), ' ', number(@Sweepflag = 'True'), ' ', @Point, ' ')" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'BezierSegment']">
  <xsl:value-of select="concat('C ', @Point1, ' ', @Point2, ' ', @Point3, ' ')" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'QuadraticBezierSegment']">
  <xsl:value-of select="concat('Q ', @Point1, ' ', @Point2, ' ', @Point3, ' ')" />
</xsl:template>

<xsl:template name="PrintPolyBezierPoints">
  <xsl:param name="segments" />
  <xsl:param name="segmentsize" />
  <xsl:param name="cursor" />
  <xsl:text>C </xsl:text>
  <xsl:value-of select="concat(substring-before($segments, ','), ',')" />
  <xsl:variable name="segments1"><xsl:value-of select="substring-after($segments, ',')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments1, ' '), ' ')" />
  <xsl:variable name="segments2"><xsl:value-of select="substring-after($segments1, ' ')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments2, ','), ',')" />
  <xsl:variable name="segments3"><xsl:value-of select="substring-after($segments2, ',')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments3, ' '), ' ')" />
  <xsl:variable name="segments4"><xsl:value-of select="substring-after($segments3, ' ')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments4, ','), ',')" />
  <xsl:variable name="segments5"><xsl:value-of select="substring-after($segments4, ',')" /></xsl:variable>
  <xsl:choose>
    <xsl:when test="contains($segments5, ' ')">
      <xsl:value-of select="concat(substring-before($segments5, ' '), ' ')" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$segments5" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:variable name="segments6"><xsl:value-of select="substring-after($segments5, ' ')" /></xsl:variable>
  <xsl:if test="contains($segments6, ' ')">
    <xsl:call-template name="PrintPolyBezierPoints">
      <xsl:with-param name="segments" select="$segments6" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<xsl:template name="PrintPolyLinePoints">
  <xsl:param name="segments" />
  <xsl:text>L </xsl:text>
  <xsl:value-of select="concat(substring-before($segments, ','), ',')" />
  <xsl:variable name="segments1"><xsl:value-of select="substring-after($segments, ',')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments1, ' '), ' ')" />
  <xsl:variable name="segments2"><xsl:value-of select="substring-after($segments1, ' ')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments2, ','), ',')" />
  <xsl:variable name="segments3"><xsl:value-of select="substring-after($segments2, ',')" /></xsl:variable>
  <xsl:choose>
    <xsl:when test="contains($segments3, ' ')">
      <xsl:value-of select="concat(substring-before($segments3, ' '), ' ')" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$segments3" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:variable name="segments4"><xsl:value-of select="substring-after($segments3, ' ')" /></xsl:variable>
  <xsl:if test="contains($segments4, ' ')">
    <xsl:call-template name="PrintPolyLinePoints">
      <xsl:with-param name="segments" select="$segments4" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<xsl:template name="PrintQuadraticBezierPoints">
  <xsl:param name="segments" />
  <xsl:text>Q </xsl:text>
  <xsl:value-of select="concat(substring-before($segments, ','), ',')" />
  <xsl:variable name="segments1"><xsl:value-of select="substring-after($segments, ',')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments1, ' '), ' ')" />
  <xsl:variable name="segments2"><xsl:value-of select="substring-after($segments1, ' ')" /></xsl:variable>
  <xsl:value-of select="concat(substring-before($segments2, ','), ',')" />
  <xsl:variable name="segments3"><xsl:value-of select="substring-after($segments2, ',')" /></xsl:variable>
  <xsl:choose>
    <xsl:when test="contains($segments3, ' ')">
      <xsl:value-of select="concat(substring-before($segments3, ' '), ' ')" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$segments3" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:variable name="segments4"><xsl:value-of select="substring-after($segments3, ' ')" /></xsl:variable>
  <xsl:if test="contains($segments4, ' ')">
    <xsl:call-template name="PrintQuadraticBezierPoints">
      <xsl:with-param name="segments" select="$segments4" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PolyBezierSegment']">
  <xsl:call-template name="PrintPolyBezierPoints">
    <xsl:with-param name="segments" select="@Points" />
  </xsl:call-template>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PolyLineSegment']">
  <xsl:call-template name="PrintPolyLinePoints">
    <xsl:with-param name="segments" select="@Points" />
  </xsl:call-template>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'PolyQuadraticBezierSegment']">
  <xsl:call-template name="PrintQuadraticBezierPoints">
    <xsl:with-param name="segments" select="@Points" />
  </xsl:call-template>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'RectangleGeometry']">
  <xsl:variable name="rect" select="RectangleGeometry.Rect/Rect" />
  <xsl:choose>
    <xsl:when test="$rect">
      <xsl:value-of select="concat('M ', $rect/@X, ',', $rect/@Y, ' ')" />
      <xsl:value-of select="concat('H ', $rect/@X + $rect/@Width, ' V ',  $rect/@Y + $rect/@Height, ' H ', $rect/@X,' V ', $rect/@Y, ' ')" />
    </xsl:when>
    <xsl:when test="@Rect">
      <xsl:variable name="x" select="substring-before(substring-before(@Rect, ' '), ',')" />
      <xsl:variable name="y" select="substring-after(substring-before(@Rect, ' '), ',')" />
      <xsl:variable name="width" select="substring-before(substring-after(@Rect, ' '), ',')" />
      <xsl:variable name="height" select="substring-after(substring-after(@Rect, ' '), ',')" />
      <xsl:value-of select="concat('M ', $x, ',', $y, ' ')" />
      <xsl:value-of select="concat('H ', $x + $width, ' V ',  $y + $height, ' H ', $x,' V ', $y, ' ')" />
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!--
<xsl:template mode="forward" match="*[name(.) = 'GeometryCollection']">
  <xsl:apply-templates mode="forward" />
</xsl:template>
-->

<xsl:template mode="forward" match="*[name(.) = 'GeometryGroup']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'GeometryDrawing']">
  <path>
    <xsl:attribute name="d"><xsl:apply-templates select="*[name(.) = 'GeometryDrawing.Geometry']" mode="forward" /></xsl:attribute>
    <xsl:attribute name="fill"><xsl:value-of select="@Brush" /></xsl:attribute>
    <xsl:apply-templates mode="forward" select="*[name(.) = 'GeometryDrawing.Pen']" />
  </path>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'GeometryDrawing.Geometry']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'GeometryDrawing.Pen']">
  <xsl:variable name="pen" select="*[name(.) = 'Pen']" />
  <xsl:if test="$pen/@Brush"><xsl:attribute name="stroke"><xsl:value-of select="$pen/@Brush" /></xsl:attribute></xsl:if>
  <xsl:if test="$pen/@Thickness"><xsl:attribute name="stroke-width"><xsl:value-of select="$pen/@Thickness" /></xsl:attribute></xsl:if>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'CombinedGeometry' or name(.) = 'CombinedGeometry.Geometry1' or name(.) = 'CombinedGeometry.Geometry2']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

</xsl:stylesheet>
