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
20070912 starts-with(@Data, 'F0 ') to strip of F0 from path data
20070912 nonzero and evenodd were outside xsl:attribute (reported by bulia byak and Ted Gould)

-->

<xsl:stylesheet version="1.0" 
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:svg="http://www.w3.org/2000/svg"
xmlns:def="Definition"
exclude-result-prefixes="def">
<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="ISO-8859-1"/>

<xsl:template mode="forward" match="*[name(.) = 'Path']">
  <path>
    <xsl:if test="@Data">
      <xsl:attribute name="d">
        <xsl:choose>
          <xsl:when test="starts-with(@Data, 'F0 ')"><xsl:value-of select="substring-after(@Data, 'F0 ')" /></xsl:when>
          <xsl:when test="starts-with(@Data, 'F1 ')"><xsl:value-of select="substring-after(@Data, 'F1 ')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="@Data" /></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@FillRule = 'nonzero' or starts-with(@Data, 'F1 ')"><xsl:attribute name="fill-rule">nonzero</xsl:attribute></xsl:when>
      <xsl:when test="@FillRule = 'evenodd' or starts-with(@Data, 'F0 ')"><xsl:attribute name="fill-rule">evenodd</xsl:attribute></xsl:when>
    </xsl:choose>
    <xsl:call-template name="template_properties" />  
    <xsl:call-template name="template_transform" />  
    <xsl:apply-templates mode="forward" />
  </path>
</xsl:template>    

<xsl:template mode="forward" match="*[name(.) = 'Path.Data']">
  <xsl:attribute name="d">
    <xsl:apply-templates mode="forward" />
  </xsl:attribute>
  <xsl:if test="@FillRule">
    <xsl:attribute name="fill-rule">
      <xsl:choose>
        <xsl:when test="@FillRule = 'nonzero'">nonzero</xsl:when>
        <xsl:otherwise>evenodd</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Line']">
  <line>
    <xsl:if test="@X1"><xsl:attribute name="x1"><xsl:value-of select="@X1" /></xsl:attribute></xsl:if> 
    <xsl:if test="@Y1"><xsl:attribute name="y1"><xsl:value-of select="@Y1" /></xsl:attribute></xsl:if> 
    <xsl:if test="@X2"><xsl:attribute name="x2"><xsl:value-of select="@X2" /></xsl:attribute></xsl:if> 
    <xsl:if test="@Y2"><xsl:attribute name="y2"><xsl:value-of select="@Y2" /></xsl:attribute></xsl:if> 
    <xsl:call-template name="template_properties" />  
    <xsl:call-template name="template_transform" />  
    <xsl:apply-templates mode="forward" />
  </line>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Ellipse']">
  <ellipse>
    <xsl:if test="@Width">
      <xsl:attribute name="rx"><xsl:value-of select="@Width div 2" /></xsl:attribute>
      <xsl:if test="@Canvas.Left">
        <xsl:attribute name="cx"><xsl:value-of select="@Canvas.Left + @Width div 2" /></xsl:attribute>
      </xsl:if>
    </xsl:if>
    <xsl:if test="@Height">
      <xsl:attribute name="ry"><xsl:value-of select="@Height div 2" /></xsl:attribute>
      <xsl:if test="@Canvas.Top">
        <xsl:attribute name="cy"><xsl:value-of select="@Canvas.Top + @Height div 2" /></xsl:attribute>
      </xsl:if>
    </xsl:if>
    <xsl:call-template name="template_properties" />  
    <xsl:call-template name="template_transform" />  
    <xsl:apply-templates mode="forward" />
  </ellipse>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Rectangle']">
  <rect>
    <xsl:if test="@Canvas.Left"><xsl:attribute name="x"><xsl:value-of select="@Canvas.Left" /></xsl:attribute></xsl:if>  
    <xsl:if test="@Canvas.Top"><xsl:attribute name="y"><xsl:value-of select="@Canvas.Top" /></xsl:attribute></xsl:if>  
    <xsl:if test="@Width"><xsl:attribute name="width"><xsl:value-of select="@Width" /></xsl:attribute></xsl:if>  
    <xsl:if test="@Height"><xsl:attribute name="height"><xsl:value-of select="@Height" /></xsl:attribute></xsl:if>  
    <xsl:if test="@RadiusX"><xsl:attribute name="rx"><xsl:value-of select="@RadiusX" /></xsl:attribute></xsl:if>  
    <xsl:if test="@RadiusY"><xsl:attribute name="ry"><xsl:value-of select="@RadiusY" /></xsl:attribute></xsl:if>  
    <xsl:call-template name="template_properties" /> 
    <xsl:call-template name="template_transform" />
    <xsl:call-template name="template_timeline_animations" />
    <xsl:apply-templates mode="forward" />
  </rect>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Polyline']">
  <polyline>
    <xsl:if test="@Points"><xsl:attribute name="points"><xsl:value-of select="@Points" /></xsl:attribute></xsl:if>
    <xsl:attribute name="fill-rule">
      <xsl:choose>
        <xsl:when test="@FillRule = 'nonzero'">nonzero</xsl:when>
        <xsl:when test="@FillRule = 'evenodd'">evenodd</xsl:when>
      </xsl:choose>
    </xsl:attribute>
    <xsl:call-template name="template_properties" />  
    <xsl:call-template name="template_transform" />  
    <xsl:apply-templates mode="forward" />
  </polyline>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Polygon']">
  <polygon>
    <xsl:if test="@Points"><xsl:attribute name="points"><xsl:value-of select="@Points" /></xsl:attribute></xsl:if>
    <xsl:attribute name="fill-rule">
      <xsl:choose>
        <xsl:when test="@FillRule = 'nonzero'">nonzero</xsl:when>
        <xsl:when test="@FillRule = 'evenodd'">evenodd</xsl:when>
      </xsl:choose>
    </xsl:attribute>
    <xsl:call-template name="template_properties" />  
    <xsl:call-template name="template_transform" />  
    <xsl:apply-templates mode="forward" />
  </polygon>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Glyphs']">
  <defs>
    <font-face>
      <xsl:attribute name="font-family"><xsl:value-of select="concat('TrueType ', generate-id(.))" /></xsl:attribute>
      <font-face-src><font-face-uri><xsl:attribute name="xlink:href"><xsl:value-of select="@FontUri" /></xsl:attribute></font-face-uri></font-face-src>
    </font-face>
  </defs>
  <text>
    <xsl:if test="@FontRenderingEmSize"><xsl:attribute name="font-size"><xsl:value-of select="@FontRenderingEmSize" /></xsl:attribute></xsl:if>
    <xsl:if test="@OriginX"><xsl:attribute name="x"><xsl:value-of select="@OriginX" /></xsl:attribute></xsl:if>
    <xsl:if test="@OriginY"><xsl:attribute name="y"><xsl:value-of select="@OriginY" /></xsl:attribute></xsl:if>
    <xsl:attribute name="font-family"><xsl:value-of select="concat('TrueType ', generate-id(.))" /></xsl:attribute>
    <xsl:call-template name="template_properties" />  
    <xsl:call-template name="template_transform" />  
    <xsl:if test="@UnicodeString"><xsl:value-of select="@UnicodeString" /></xsl:if>
  </text>
</xsl:template>

</xsl:stylesheet>
