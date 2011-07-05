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

<xsl:template name="template_transform">
  <xsl:variable name="transform_value">
    <xsl:if test="@Transform"><xsl:value-of select="@Transform" /></xsl:if>
    <xsl:apply-templates select="*[name(.) = 'UIElement.RenderTransform' or name(.) = 'Shape.RenderTransform' or name(.) = concat(name(..), '.RenderTransform')]/*" />
  </xsl:variable>  
  <xsl:if test="string-length($transform_value) &gt; 0"><xsl:attribute name="transform"><xsl:value-of select="$transform_value" /></xsl:attribute></xsl:if>  
</xsl:template>

<xsl:template name="template_gradienttransform">
  <xsl:variable name="transform_value">
    <xsl:if test="@Canvas.Left and @Canvas.Top"><xsl:value-of select="concat('translate(', @Canvas.Left, ',', @Canvas.Top, ')')" /></xsl:if>
    <xsl:if test="@Transform"><xsl:value-of select="@Transform" /></xsl:if>
    <xsl:apply-templates select="*[name(.) = 'Brush.Transform' or name(.) = concat(name(..), '.Transform')]" />
    <xsl:apply-templates select="*[name(.) = 'Brush.RelativeTransform' or name(.) = concat(name(..), '.RelativeTransform')]" />
  </xsl:variable>  
  <xsl:if test="string-length($transform_value) &gt; 0"><xsl:attribute name="gradientTransform"><xsl:value-of select="$transform_value" /></xsl:attribute></xsl:if>  
</xsl:template>

<xsl:template match="*[name(.) = 'UIElement.RenderTransform' or name(.) = 'Shape.RenderTransform' or name(.) = concat(name(..), '.RenderTransform')]">
  <!-- xsl:apply-templates /-->
</xsl:template>

<xsl:template match="*[name(.) = 'Brush.Transform' or name(.) = concat(name(..), '.Transform')]">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'Brush.RelativeTransform' or name(.) = concat(name(..), '.RelativeTransform')]">
  <xsl:apply-templates />
</xsl:template>

<!--
<xsl:template match="*[name(.) = 'TransformCollection' or name(.) = 'TransformGroup']">
  <xsl:apply-templates />
</xsl:template>
-->
<xsl:template match="*[name(.) = 'TransformGroup']">
  <xsl:apply-templates />
</xsl:template>


<!--
<xsl:template mode="forward" match="*[name(.) = 'TransformDecorator']">
  <g>
    <xsl:attribute name="transform">
      <xsl:if test="@Transform"><xsl:value-of select="@Transform" /></xsl:if>
      <xsl:apply-templates select="*[name(.) = 'TransformDecorator.Transform']/*" />
    </xsl:attribute>
  <xsl:apply-templates select="*[name(.) = 'TransformDecorator.Transform']/*/*" />  
  <xsl:apply-templates mode="forward" select="*[name(.) != 'TransformDecorator.Transform']" />
  </g>
</xsl:template>
-->

<xsl:template match="*[name(.) = 'TranslateTransform']">
  <xsl:if test="@X">
    <xsl:value-of select="concat('translate(', @X)" />
    <xsl:if test="@Y"><xsl:value-of select="concat(', ', @Y)" /></xsl:if>
    <xsl:value-of select="') '" />      
  </xsl:if>  
</xsl:template>

<xsl:template match="*[name(.) = 'ScaleTransform']">
  <xsl:if test="@ScaleX">
    <xsl:value-of select="concat('scale(', @ScaleX)" />
    <xsl:if test="@ScaleY"><xsl:value-of select="concat(', ', @ScaleY)" /></xsl:if>
    <xsl:value-of select="') '" />      
  </xsl:if>   
</xsl:template>

<xsl:template match="*[name(.) = 'RotateTransform']">
  <xsl:if test="@Angle">
    <xsl:value-of select="concat('rotate(', @Angle)" />
    <xsl:if test="@Center"><xsl:value-of select="concat(',', @Center)" /></xsl:if>
    <xsl:value-of select="') '" />      
  </xsl:if>
</xsl:template>

<xsl:template match="*[name(.) = 'SkewTransform']">
  <xsl:if test="@AngleX"><xsl:value-of select="concat('skewX(', @AngleX,') ')" /></xsl:if>  
  <xsl:if test="@AngleY"><xsl:value-of select="concat('skewY(', @AngleY,') ')" /></xsl:if>
</xsl:template>

<xsl:template match="*[name(.) = 'MatrixTransform']">
  <xsl:if test="@Matrix"><xsl:value-of select="concat('matrix(', @Matrix,') ')" /></xsl:if>  
</xsl:template>

</xsl:stylesheet>
