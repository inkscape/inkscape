<?xml version="1.0" encoding="UTF-8"?>

<!--
Authors:
  Nicolas Dufour <nicoduf@yahoo.fr>
  
Copyright (c) 2010 authors

Released under GNU GPL, read the file 'COPYING' for more information
-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:xs="http://www.w3.org/2001/XMLSchema"
xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
xmlns="http://ns.adobe.com/fxg/2008"
xmlns:fxg="http://ns.adobe.com/fxg/2008"
xmlns:d="http://ns.adobe.com/fxg/2008/dt"
xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
xmlns:exsl="http://exslt.org/common"
xmlns:math="http://exslt.org/math"
xmlns:libxslt="http://xmlsoft.org/XSLT/namespace"
xmlns:svg="http://www.w3.org/Graphics/SVG/1.1/DTD/svg11-tiny.dtd"
exclude-result-prefixes="rdf xlink xs exsl libxslt"
extension-element-prefixes="math">

<xsl:strip-space elements="*" />
<xsl:output method="xml" encoding="UTF-8" indent="yes"/>

<!-- 
  // Containers //

  * Root templace
  * Graphic attributes
  * Groups
-->

<!-- 
  // Root template //
-->
<xsl:template match="/">
    <xsl:apply-templates mode="forward" />
</xsl:template>
<!--
  // Graphic //
  First SVG element is converted to Graphic
-->
<xsl:template mode="forward" match="/*[name(.) = 'svg']" priority="1">
  <Graphic>
    <xsl:attribute name="version">2.0</xsl:attribute>
    <xsl:if test="@width and not(contains(@width, '%'))">
      <xsl:attribute name="viewWidth">
        <xsl:call-template name="convert_unit">
          <xsl:with-param name="convert_value" select="@width" />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@height and not(contains(@height, '%'))">
      <xsl:attribute name="viewHeight">
        <xsl:call-template name="convert_unit">
          <xsl:with-param name="convert_value" select="@height" />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@width and not(contains(@width, '%')) and @height and not(contains(@height, '%'))">
      <mask>
        <Group>
          <Rect>
            <xsl:attribute name="width">
              <xsl:call-template name="convert_unit">
                <xsl:with-param name="convert_value" select="@width" />
              </xsl:call-template>
            </xsl:attribute>
            <xsl:attribute name="height">
              <xsl:call-template name="convert_unit">
                <xsl:with-param name="convert_value" select="@height" />
              </xsl:call-template>
            </xsl:attribute>
            <fill>
              <SolidColor color="#ffffff" alpha="1"/>
            </fill>
          </Rect>
        </Group>
      </mask>
    </xsl:if>
    <xsl:apply-templates mode="forward" />
  </Graphic>
</xsl:template>

<!--
  // inner SVG elements //
  Converted to groups
-->
<xsl:template mode="forward" match="*[name(.) = 'svg']">
  <Group>
    <xsl:if test="@x">
      <xsl:attribute name="x">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@x" />
          </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@y">
      <xsl:attribute name="y">
        <xsl:call-template name="convert_unit">
          <xsl:with-param name="convert_value" select="@y" />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@width and not(contains(@width, '%')) and @height and not(contains(@height, '%'))">
      <xsl:attribute name="maskType"><xsl:value-of select="'clip'"/></xsl:attribute>
      <mask>
        <Group>
          <Rect>
            <xsl:attribute name="width">
              <xsl:call-template name="convert_unit">
                <xsl:with-param name="convert_value" select="@width" />
              </xsl:call-template>
            </xsl:attribute>
            <xsl:attribute name="height">
              <xsl:call-template name="convert_unit">
                <xsl:with-param name="convert_value" select="@height" />
              </xsl:call-template>
            </xsl:attribute>
            <fill>
              <SolidColor color="#ffffff" alpha="1"/>
            </fill>
          </Rect>
        </Group>
      </mask>
    </xsl:if>
    <xsl:apply-templates mode="forward" />
  </Group>
</xsl:template>

<!--
  // Groups //
  (including layers)
  
  FXG's Group doesn't support other elements attributes (such as font-size, etc.) 
-->
<xsl:template mode="forward" match="*[name(.) = 'g']">
  <xsl:variable name="object">
    <Group>
      <xsl:if test="@style and contains(@style, 'display:none')">
        <xsl:attribute name="Visibility">Collapsed</xsl:attribute>
      </xsl:if>
      <xsl:if test="@width and not(contains(@width, '%'))">
        <xsl:attribute name="width">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@width" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@height and not(contains(@height, '%'))">
        <xsl:attribute name="height">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@height" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@x">
        <xsl:attribute name="x">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@x" />
            </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@y">
        <xsl:attribute name="y">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@y" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />
      
      <xsl:apply-templates mode="layer_blend" select="." />
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="forward" select="*" />
    </Group>
  </xsl:variable>
  
  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Transforms //
  All the matrix, translate, rotate... stuff.
  * Parse objects transform
  * Object transform
  * Parse gradient transform  
  * Gradient transform
  
  Not supported by FXG:
  * Skew transform.
  * Multiple values rotation.
-->

<!-- 
  // Parse object transform //
-->
<xsl:template name="parse_object_transform">
  <xsl:param name="input" />  
    <xsl:choose>
  <!-- Matrix transform -->
    <xsl:when test="starts-with($input, 'matrix(')">
      <transform>
      <Transform>
      <matrix>
        <Matrix>
          <xsl:variable name="matrix" select="normalize-space(translate(substring-before(substring-after($input, 'matrix('), ')'), ',', ' '))" />
          <xsl:variable name="a" select="substring-before($matrix, ' ')"/>
          <xsl:variable name="ra" select="substring-after($matrix, ' ')"/>
          <xsl:variable name="b" select="substring-before($ra, ' ')"/>
          <xsl:variable name="rb" select="substring-after($ra, ' ')"/>
          <xsl:variable name="c" select="substring-before($rb, ' ')"/>
          <xsl:variable name="rc" select="substring-after($rb, ' ')"/>
          <xsl:variable name="d" select="substring-before($rc, ' ')"/>
          <xsl:variable name="rd" select="substring-after($rc, ' ')"/>
          <xsl:variable name="tx" select="substring-before($rd, ' ')"/>
          <xsl:variable name="ty" select="substring-after($rd, ' ')"/>
          <xsl:attribute name="a"><xsl:value-of select="$a" /></xsl:attribute>
          <xsl:attribute name="b"><xsl:value-of select="$b" /></xsl:attribute>
          <xsl:attribute name="c"><xsl:value-of select="$c" /></xsl:attribute>
          <xsl:attribute name="d"><xsl:value-of select="$d" /></xsl:attribute>
          <xsl:attribute name="tx"><xsl:value-of select='format-number($tx, "#.##")' /></xsl:attribute>
          <xsl:attribute name="ty"><xsl:value-of select='format-number($ty, "#.##")' /></xsl:attribute>
        </Matrix>
      </matrix>
      </Transform>
      </transform>  
    </xsl:when>

  <!-- Scale transform -->
    <xsl:when test="starts-with($input, 'scale(')">
      <xsl:variable name="scale" select="normalize-space(translate(substring-before(substring-after($input, 'scale('), ')'), ',', ' '))" />
      <xsl:choose>
        <xsl:when test="contains($scale, ' ')">
          <xsl:attribute name="scaleX">
            <xsl:value-of select="substring-before($scale, ' ')" />
          </xsl:attribute>
          <xsl:attribute name="scaleY">
            <xsl:value-of select="substring-after($scale, ' ')" />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="scaleX">
            <xsl:value-of select="$scale" />
          </xsl:attribute>
          <xsl:attribute name="scaleY">
            <xsl:value-of select="$scale" />
          </xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
  
  <!-- Rotate transform -->
    <xsl:when test="starts-with($input, 'rotate(')">
      <xsl:variable name="rotate" select="normalize-space(translate(substring-before(substring-after($input, 'rotate('), ')'), ',', ' '))" />
      <xsl:attribute name="rotation">
        <xsl:choose>
          <xsl:when test="contains($rotate, ' ')">
            <xsl:value-of select="substring-before($rotate, ' ')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$rotate" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:if test="@rx">
        <xsl:attribute name="CenterX">
          <xsl:value-of select="@rx" />
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@ry">
        <xsl:attribute name="CenterY">
          <xsl:value-of select="@ry" />
        </xsl:attribute>
      </xsl:if>
    </xsl:when>
  
  <!-- Translate transform -->
    <xsl:when test="starts-with($input, 'translate(')">
      <xsl:variable name="translate" select="normalize-space(translate(substring-before(substring-after($input, 'translate('), ')'), ',', ' '))" />
      <xsl:choose>
        <xsl:when test="contains($translate, ' ')">
          <xsl:attribute name="x">
            <xsl:value-of select="substring-before($translate, ' ')" />
          </xsl:attribute>
          <xsl:attribute name="y">
            <xsl:value-of select="substring-after($translate, ' ')" />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="x">
            <xsl:value-of select="$translate" />
          </xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!-- 
  // Object transform //
  FXG needs a separate group for each transform type in the transform attribute (scale+translate != translate+scale)
-->
<xsl:template name="object_transform">
  <xsl:param name="object" />  
  <xsl:param name="transform" />

  <xsl:variable name="values" select="normalize-space(translate($transform, ',', ' '))" />
  <xsl:choose>
    <xsl:when test="contains($values, ') ')">
      <xsl:call-template name="parse_object_transform">
        <xsl:with-param name="input" select="concat(substring-before($values, ') '), ')')" />
      </xsl:call-template>
      <xsl:variable name="values2" select="substring-after($values, ') ')" />
      <Group>
      <xsl:choose>
        <xsl:when test="contains($values2, ') ')">
          <xsl:call-template name="parse_object_transform">
            <xsl:with-param name="input" select="concat(substring-before($values2, ') '), ')')" />
          </xsl:call-template>
          <xsl:variable name="values3" select="substring-after($values2, ') ')" />
          <Group>
            <xsl:call-template name="parse_object_transform">
              <xsl:with-param name="input" select="concat($values3, ')')" />
            </xsl:call-template>
            <xsl:copy-of select="$object" />
          </Group>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="parse_object_transform">
            <xsl:with-param name="input" select="$values2" />
          </xsl:call-template>
          <xsl:copy-of select="$object" />
        </xsl:otherwise>
      </xsl:choose>
      </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="parse_object_transform">
        <xsl:with-param name="input" select="$values" />
      </xsl:call-template>
      <xsl:copy-of select="$object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Parse gradient transform //
-->
<xsl:template name="parse_gradient_transform">
  <xsl:param name="input" />  
  <xsl:param name="type" />
  <xsl:choose>
  <!-- Scale transform -->
    <xsl:when test="starts-with($input, 'scale(')">
      <xsl:variable name="scale" select="normalize-space(translate(substring-before(substring-after($input, 'scale('), ')'), ',', ' '))" />
      <xsl:choose>
        <xsl:when test="$type='radial'">
          <xsl:choose>
            <xsl:when test="contains($scale, ' ')">
              <xsl:attribute name="scaleX">
                <xsl:value-of select="substring-before($scale, ' ')" />
              </xsl:attribute>
              <xsl:attribute name="scaleY">
                <xsl:value-of select="substring-after($scale, ' ')" />
              </xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
              <xsl:attribute name="scaleX">
                <xsl:value-of select="$scale" />
              </xsl:attribute>
              <xsl:attribute name="scaleY">
                <xsl:value-of select="$scale" />
              </xsl:attribute>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="scaleX">
            <xsl:value-of select="$scale" />
          </xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    
  <!-- Rotate transform -->
    <xsl:when test="starts-with($input, 'rotate(')">
      <xsl:variable name="rotate" select="normalize-space(translate(substring-before(substring-after($input, 'rotate('), ')'), ',', ' '))" />
      <xsl:attribute name="rotation">
        <xsl:choose>
          <xsl:when test="contains($rotate, ' ')">
            <xsl:value-of select="substring-before($rotate, ' ')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$rotate" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:when>
    
  <!-- Translate transform -->
    <xsl:when test="starts-with($input, 'translate(')">
      <xsl:variable name="translate" select="normalize-space(translate(substring-before(substring-after($input, 'translate('), ')'), ',', ' '))" />
      <xsl:choose>
        <xsl:when test="contains($translate, ' ')">
          <xsl:attribute name="x">
            <xsl:value-of select="substring-before($translate, ' ')" />
          </xsl:attribute>
          <xsl:attribute name="y">
            <xsl:value-of select="substring-after($translate, ' ')" />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="x">
            <xsl:value-of select="$translate" />
          </xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!-- 
  // Gradient transform //
  Not implemented yet
  Gradient positioning and transformation are very different in FXG
-->
<xsl:template name="gradient_transform">
  <xsl:param name="transform" />
  <xsl:param name="type" />
  
  <xsl:if test="contains($transform, 'translate')">
    <xsl:call-template name="parse_gradient_transform">
      <xsl:with-param name="input" select="concat('translate(', substring-before(substring-after($transform, 'translate('), ')'), ')')" />
      <xsl:with-param name="type" select="$type" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<!--
  // Resources (defs) //

  * Resources ids
  * Generic defs template
  * Defs gradients
  * Layers blend mode
  * Generic filters template
  * Filter effects
  * Linked filter effects
  * Linear gradients
  * Radial gradients
  * Gradient stops list
  * Gradient stop
  * Clipping
-->

<!-- 
  // Resources ids //
-->
<xsl:template mode="resources" match="*">
  <!-- should be in-depth -->
  <xsl:if test="ancestor::*[name(.) = 'defs']"><xsl:attribute name="id"><xsl:value-of select="@id" /></xsl:attribute></xsl:if>
</xsl:template>

<!--
  // Generic defs template //
-->
<xsl:template mode="forward" match="defs">
<!-- Ignoring defs, do nothing  
  <xsl:apply-templates mode="forward" />
-->
</xsl:template>

<!--
  // Defs gradients //
  ignored
-->
<xsl:template mode="forward" match="*[name(.) = 'linearGradient' or name(.) = 'radialGradient']">

</xsl:template>

<!-- 
  // Layers blend mode //

  Partial support
  Looks good with normal, multiply, and darken
-->
<xsl:template mode="layer_blend" match="*">
  <xsl:if test="@inkscape:groupmode='layer' and @style and contains(@style, 'filter:url(#')">
    <xsl:variable name="id" select="substring-before(substring-after(@style, 'filter:url(#'), ')')" />
    <xsl:for-each select="//*[@id=$id]">
      <xsl:if test="name(child::node()) = 'feBlend'">
        <xsl:attribute name="blendMode">
          <xsl:value-of select="child::node()/@mode"/>
        </xsl:attribute>
      </xsl:if>
    </xsl:for-each>
  </xsl:if>
</xsl:template>

<!--
  // Generic filters template //
  Limited to one filter (can be improved)
-->
<xsl:template mode="forward" match="*[name(.) = 'filter']">
  <xsl:if test="count(*) = 1">
    <xsl:apply-templates mode="forward" />
  </xsl:if> 
</xsl:template>

<!--
  // GaussianBlur filter effects //
  Blur values approximated with d = floor(s * 3*sqrt(2*pi)/4 + 0.5) from:
    http://www.w3.org/TR/SVG/filters.html#feGaussianBlurElement
  Blur quality=2 recommended by the FXG specifications:
    http://opensource.adobe.com/wiki/display/flexsdk/FXG+2.0+Specification#FXG2.0Specification-FilterEffects
-->
<xsl:template mode="forward" match="*[name(.) = 'feGaussianBlur']">
  <xsl:if test="name(.)='feGaussianBlur'">
    <filters>
      <BlurFilter>
        <xsl:attribute name="quality">2</xsl:attribute>
        <xsl:if test="@stdDeviation">
          <xsl:variable name="blur" select="normalize-space(translate(@stdDeviation, ',', ' '))" />
          <xsl:choose>
            <xsl:when test="not(contains($blur, ' '))">
              <xsl:attribute name="blurX">
                <xsl:value-of select="floor($blur * 1.88 + 0.5)" />
              </xsl:attribute>
              <xsl:attribute name="blurY">
                <xsl:value-of select="floor($blur * 1.88 + 0.5)" />
              </xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
              <xsl:attribute name="blurX">
                <xsl:value-of select="floor(substring-before($blur, ' ') * 1.88 + 0.5)" />
              </xsl:attribute>
              <xsl:attribute name="blurY">
                <xsl:value-of select="floor(substring-after($blur, ' ') * 1.88 + 0.5)" />
              </xsl:attribute>
            </xsl:otherwise>
          </xsl:choose>          
        </xsl:if>
      </BlurFilter>
    </filters>
  </xsl:if>
</xsl:template>

<!--
  // Linked filter effect //
  Only supports blurs
-->
<xsl:template mode="filter_effect" match="*">
  <xsl:variable name="id">
    <xsl:choose>
      <xsl:when test="@filter and starts-with(@filter, 'url(#')">
        <xsl:value-of select="substring-before(substring-after(@filter, 'url(#'), ')')" />
      </xsl:when>
      <xsl:when test="@style and contains(@style, 'filter:url(#')">
        <xsl:value-of select="substring-before(substring-after(@style, 'filter:url(#'), ')')" />
      </xsl:when>
    </xsl:choose>
  </xsl:variable>

  <xsl:for-each select="//*[@id=$id]">
    <xsl:apply-templates mode="forward" />
  </xsl:for-each>
</xsl:template>

<!--
  // Linear gradient //
  Full convertion to FXG would require some math.
-->
<xsl:template name="linearGradient">
  <xsl:param name="id" />
  <xsl:for-each select="//*[@id=$id]">
    <xsl:if test="@id">
      <xsl:attribute name="id">
        <xsl:value-of select="@id" />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@spreadMethod">
      <xsl:attribute name="spreadMethod">
        <xsl:choose>
          <xsl:when test="@spreadMethod = 'pad'">pad</xsl:when>
          <xsl:when test="@spreadMethod = 'reflect'">reflect</xsl:when>
          <xsl:when test="@spreadMethod = 'repeat'">repeat</xsl:when>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@color-interpolation">
      <xsl:attribute name="interpolationMethod">
        <xsl:choose>
          <xsl:when test="@color-interpolation = 'linearRGB'">linearRGB</xsl:when>
          <xsl:otherwise>rgb</xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@x1 and @x2 and @y1 and @y2 and function-available('math:atan')">
      <xsl:attribute name="rotation">
        <xsl:value-of select="57.3 * math:atan((@y2 - @y1) div (@x2 - @x1))" />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@gradientTransform">
      <xsl:call-template name="gradient_transform">
        <xsl:with-param name="transform" select="@gradientTransform" />
        <xsl:with-param name="type" select="linear" />
      </xsl:call-template>
    </xsl:if> 
    <xsl:choose>
      <xsl:when test="@xlink:href">
        <xsl:variable name="reference_id" select="@xlink:href" />
        <xsl:call-template name="gradientStops" >
          <xsl:with-param name="id">
            <xsl:value-of select="substring-after($reference_id, '#')" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise><xsl:apply-templates mode="forward" /></xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>

<!--
  // Radial gradient //
    
  Full convertion to FXG would require some math.
-->
<xsl:template name="radialGradient">
  <xsl:param name="id" />
  <xsl:for-each select="//*[@id=$id]">
    <xsl:if test="@id">
      <xsl:attribute name="id">
        <xsl:value-of select="@id" />
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@spreadMethod">
      <xsl:attribute name="spreadMethod">
        <xsl:choose>
          <xsl:when test="@spreadMethod = 'pad'">pad</xsl:when>
          <xsl:when test="@spreadMethod = 'reflect'">reflect</xsl:when>
          <xsl:when test="@spreadMethod = 'repeat'">repeat</xsl:when>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@color-interpolation">
      <xsl:attribute name="interpolationMethod">
        <xsl:choose>
          <xsl:when test="@color-interpolation = 'linearRGB'">linearRGB</xsl:when>
          <xsl:otherwise>rgb</xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="@gradientTransform">
      <xsl:call-template name="gradient_transform">
        <xsl:with-param name="transform" select="@gradientTransform" />
        <xsl:with-param name="type" select="radial" />
      </xsl:call-template>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@xlink:href">
        <xsl:variable name="reference_id" select="@xlink:href" />
        <xsl:call-template name="gradientStops" >
          <xsl:with-param name="id">
            <xsl:value-of select="substring-after($reference_id, '#')" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise><xsl:apply-templates mode="forward" /></xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>

<!--
  // Gradient stops list //
  
  TODO: Find a way to test the existence of the node-set
-->
<xsl:template name="gradientStops">
  <xsl:param name="id" />
  <xsl:variable name="stops">
    <xsl:for-each select="//*[@id=$id]">
      <xsl:apply-templates mode="forward" />
    </xsl:for-each>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="not($stops)">
      <GradientEntry>
        <xsl:attribute name="alpha">0</xsl:attribute>
      </GradientEntry>
    </xsl:when>
    <xsl:otherwise><xsl:copy-of select="$stops" /></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Gradient stop //
-->
<xsl:template mode="forward" match="*[name(.) = 'stop']">
  <GradientEntry>
    <!--xsl:apply-templates mode="stop_opacity" select="." /-->
    <xsl:apply-templates mode="stop_color" select="." />
    <xsl:apply-templates mode="offset" select="." />
    <xsl:apply-templates mode="forward" />
  </GradientEntry>
</xsl:template>

<!--
  // Clipping and masking//
-->
<xsl:template mode="clip" match="*">
  <xsl:param name="object" />
  <xsl:param name="clip_type" />

  <xsl:choose>
    <xsl:when test="$clip_type='clip' and @clip-path and contains(@clip-path, 'url')">
      <Group>
        <xsl:attribute name="maskType"><xsl:value-of select="'clip'"/></xsl:attribute>
        <mask>
          <Group>
            <xsl:variable name="clip_id" select="substring-before(substring-after(@clip-path, 'url(#'), ')')"/>
            <xsl:for-each select="//*[@id=$clip_id]">
              <xsl:if test="not(@clipPathUnits) or @clipPathUnits != 'objectBoundingBox'">
                <xsl:apply-templates mode="forward" />
              </xsl:if>
            </xsl:for-each>
          </Group>
        </mask>
        <xsl:copy-of select="$object"/>
      </Group>
    </xsl:when>
    <xsl:when test="$clip_type='mask' and @mask and contains(@mask, 'url')">
      <Group>
        <xsl:attribute name="maskType"><xsl:value-of select="'alpha'"/></xsl:attribute>
        <mask>
          <Group>
            <xsl:variable name="mask_id" select="substring-before(substring-after(@mask, 'url(#'), ')')"/>
            <xsl:for-each select="//*[@id=$mask_id]">
              <xsl:if test="not(@maskUnits) or @maskUnits != 'objectBoundingBox'">
                <xsl:apply-templates mode="forward" />
              </xsl:if>
            </xsl:for-each>
          </Group>
        </mask>
        <xsl:copy-of select="$object"/>
      </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Misc templates //

  * Id converter
  * Decimal to hexadecimal converter
  * Unit to pixel converter
  * Switch
  * Unknows tags
  * Object description (not supported)
  * Title and description (not supported)
  * Symbols (not supported)
  * Use (not supported)
  * RDF and foreign objects (not supported)
  * Misc ignored stuff (markers, patterns, styles)
-->

<!--
  // Id converter //
  Removes "-" from the original id
  (Not sure FXG really needs it)
-->
<xsl:template mode="id" match="*">
  <xsl:if test="@id">
    <xsl:attribute name="id"><xsl:value-of select="translate(@id, '- ', '')" /></xsl:attribute>
  </xsl:if>
</xsl:template>

<!--
  // Decimal to hexadecimal converter //
-->
<xsl:template name="to_hex">
  <xsl:param name="convert" />
  <xsl:value-of select="concat(substring('0123456789ABCDEF', 1 + floor(round($convert) div 16), 1), substring('0123456789ABCDEF', 1 + round($convert) mod 16, 1))" />
</xsl:template>

<!-- 
  // Unit to pixel converter //
  Values with units (except %) are converted to pixels and rounded.
  Unknown units are kept.
  em, ex and % not implemented
-->
<xsl:template name="convert_unit">
  <xsl:param name="convert_value" />
  <xsl:choose>
    <xsl:when test="contains($convert_value, 'px')">
      <xsl:value-of select="round(translate($convert_value, 'px', ''))" />
    </xsl:when>
    <xsl:when test="contains($convert_value, 'pt')">
      <xsl:value-of select="round(translate($convert_value, 'pt', '') * 1.333333)" />
    </xsl:when>
    <xsl:when test="contains($convert_value, 'pc')">
      <xsl:value-of select="round(translate($convert_value, 'pc', '') * 16)" />
    </xsl:when>
    <xsl:when test="contains($convert_value, 'mm')">
      <xsl:value-of select="round(translate($convert_value, 'mm', '') * 3.779527)" />
    </xsl:when>
    <xsl:when test="contains($convert_value, 'cm')">
      <xsl:value-of select="round(translate($convert_value, 'cm', '') * 37.79527)" />
    </xsl:when>
    <xsl:when test="contains($convert_value, 'in')">
      <xsl:value-of select="round(translate($convert_value, 'in', '') * 96)" />
    </xsl:when>
    <xsl:when test="contains($convert_value, 'ft')">
      <xsl:value-of select="round(translate($convert_value, 'ft', '') * 1152)" />
    </xsl:when>
    <xsl:when test="not(string(number($convert_value))='NaN')">
      <xsl:value-of select="round($convert_value)" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$convert_value" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Switch //
-->
<xsl:template mode="forward" match="*[name(.) = 'switch']">
  <xsl:apply-templates mode="forward" />
</xsl:template>

<!-- 
  // Unknown tags //
  With generic and mode="forward" templates
-->
<xsl:template match="*">
  <xsl:comment><xsl:value-of select="concat('Unknown tag: ', name(.))" /></xsl:comment>
</xsl:template>

<xsl:template mode="forward" match="*">
  <xsl:comment><xsl:value-of select="concat('Unknown tag: ', name(.))" /></xsl:comment>
</xsl:template>

<!-- 
  // Object description //
-->
<xsl:template mode="desc" match="*">

</xsl:template>

<!-- 
  // Title and description //
  Title is ignored and desc is converted to Tag in the mode="desc" template
-->
<xsl:template mode="forward" match="*[name(.) = 'title' or name(.) = 'desc']">

</xsl:template>

<!--
  // Symbols //
-->
<xsl:template mode="forward" match="*[name(.) = 'symbol']">

</xsl:template>

<!--
  // Use //
  Could be implemented via librairies
  (but since it is not supported by Inkscape, not implemented yet)
-->
<xsl:template mode="forward" match="*[name(.) = 'use']">

</xsl:template>

<!--
  // RDF and foreign objects //
-->
<xsl:template mode="forward" match="rdf:RDF | *[name(.) = 'foreignObject']">

</xsl:template>

<!--
  // Misc ignored stuff (markers, patterns, styles) //
-->
<xsl:template mode="forward" match="*[name(.) = 'marker' or name(.) = 'pattern' or name(.) = 'style']">

</xsl:template>

<!--
  // Colors and patterns //

  * Generic color template
  * Object opacity
  * Fill
  * Fill opacity
  * Fill rule
  * Generic fill template
  * Stroke
  * Stroke opacity
  * Generic stroke template
  * Stroke width
  * Stroke mitterlimit
  * Stroke dasharray (not supported in FXG)
  * Stroke dashoffset (not supported in FXG)
  * Linejoin SVG to FxG converter
  * Stroke linejoin
  * Linecap SVG to FXG converter
  * Stroke linecap
  * Gradient stop
  * Gradient stop opacity
  * Gradient stop offset
-->

<!--
  // Generic color template //
-->
<xsl:template name="template_color">
  <xsl:param name="colorspec" />
  <xsl:choose>
    <xsl:when test="starts-with($colorspec, 'rgb(') and not(contains($colorspec , '%'))">
      <xsl:value-of select="'#'" />
      <xsl:call-template name="to_hex">
        <xsl:with-param name="convert">
          <xsl:value-of select="substring-before(substring-after($colorspec, 'rgb('), ',')" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="to_hex">
        <xsl:with-param name="convert">
          <xsl:value-of select="substring-before(substring-after(substring-after($colorspec, 'rgb('), ','), ',')" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="to_hex">
        <xsl:with-param name="convert">
          <xsl:value-of select="substring-before(substring-after(substring-after(substring-after($colorspec, 'rgb('), ','), ','), ')')" />
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($colorspec, 'rgb(') and contains($colorspec , '%')">
      <xsl:value-of select="'#'" />
      <xsl:call-template name="to_hex">
        <xsl:with-param name="convert">
          <xsl:value-of select="number(substring-before(substring-after($colorspec, 'rgb('), '%,')) * 255 div 100" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="to_hex">
        <xsl:with-param name="convert">
          <xsl:value-of select="number(substring-before(substring-after(substring-after($colorspec, 'rgb('), ','), '%,')) * 255 div 100" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="to_hex">
        <xsl:with-param name="convert">
          <xsl:value-of select="number(substring-before(substring-after(substring-after(substring-after($colorspec, 'rgb('), ','), ','), '%)')) * 255 div 100" />
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="starts-with($colorspec, '#')">
      <xsl:value-of select="'#'" />
      <xsl:choose>
        <xsl:when test="string-length(substring-after($colorspec, '#')) = 3">
          <xsl:variable name="colorspec3">
            <xsl:value-of select="translate(substring-after($colorspec, '#'), 'abcdefgh', 'ABCDEFGH')" />
          </xsl:variable>
          <xsl:value-of select="concat(substring($colorspec3, 1, 1), substring($colorspec3, 1, 1))" />
          <xsl:value-of select="concat(substring($colorspec3, 2, 1), substring($colorspec3, 2, 1))" />
          <xsl:value-of select="concat(substring($colorspec3, 3, 1), substring($colorspec3, 3, 1))" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate(substring-after($colorspec, '#'), 'abcdefgh', 'ABCDEFGH')" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="named_color_hex" select="document('colors.xml')/colors/color[@name = translate($colorspec, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')]/@hex" />
      <xsl:choose>
        <xsl:when test="$named_color_hex and $named_color_hex != ''">
          <xsl:value-of select="'#'" />
          <xsl:value-of select="substring-after($named_color_hex, '#')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$colorspec" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Object opacity //
-->
<xsl:template mode="object_opacity" match="*">
  <xsl:if test="@opacity or (@style and (contains(@style, ';opacity:') or starts-with(@style, 'opacity:')))">
    <xsl:variable name="value">
    <xsl:choose>
      <xsl:when test="@opacity">
        <xsl:value-of select="@opacity" />
      </xsl:when>
      <xsl:when test="@style and contains(@style, ';opacity:')">
        <xsl:variable name="Opacity" select="substring-after(@style, ';opacity:')" />
        <xsl:choose>
          <xsl:when test="contains($Opacity, ';')">
            <xsl:value-of select="substring-before($Opacity, ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$Opacity" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="@style and starts-with(@style, 'opacity:')">
        <xsl:variable name="Opacity" select="substring-after(@style, 'opacity:')" />
        <xsl:choose>
          <xsl:when test="contains($Opacity, ';')">
            <xsl:value-of select="substring-before($Opacity, ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$Opacity" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
        <xsl:apply-templates mode="object_opacity" select="parent::*" />
      </xsl:when>
      <xsl:otherwise>1</xsl:otherwise>
    </xsl:choose>
    </xsl:variable>
    <xsl:attribute name="alpha">
    <xsl:choose>
      <xsl:when test="$value &lt; 0">0</xsl:when>
      <xsl:when test="$value &gt; 1">1</xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$value" />
      </xsl:otherwise>
    </xsl:choose>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!--
  // Fill //
-->
<xsl:template mode="fill" match="*">
  <xsl:variable name="value">
    <xsl:choose>
      <xsl:when test="@fill">
        <xsl:value-of select="@fill" />
      </xsl:when>
      <xsl:when test="@style and contains(@style, 'fill:')">
        <xsl:variable name="Fill" select="normalize-space(substring-after(@style, 'fill:'))" />
        <xsl:choose>
          <xsl:when test="contains($Fill, ';')">
            <xsl:value-of select="substring-before($Fill, ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$Fill" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
        <xsl:apply-templates mode="fill" select="parent::*"/>
      </xsl:when>
    </xsl:choose>
  </xsl:variable>
  <xsl:if test="$value">
    <!-- Removes unwanted characters in the color link (TODO: export to a specific template)-->
    <xsl:value-of select="normalize-space(translate($value, '&quot;', ''))" />
  </xsl:if>
</xsl:template>

<!--
  // Fill opacity //
-->
<xsl:template mode="fill_opacity" match="*">
  <xsl:variable name="value">
  <xsl:choose>
    <xsl:when test="@fill-opacity">
      <xsl:value-of select="@fill-opacity" />
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'fill-opacity:')">
      <xsl:variable name="Opacity" select="substring-after(@style, 'fill-opacity:')" />
      <xsl:choose>
        <xsl:when test="contains($Opacity, ';')">
          <xsl:value-of select="substring-before($Opacity, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$Opacity" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="fill_opacity" select="parent::*" />
    </xsl:when>
    <xsl:otherwise>1</xsl:otherwise>
  </xsl:choose>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="$value &lt; 0">0</xsl:when>
    <xsl:when test="$value &gt; 1">1</xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$value" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Fill rule //
-->
<xsl:template mode="fill_rule" match="*">
  <xsl:choose>
    <xsl:when test="@fill-rule and (@fill-rule = 'nonzero' or @fill-rule = 'evenodd')">
      <xsl:if test="@fill-rule = 'nonzero'">
        <xsl:attribute name="winding">nonZero</xsl:attribute>
      </xsl:if>
      <xsl:if test="@fill-rule = 'evenodd'">
        <xsl:attribute name="winding">evenOdd</xsl:attribute>
      </xsl:if>
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'fill-rule:')">
      <xsl:variable name="FillRule" select="normalize-space(substring-after(@style, 'fill-rule:'))" />
      <xsl:choose>
        <xsl:when test="contains($FillRule, ';')">
          <xsl:if test="substring-before($FillRule, ';') = 'nonzero'">
            <xsl:attribute name="winding">nonZero</xsl:attribute>
          </xsl:if>
          <xsl:if test="substring-before($FillRule, ';') = 'evenodd'">
            <xsl:attribute name="winding">evenOdd</xsl:attribute>
          </xsl:if>
        </xsl:when>
        <xsl:when test="$FillRule = 'nonzero'">
          <xsl:attribute name="winding">nonZero</xsl:attribute>
        </xsl:when>
        <xsl:when test=" $FillRule = 'evenodd'">
          <xsl:attribute name="winding">evenOdd</xsl:attribute>
        </xsl:when>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="fill_rule" select="parent::*"/>
        </xsl:when>
    <xsl:otherwise>
      <xsl:attribute name="winding">nonZero</xsl:attribute>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Generic fill template //
-->
<xsl:template mode="template_fill" match="*">
  <xsl:variable name="fill"><xsl:apply-templates mode="fill" select="." /></xsl:variable>
  <xsl:variable name="fill_opacity"><xsl:apply-templates mode="fill_opacity" select="." /></xsl:variable>
    
  <xsl:choose>
    <xsl:when test="$fill != '' and $fill != 'none' and not(starts-with($fill, 'url(#'))">
    <!-- Solid color -->
      <fill>
        <SolidColor>
          <xsl:attribute name="color">
            <xsl:call-template name="template_color">
              <xsl:with-param name="colorspec">
                <xsl:value-of select="$fill" />
              </xsl:with-param>
            </xsl:call-template>
          </xsl:attribute>
          <xsl:attribute name="alpha">
            <xsl:value-of select="$fill_opacity" />
          </xsl:attribute>
        </SolidColor>
      </fill>
    </xsl:when>
    <!-- Gradients -->
    <xsl:when test="starts-with($fill, 'url(#')">
      <xsl:for-each select="//*[@id=substring-before(substring-after($fill, 'url(#'), ')')]">
        <xsl:if test="name(.) = 'linearGradient'">
          <fill>
            <LinearGradient>
              <xsl:call-template name="linearGradient" >
                <xsl:with-param name="id">
                  <xsl:value-of select="substring-before(substring-after($fill, 'url(#'), ')')" />
                </xsl:with-param>
              </xsl:call-template>
            </LinearGradient>
          </fill>
        </xsl:if>
        <xsl:if test="name(.) = 'radialGradient'">
          <fill>
            <RadialGradient>
              <xsl:call-template name="radialGradient" >
                <xsl:with-param name="id">
                  <xsl:value-of select="substring-before(substring-after($fill, 'url(#'), ')')" />
                </xsl:with-param>
              </xsl:call-template>
            </RadialGradient>
          </fill>
        </xsl:if>
      </xsl:for-each>
    </xsl:when>
    <xsl:when test="$fill = 'none'">
    </xsl:when>
    <xsl:otherwise>
      <fill>
        <SolidColor color="#ffffff" alpha="1"/>
      </fill>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Stroke //
-->
<xsl:template mode="stroke" match="*">
  <xsl:choose>
    <xsl:when test="@stroke">
      <xsl:value-of select="@stroke" />
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke:')">
      <xsl:variable name="Stroke" select="normalize-space(substring-after(@style, 'stroke:'))" />
      <xsl:choose>
        <xsl:when test="contains($Stroke, ';')">
          <xsl:value-of select="substring-before($Stroke, ';')" />
        </xsl:when>
        <xsl:when test="$Stroke">
          <xsl:value-of select="$Stroke" />
        </xsl:when>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stroke" select="parent::*"/>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!--
  // Stroke opacity //
-->
<xsl:template mode="stroke_opacity" match="*">
  <xsl:variable name="value">
  <xsl:choose>
    <xsl:when test="@stroke-opacity"><xsl:value-of select="@stroke-opacity" /></xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-opacity:')">
      <xsl:variable name="Opacity" select="substring-after(@style, 'stroke-opacity:')" />
      <xsl:choose>
        <xsl:when test="contains($Opacity, ';')">
          <xsl:value-of select="substring-before($Opacity, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$Opacity" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stroke_opacity" select="parent::*" />
    </xsl:when>
    <xsl:otherwise>1</xsl:otherwise>
  </xsl:choose>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="$value &lt; 0">0</xsl:when>
    <xsl:when test="$value &gt; 1">1</xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$value" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Generic stroke template //
  
  Not supported in FXG:
  * stroke-dasharray
  * stroke-dashoffset
  
 -->
<xsl:template mode="template_stroke" match="*">
  <xsl:variable name="stroke"><xsl:apply-templates mode="stroke" select="." /></xsl:variable>
  <xsl:variable name="stroke_opacity"><xsl:apply-templates mode="stroke_opacity" select="." /></xsl:variable>
  <xsl:variable name="stroke_width"><xsl:apply-templates mode="stroke_width" select="." /></xsl:variable>
  <xsl:variable name="stroke_miterlimit"><xsl:apply-templates mode="stroke_miterlimit" select="." /></xsl:variable>
  <xsl:variable name="stroke_linejoin"><xsl:apply-templates mode="stroke_linejoin" select="." /></xsl:variable>
  <xsl:variable name="stroke_linecap"><xsl:apply-templates mode="stroke_linecap" select="." /></xsl:variable>

  <!-- Solid color -->
  <xsl:if  test="$stroke != '' and $stroke != 'none' and not(starts-with($stroke, 'url(#'))">
    <stroke>
      <SolidColorStroke>
        <xsl:attribute name="color">
          <xsl:call-template name="template_color">
            <xsl:with-param name="colorspec">
              <xsl:value-of select="$stroke" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:attribute>
        <xsl:attribute name="alpha">
          <xsl:value-of select="$stroke_opacity" />
        </xsl:attribute>
        <xsl:attribute name="weight">
          <xsl:value-of select="$stroke_width" />
        </xsl:attribute>
        <xsl:if test="$stroke_miterlimit != ''">
          <xsl:attribute name="miterLimit">
            <xsl:value-of select="$stroke_miterlimit" />
          </xsl:attribute>
        </xsl:if>
        <xsl:attribute name="joints">
          <xsl:value-of select="$stroke_linejoin" />
        </xsl:attribute>
        <xsl:attribute name="caps">
          <xsl:value-of select="$stroke_linecap" />
        </xsl:attribute>
      </SolidColorStroke>  
    </stroke>
  </xsl:if>
    
  <!-- Gradients -->
  <xsl:if test="starts-with($stroke, 'url(#')">
    <xsl:for-each select="//*[@id=substring-before(substring-after($stroke, 'url(#'), ')')]">
      <xsl:if test="name(.) = 'linearGradient'">
        <stroke>
          <LinearGradientStroke>
            <xsl:attribute name="weight">
              <xsl:value-of select="$stroke_width" />
            </xsl:attribute>
            <xsl:if test="$stroke_miterlimit != ''">
              <xsl:attribute name="miterLimit">
                <xsl:value-of select="$stroke_miterlimit" />
              </xsl:attribute>
            </xsl:if>
            <xsl:attribute name="joints">
              <xsl:value-of select="$stroke_linejoin" />
            </xsl:attribute>
            <xsl:attribute name="caps">
              <xsl:value-of select="$stroke_linecap" />
            </xsl:attribute>
            <xsl:call-template name="linearGradient" >
              <xsl:with-param name="id">
                <xsl:value-of select="substring-before(substring-after($stroke, 'url(#'), ')')" />
              </xsl:with-param>
            </xsl:call-template>
          </LinearGradientStroke>
        </stroke>
      </xsl:if>
      <xsl:if test="name(.) = 'radialGradient'">
        <stroke>
          <RadialGradientStroke>
            <xsl:attribute name="weight">
              <xsl:value-of select="$stroke_width" />
            </xsl:attribute>
            <xsl:if test="$stroke_miterlimit != ''">
              <xsl:attribute name="miterLimit">
                <xsl:value-of select="$stroke_miterlimit" />
              </xsl:attribute>
            </xsl:if>
            <xsl:attribute name="joints">
              <xsl:value-of select="$stroke_linejoin" />
            </xsl:attribute>
            <xsl:attribute name="caps">
              <xsl:value-of select="$stroke_linecap" />
            </xsl:attribute>
            <xsl:call-template name="radialGradient" >
              <xsl:with-param name="id">
                <xsl:value-of select="substring-before(substring-after($stroke, 'url(#'), ')')" />
              </xsl:with-param>
            </xsl:call-template>
          </RadialGradientStroke>
        </stroke>
      </xsl:if>
    </xsl:for-each>
  </xsl:if>
</xsl:template>

<!--
  // Stroke width //
-->
<xsl:template mode="stroke_width" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-width">
      <xsl:call-template name="convert_unit">
        <xsl:with-param name="convert_value">
          <xsl:value-of select="@stroke-width" />
        </xsl:with-param>
      </xsl:call-template> 
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-width:')">
      <xsl:call-template name="convert_unit">
        <xsl:with-param name="convert_value">
        <xsl:choose>
          <xsl:when test="contains(substring-after(@style, 'stroke-width:'), ';')">
            <xsl:value-of select="substring-before(substring-after(@style, 'stroke-width:'), ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="substring-after(@style, 'stroke-width:')" />
          </xsl:otherwise>
        </xsl:choose>
        </xsl:with-param>
      </xsl:call-template> 
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stroke_width" select="parent::*"/>
    </xsl:when>
    <xsl:otherwise>1</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Stroke miterlimit //

  Probably not calculated the same way in SVG and FXG (same value but different result)
-->
<xsl:template mode="stroke_miterlimit" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-miterlimit">
      <xsl:value-of select="@stroke-miterlimit" />
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-miterlimit:')">
      <xsl:variable name="miterLimit" select="substring-after(@style, 'stroke-miterlimit:')" />
      <xsl:choose>
        <xsl:when test="contains($miterLimit, ';')">
          <xsl:value-of select="substring-before($miterLimit, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$miterLimit" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stroke_miterlimit" select="parent::*"/>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!-- 
  // Stroke dasharray //
  !! Not supported !!
-->
<xsl:template mode="stroke_dasharray" match="*">
  <xsl:comment>FXG does not support dasharrays</xsl:comment>
</xsl:template>

<!-- 
  // Stroke dashoffset //
  !! Not supported !!
-->
<xsl:template mode="stroke_dashoffset" match="*">
  <xsl:comment>FXG does not support dashoffsets</xsl:comment>
</xsl:template>

<!-- 
  // Linejoin SVG to FXG converter //
-->
<xsl:template name="linejoin_svg_to_fxg">
  <xsl:param name="linejoin" />
  <xsl:choose>
    <xsl:when test="$linejoin = 'bevel'">bevel</xsl:when>
    <xsl:when test="$linejoin = 'round'">round</xsl:when>
    <xsl:otherwise>miter</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Stroke linejoin //
-->
<xsl:template mode="stroke_linejoin" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-linejoin">
      <xsl:call-template name="linejoin_svg_to_fxg">
        <xsl:with-param name="linejoin">
          <xsl:value-of select="@stroke-linejoin" />
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-linejoin:')">
      <xsl:variable name="joints" select="substring-after(@style, 'stroke-linejoin:')" />
      <xsl:choose>
        <xsl:when test="contains($joints, ';')">
          <xsl:call-template name="linejoin_svg_to_fxg">
            <xsl:with-param name="linejoin">
              <xsl:value-of select="substring-before($joints, ';')" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="linejoin_svg_to_fxg">
            <xsl:with-param name="linejoin">
              <xsl:value-of select="$joints" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stroke_linejoin" select="parent::*"/>
    </xsl:when>
    <xsl:otherwise>miter</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Linecap SVG to FXG converter //
  
  Not supported in FXG:
  * butt linecap
-->
<xsl:template name="linecap_svg_to_fxg">
  <xsl:param name="linecap" />
  <xsl:choose>
    <xsl:when test="$linecap = 'round'">round</xsl:when>
    <xsl:when test="$linecap = 'square'">square</xsl:when>
    <xsl:when test="$linecap = 'butt'">round</xsl:when>
    <xsl:otherwise>none</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!--
  // Stroke linecap //
-->
<xsl:template mode="stroke_linecap" match="*">
  <xsl:choose>
    <xsl:when test="@stroke-linecap">
      <xsl:call-template name="linecap_svg_to_fxg">
        <xsl:with-param name="linecap">
          <xsl:value-of select="@stroke-linecap" />
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'stroke-linecap:')">
      <xsl:variable name="caps" select="substring-after(@style, 'stroke-linecap:')" />
      <xsl:choose>
        <xsl:when test="contains($caps, ';')">
          <xsl:call-template name="linecap_svg_to_fxg">
            <xsl:with-param name="linecap">
              <xsl:value-of select="substring-before($caps, ';')" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="linecap_svg_to_fxg">
            <xsl:with-param name="linecap">
              <xsl:value-of select="$caps" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stroke_linecap" select="parent::*"/>
    </xsl:when>
    <xsl:otherwise>none</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Gradient stops //
-->
<xsl:template mode="stop_color" match="*">
  <xsl:variable name="Opacity">
    <xsl:choose>
      <xsl:when test="@stop-opacity">
        <xsl:value-of select="@stop-opacity" />
      </xsl:when>
      <xsl:when test="@style and contains(@style, 'stop-opacity:')">
        <xsl:variable name="temp_opacity" select="substring-after(@style, 'stop-opacity:')" />
        <xsl:choose>
          <xsl:when test="contains($temp_opacity, ';')">
            <xsl:value-of select="substring-before($temp_opacity, ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$temp_opacity" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise><xsl:value-of select="''" /></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="hex_opacity">
    <xsl:choose>
      <xsl:when test="$Opacity != ''">
        <xsl:call-template name="to_hex">
          <xsl:with-param name="convert">
            <xsl:value-of select="number($Opacity) * 255" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$Opacity" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="stopcolor">
    <xsl:choose>
      <xsl:when test="@stop-color">
        <xsl:call-template name="template_color">
          <xsl:with-param name="colorspec">
            <xsl:value-of select="@stop-color" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@style and contains(@style, 'stop-color:')">
        <xsl:variable name="Color" select="normalize-space(substring-after(@style, 'stop-color:'))" />
        <xsl:choose>
          <xsl:when test="contains($Color, ';')">
            <xsl:call-template name="template_color">
              <xsl:with-param name="colorspec">
                <xsl:value-of select="substring-before($Color, ';')" />
              </xsl:with-param>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="template_color">
              <xsl:with-param name="colorspec">
                <xsl:value-of select="$Color" />
              </xsl:with-param>
            </xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
        <xsl:apply-templates mode="stop_color" select="parent::*"/>
      </xsl:when>
      <xsl:otherwise>#000</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:attribute name="color">
    <xsl:value-of select="$stopcolor" />
  </xsl:attribute>
  <xsl:if test="$Opacity != ''">
    <xsl:attribute name="alpha">
      <xsl:value-of select="$Opacity" />
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!--
  // Gradient stop opacity //
-->
<xsl:template mode="stop_opacity" match="*">
  <xsl:choose>
    <xsl:when test="@stop-opacity">
      <xsl:attribute name="Opacity">
        <xsl:value-of select="@stop-opacity" />
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'stop-opacity:')">
      <xsl:variable name="Opacity" select="substring-after(@style, 'stop-opacity:')" />
      <xsl:attribute name="opacity">
        <xsl:choose>
          <xsl:when test="contains($Opacity, ';')">
            <xsl:value-of select="substring-before($Opacity, ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$Opacity" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stop_opacity" select="parent::*"/>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!--
  // Gradient stop offset //
-->
<xsl:template mode="offset" match="*">
  <xsl:choose>
    <xsl:when test="@offset">
      <xsl:attribute name="ratio">
        <xsl:choose>
          <xsl:when test="contains(@offset, '%')">
            <xsl:value-of select="number(substring-before(@offset, '%')) div 100" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@offset" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'offset:')">
      <xsl:variable name="Offset" select="substring-after(@style, 'offset:')" />
      <xsl:attribute name="ratio">
        <xsl:choose>
          <xsl:when test="contains($Offset, '%')">
            <xsl:value-of select="number(substring-before($Offset, '%')) div 100" />
          </xsl:when>    
          <xsl:when test="contains($Offset, ';')">
            <xsl:value-of select="substring-before($Offset, ';')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$Offset" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="stop_offset" select="parent::*"/>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!-- 
  // Text specific templates //

  * Text tspan
  * Text flowPara
  * Text flowRegion (text frame)
  * Get font size
  * Font size
  * Font weight
  * Font family
  * Font style
  * Baseline shift
  * Line height
  * Writing mode
  * Text decoration
  * Text fill
  * Text direction
  * Text size
  * Text position
  * Text object
  * FlowRoot object
-->

 <!-- 
  // Text span //
  SVG: tspan, flowSpan, FXG: span
  
  Not supported in FXG:
  * span position
-->
<xsl:template mode="forward" match="*[name(.) = 'tspan'  or name(.) = 'flowSpan']">
  <span>
    <xsl:if test="../@xml:space='preserve'">
      <xsl:attribute name="whiteSpaceCollapse">preserve</xsl:attribute>
    </xsl:if>
    <xsl:variable name="fill">
      <xsl:apply-templates mode="fill" select="." />
    </xsl:variable>
    <xsl:variable name="fill_opacity">
      <xsl:apply-templates mode="fill_opacity" select="." />
    </xsl:variable>
    <xsl:if test="starts-with($fill, '#') or (not(starts-with($fill, 'url')) and $fill != '' and $fill != 'none')">
      <xsl:attribute name="color">
        <xsl:call-template name="template_color">
          <xsl:with-param name="colorspec">
            <xsl:value-of select="$fill" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:attribute>
      <xsl:attribute name="textAlpha">
        <xsl:value-of select="$fill_opacity" />
      </xsl:attribute>
    </xsl:if>
    <xsl:apply-templates mode="font_size" select="." />
    <xsl:apply-templates mode="font_weight" select="." />
    <xsl:apply-templates mode="font_family" select="." />
    <xsl:apply-templates mode="font_style" select="." />
    <xsl:apply-templates mode="text_fill" select="." />
    <xsl:apply-templates mode="text_decoration" select="." />
    <xsl:apply-templates mode="line_height" select="." />
    <xsl:apply-templates mode="baseline_shift" select="." />
    
    <xsl:if test="text()">
      <xsl:value-of select="text()" />
    </xsl:if>
  </span>
</xsl:template>

 <!-- 
  // Text flowPara //
  SVG: flowPara, flowDiv FXG: p
  
  Not supported in FXG:
  * paragraph position
-->
<xsl:template mode="forward" match="*[name(.) = 'flowPara' or name(.) = 'flowDiv']">
  <p>
    <xsl:if test="../@xml:space='preserve'">
      <xsl:attribute name="whiteSpaceCollapse">preserve</xsl:attribute>
    </xsl:if>
    <xsl:variable name="fill">
      <xsl:apply-templates mode="fill" select="." />
    </xsl:variable>
    <xsl:variable name="fill_opacity">
      <xsl:apply-templates mode="fill_opacity" select="." />
    </xsl:variable>
    <xsl:if test="starts-with($fill, '#') or (not(starts-with($fill, 'url')) and $fill != '' and $fill != 'none')">
      <xsl:attribute name="color">
        <xsl:call-template name="template_color">
          <xsl:with-param name="colorspec">
            <xsl:value-of select="$fill" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:attribute>
      <xsl:attribute name="textAlpha">
        <xsl:value-of select="$fill_opacity" />
      </xsl:attribute>
    </xsl:if>
    <xsl:apply-templates mode="font_size" select="." />
    <xsl:apply-templates mode="font_weight" select="." />
    <xsl:apply-templates mode="font_family" select="." />
    <xsl:apply-templates mode="font_style" select="." />
    <xsl:apply-templates mode="text_fill" select="." />
    <xsl:apply-templates mode="text_decoration" select="." />
    <xsl:apply-templates mode="line_height" select="." />
    <xsl:apply-templates mode="baseline_shift" select="." />

    <xsl:choose>
      <xsl:when test="*[name(.) = 'flowSpan']/text()">
        <xsl:apply-templates mode="forward" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="@xml:space='preserve'">
            <xsl:copy-of select="translate(text(), '&#x9;&#xA;&#xD;', ' ')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:copy-of select="normalize-space(translate(text(), '&#x9;&#xA;&#xD;', ' '))" />
          </xsl:otherwise>
        </xsl:choose>  
      </xsl:otherwise>
    </xsl:choose>
  </p>
</xsl:template>


 <!-- 
  // Text flowRegion //
-->
<xsl:template mode="flow_region" match="*">
  <xsl:apply-templates mode="text_size" select="." />
  <xsl:apply-templates mode="text_position" select="." />
</xsl:template>

<!-- 
  // Get text font size //
-->
<xsl:template mode="get_font_size" match="*">
  <xsl:choose>
    <xsl:when test="@font-size">
      <xsl:value-of select="@font-size" />
    </xsl:when>
    <xsl:when test="@style and contains(@style, 'font-size:')">
      <xsl:variable name="font_size" select="normalize-space(substring-after(@style, 'font-size:'))" />
      <xsl:choose>
        <xsl:when test="contains($font_size, ';')">
          <xsl:value-of select="substring-before($font_size, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$font_size" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="name(..) = 'g' or name(..) = 'svg'">
      <xsl:apply-templates mode="get_font_size" select="parent::*"/>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!-- 
  // Text font size //
  SVG: font-size, FXG: fontSize
-->
<xsl:template mode="font_size" match="*">
  <xsl:variable name="value">
    <xsl:apply-templates mode="get_font_size" select="." />
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="fontSize">
      <xsl:call-template name="convert_unit">
        <xsl:with-param name="convert_value" select="$value" />
      </xsl:call-template>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text font weight //
  SVG: font-weight, FXG: fontWeight
-->
<xsl:template mode="font_weight" match="*">
  <xsl:variable name="value">
    <xsl:if test="@font-weight">
      <xsl:value-of select="@font-weight" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'font-weight:')">
      <xsl:variable name="font_weight" select="normalize-space(substring-after(@style, 'font-weight:'))" />
      <xsl:choose>
        <xsl:when test="contains($font_weight, ';')">
          <xsl:value-of select="substring-before($font_weight, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$font_weight" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="fontWeight">
      <xsl:choose>
        <xsl:when test="$value='normal' or $value='bold'">
          <xsl:value-of select="$value" />
        </xsl:when>
        <xsl:when test="$value &lt; 500 or $value = 'lighter'">normal</xsl:when>
        <xsl:when test="$value &gt; 499 or $value = 'bolder'">bold</xsl:when>
        <xsl:otherwise>normal</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text font family //
  SVG: font-family, FXG: fontFamily
-->
<xsl:template mode="font_family" match="*">
  <xsl:variable name="value">
    <xsl:if test="@font-family">
      <xsl:value-of select="translate(@font-family, &quot;'&quot;, '')" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'font-family:')">
      <xsl:variable name="font_family" select="normalize-space(substring-after(@style, 'font-family:'))" />
      <xsl:choose>
        <xsl:when test="contains($font_family, ';')">
          <xsl:value-of select="translate(substring-before($font_family, ';'), &quot;'&quot;, '')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate($font_family, &quot;'&quot;, '')" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="fontFamily">
      <xsl:choose>
        <xsl:when test="$value='Sans'">Arial</xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$value" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text font style //
  SVG: font-style, FXG: fontStyle
-->
<xsl:template mode="font_style" match="*">
  <xsl:variable name="value">
    <xsl:if test="@font-style">
      <xsl:value-of select="@font-style" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'font-style:')">
      <xsl:variable name="font_style" select="normalize-space(substring-after(@style, 'font-style:'))" />
      <xsl:choose>
        <xsl:when test="contains($font_style, ';')">
          <xsl:value-of select="substring-before($font_style, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$font_style" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="fontStyle">
      <xsl:value-of select="$value" />
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text baseline shift //
  SVG: baseline-shift, FXG: baselineShift
-->
<xsl:template mode="baseline_shift" match="*">
  <xsl:variable name="value">
    <xsl:if test="@baseline-shift">
      <xsl:value-of select="@baseline-shift" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'baseline-shift:') and not(contains(substring-after(@style, 'baseline-shift:'), ';'))">
      <xsl:value-of select="substring-after(@style, 'baseline-shift:')" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'baseline-shift:') and contains(substring-after(@style, 'baseline-shift:'), ';')">
      <xsl:value-of select="substring-before(substring-after(@style, 'baseline-shift:'), ';')" />
    </xsl:if>   
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="baselineShift">  
      <xsl:choose>
        <xsl:when test="$value='baseline'">0</xsl:when>
        <xsl:when test="$value='super'">superscript</xsl:when>
        <xsl:when test="$value='sub'">subscript</xsl:when>
        <xsl:when test="translate($value, '%', '') &lt; -1000">-1000</xsl:when>
        <xsl:when test="translate($value, '%', '') &gt; 1000">1000</xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate($value, '%', '')" />
        </xsl:otherwise>
      </xsl:choose>  
      <xsl:if test="contains($value, '%')">%</xsl:if>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text line height //
  SVG: line-height, FXG: lineHeight
-->
<xsl:template mode="line_height" match="*">
  <xsl:variable name="value">
    <xsl:if test="@line-height">
      <xsl:value-of select="@line-height" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'line-height:')">
      <xsl:variable name="line_height" select="normalize-space(substring-after(@style, 'line-height:'))" />
      <xsl:choose>
        <xsl:when test="contains($line_height, ';')">
          <xsl:value-of select="substring-before($line_height, ';')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$line_height" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="lineHeight">
      <xsl:call-template name="convert_unit">
        <xsl:with-param name="convert_value" select="$value" />
      </xsl:call-template>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text writing mode //
  SVG: writing-mode, FXG: blockProgression 
  
  Values inverted in FXG...
-->
<xsl:template mode="writing_mode" match="*">
  <xsl:variable name="value">
    <xsl:if test="@writing-mode">
      <xsl:value-of select="@writing-mode" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'writing-mode:') and not(contains(substring-after(@style, 'writing-mode:'), ';'))">
      <xsl:value-of select="substring-after(@style, 'writing-mode:')" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'writing-mode:') and contains(substring-after(@style, 'writing-mode:'), ';')">
      <xsl:value-of select="substring-before(substring-after(@style, 'writing-mode:'), ';')" />
    </xsl:if>   
  </xsl:variable>
  <xsl:if test="$value != ''">
    <xsl:attribute name="blockProgression">  
      <xsl:choose>
        <xsl:when test="$value='tb'">rl</xsl:when>
        <xsl:otherwise>tb</xsl:otherwise>
      </xsl:choose>  
    </xsl:attribute>
    <xsl:if test="$value='tb'">
      <xsl:attribute name="textRotation">rotate270</xsl:attribute>
    </xsl:if>  
  </xsl:if>
</xsl:template>
  
<!-- 
  // Text decoration //
  SVG: text-decoration, FXG: textDecoration, lineThrough 
-->
<xsl:template mode="text_decoration" match="*">
  <xsl:variable name="value">
    <xsl:if test="@text-decoration">
      <xsl:value-of select="@text-decoration" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'text-decoration:') and not(contains(substring-after(@style, 'text-decoration:'), ';'))">
      <xsl:value-of select="substring-after(@style, 'text-decoration:')" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'text-decoration:') and contains(substring-after(@style, 'text-decoration:'), ';')">
      <xsl:value-of select="substring-before(substring-after(@style, 'text-decoration:'), ';')" />
    </xsl:if>   
  </xsl:variable>
  <xsl:if test="$value != ''">  
    <xsl:choose>
      <xsl:when test="$value='underline'">
        <xsl:attribute name="textDecoration">underline</xsl:attribute>
      </xsl:when>
      <xsl:when test="$value='line-through'">
        <xsl:attribute name="lineThrough">true</xsl:attribute>
      </xsl:when>
    </xsl:choose>  
  </xsl:if>
</xsl:template>

<!-- 
  // Text fill //
  SVG: fill, fill-opacity, FXG: color, textAlpha
-->
<xsl:template mode="text_fill" match="*">
  <xsl:variable name="fill">
    <xsl:apply-templates mode="fill" select="." />
  </xsl:variable>
  <xsl:variable name="fill_opacity">
    <xsl:apply-templates mode="fill_opacity" select="." />
  </xsl:variable>
  <xsl:if test="starts-with($fill, '#') or (not(starts-with($fill, 'url')) and $fill != '' and $fill != 'none')">
    <xsl:attribute name="color">
      <xsl:call-template name="template_color">
        <xsl:with-param name="colorspec">
          <xsl:value-of select="$fill" />
        </xsl:with-param>
      </xsl:call-template>
    </xsl:attribute>
    <xsl:attribute name="textAlpha">
      <xsl:value-of select="$fill_opacity" />
    </xsl:attribute>
  </xsl:if>
</xsl:template>

<!-- 
  // Text direction //
  SVG: direction, unicode-bidi, FXG: direction
-->
<xsl:template mode="direction" match="*">
  <xsl:variable name="value">
    <xsl:if test="@direction">
      <xsl:value-of select="@direction" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'direction:') and not(contains(substring-after(@style, 'direction:'), ';'))">
      <xsl:value-of select="substring-after(@style, 'direction:')" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'direction:') and contains(substring-after(@style, 'direction:'), ';')">
      <xsl:value-of select="substring-before(substring-after(@style, 'direction:'), ';')" />
    </xsl:if>   
  </xsl:variable>
  <xsl:variable name="bidi">
    <xsl:if test="@unicode-bidi">
      <xsl:value-of select="@unicode-bidi" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'unicode-bidi:') and not(contains(substring-after(@style, 'unicode-bidi:'), ';'))">
      <xsl:value-of select="substring-after(@style, 'unicode-bidi:')" />
    </xsl:if>
    <xsl:if test="@style and contains(@style, 'unicode-bidi:') and contains(substring-after(@style, 'unicode-bidi:'), ';')">
      <xsl:value-of select="substring-before(substring-after(@style, 'unicode-bidi:'), ';')" />
    </xsl:if>   
  </xsl:variable>

  <xsl:if test="$value != '' and ($bidi='embed' or $bidi='bidi-override')">  
    <xsl:attribute name="direction">
      <xsl:choose>
        <xsl:when test="$value='ltr'">ltr</xsl:when>
        <xsl:when test="$value='rtl'">rtl</xsl:when>
      </xsl:choose>  
    </xsl:attribute>
  </xsl:if>
</xsl:template>

 <!-- 
  // Text size //
-->
<xsl:template mode="text_size" match="*">
  <xsl:if test="@width">
    <xsl:attribute name="width">
      <xsl:call-template name="convert_unit">
        <xsl:with-param name="convert_value" select="@width" />
      </xsl:call-template>
    </xsl:attribute>
  </xsl:if>
  <xsl:if test="@height">
    <xsl:attribute name="height">
      <xsl:call-template name="convert_unit">
        <xsl:with-param name="convert_value" select="@height" />
      </xsl:call-template>
    </xsl:attribute>
  </xsl:if>
</xsl:template>

 <!-- 
  // Text position //
-->
<xsl:template mode="text_position" match="*">
  <!-- Keep the first x value only -->
  <xsl:if test="@x">
    <xsl:attribute name="x">
      <xsl:choose>
        <xsl:when test="contains(@x, ' ')">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="substring-before(@x, ' ')" />
          </xsl:call-template>   
          </xsl:when>
          <xsl:otherwise>
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@x" />
          </xsl:call-template>  
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>
  <!-- Keep the first y value only -->
  <xsl:if test="@y">
    <xsl:attribute name="y">
      <xsl:choose>
        <xsl:when test="contains(@y, ' ')">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="substring-before(@y, ' ')" />
          </xsl:call-template>   
          </xsl:when>
          <xsl:otherwise>
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@y" />
          </xsl:call-template>  
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:if>
</xsl:template>


<!-- 
  // Text objects //
  SVG: text, FXG: RichText

  Not supported by FXG:
  * Generic fonts.
  * Embedded fonts (in defs).
  * Character rotation.
  * Character positionning (x and y).
  * Text-anchor.
  * Text stroke.
  
  Partial support:
  * Text rotation (0, 90, 180 and 270 degrees only) -> not implemented.
  * Font weight (normal and bold only) -> values under 500 are considered normal, the others bold.
  * Whitespace handling, issues with xml:whitespace='preserve'.
-->
<xsl:template mode="forward" match="*[name(.) = 'text']">
  <xsl:variable name="object">
    <RichText>
      <!-- Force default baseline to "ascent" -->
      <xsl:attribute name="alignmentBaseline">ascent</xsl:attribute>
      
      <xsl:apply-templates mode="font_size" select="." />
      <xsl:apply-templates mode="font_weight" select="." />
      <xsl:apply-templates mode="font_family" select="." />
      <xsl:apply-templates mode="font_style" select="." />
      <xsl:apply-templates mode="text_fill" select="." />
      <xsl:apply-templates mode="text_decoration" select="." />
      <xsl:apply-templates mode="line_height" select="." />
      <xsl:apply-templates mode="text_size" select="." />
      <xsl:apply-templates mode="text_position" select="." />
      <xsl:apply-templates mode="direction" select="." />
      <xsl:apply-templates mode="writing_mode" select="." />
      <xsl:apply-templates mode="id" select="." />

      <xsl:if test="not(*[name(.) = 'tspan']/text())">
        <xsl:attribute name="whiteSpaceCollapse">preserve</xsl:attribute>
      </xsl:if>
      
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="desc" select="." />

      <!--xsl:apply-templates mode="forward" /-->
      <content>
        <xsl:choose>
          <xsl:when test="*[name(.) = 'tspan']/text()">
            <xsl:apply-templates mode="forward" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="@xml:space='preserve'">
                <xsl:copy-of select="translate(text(), '&#x9;&#xA;&#xD;', ' ')" />
              </xsl:when>
              <xsl:otherwise>
                <xsl:copy-of select="normalize-space(translate(text(), '&#x9;&#xA;&#xD;', ' '))" />
              </xsl:otherwise>
            </xsl:choose>  
          </xsl:otherwise>
        </xsl:choose>
      </content>
    </RichText>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
 
 <!-- 
  // FlowRoot objects //
  SVG: flowRoot, FXG: RichText

  Not supported by FXG:
  * See text objects
-->
<xsl:template mode="forward" match="*[name(.) = 'flowRoot']">
  <xsl:variable name="object">
    <RichText>
      <!-- Force default baseline to "ascent" -->
      <xsl:attribute name="alignmentBaseline">ascent</xsl:attribute>
      
      <xsl:apply-templates mode="font_size" select="." />
      <xsl:apply-templates mode="font_weight" select="." />
      <xsl:apply-templates mode="font_family" select="." />
      <xsl:apply-templates mode="font_style" select="." />
      <xsl:apply-templates mode="text_fill" select="." />
      <xsl:apply-templates mode="text_decoration" select="." />
      <xsl:apply-templates mode="line_height" select="." />
      <xsl:apply-templates mode="direction" select="." />
      <xsl:apply-templates mode="writing_mode" select="." />
      <xsl:apply-templates mode="id" select="." />
      <xsl:apply-templates mode="flow_region" select="*[name(.) = 'flowRegion']/child::node()" />
      
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="desc" select="." />

      <content>
        <xsl:choose>
          <xsl:when test="*[name(.) = 'flowPara' or name(.) = 'flowDiv']/text()">
            <xsl:apply-templates mode="forward" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="@xml:space='preserve'">
                <xsl:copy-of select="translate(text(), '&#x9;&#xA;&#xD;', ' ')" />
              </xsl:when>
              <xsl:otherwise>
                <xsl:copy-of select="normalize-space(translate(text(), '&#x9;&#xA;&#xD;', ' '))" />
              </xsl:otherwise>
            </xsl:choose>  
          </xsl:otherwise>
        </xsl:choose>
      </content>
    </RichText>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Shapes //
  
  * Lines
  * Rectangle
  * Path
  * Ellipse
  * Circle
  * Image
  * Polygon (not supported)
  * Polyline (not supported)
-->

<!-- 
  // Line object //
  SVG: line, FXG: Line
-->
<xsl:template mode="forward" match="*[name(.) = 'line']">
  <xsl:variable name="object">
    <Line>
      <xsl:if test="@x1">
        <xsl:attribute name="xFrom">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@x1" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@y1">
        <xsl:attribute name="yFrom">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@y1" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@x2">
        <xsl:attribute name="xTo">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@x2" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@y2">
        <xsl:attribute name="yTo">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@y2" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />

      <xsl:apply-templates mode="template_fill" select="." />
      <xsl:apply-templates mode="template_stroke" select="." />
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="desc" select="." />   

      <xsl:apply-templates mode="forward" />
    </Line>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Rectangle object //
  SVG: rect, FXG: Rect
-->
<xsl:template mode="forward" match="*[name(.) = 'rect']">
  <xsl:variable name="object">
    <Rect>
      <xsl:if test="@x">
        <xsl:attribute name="x">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@x" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@y">
        <xsl:attribute name="y">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@y" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@width">
        <xsl:attribute name="width">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@width" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@height">
        <xsl:attribute name="height">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@height" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@rx">
        <xsl:attribute name="radiusX">
          <xsl:value-of select="@rx" />
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@ry">
        <xsl:attribute name="radiusY">
          <xsl:value-of select="@ry" />
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@rx and not(@ry)">
        <xsl:attribute name="radiusX">
          <xsl:value-of select="@rx" />
        </xsl:attribute>
        <xsl:attribute name="radiusY">
          <xsl:value-of select="@rx" />
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@ry and not(@rx)">
        <xsl:attribute name="radiusX">
          <xsl:value-of select="@ry" />
        </xsl:attribute>
        <xsl:attribute name="radiusY">
          <xsl:value-of select="@ry" />
        </xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />

      <xsl:apply-templates mode="template_fill" select="." />
      <xsl:apply-templates mode="template_stroke" select="." />
      <xsl:apply-templates mode="filter_effect" select="." />
      <!--  <xsl:apply-templates mode="resources" select="." /> -->
      <xsl:apply-templates mode="desc" select="." />

      <xsl:apply-templates mode="forward" />
    </Rect>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Path //
  SVG: path, FXG: Path
  
  Not supported by FXG:
  * elliptical arc curve commands (workaround: convert to path first)
  TODO:
  * Implement an arc to curve convertor
-->
<xsl:template mode="forward" match="*[name(.) = 'path']">
  <xsl:variable name="object">
    <Path>
      <!-- Path element -->
      <!-- Exclude arcs in order to prevent the mxml compiler from failing -->
      <xsl:if test="@d and not(contains(@d, 'a') or contains(@d, 'A'))">
        <xsl:attribute name="data">
          <xsl:value-of select="normalize-space(translate(@d , ',', ' '))" />
        </xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="fill_rule" select="." />
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />

      <!-- Child elements -->
      <xsl:apply-templates mode="template_fill" select="." />
      <xsl:apply-templates mode="template_stroke" select="." />
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="desc" select="." />

      <xsl:apply-templates mode="forward" />
    </Path>
  </xsl:variable>
  
  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:if test="contains(@d, 'a') or contains(@d, 'A')">
    <xsl:comment><xsl:value-of select="'Elliptic arc command in path data not supported, please convert to path (arcs are thus converted to curves) before exporting.'" /></xsl:comment>
  </xsl:if> 
</xsl:template>

<!-- 
  // Ellipse object //
  SVG: ellipse, FXG: Ellipse
-->
<xsl:template mode="forward" match="*[name(.) = 'ellipse']">
  <xsl:variable name="object">
    <Ellipse>
      <xsl:variable name="cx">
        <xsl:choose>
          <xsl:when test="@cx">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@cx" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="cy">
        <xsl:choose>
          <xsl:when test="@cy">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@cy" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="rx">
        <xsl:choose>
          <xsl:when test="@rx">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@rx" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="ry">
        <xsl:choose>
          <xsl:when test="@ry">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@ry" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:choose>
        <xsl:when test="$rx != 0">
          <xsl:attribute name="x">
            <xsl:value-of select='format-number($cx - $rx, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="width">
            <xsl:value-of select='format-number(2 * $rx, "#.#")' />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="x">
            <xsl:value-of select='format-number($cx, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="width">0</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
        <xsl:when test="$ry != 0">
          <xsl:attribute name="y">
            <xsl:value-of select='format-number($cy - $ry, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="height">
            <xsl:value-of select='format-number(2 * $ry, "#.#")' />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="y">
            <xsl:value-of select='format-number($cy, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="height">0</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />

      <!-- Child elements -->
      <xsl:apply-templates mode="template_fill" select="." />
      <xsl:apply-templates mode="template_stroke" select="." />
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="desc" select="." />

      <xsl:apply-templates mode="forward" />
    </Ellipse>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Circle object //
  SVG: circle, FXG: Ellipse
-->
<xsl:template mode="forward" match="*[name(.) = 'circle']">
  <xsl:variable name="object">
    <Ellipse>
      <xsl:variable name="cx">
        <xsl:choose>
          <xsl:when test="@cx">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@cx" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="cy">
        <xsl:choose>
          <xsl:when test="@cy">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@cy" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="r">
        <xsl:choose>
          <xsl:when test="@r">
            <xsl:call-template name="convert_unit">
              <xsl:with-param name="convert_value" select="@r" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      
      <xsl:choose>
        <xsl:when test="$r != 0">
          <xsl:attribute name="x">
            <xsl:value-of select='format-number($cx - $r, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="y">
            <xsl:value-of select='format-number($cy - $r, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="width">
            <xsl:value-of select='format-number(2 * $r, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="height">
            <xsl:value-of select='format-number(2 * $r, "#.#")' />
          </xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="x">
            <xsl:value-of select='format-number($cx, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="y">
            <xsl:value-of select='format-number($cy, "#.#")' />
          </xsl:attribute>
          <xsl:attribute name="width">0</xsl:attribute>
          <xsl:attribute name="height">0</xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />

      <!-- Child elements -->
      <xsl:apply-templates mode="template_fill" select="." />
      <xsl:apply-templates mode="template_stroke" select="." />
      <xsl:apply-templates mode="filter_effect" select="." />
      <xsl:apply-templates mode="desc" select="." />

      <xsl:apply-templates mode="forward" />
    </Ellipse>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Image objects //
  SVG: image, FXG: Rect+BitmapFill
  
  Not supported by FXG:
  * Embedded images (base64).
  * Preserve ratio.
-->
<xsl:template mode="forward" match="*[name(.) = 'image']">
  <xsl:variable name="object">
    <Rect>
      <xsl:if test="@x">
        <xsl:attribute name="x">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@x" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@y">
        <xsl:attribute name="y">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@y" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@width">
        <xsl:attribute name="width">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@width" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:if test="@height">
        <xsl:attribute name="height">
          <xsl:call-template name="convert_unit">
            <xsl:with-param name="convert_value" select="@height" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <xsl:apply-templates mode="object_opacity" select="." />
      <xsl:apply-templates mode="id" select="." />
      
      <xsl:apply-templates mode="desc" select="." />
      
      <xsl:if test="@xlink:href">
        <fill>
          <BitmapFill>
            <xsl:attribute name="source">@Embed('<xsl:value-of select="@xlink:href"/>')</xsl:attribute>
          </BitmapFill>
        </fill> 
      </xsl:if>
      
      <xsl:apply-templates mode="forward" />
    </Rect>
  </xsl:variable>

  <xsl:variable name="clipped_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$object" />
      <xsl:with-param name="clip_type" select="'clip'" />
    </xsl:apply-templates>
  </xsl:variable>

  <xsl:variable name="masked_object">
    <xsl:apply-templates mode="clip" select="." >
      <xsl:with-param name="object" select="$clipped_object" />
      <xsl:with-param name="clip_type" select="'mask'" />
    </xsl:apply-templates>
  </xsl:variable>
  
  <xsl:choose>
    <xsl:when test="@transform">
    <Group>
      <xsl:call-template name="object_transform">
        <xsl:with-param name="object" select="$masked_object" />
        <xsl:with-param name="transform" select="@transform" />
      </xsl:call-template>
    </Group>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select="$masked_object" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
  // Polygon object //
  !! Not supported !!
-->
<xsl:template mode="forward" match="*[name(.) = 'polygon']">
  <xsl:comment>FXG does not support polygons</xsl:comment>
</xsl:template>

<!-- 
  // Polyline object //
  !! Not supported !!
-->
<xsl:template mode="forward" match="*[name(.) = 'polyline']">
  <xsl:comment>FXG does not support polylines</xsl:comment>
</xsl:template>

</xsl:stylesheet>
