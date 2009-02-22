<?xml version='1.0'?>
<xsl:stylesheet
          xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>
<xsl:output method="html"/>

<xsl:template match="book">
  <html>
  <head>
  <style  TYPE="text/css">
      p
        {
        text-indent: 0.25in
        }
      h3
        {
        font-family: Arial, Helvetica, 'Bitstream Vera Sans', 'Luxi Sans', Verdana, Sans-Serif;
        font-size: 24px;
        }
      h4
        {
        font-family: Arial, Helvetica, 'Bitstream Vera Sans', 'Luxi Sans', Verdana, Sans-Serif;
        font-size: 16px;
        }

  </style>
  </head>
  <body>
  <xsl:apply-templates/>
  </body>
  </html>
</xsl:template>

<xsl:template match="bookinfo">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="title">
  <h3>
  <xsl:apply-templates/>
  </h3>
</xsl:template>

<xsl:template match="author">
  <h4>
  <xsl:value-of select="./firstname"/>
  <xsl:text> </xsl:text>
  <xsl:value-of select="./surname"/>
  </h4>
</xsl:template>


<xsl:template match="chapter">
  <hr/>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="para">
  <p>
  <xsl:apply-templates/>
  </p>
</xsl:template>

</xsl:stylesheet>
