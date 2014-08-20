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

<xsl:template name="template_animation">
  <xsl:if test="@From"><xsl:attribute name="from"><xsl:value-of select="@From" /></xsl:attribute></xsl:if>
  <xsl:if test="@To"><xsl:attribute name="to"><xsl:value-of select="@To" /></xsl:attribute></xsl:if>
  <xsl:if test="@Duration"><xsl:attribute name="dur"><xsl:value-of select="@Duration" /></xsl:attribute></xsl:if>
  <xsl:if test="@RepeatDuration"><xsl:attribute name="repeatDur"><xsl:value-of select="translate(@RepeatDuration, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')" /></xsl:attribute></xsl:if>
  <xsl:if test="@RepeatCount"><xsl:attribute name="repeatCount"><xsl:value-of select="@RepeatCount" /></xsl:attribute></xsl:if>
  <xsl:if test="@AutoReverse">
    <xsl:attribute name="fill">
      <xsl:choose>
        <xsl:when test="@AutoReverse = 'True'">remove</xsl:when>
        <xsl:otherwise>freeze</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>  
  </xsl:if>
</xsl:template>

<xsl:template name="template_timeline_animations">
  <xsl:variable name="id" select="@ID" />
  <xsl:variable name="name" select="@Name" />
  <xsl:apply-templates select="//*[name(.) = 'SetterTimeline' and ($id = @TargetID or $name = @TargetName)]" />
</xsl:template>

<xsl:template name="template_animation_path">
  <xsl:param name="target" />
  <xsl:choose>
    <xsl:when test="$target = '(Line.X2)'">x2</xsl:when>
    <xsl:when test="$target = '(Rectangle.Opacity)'">opacity</xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template match="*[name(.) = concat(name(..), '.Storyboards')]">
  <!--xsl:apply-templates /-->
</xsl:template>

<xsl:template match="*[name(.) = 'ParallelTimeLine']">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'SetterTimeline']">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'ByteAnimationCollection' or name(.) = 'DecimalAnimationCollection' or name(.) = 'DoubleAnimationCollection' or name(.) = 'Int16AnimationCollection' or name(.) = 'Int32AnimationCollection' or name(.) = 'Int64AnimationCollection' or name(.) = 'LengthAnimationCollection' or name(.) = 'SingleAnimationCollection' or name(.) = 'SizeAnimationCollection' or name(.) = 'ThicknessAnimationCollection']">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'ByteAnimation' or name(.) = 'DecimalAnimation' or name(.) = 'DoubleAnimation' or name(.) = 'Int16Animation' or name(.) = 'Int32Animation' or name(.) = 'Int64Animation' or name(.) = 'LengthAnimation' or name(.) = 'SingleAnimation' or name(.) = 'SizeAnimation' or name(.) = 'ThicknessAnimation']">
  <xsl:choose>
    <xsl:when test="../@Path">
      <animate>
        <xsl:attribute name="attributeName"><xsl:call-template name="template_animation_path"><xsl:with-param name="target" select="../@Path" /></xsl:call-template></xsl:attribute>
        <xsl:call-template name="template_animation" />
      </animate>
    </xsl:when>
    <xsl:when test="name(..) = concat(name(.), 'Collection')">
      <animate>
        <xsl:attribute name="attributeName"><xsl:value-of select="translate(substring-after(name(../..), concat(name(../../..), '.')), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')" /></xsl:attribute>
        <xsl:call-template name="template_animation" />
      </animate>
    </xsl:when>
    <xsl:when test="name(..) = concat(name(../..), '.AngleAnimations')">
      <animateTransform attributeName="transform" type="rotate">
        <xsl:call-template name="template_animation" />
      </animateTransform>
    </xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template match="*[name(.) = concat(name(..), '.ColorAnimations')]">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = concat(name(..), '.AngleAnimations')]">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="*[name(.) = 'ColorAnimation']">
  <animateColor>
    <xsl:if test="../@Path">
      <xsl:attribute name="attributeName"><xsl:call-template name="template_animation_path"><xsl:with-param name="target" select="../@Path" /></xsl:call-template></xsl:attribute>
    </xsl:if>
    <xsl:if test="name(..) = concat(name(../..), '.ColorAnimations')">
      <xsl:choose>
        <xsl:when test="name(../..) = 'SolidColorBrush'">
          <xsl:attribute name="attributeName">
            <xsl:value-of select="translate(substring-after(name(../../..), concat(name(../../../..), '.')), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')" />
          </xsl:attribute>
        </xsl:when>
      </xsl:choose>  
    </xsl:if>    
    <xsl:call-template name="template_animation" />
  </animateColor>
</xsl:template>

<xsl:template match="*[name(.) = 'PointAnimation']">
  <animateMotion>
    <xsl:if test="../@Path">
      <xsl:attribute name="attributeName"><xsl:call-template name="template_animation_path"><xsl:with-param name="target" select="../@Path" /></xsl:call-template></xsl:attribute>
    </xsl:if>  
    <xsl:call-template name="template_animation" />
  </animateMotion>
</xsl:template>

<xsl:template match="*[name(.) = 'RectAnimation']">
<!-- -->
</xsl:template>

</xsl:stylesheet>
