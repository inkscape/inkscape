<?xml version="1.0"?>
<xsl:stylesheet  xml:space="default" version="2.0" 
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns="http://www.w3.org/2000/svg"
xmlns:svg="http://www.w3.org/2000/svg"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:x="http://ns.adobe.com/Extensibility/1.0/" 
xmlns:i="http://ns.adobe.com/AdobeIllustrator/10.0/" 
xmlns:graph="http://ns.adobe.com/Graphs/1.0/" 
xmlns:a="http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/" >
<xsl:output indent="yes"/>
	<xsl:template match="/">
			<xsl:apply-templates />
	</xsl:template>
	<xsl:template match="*">
		<xsl:copy>
		<xsl:apply-templates select="@*|node()|text"/>
		</xsl:copy>
	</xsl:template>
	<xsl:template match="@*">
		<xsl:copy/>
	</xsl:template>
	<xsl:template match="i:*|@i:*|x:*|@x:*|graph:*|@graph:*|a:*|@a:*">
	</xsl:template>
</xsl:stylesheet>