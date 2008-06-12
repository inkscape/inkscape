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
xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
xmlns:msxsl="urn:schemas-microsoft-com:xslt"
exclude-result-prefixes="rdf xlink msxsl">

<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="UTF-8"/>

<xsl:param name="silverlight_compatible" select="1" />

<xsl:template name="template_color">
  <xsl:param name="colorspec" />
  <xsl:param name="opacityspec" />
  <xsl:choose>
    <xsl:when test="starts-with($colorspec, 'rgb(') and not(contains($colorspec , '%'))">
      <xsl:value-of select="'#'" />
      <xsl:if test="$opacityspec != '' and number($opacityspec) != 1"><xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number($opacityspec) * 255" /></xsl:with-param></xsl:call-template></xsl:if>
      <xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="substring-before(substring-after($colorspec, 'rgb('), ',')" /></xsl:with-param></xsl:call-template>
      <xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="substring-before(substring-after(substring-after($colorspec, 'rgb('), ','), ',')" /></xsl:with-param></xsl:call-template>
      <xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="substring-before(substring-after(substring-after(substring-after($colorspec, 'rgb('), ','), ','), ')')" /></xsl:with-param></xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($colorspec, 'rgb(') and contains($colorspec , '%')">
      <xsl:value-of select="'#'" />
      <xsl:if test="$opacityspec != '' and number($opacityspec) != 1"><xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number($opacityspec) * 255" /></xsl:with-param></xsl:call-template></xsl:if>
      <xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number(substring-before(substring-after($colorspec, 'rgb('), '%,')) * 255 div 100" /></xsl:with-param></xsl:call-template>
      <xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number(substring-before(substring-after(substring-after($colorspec, 'rgb('), ','), '%,')) * 255 div 100" /></xsl:with-param></xsl:call-template>
      <xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number(substring-before(substring-after(substring-after(substring-after($colorspec, 'rgb('), ','), ','), '%)')) * 255 div 100" /></xsl:with-param></xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($colorspec, '#')">
      <xsl:value-of select="'#'" />
      <xsl:if test="$opacityspec != ''"><xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number($opacityspec) * 255" /></xsl:with-param></xsl:call-template></xsl:if>
      <xsl:choose>
        <xsl:when test="string-length(substring-after($colorspec, '#')) = 3">
          <xsl:variable name="colorspec3"><xsl:value-of select="translate(substring-after($colorspec, '#'), 'abcdefgh', 'ABCDEFGH')" /></xsl:variable>
          <xsl:value-of select="concat(substring($colorspec3, 1, 1), substring($colorspec3, 1, 1))" />
          <xsl:value-of select="concat(substring($colorspec3, 2, 1), substring($colorspec3, 2, 1))" />
          <xsl:value-of select="concat(substring($colorspec3, 3, 1), substring($colorspec3, 3, 1))" />
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="translate(substring-after($colorspec, '#'), 'abcdefgh', 'ABCDEFGH')" /></xsl:otherwise>
      </xsl:choose>  
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="named_color_hex" select="document('colors.xml')/colors/color[@name = translate($colorspec, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')]/@hex" />
      <xsl:choose>
        <xsl:when test="$named_color_hex and $named_color_hex != ''">
          <xsl:value-of select="'#'" />
          <xsl:if test="$opacityspec != '' and number($opacityspec) != 1"><xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number($opacityspec) * 255" /></xsl:with-param></xsl:call-template></xsl:if>
          <xsl:value-of select="substring-after($named_color_hex, '#')" />
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$colorspec" /></xsl:otherwise>
      </xsl:choose>  
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="fill" match="*">
  <xsl:choose>
    <xsl:when test="@fill and starts-with(@fill, 'url(#')"><xsl:value-of select="concat('{StaticResource ', substring-before(substring-after(@fill, 'url(#'), ')'), '}')" /></xsl:when>
    <xsl:when test="@fill"><xsl:value-of select="@fill" /></xsl:when>
    <xsl:when test="@style and contains(@style, 'fill:') and starts-with(substring-after(@style, 'fill:'), 'url(#')"><xsl:value-of select="concat('{StaticResource ', substring-before(substring-after(@style, 'url(#'), ')'), '}')" /></xsl:when>
    <xsl:when test="@style and contains(@style, 'fill:')">
      <xsl:variable name="Fill" select="substring-after(@style, 'fill:')" />
      <xsl:choose>
        <xsl:when test="contains($Fill, ';')">
          <xsl:value-of select="substring-before($Fill, ';')" />
        </xsl:when>  
        <xsl:otherwise><xsl:value-of select="$Fill" /></xsl:otherwise>
      </xsl:choose> 
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="fill" select="parent::*"/></xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template mode="fill_opacity" match="*">
  <xsl:choose>
    <xsl:when test="@fill-opacity"><xsl:value-of select="@fill-opacity" /></xsl:when>
    <xsl:when test="@style and contains(@style, 'fill-opacity:')">
      <xsl:variable name="Opacity" select="substring-after(@style, 'fill-opacity:')" />
      <xsl:choose>
        <xsl:when test="contains($Opacity, ';')"><xsl:value-of select="substring-before($Opacity, ';')" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="$Opacity" /></xsl:otherwise>
      </xsl:choose>  
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="fill_opacity" select="parent::*" /></xsl:when>
  </xsl:choose>  
</xsl:template>


<xsl:template mode="fill_rule" match="*">
  <xsl:choose>
    <xsl:when test="@fill-rule and (@fill-rule = 'nonzero' or @fill-rule = 'evenodd')"><xsl:attribute name="FillRule"><xsl:value-of select="@fill-rule" /></xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'fill-rule:')">
      <xsl:variable name="FillRule" select="substring-after(@style, 'fill-rule:')" />
      <xsl:choose>
        <xsl:when test="contains($FillRule, ';')">
          <xsl:if test="substring-before($FillRule, ';') = 'nonzero' or substring-before($FillRule, ';') = 'evenodd'"><xsl:attribute name="FillRule"><xsl:value-of select="substring-before($FillRule, ';')" /></xsl:attribute></xsl:if>
        </xsl:when>
        <xsl:when test="$FillRule = 'nonzero' or $FillRule = 'evenodd'"><xsl:attribute name="FillRule"><xsl:value-of select="$FillRule" /></xsl:attribute></xsl:when>
      </xsl:choose>  
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="fill_rule" select="parent::*"/></xsl:when>
    <xsl:otherwise><xsl:attribute name="FillRule">NonZero</xsl:attribute></xsl:otherwise>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="template_fill" match="*">
  <xsl:variable name="fill"><xsl:apply-templates mode="fill" select="." /></xsl:variable>
  <xsl:variable name="fill_opacity"><xsl:apply-templates mode="fill_opacity" select="." /></xsl:variable>
  <xsl:if test="not($fill = 'none')">
    <xsl:attribute name="Fill">
      <xsl:choose>
        <xsl:when test="$fill != ''">
          <xsl:call-template name="template_color">
            <xsl:with-param name="colorspec">
              <xsl:if test="$fill != 'none'"><xsl:value-of select="$fill" /></xsl:if>
            </xsl:with-param>
            <xsl:with-param name="opacityspec"><xsl:value-of select="$fill_opacity" /></xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>#000000</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>   
</xsl:template>

<xsl:template mode="stroke" match="*">
  <xsl:choose>
    <xsl:when test="@stroke and starts-with(@stroke, 'url(#')"><xsl:value-of select="concat('{StaticResource ', substring-before(substring-after(@stroke, 'url(#'), ')'), '}')" /></xsl:when>
    <xsl:when test="@stroke and @stroke != 'none'"><xsl:value-of select="@stroke" /></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke:') and starts-with(substring-after(@style, 'stroke:'), 'url(#')"><xsl:value-of select="concat('{StaticResource ', substring-before(substring-after(@style, 'url(#'), ')'), '}')" /></xsl:when>    
    <xsl:when test="@style and contains(@style, 'stroke:')">
      <xsl:variable name="Stroke" select="substring-after(@style, 'stroke:')" />
      <xsl:choose>
        <xsl:when test="contains($Stroke, ';')">
          <xsl:if test="substring-before($Stroke, ';') != 'none'"><xsl:value-of select="substring-before($Stroke, ';')" /></xsl:if>
        </xsl:when>  
        <xsl:when test="$Stroke != 'none'"><xsl:value-of select="$Stroke" /></xsl:when>
      </xsl:choose>  
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke" select="parent::*"/></xsl:when>
  </xsl:choose>
</xsl:template>


<xsl:template mode="stroke_opacity" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-opacity"><xsl:value-of select="@stroke-opacity" /></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-opacity:')">
      <xsl:variable name="Opacity" select="substring-after(@style, 'stroke-opacity:')" />
      <xsl:choose>
        <xsl:when test="contains($Opacity, ';')"><xsl:value-of select="substring-before($Opacity, ';')" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="$Opacity" /></xsl:otherwise>
      </xsl:choose>  
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_opacity" select="parent::*" /></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="template_stroke" match="*">
  <xsl:variable name="stroke"><xsl:apply-templates mode="stroke" select="." /></xsl:variable>
  <xsl:variable name="stroke_opacity"><xsl:apply-templates mode="stroke_opacity" select="." /></xsl:variable>
  <xsl:if test="$stroke != ''">
    <xsl:attribute name="Stroke">
      <xsl:call-template name="template_color">
        <xsl:with-param name="colorspec"><xsl:value-of select="$stroke" /></xsl:with-param>
        <xsl:with-param name="opacityspec"><xsl:value-of select="$stroke_opacity" /></xsl:with-param>
      </xsl:call-template>  
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<xsl:template mode="stroke_width" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-width"><xsl:attribute name="StrokeThickness"><xsl:value-of select="@stroke-width" /></xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-width:')">
      <xsl:variable name="StrokeThickness" select="substring-after(@style, 'stroke-width:')" />
      <xsl:attribute name="StrokeThickness">
        <xsl:choose>
          <xsl:when test="contains($StrokeThickness, ';')"><xsl:value-of select="substring-before($StrokeThickness, ';')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="$StrokeThickness" /></xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_width" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="stroke_miterlimit" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-miterlimit"><xsl:attribute name="StrokeMiterLimit"><xsl:value-of select="@stroke-miterlimit" /></xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-miterlimit:')">
      <xsl:variable name="StrokeMiterLimit" select="substring-after(@style, 'stroke-miterlimit:')" />
      <xsl:attribute name="StrokeMiterLimit">
        <xsl:choose>
          <xsl:when test="contains($StrokeMiterLimit, ';')"><xsl:value-of select="substring-before($StrokeMiterLimit, ';')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="$StrokeMiterLimit" /></xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_miterlimit" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="stroke_dasharray" match="*">
  <!-- stroke-dasharray="10,30,20,30" becomes StrokeDashArray="1 3 2 3" ?? -->
  <xsl:choose>
    <xsl:when test="@stroke-dasharray and @stroke-dasharray != 'none'"><xsl:attribute name="StrokeDashArray"><xsl:value-of select="@stroke-dasharray" /></xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-dasharray:')">
      <xsl:variable name="StrokeDashArray" select="substring-after(@style, 'stroke-dasharray:')" />
      <xsl:choose>
        <xsl:when test="contains($StrokeDashArray, ';')">
          <xsl:if test="substring-before($StrokeDashArray, ';') != 'none'"><xsl:attribute name="StrokeDashArray"><xsl:value-of select="substring-before($StrokeDashArray, ';')" /></xsl:attribute></xsl:if>
        </xsl:when>
        <xsl:when test="$StrokeDashArray != 'none'"><xsl:attribute name="StrokeDashArray"><xsl:value-of select="$StrokeDashArray" /></xsl:attribute></xsl:when>
      </xsl:choose>  
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_dasharray" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="stroke_dashoffset" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-dashoffset"><xsl:attribute name="StrokeDashOffset"><xsl:value-of select="@stroke-dashoffset" /></xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-dashoffset:')">
      <xsl:variable name="StrokeDashOffset" select="substring-after(@style, 'stroke-dashoffset:')" />
      <xsl:attribute name="StrokeDashOffset">
        <xsl:choose>
          <xsl:when test="contains($StrokeDashOffset, ';')"><xsl:value-of select="substring-before($StrokeDashOffset, ';')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="$StrokeDashOffset" /></xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_dashoffset" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template name="linejoin_svg_to_xaml">
  <xsl:param name="linejoin" />
  <xsl:choose>
    <xsl:when test="$linejoin = 'bevel'">Bevel</xsl:when>
    <xsl:when test="$linejoin = 'round'">Round</xsl:when>
    <xsl:otherwise>Miter</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="stroke_linejoin" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-miterlimit">
      <xsl:attribute name="StrokeLineJoin">
        <xsl:call-template name="linejoin_svg_to_xaml"><xsl:with-param name="linejoin"><xsl:value-of select="@stroke-linejoin" /></xsl:with-param></xsl:call-template>
      </xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-linejoin:')">
      <xsl:variable name="StrokeLineJoin" select="substring-after(@style, 'stroke-linejoin:')" />
      <xsl:attribute name="StrokeLineJoin">
        <xsl:choose>
          <xsl:when test="contains($StrokeLineJoin, ';')">
            <xsl:call-template name="linejoin_svg_to_xaml"><xsl:with-param name="linejoin"><xsl:value-of select="substring-before($StrokeLineJoin, ';')" /></xsl:with-param></xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="linejoin_svg_to_xaml"><xsl:with-param name="linejoin"><xsl:value-of select="$StrokeLineJoin" /></xsl:with-param></xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_linejoin" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template name="linecap_svg_to_xaml">
  <xsl:param name="linecap" />
  <xsl:choose>
    <xsl:when test="$linecap = 'round'">Round</xsl:when>
    <xsl:when test="$linecap = 'square'">Square</xsl:when>
    <xsl:otherwise>Flat</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="stroke_linecap" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-linecap">
      <xsl:attribute name="StrokeStartLineCap">
        <xsl:call-template name="linecap_svg_to_xaml"><xsl:with-param name="linecap"><xsl:value-of select="@stroke-linecap" /></xsl:with-param></xsl:call-template>
      </xsl:attribute>
      <xsl:attribute name="StrokeEndLineCap">
        <xsl:call-template name="linecap_svg_to_xaml"><xsl:with-param name="linecap"><xsl:value-of select="@stroke-linecap" /></xsl:with-param></xsl:call-template>
      </xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-linecap:')">
      <xsl:variable name="StrokeStartLineCap" select="substring-after(@style, 'stroke-linecap:')" />
      <xsl:variable name="StrokeEndLineCap" select="substring-after(@style, 'stroke-linecap:')" />
      <xsl:attribute name="StrokeStartLineCap">
        <xsl:choose>
          <xsl:when test="contains($StrokeStartLineCap, ';')">
            <xsl:call-template name="linecap_svg_to_xaml"><xsl:with-param name="linecap"><xsl:value-of select="substring-before($StrokeStartLineCap, ';')" /></xsl:with-param></xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="linecap_svg_to_xaml"><xsl:with-param name="linecap"><xsl:value-of select="$StrokeStartLineCap" /></xsl:with-param></xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
      <xsl:attribute name="StrokeEndLineCap">
        <xsl:choose>
          <xsl:when test="contains($StrokeEndLineCap, ';')">
            <xsl:call-template name="linecap_svg_to_xaml"><xsl:with-param name="linecap"><xsl:value-of select="substring-before($StrokeEndLineCap, ';')" /></xsl:with-param></xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="linecap_svg_to_xaml"><xsl:with-param name="linecap"><xsl:value-of select="$StrokeEndLineCap" /></xsl:with-param></xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stroke_linecap" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="resources" match="*">
  <!-- should be in-depth -->
  <xsl:if test="ancestor::*[name(.) = 'defs']"><xsl:attribute name="x:Key"><xsl:value-of select="@id" /></xsl:attribute></xsl:if>
</xsl:template>  

<xsl:template name="to_hex">
  <xsl:param name="convert" />
  <xsl:value-of select="concat(substring('0123456789ABCDEF', 1 + floor(round($convert) div 16), 1), substring('0123456789ABCDEF', 1 + round($convert) mod 16, 1))" />
</xsl:template>

<xsl:template mode="stop_color" match="*">
  <xsl:variable name="Opacity">
    <xsl:choose>
      <xsl:when test="@stop-opacity"><xsl:value-of select="@stop-opacity" /></xsl:when>
      <xsl:when test="@style and contains(@style, 'stop-opacity:')">
        <xsl:variable name="temp_opacity" select="substring-after(@style, 'stop-opacity:')" />
        <xsl:choose>
          <xsl:when test="contains($temp_opacity, ';')"><xsl:value-of select="substring-before($temp_opacity, ';')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="$temp_opacity" /></xsl:otherwise>
        </xsl:choose>          
      </xsl:when>
      <xsl:otherwise><xsl:value-of select="''" /></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="hex_opacity">
    <xsl:choose>
      <xsl:when test="$Opacity != ''"><xsl:call-template name="to_hex"><xsl:with-param name="convert"><xsl:value-of select="number($Opacity) * 255" /></xsl:with-param></xsl:call-template>  </xsl:when>  
      <xsl:otherwise><xsl:value-of select="$Opacity" /></xsl:otherwise>
    </xsl:choose>  
  </xsl:variable>
  <xsl:variable name="stopcolor">
    <xsl:choose>
      <xsl:when test="@stop-color">
        <xsl:call-template name="template_color"><xsl:with-param name="colorspec"><xsl:value-of select="@stop-color" /></xsl:with-param></xsl:call-template>  
      </xsl:when>
      <xsl:when test="@style and contains(@style, 'stop-color:')">
        <xsl:variable name="Color" select="substring-after(@style, 'stop-color:')" />
        <xsl:choose>
          <xsl:when test="contains($Color, ';')">
            <xsl:call-template name="template_color"><xsl:with-param name="colorspec"><xsl:value-of select="substring-before($Color, ';')" /></xsl:with-param></xsl:call-template>  
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="template_color"><xsl:with-param name="colorspec"><xsl:value-of select="$Color" /></xsl:with-param></xsl:call-template>  
          </xsl:otherwise>
        </xsl:choose>  
      </xsl:when>
      <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stop_color" select="parent::*"/></xsl:when>
      <xsl:otherwise>#000</xsl:otherwise>
    </xsl:choose>  
  </xsl:variable>
  <xsl:attribute name="Color">
    <xsl:choose>
      <xsl:when test="$hex_opacity != '' and starts-with($stopcolor, '#')"><xsl:value-of select="concat('#', $hex_opacity, substring-after($stopcolor, '#'))" /></xsl:when>
      <xsl:otherwise><xsl:value-of select="$stopcolor" /></xsl:otherwise>
    </xsl:choose>
  </xsl:attribute>
</xsl:template>

<xsl:template mode="stop_opacity" match="*">
  <xsl:choose>
    <xsl:when test="@stop-opacity"><xsl:attribute name="Opacity"><xsl:value-of select="@stop-opacity" /></xsl:attribute></xsl:when>
    <xsl:when test="@style and contains(@style, 'stop-opacity:')">
      <xsl:variable name="Opacity" select="substring-after(@style, 'stop-opacity:')" />
      <xsl:attribute name="Opacity">
        <xsl:choose>
          <xsl:when test="contains($Opacity, ';')"><xsl:value-of select="substring-before($Opacity, ';')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="$Opacity" /></xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stop_opacity" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="offset" match="*">
  <xsl:choose>
    <xsl:when test="@offset">
      <xsl:attribute name="Offset">
        <xsl:choose>
          <xsl:when test="contains(@offset, '%')"><xsl:value-of select="number(substring-before(@offset, '%')) div 100" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="@offset" /></xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'offset:')">
      <xsl:variable name="Offset" select="substring-after(@style, 'offset:')" />
      <xsl:attribute name="Offset">
        <xsl:choose>
          <xsl:when test="contains($Offset, '%')"><xsl:value-of select="number(substring-before($Offset, '%')) div 100" /></xsl:when>        
          <xsl:when test="contains($Offset, ';')"><xsl:value-of select="substring-before($Offset, ';')" /></xsl:when>
          <xsl:otherwise><xsl:value-of select="$Offset" /></xsl:otherwise>
        </xsl:choose>  
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'"><xsl:apply-templates mode="stop_offset" select="parent::*"/></xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template name="parse_transform">
  <xsl:param name="input" />
  <xsl:choose>
    <xsl:when test="starts-with($input, 'matrix(')">
      <MatrixTransform><xsl:attribute name="Matrix"><xsl:value-of select="substring-before(substring-after($input, 'matrix('), ')')" /></xsl:attribute></MatrixTransform>
      <xsl:call-template name="parse_transform"><xsl:with-param name="input" select="substring-after($input, ') ')" /></xsl:call-template>      
    </xsl:when>
    <xsl:when test="starts-with($input, 'scale(')">
      <ScaleTransform>
        <xsl:variable name="scale" select="substring-before(substring-after($input, 'scale('), ')')" />
        <xsl:choose>
          <xsl:when test="contains($scale, ',')">
            <xsl:attribute name="ScaleX"><xsl:value-of select="substring-before($scale, ',')" /></xsl:attribute>
            <xsl:attribute name="ScaleY"><xsl:value-of select="substring-after($scale, ',')" /></xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="ScaleX"><xsl:value-of select="$scale" /></xsl:attribute>
            <xsl:attribute name="ScaleY"><xsl:value-of select="$scale" /></xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>  
      </ScaleTransform>
      <xsl:call-template name="parse_transform"><xsl:with-param name="input" select="substring-after($input, ') ')" /></xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($input, 'rotate(')">
      <RotateTransform>
        <xsl:attribute name="Angle"><xsl:value-of select="substring-before(substring-after($input, 'rotate('), ')')" /></xsl:attribute>
        <xsl:if test="@rx"><xsl:attribute name="CenterX"><xsl:value-of select="@rx" /></xsl:attribute></xsl:if>
        <xsl:if test="@ry"><xsl:attribute name="CenterY"><xsl:value-of select="@ry" /></xsl:attribute></xsl:if>
      </RotateTransform>
      <xsl:call-template name="parse_transform"><xsl:with-param name="input" select="substring-after($input, ') ')" /></xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($input, 'skewX(')">
      <SkewTransform>
        <xsl:attribute name="AngleX"><xsl:value-of select="substring-before(substring-after($input, 'skewX('), ')')" /></xsl:attribute>
        <xsl:call-template name="parse_transform"><xsl:with-param name="input" select="substring-after($input, ') ')" /></xsl:call-template>
      </SkewTransform>
    </xsl:when>
    <xsl:when test="starts-with($input, 'skewY(')">
      <SkewTransform>
        <xsl:attribute name="AngleY"><xsl:value-of select="substring-before(substring-after($input, 'skewY('), ')')" /></xsl:attribute>
        <xsl:call-template name="parse_transform"><xsl:with-param name="input" select="substring-after($input, ') ')" /></xsl:call-template>
      </SkewTransform>
    </xsl:when>
    <xsl:when test="starts-with($input, 'translate(')">
      <TranslateTransform>
        <xsl:variable name="translate" select="substring-before(substring-after($input, 'translate('), ')')" />
        <xsl:choose>
          <xsl:when test="contains($translate, ',')">
            <xsl:attribute name="X"><xsl:value-of select="substring-before($translate, ',')" /></xsl:attribute>
            <xsl:attribute name="Y"><xsl:value-of select="substring-after($translate, ',')" /></xsl:attribute>
          </xsl:when>
          <xsl:when test="contains($translate, ' ')">
            <xsl:attribute name="X"><xsl:value-of select="substring-before($translate, ' ')" /></xsl:attribute>
            <xsl:attribute name="Y"><xsl:value-of select="substring-after($translate, ' ')" /></xsl:attribute>
          </xsl:when>
          <xsl:otherwise><xsl:attribute name="X"><xsl:value-of select="$translate" /></xsl:attribute></xsl:otherwise>
        </xsl:choose>  
      </TranslateTransform>
      <xsl:call-template name="parse_transform"><xsl:with-param name="input" select="substring-after($input, ') ')" /></xsl:call-template>
    </xsl:when>  
  </xsl:choose>
</xsl:template>

<xsl:template mode="transform" match="*[@transform or @gradientTransform]">
  <xsl:param name="mapped_type" />
  <xsl:variable name="transform">
    <xsl:choose>
       <xsl:when test="@transform"><xsl:value-of select="@transform" /></xsl:when>
       <xsl:otherwise><xsl:value-of select="@gradientTransform" /></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="transform_nodes">
    <xsl:call-template name="parse_transform">
      <xsl:with-param name="input" select="$transform" />
    </xsl:call-template>
  </xsl:variable>
  
  <xsl:comment>
    <xsl:value-of select="name(.)" />
  </xsl:comment>

  <xsl:choose>
    <xsl:when test="system-property('xsl:vendor') = 'Microsoft' and $mapped_type and $mapped_type != ''">
      <xsl:element name="{$mapped_type}.RenderTransform">
        <xsl:choose>
          <xsl:when test="count(msxsl:node-set($transform_nodes)/*) = 1"><xsl:copy-of select="msxsl:node-set($transform_nodes)" /></xsl:when>
          <xsl:when test="count(msxsl:node-set($transform_nodes)/*) &gt; 1"><TransformGroup><xsl:copy-of select="msxsl:node-set($transform_nodes)" /></TransformGroup></xsl:when>
        </xsl:choose>  
      </xsl:element>
    </xsl:when>
    <xsl:when test="system-property('xsl:vendor') = 'Microsoft'">
      <!-- For instance LinearGradient.Transform -->
      <xsl:choose>
        <xsl:when test="count(msxsl:node-set($transform_nodes)/*) = 1"><xsl:copy-of select="msxsl:node-set($transform_nodes)" /></xsl:when>
        <xsl:when test="count(msxsl:node-set($transform_nodes)/*) &gt; 1"><TransformGroup><xsl:copy-of select="msxsl:node-set($transform_nodes)" /></TransformGroup></xsl:when>
      </xsl:choose>  
    </xsl:when>
  </xsl:choose>  
</xsl:template>

<xsl:template mode="clip" match="*">
  <xsl:choose>
    <xsl:when test="@clip-path and defs/clipPath/path/@d"><xsl:attribute name="Clip"><xsl:value-of select="defs/clipPath/path/@d" /></xsl:attribute></xsl:when>  
    <xsl:when test="@clip-path and starts-with(@clip-path, 'url(#')"><xsl:attribute name="Clip"><xsl:value-of select="concat('{StaticResource ', substring-before(substring-after(@clip-path, 'url(#'), ')'), '}')" /></xsl:attribute></xsl:when>  
    <xsl:when test="@style and contains(@style, 'clip-path:url(#')"><xsl:attribute name="Clip"><xsl:value-of select="concat('{StaticResource ', substring-before(substring-after(@style, 'url(#'), ')'), '}')" /></xsl:attribute></xsl:when>  
    <xsl:when test="clipPath"><xsl:apply-templates mode="forward" /></xsl:when>  
  </xsl:choose>
</xsl:template>

<xsl:template mode="id" match="*">
<xsl:if test="@id">
  <xsl:attribute name="Name"><xsl:value-of select="translate(@id, '- ', '')" /></xsl:attribute>
  <!--
    <xsl:attribute name="x:Key"><xsl:value-of select="translate(@id, '- ', '')" /></xsl:attribute>
  -->
</xsl:if>
</xsl:template>

<xsl:template match="/">
  <xsl:choose>
    <xsl:when test="$silverlight_compatible = 1">
      <xsl:apply-templates mode="forward" />
    </xsl:when>
    <xsl:otherwise>
      <Viewbox Stretch="Uniform">
        <xsl:apply-templates mode="forward" />
      </Viewbox>
    </xsl:otherwise>   
  </xsl:choose>   
</xsl:template>

<xsl:template mode="forward" match="defs">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'svg' or name(.) = 'g']">
  <xsl:choose>
    <xsl:when test="name(.) = 'svg' or @transform or @viewBox or @id or @clip-path or (@style and contains(@style, 'clip-path:url(#')) or (@width and not(contains(@width, '%'))) or @x or @y or (@height and not(contains(@height, '%'))) or *[name(.) = 'linearGradient' or name(.) = 'radialGradient' or name(.) = 'defs' or name(.) = 'clipPath']">
      <Canvas>
        <xsl:apply-templates mode="id" select="." />
        <!--
        <xsl:apply-templates mode="clip" select="." />
        -->
        <xsl:if test="@width and not(contains(@width, '%'))"><xsl:attribute name="Width"><xsl:value-of select="@width" /></xsl:attribute></xsl:if>
        <xsl:if test="@height and not(contains(@height, '%'))"><xsl:attribute name="Height"><xsl:value-of select="@height" /></xsl:attribute></xsl:if>    
        <xsl:if test="@x"><xsl:attribute name="Canvas.Left"><xsl:value-of select="@x" /></xsl:attribute></xsl:if>
        <xsl:if test="@y"><xsl:attribute name="Canvas.Top"><xsl:value-of select="@y" /></xsl:attribute></xsl:if>    
        <xsl:if test="@viewBox">
          <xsl:variable name="viewBox"><xsl:value-of select="normalize-space(translate(@viewBox, ',', ' '))" /></xsl:variable>
          <xsl:attribute name="Width"><xsl:value-of select="substring-before(substring-after(substring-after($viewBox, ' '), ' '), ' ')" /></xsl:attribute>
          <xsl:attribute name="Height"><xsl:value-of select="substring-after(substring-after(substring-after($viewBox, ' '), ' '), ' ')" /></xsl:attribute>
          <Canvas.RenderTransform>
            <TranslateTransform>
              <xsl:attribute name="X"><xsl:value-of select="-number(substring-before($viewBox, ' '))" /></xsl:attribute>
              <xsl:attribute name="Y"><xsl:value-of select="-number(substring-before(substring-after($viewBox, ' '), ' '))" /></xsl:attribute>
            </TranslateTransform>
          </Canvas.RenderTransform>
        </xsl:if>
    <xsl:if test="@transform">
      <Canvas>
        <Canvas.RenderTransform>
          <TransformGroup><xsl:apply-templates mode="transform" select="." /></TransformGroup>
        </Canvas.RenderTransform>
        <xsl:apply-templates mode="forward" select="*" />
      </Canvas>
    </xsl:if>

        <xsl:if test="*[name(.) = 'linearGradient' or name(.) = 'radialGradient' or name(.) = 'defs' or name(.) = 'clipPath']">
          <Canvas.Resources>
            <xsl:apply-templates mode="forward" select="*[name(.) = 'linearGradient' or name(.) = 'radialGradient' or name(.) = 'defs' or name(.) = 'clipPath']" />
          </Canvas.Resources>
        </xsl:if>
        <xsl:if test="not(@transform)">
          <xsl:apply-templates mode="forward" select="*[name(.) != 'linearGradient' and name(.) != 'radialGradient' and name(.) != 'defs' and name(.) != 'clipPath']" />
        </xsl:if>  
      </Canvas>
    </xsl:when>
    <xsl:when test="not(@transform)">
      <xsl:apply-templates mode="forward" select="*" />
    </xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template mode="transform2" match="*">
  <xsl:choose>
    <xsl:when test="@transform">
      <Canvas>
        <Canvas.RenderTransform>
          <TransformGroup><xsl:apply-templates mode="transform" select="." /></TransformGroup>
        </Canvas.RenderTransform>
        <xsl:apply-templates mode="forward" select="." />
      </Canvas>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates mode="forward" select="." />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'image']">
  <Image>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="clip" select="." />
    <xsl:if test="@xlink:href"><xsl:attribute name="Source"><xsl:value-of select="@xlink:href" /></xsl:attribute></xsl:if>
    <xsl:if test="@width"><xsl:attribute name="Width"><xsl:value-of select="@width" /></xsl:attribute></xsl:if>
    <xsl:if test="@height"><xsl:attribute name="Height"><xsl:value-of select="@height" /></xsl:attribute></xsl:if>
    <!--xsl:apply-templates mode="transform" /-->
    <xsl:apply-templates mode="forward" />
  </Image>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'text']">
  <TextBlock>
    <xsl:if test="@font-size"><xsl:attribute name="FontSize"><xsl:value-of select="@font-size" /></xsl:attribute></xsl:if>
    <xsl:if test="@style and contains(@style, 'font-size:')">
      <xsl:variable name="font_size" select="substring-after(@style, 'font-size:')" />
      <xsl:attribute name="FontSize">
        <xsl:choose>
          <xsl:when test="contains($font_size, ';')">
            <xsl:value-of select="substring-before($font_size, ';')" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$font_size" /></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>  
    </xsl:if>
    <xsl:if test="@font-weight"><xsl:attribute name="FontWeight"><xsl:value-of select="@font-weight" /></xsl:attribute></xsl:if>
    <xsl:if test="@style and contains(@style, 'font-weight:')">
      <xsl:variable name="font_weight" select="substring-after(@style, 'font-weight:')" />
      <xsl:attribute name="FontWeight">
        <xsl:choose>
          <xsl:when test="contains($font_weight, ';')">
            <xsl:value-of select="substring-before($font_weight, ';')" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$font_weight" /></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>  
    </xsl:if>
    <xsl:if test="@font-family"><xsl:attribute name="FontFamily"><xsl:value-of select="@font-family" /></xsl:attribute></xsl:if>
    <xsl:if test="@style and contains(@style, 'font-family:')">
      <xsl:variable name="font_family" select="substring-after(@style, 'font-family:')" />
      <xsl:attribute name="FontFamily">
        <xsl:choose>
          <xsl:when test="contains($font_family, ';')">
            <xsl:value-of select="substring-before($font_family, ';')" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$font_family" /></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>  
    </xsl:if>
    <xsl:if test="@font-style"><xsl:attribute name="FontStyle"><xsl:value-of select="@font-style" /></xsl:attribute></xsl:if>
    <xsl:if test="@style and contains(@style, 'font-style:')">
      <xsl:variable name="font_style" select="substring-after(@style, 'font-style:')" />
      <xsl:attribute name="FontStyle">
        <xsl:choose>
          <xsl:when test="contains($font_style, ';')">
            <xsl:value-of select="substring-before($font_style, ';')" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$font_style" /></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>  
    </xsl:if>
    <xsl:if test="@fill"><xsl:attribute name="Foreground"><xsl:value-of select="@fill" /></xsl:attribute></xsl:if>
    <xsl:if test="@style and contains(@style, 'fill')">
      <xsl:variable name="fill" select="substring-after(@style, 'fill:')" />
      <xsl:attribute name="Foreground">
        <xsl:choose>
          <xsl:when test="contains($fill, ';')">
            <xsl:value-of select="substring-before($fill, ';')" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$fill" /></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>  
    </xsl:if>
    <xsl:if test="@text-anchor">
      <xsl:attribute name="HorizontalAlignment">
        <xsl:choose>
          <xsl:when test="@text-anchor = 'start'">Left</xsl:when>
          <xsl:when test="@text-anchor = 'middle'">Center</xsl:when>
          <xsl:when test="@text-anchor = 'end'">Right</xsl:when>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@width"><xsl:attribute name="Width"><xsl:value-of select="@width" /></xsl:attribute></xsl:if>
    <xsl:if test="@height"><xsl:attribute name="Height"><xsl:value-of select="@height" /></xsl:attribute></xsl:if>
    <xsl:if test="@x"><xsl:attribute name="Canvas.Left"><xsl:value-of select="@x" /></xsl:attribute></xsl:if>
    <xsl:if test="@y"><xsl:attribute name="Canvas.Top"><xsl:value-of select="@y" /></xsl:attribute></xsl:if>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="clip" select="." />
    <!--xsl:apply-templates mode="transform" select="." /-->
    <!--xsl:apply-templates mode="forward" /-->
    <xsl:if test="text()"><xsl:value-of select="text()" /></xsl:if>
    <xsl:if test="*[name(.) = 'tspan']/text()"><xsl:value-of select="*[name(.) = 'tspan']/text()" /></xsl:if>
  </TextBlock>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'title' or name(.) = 'desc']">
  <!-- -->
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'marker' or name(.) = 'pattern' or name(.) = 'style']">
  <!-- -->
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'symbol']">
  <Style>
    <xsl:if test="@id"><xsl:attribute name="x:Key"><xsl:value-of select="@id" /></xsl:attribute></xsl:if>
    <Canvas>
      <xsl:apply-templates mode="forward" />
    </Canvas>
  </Style>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'use']">
  <Canvas>
    <xsl:if test="@xlink:href"><xsl:attribute name="Style"><xsl:value-of select="@xlink:href" /></xsl:attribute></xsl:if>
    <!--xsl:apply-templates mode="transform" select="." /-->
    <xsl:apply-templates mode="forward" />
  </Canvas>
</xsl:template>

<xsl:template mode="forward" match="rdf:RDF | *[name(.) = 'foreignObject']">
  <!-- -->
</xsl:template>

<xsl:template match="*">
<xsl:comment><xsl:value-of select="concat('Unknown tag: ', name(.))" /></xsl:comment>
</xsl:template>

<!-- BRUSHES -->

<xsl:template mode="forward" match="*[name(.) = 'linearGradient']">
  <LinearGradientBrush>
    <xsl:if test="@id"><xsl:attribute name="x:Key"><xsl:value-of select="@id" /></xsl:attribute></xsl:if>
    <xsl:attribute name="MappingMode">
      <xsl:choose>
        <xsl:when test="@gradientUnits = 'userSpaceOnUse' ">Absolute</xsl:when>
        <xsl:otherwise>RelativeToBoundingBox</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <xsl:if test="@spreadMethod">
      <xsl:attribute name="SpreadMethod">
        <xsl:choose>
          <xsl:when test="@spreadMethod = 'pad'">Pad</xsl:when>
          <xsl:when test="@spreadMethod = 'reflect'">Reflect</xsl:when>
          <xsl:when test="@spreadMethod = 'repeat'">Repeat</xsl:when>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@x1 and @y1 and @x2 and @y2">
        <xsl:choose>
          <xsl:when test="contains(@x1, '%') and contains(@y1, '%')">
            <xsl:attribute name="StartPoint"><xsl:value-of select="concat(substring-before(@x1, '%') div 100, ',', substring-before(@y1,'%') div 100)" /></xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="StartPoint"><xsl:value-of select="concat(@x1, ',', @y1)" /></xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>  
        <xsl:choose>
          <xsl:when test="contains(@x2, '%') and contains(@y2, '%')">
            <xsl:attribute name="EndPoint"><xsl:value-of select="concat(substring-before(@x2, '%') div 100, ',', substring-before(@y2,'%') div 100)" /></xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="EndPoint"><xsl:value-of select="concat(@x2, ',', @y2)" /></xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>  
      </xsl:when>
      <xsl:otherwise>
        <xsl:attribute name="StartPoint"><xsl:value-of select="'0,0'" /></xsl:attribute>
        <xsl:attribute name="EndPoint"><xsl:value-of select="'1,1'" /></xsl:attribute>
      </xsl:otherwise>
    </xsl:choose>
    <LinearGradientBrush.GradientStops>
      <GradientStopCollection>
        <xsl:choose>
          <xsl:when test="@xlink:href">
            <xsl:variable name="reference_id" select="@xlink:href" />
            <xsl:apply-templates mode="forward" select="//*[name(.) = 'linearGradient' and $reference_id = concat('#', @id)]/*" />
          </xsl:when>
          <xsl:otherwise><xsl:apply-templates mode="forward" /></xsl:otherwise>  
        </xsl:choose>  
      </GradientStopCollection>
    </LinearGradientBrush.GradientStops>
    <xsl:if test="@gradientTransform">
    <LinearGradientBrush.Transform>
      <xsl:apply-templates mode="transform" select="." />
    </LinearGradientBrush.Transform>
  </xsl:if>  
  </LinearGradientBrush>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'radialGradient']">
  <RadialGradientBrush>
    <xsl:if test="@id"><xsl:attribute name="x:Key"><xsl:value-of select="@id" /></xsl:attribute></xsl:if>
    <xsl:attribute name="MappingMode">
      <xsl:choose>
        <xsl:when test="@gradientUnits = 'userSpaceOnUse' ">Absolute</xsl:when>
        <xsl:otherwise>RelativeToBoundingBox</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
    <xsl:if test="@spreadMethod">
      <xsl:attribute name="SpreadMethod">
        <xsl:choose>
          <xsl:when test="@spreadMethod = 'pad'">Pad</xsl:when>
          <xsl:when test="@spreadMethod = 'reflect'">Reflect</xsl:when>
          <xsl:when test="@spreadMethod = 'repeat'">Repeat</xsl:when>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@cx and @cy">
      <xsl:attribute name="Center">
        <xsl:choose>
          <xsl:when test="contains(@cx, '%') and contains(@cy, '%')">
            <xsl:value-of select="concat(number(substring-before(@cx, '%')) div 100, ',', number(substring-before(@cy, '%')) div 100)" />
          </xsl:when>  
          <xsl:otherwise>
            <xsl:value-of select="concat(@cx, ',', @cy)" />
          </xsl:otherwise>  
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>  
    <xsl:if test="@fx and @fy">
      <xsl:attribute name="GradientOrigin">
        <xsl:choose>
          <xsl:when test="contains(@fx, '%') and contains(@fy, '%')">
            <xsl:value-of select="concat(number(substring-before(@fx, '%')) div 100, ',', number(substring-before(@fy, '%')) div 100)" />
          </xsl:when>  
          <xsl:otherwise>
            <xsl:value-of select="concat(@fx, ',', @fy)" />
          </xsl:otherwise>  
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>  
    <xsl:if test="@r">
      <xsl:choose>
        <xsl:when test="contains(@r, '%')">
          <xsl:attribute name="RadiusX"><xsl:value-of select="number(substring-before(@r, '%')) div 100" /></xsl:attribute>
          <xsl:attribute name="RadiusY"><xsl:value-of select="number(substring-before(@r, '%')) div 100" /></xsl:attribute>
        </xsl:when>  
        <xsl:otherwise>
          <xsl:attribute name="RadiusX"><xsl:value-of select="@r" /></xsl:attribute>
          <xsl:attribute name="RadiusY"><xsl:value-of select="@r" /></xsl:attribute>
        </xsl:otherwise>  
      </xsl:choose>
    </xsl:if>  
    <RadialGradientBrush.GradientStops>
      <GradientStopCollection>
        <xsl:choose>
          <xsl:when test="@xlink:href">
            <xsl:variable name="reference_id" select="@xlink:href" />
            <xsl:apply-templates mode="forward" select="//*[name(.) = 'linearGradient' and $reference_id = concat('#', @id)]/*" />
          </xsl:when>
          <xsl:otherwise><xsl:apply-templates mode="forward" /></xsl:otherwise>
        </xsl:choose>
      </GradientStopCollection>
    </RadialGradientBrush.GradientStops>
    <xsl:if test="@gradientTransform">
    <RadialGradientBrush.Transform>
      <xsl:apply-templates mode="transform" select="." />
    </RadialGradientBrush.Transform>
    </xsl:if>  
  </RadialGradientBrush>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'stop']">
  <GradientStop>
    <!--xsl:apply-templates mode="stop_opacity" select="." /-->
    <xsl:apply-templates mode="stop_color" select="." />
    <xsl:apply-templates mode="offset" select="." />
    <xsl:apply-templates mode="forward" />
  </GradientStop>
</xsl:template>

<!-- SHAPES -->

<xsl:template mode="forward" match="*[name(.) = 'line']">
  <Line>
    <xsl:if test="@x1"><xsl:attribute name="X1"><xsl:value-of select="@x1" /></xsl:attribute></xsl:if> 
    <xsl:if test="@y1"><xsl:attribute name="Y1"><xsl:value-of select="@y1" /></xsl:attribute></xsl:if> 
    <xsl:if test="@x2"><xsl:attribute name="X2"><xsl:value-of select="@x2" /></xsl:attribute></xsl:if> 
    <xsl:if test="@y2"><xsl:attribute name="Y2"><xsl:value-of select="@y2" /></xsl:attribute></xsl:if>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Line'" />
    </xsl:apply-templates>    
    
    <xsl:apply-templates mode="forward" />
  </Line>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'rect']">
  <Rectangle>
    <xsl:if test="@x"><xsl:attribute name="Canvas.Left"><xsl:value-of select="@x" /></xsl:attribute></xsl:if>  
    <xsl:if test="@y"><xsl:attribute name="Canvas.Top"><xsl:value-of select="@y" /></xsl:attribute></xsl:if>  
    <xsl:if test="@width"><xsl:attribute name="Width"><xsl:value-of select="@width" /></xsl:attribute></xsl:if>  
    <xsl:if test="@height"><xsl:attribute name="Height"><xsl:value-of select="@height" /></xsl:attribute></xsl:if>  
    <xsl:if test="@rx"><xsl:attribute name="RadiusX"><xsl:value-of select="@rx" /></xsl:attribute></xsl:if>  
    <xsl:if test="@ry"><xsl:attribute name="RadiusY"><xsl:value-of select="@ry" /></xsl:attribute></xsl:if>  
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    
    <xsl:apply-templates mode="resources" select="." />
    
    <xsl:apply-templates mode="clip" select="." />
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Rectangle'" />
    </xsl:apply-templates>    
    
    <xsl:apply-templates mode="forward" />
  </Rectangle>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'polygon']">
  <Polygon>
    <xsl:if test="@points"><xsl:attribute name="Points"><xsl:value-of select="@points" /></xsl:attribute></xsl:if>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="fill_rule" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Polygon'" />
    </xsl:apply-templates>    
    
    <xsl:apply-templates mode="forward" />
  </Polygon>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'polyline']">
  <Polyline>
    <xsl:if test="@points"><xsl:attribute name="Points"><xsl:value-of select="@points" /></xsl:attribute></xsl:if>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="fill_rule" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Polyline'" />
    </xsl:apply-templates>    
    
    <xsl:apply-templates mode="forward" />
  </Polyline>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'path']">
  <Path>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    <xsl:if test="@d">
      <xsl:choose>
        <xsl:when test="$silverlight_compatible = 1">
          <xsl:attribute name="Data">
	    <xsl:value-of select="translate(@d , ',', ' ')" />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <Path.Data>
            <PathGeometry>
              <xsl:attribute name="Figures">
                <xsl:value-of select="translate(@d , ',', ' ')" />
              </xsl:attribute>
              <xsl:apply-templates mode="fill_rule" select="." />
            </PathGeometry>  
          </Path.Data>
         </xsl:otherwise>   
      </xsl:choose>   
    </xsl:if>
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Path'" />
    </xsl:apply-templates>
    
    <xsl:apply-templates mode="forward" />
  </Path>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'ellipse']">
  <Ellipse>
    <xsl:variable name="cx">
      <xsl:choose>
        <xsl:when test="@cx"><xsl:value-of select="@cx" /></xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>  
    </xsl:variable>
    <xsl:variable name="cy">
      <xsl:choose>
        <xsl:when test="@cy"><xsl:value-of select="@cy" /></xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>  
    </xsl:variable>
    <xsl:if test="@rx">
      <xsl:attribute name="Canvas.Left"><xsl:value-of select="$cx - @rx" /></xsl:attribute>
      <xsl:attribute name="Width"><xsl:value-of select="2 * @rx" /></xsl:attribute>
    </xsl:if>
    <xsl:if test="@ry">
      <xsl:attribute name="Canvas.Top"><xsl:value-of select="$cy - @ry" /></xsl:attribute>
      <xsl:attribute name="Height"><xsl:value-of select="2 * @ry" /></xsl:attribute>
    </xsl:if>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    
    <xsl:apply-templates mode="clip" select="." />
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Ellipse'" />
    </xsl:apply-templates>
    
    <xsl:apply-templates mode="forward" />
  </Ellipse>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'circle']">
  <Ellipse>
    <xsl:variable name="cx">
      <xsl:choose>
        <xsl:when test="@cx"><xsl:value-of select="@cx" /></xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>  
    </xsl:variable>
    <xsl:variable name="cy">
      <xsl:choose>
        <xsl:when test="@cy"><xsl:value-of select="@cy" /></xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>  
    </xsl:variable>
    <xsl:if test="@r">
      <xsl:attribute name="Canvas.Left"><xsl:value-of select="$cx - @r" /></xsl:attribute>
      <xsl:attribute name="Canvas.Top"><xsl:value-of select="$cy - @r" /></xsl:attribute>
      <xsl:attribute name="Width"><xsl:value-of select="2 * @r" /></xsl:attribute>
      <xsl:attribute name="Height"><xsl:value-of select="2 * @r" /></xsl:attribute>      
    </xsl:if>
    <xsl:apply-templates mode="id" select="." />
    <xsl:apply-templates mode="template_fill" select="." />
    <xsl:apply-templates mode="template_stroke" select="." />
    <xsl:apply-templates mode="stroke_width" select="." />
    <xsl:apply-templates mode="stroke_miterlimit" select="." />
    <xsl:apply-templates mode="stroke_dasharray" select="." />
    <xsl:apply-templates mode="stroke_dashoffset" select="." />
    <xsl:apply-templates mode="stroke_linejoin" select="." />
    <xsl:apply-templates mode="stroke_linecap" select="." />
    
    <xsl:apply-templates mode="clip" select="." />
    
    <xsl:apply-templates mode="transform" select=".">
      <xsl:with-param name="mapped_type" select="'Ellipse'" />
    </xsl:apply-templates>
    
    <xsl:apply-templates mode="forward" />
  </Ellipse>
</xsl:template>

<xsl:template mode="forward" match="*[name(.) = 'clipPath']">
  <xsl:apply-templates mode="geometry" />
</xsl:template>

<xsl:template mode="geometry" match="*[name(.) = 'circle']">
  <EllipseGeometry>
    <xsl:if test="../@id"><xsl:attribute name="x:Key"><xsl:value-of select="../@id" /></xsl:attribute></xsl:if>  
    <xsl:if test="@cx and @cy"><xsl:attribute name="Center"><xsl:value-of select="concat(@cx, ',', @cy)" /></xsl:attribute></xsl:if>
    <xsl:if test="@r">
      <xsl:attribute name="RadiusX"><xsl:value-of select="@r" /></xsl:attribute>
      <xsl:attribute name="RadiusY"><xsl:value-of select="@r" /></xsl:attribute>
    </xsl:if>
  </EllipseGeometry>
</xsl:template>

<xsl:template mode="geometry" match="*[name(.) = 'rect']">
  <RectangleGeometry>
    <xsl:if test="../@id"><xsl:attribute name="x:Key"><xsl:value-of select="../@id" /></xsl:attribute></xsl:if>  
    <!--
    <xsl:if test="@x"><xsl:attribute name="Canvas.Left"><xsl:value-of select="@x" /></xsl:attribute></xsl:if>  
    <xsl:if test="@y"><xsl:attribute name="Canvas.Top"><xsl:value-of select="@y" /></xsl:attribute></xsl:if>  
    <xsl:if test="@width"><xsl:attribute name="Width"><xsl:value-of select="@width" /></xsl:attribute></xsl:if>  
    <xsl:if test="@height"><xsl:attribute name="Height"><xsl:value-of select="@height" /></xsl:attribute></xsl:if>  
    <xsl:if test="@rx"><xsl:attribute name="RadiusX"><xsl:value-of select="@rx" /></xsl:attribute></xsl:if>  
    <xsl:if test="@ry"><xsl:attribute name="RadiusY"><xsl:value-of select="@ry" /></xsl:attribute></xsl:if>  
    -->
    <xsl:attribute name="Rect"><xsl:value-of select="concat('0, 0, ', @width, ', ', @height)" /></xsl:attribute>
  </RectangleGeometry>
</xsl:template>

</xsl:stylesheet>
