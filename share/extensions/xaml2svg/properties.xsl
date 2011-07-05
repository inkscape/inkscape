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
20070912 TemplateBinding in template_color

-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:svg="http://www.w3.org/2000/svg"
xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
xmlns:msxsl="urn:schemas-microsoft-com:xslt">
<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="ISO-8859-1"/>

<xsl:template mode="boundingbox" match="*">
<xsl:if test="system-property('xsl:vendor') = 'Microsoft'">
  <xsl:choose>
    <xsl:when test="@Canvas.Left and @Canvas.Top and @Width and @Height">
      <boundingbox>
        <xsl:attribute name="x1">
          <xsl:value-of select="@Canvas.Left" />
        </xsl:attribute>  
        <xsl:attribute name="x2">
          <xsl:value-of select="@Canvas.Left + @Width" />
        </xsl:attribute>  
        <xsl:attribute name="y1">
          <xsl:value-of select="@Canvas.Top" />
        </xsl:attribute>  
        <xsl:attribute name="y2">
          <xsl:value-of select="@Canvas.Top + @Height" />
        </xsl:attribute>  
      </boundingbox>
    </xsl:when>
    <xsl:when test="count(*) &gt; 0">
      <xsl:variable name="boundingboxes"><xsl:apply-templates mode="boundingbox" select="*" />
      </xsl:variable>
      <boundingbox>
      <xsl:attribute name="x1">
        <xsl:for-each select="msxsl:node-set($boundingboxes)/boundingbox">
          <xsl:sort data-type="number" select="@x1" order="ascending"/>
          <xsl:if test="position() = 1"><xsl:value-of select="@x1" /></xsl:if>
        </xsl:for-each>
      </xsl:attribute>  
      <xsl:attribute name="x2">
        <xsl:for-each select="msxsl:node-set($boundingboxes)/boundingbox">
          <xsl:sort data-type="number" select="@x2" order="descending"/>
          <xsl:if test="position() = 1"><xsl:value-of select="@x2" /></xsl:if>
        </xsl:for-each>
      </xsl:attribute>  
      <xsl:attribute name="y1">
        <xsl:for-each select="msxsl:node-set($boundingboxes)/boundingbox">
          <xsl:sort data-type="number" select="@y1" order="ascending"/>
          <xsl:if test="position() = 1"><xsl:value-of select="@y1" /></xsl:if>
        </xsl:for-each>
      </xsl:attribute>  
      <xsl:attribute name="y2">
        <xsl:for-each select="msxsl:node-set($boundingboxes)/boundingbox">
          <xsl:sort data-type="number" select="@y2" order="descending"/>
          <xsl:if test="position() = 1"><xsl:value-of select="@y2" /></xsl:if>
        </xsl:for-each>
      </xsl:attribute>
      </boundingbox>
    </xsl:when>
  </xsl:choose>
</xsl:if>
</xsl:template>

<xsl:template mode="svg" match="*">
  <xsl:choose>
    <xsl:when test="false() and name(.) != 'Canvas' and name(.) != 'Image' and name(.) != 'Rect' and name(.) != 'Ellipse' and name(.) != 'Text' and name(.) != 'TextBlock' and (@Canvas.Left or @Canvas.Top)">
      <svg>
        <xsl:if test="@Canvas.Left and @Canvas.Top and @Width and @Height">
          <xsl:attribute name="viewBox">
	    <xsl:value-of select="concat(@Canvas.Left, ' ')" />
	    <xsl:value-of select="concat(@Canvas.Top, ' ')" />
	    <xsl:value-of select="concat(@Width - @Canvas.Left, ' ')" />
	    <xsl:value-of select="@Height - @Canvas.Top" />
	  </xsl:attribute>
        </xsl:if>
        <xsl:if test="@Canvas.Left"><xsl:attribute name="x"><xsl:value-of select="@Canvas.Left" /></xsl:attribute></xsl:if>
        <xsl:if test="@Canvas.Top"><xsl:attribute name="y"><xsl:value-of select="@Canvas.Top" /></xsl:attribute></xsl:if>
        <xsl:if test="@Width"><xsl:attribute name="width"><xsl:value-of select="@Width" /></xsl:attribute></xsl:if>
        <xsl:if test="@Height"><xsl:attribute name="height"><xsl:value-of select="@Height" /></xsl:attribute></xsl:if>
        <xsl:apply-templates mode="g" select="." />
      </svg>
    </xsl:when>
    <xsl:otherwise><xsl:apply-templates mode="g" select="." /></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="g" match="*">
  <xsl:choose>
    <xsl:when test="*[(name(.) = 'Shape.Fill' or name(.) = concat(name(..), '.Fill')) and not(*[name(.) = 'SolidColorBrush']) or (name(.) = 'Shape.Stroke' or name(.) = concat(name(..), '.Stroke')) and not(*[name(.) = 'SolidColorBrush']) or name(.) = 'UIElement.OpacityMask' or name(.) = concat(name(..), '.OpacityMask') or name(.) = 'UIElement.Clip' or name(.) = concat(name(..), '.Clip')]">
      <g>
        <xsl:apply-templates mode="defs" select="." />
        <xsl:apply-templates mode="forward" select="." />
      </g>
    </xsl:when>
    <xsl:otherwise><xsl:apply-templates mode="forward" select="." /></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="template_color">
  <xsl:param name="colorspec" />
  <xsl:choose>
    <xsl:when test="contains($colorspec, '#') and (string-length($colorspec) = 7 or string-length($colorspec) = 9)">
      <xsl:value-of select="concat('#', substring($colorspec, string-length($colorspec) - 5, 6))" />
    </xsl:when>
    <xsl:when test="contains($colorspec, '#') and (string-length($colorspec) = 4 or string-length($colorspec) = 5)">
      <xsl:value-of select="concat('#', substring($colorspec, string-length($colorspec) - 5, 3))" />
    </xsl:when>
    <xsl:when test="contains($colorspec, '{StaticResource ')"><xsl:value-of select="concat('url(#', substring-before(substring-after($colorspec, '{StaticResource '), '}'), ')')" /></xsl:when>
    <xsl:when test="contains($colorspec, '{TemplateBinding ')"><xsl:value-of select="concat('url(#', substring-before(substring-after($colorspec, '{TemplateBinding '), '}'), ')')" /></xsl:when>
    <xsl:otherwise><xsl:value-of select="$colorspec" /></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="template_opacity">
  <xsl:param name="colorspec" />
  <xsl:if test="contains($colorspec, '#') and (string-length($colorspec) = 4 or string-length($colorspec) = 9)">
    <xsl:variable name="opacityspec"><xsl:value-of select="translate(substring($colorspec, 2, 2), 'abcdefgh', 'ABCDEFGH')" /></xsl:variable>
    <xsl:choose>
      <xsl:when test="$opacityspec != 'FF'">
        <xsl:value-of select="format-number(number(string-length(substring-before('0123456789ABCDEF', substring($colorspec, 2, 1))) * 16 + string-length(substring-before('0123456789ABCDEF', substring($colorspec, 3, 1)))) div 255, '#0.00')" />
      </xsl:when>
      <xsl:otherwise><xsl:value-of select="''"/></xsl:otherwise>
    </xsl:choose>
  </xsl:if>
</xsl:template>

<xsl:template name="template_properties">
  <xsl:choose>
    <!--
    <xsl:when test="@ID"><xsl:attribute name="id"><xsl:value-of select="@ID" /></xsl:attribute></xsl:when>
    -->
    <xsl:when test="@x:Key"><xsl:attribute name="id"><xsl:value-of select="@x:Key" /></xsl:attribute></xsl:when>
    <xsl:when test="@Name"><xsl:attribute name="id"><xsl:value-of select="@Name" /></xsl:attribute></xsl:when>
  </xsl:choose>
  <xsl:choose>
    <xsl:when test="@Fill">
      <xsl:attribute name="fill"><xsl:call-template name="template_color"><xsl:with-param name="colorspec" select="@Fill" /></xsl:call-template></xsl:attribute>
      <xsl:variable name="test_opacity"><xsl:call-template name="template_opacity"><xsl:with-param name="colorspec" select="@Fill" /></xsl:call-template></xsl:variable>
      <xsl:if test="string-length($test_opacity) &gt; 0">
        <xsl:attribute name="fill-opacity"><xsl:value-of select="$test_opacity" /></xsl:attribute>
      </xsl:if>
    </xsl:when>
    <xsl:when test="not(name(.) = 'Canvas') and not(@Foreground or *[name(.) = 'Shape.Fill' or name(.) = concat(name(..), '.Fill')])"><xsl:attribute name="fill">none</xsl:attribute></xsl:when>
  </xsl:choose>
  <xsl:if test="@Stroke">
    <xsl:attribute name="stroke"><xsl:call-template name="template_color"><xsl:with-param name="colorspec" select="@Stroke" /></xsl:call-template></xsl:attribute>
    <xsl:variable name="test_opacity"><xsl:call-template name="template_opacity"><xsl:with-param name="colorspec" select="@Stroke" /></xsl:call-template></xsl:variable>
    <xsl:if test="string-length($test_opacity) &gt; 0"><xsl:attribute name="stroke-opacity"><xsl:value-of select="$test_opacity" /></xsl:attribute></xsl:if>
  </xsl:if>
  <xsl:if test="@StrokeThickness"><xsl:attribute name="stroke-width"><xsl:value-of select="@StrokeThickness" /></xsl:attribute></xsl:if>
  <xsl:if test="@StrokeMiterLimit"><xsl:attribute name="stroke-miterlimit"><xsl:value-of select="@StrokeMiterLimit" /></xsl:attribute></xsl:if>
  <xsl:if test="@StrokeDashArray"><xsl:attribute name="stroke-dasharray"><xsl:value-of select="@StrokeDashArray" /></xsl:attribute></xsl:if>
  <xsl:if test="@StrokeDashOffset"><xsl:attribute name="stroke-dashoffset"><xsl:value-of select="@StrokeDashOffset" /></xsl:attribute></xsl:if>
  <xsl:if test="@StrokeLineJoin"><xsl:attribute name="stroke-linejoin"><xsl:value-of select="@StrokeLineJoin" /></xsl:attribute></xsl:if>
  <xsl:if test="@StrokeEndLineCap"><xsl:attribute name="stroke-linecap"><xsl:value-of select="@StrokeEndLineCap" /></xsl:attribute></xsl:if>
  <xsl:if test="@Opacity"><xsl:attribute name="fill-opacity"><xsl:value-of select="@Opacity" /></xsl:attribute></xsl:if>
  <xsl:if test="@Color">
    <xsl:attribute name="fill"><xsl:call-template name="template_color"><xsl:with-param name="colorspec" select="@Color" /></xsl:call-template></xsl:attribute>
    <xsl:variable name="test_opacity"><xsl:call-template name="template_opacity"><xsl:with-param name="colorspec" select="@Color" /></xsl:call-template></xsl:variable>
    <xsl:if test="string-length($test_opacity) &gt; 0"><xsl:attribute name="fill-opacity"><xsl:value-of select="$test_opacity" /></xsl:attribute></xsl:if>
  </xsl:if>
  <xsl:if test="@Clip">
    <xsl:choose>
      <xsl:when test="contains(@Clip, '{')"><xsl:attribute name="fill"><xsl:value-of select="concat('url(#', substring-before(substring-after(@Clip, '{'), '}'), ')')" /></xsl:attribute></xsl:when>
      <xsl:otherwise>
        <xsl:attribute name="clip-path"><xsl:value-of select="concat('url(#clippath_', generate-id(.),')')" /></xsl:attribute>
	<defs>
	  <clipPath>
	    <xsl:attribute name="id"><xsl:value-of select="concat('clippath_', generate-id(.))" /></xsl:attribute>
	    <path>
	      <xsl:attribute name="d">
                <xsl:choose>
                  <xsl:when test="contains(@Clip, 'F1')"><xsl:value-of select="substring-after(@Clip, 'F1')" /></xsl:when>
                  <xsl:otherwise><xsl:value-of select="@Clip" /></xsl:otherwise>
                </xsl:choose>	
	      </xsl:attribute></path>
	  </clipPath>
        </defs>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = concat(name(..), '.Resources')]">
  <defs><xsl:apply-templates mode="forward" /></defs>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = concat(name(..), '.Children')]">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Shape.Fill' or name(.) = concat(name(..), '.Fill')]">
  <xsl:choose>
    <xsl:when test="not(*[name(.) = 'SolidColorBrush'])">
      <xsl:attribute name="fill"><xsl:value-of select="concat('url(#id_', generate-id(.), ')')" /></xsl:attribute>
    </xsl:when>
    <xsl:otherwise><xsl:apply-templates select="*[name(.) = 'SolidColorBrush']" /></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="defs" match="*[(name(.) = 'Shape.Fill' or name(.) = concat(name(..), '.Fill')) and not(*[name(.) = 'SolidColorBrush'])]">
  <defs><xsl:apply-templates mode="forward" /></defs>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Shape.Opacity' or name(.) = concat(name(..), '.Opacity')]">
  <xsl:apply-templates />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = concat(name(..), '.Height') or name(.) = concat(name(..), '.Width')]">
  <xsl:apply-templates />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'Shape.Stroke' or name(.) = concat(name(..), '.Stroke')]">
  <xsl:choose>
    <xsl:when test="not(*[name(.) = 'SolidColorBrush'])">
      <xsl:attribute name="stroke"><xsl:value-of select="concat('url(#id_', generate-id(.), ')')" /></xsl:attribute>
    </xsl:when>
    <xsl:otherwise><xsl:apply-templates select="*[name(.) = 'SolidColorBrush']" /></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="defs" match="*[(name(.) = 'Shape.Stroke' or name(.) = concat(name(..), '.Stroke')) and not(*[name(.) = 'SolidColorBrush'])]">
  <defs><xsl:apply-templates mode="forward" /></defs>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'UIElement.Clip' or name(.) = concat(name(..), '.Clip')]">
  <xsl:attribute name="clip-path"><xsl:value-of select="concat('url(#clippath_', generate-id(.),')')" /></xsl:attribute>
</xsl:template>

<xsl:template mode="defs" match="*[name(.) = 'UIElement.Clip' or name(.) = concat(name(..), '.Clip')]">
  <defs>
    <clipPath>
      <xsl:attribute name="id"><xsl:value-of select="concat('clippath_', generate-id(.))" /></xsl:attribute>
      <path>
        <xsl:attribute name="d">
          <xsl:apply-templates mode="forward" />
        </xsl:attribute>
      </path>
    </clipPath>
  </defs>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'UIElement.OpacityMask' or name(.) = concat(name(..), '.OpacityMask')]">
  <xsl:attribute name="mask"><xsl:value-of select="concat('url(#mask_', generate-id(.),')')" /></xsl:attribute>
</xsl:template>

<xsl:template mode="defs" match="*[name(.) = 'UIElement.OpacityMask' or name(.) = concat(name(..), '.OpacityMask')]">
  <defs>
    <mask>
      <xsl:attribute name="id"><xsl:value-of select="concat('mask_', generate-id(.))" /></xsl:attribute>
      <xsl:apply-templates mode="svg" />
    </mask>
  </defs>
</xsl:template>


</xsl:stylesheet>
