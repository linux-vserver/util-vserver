<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml"
                version="1.0">

  <xsl:output method="xml"
              indent="yes"
              doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>

  <xsl:template match="/">
    <html>
      <head>
        <title><xsl:apply-templates select="compatibility/title"/></title>
        <link rel="stylesheet" type="text/css" href="compatibility.css" />
      </head>
      <body>
	<h1><xsl:apply-templates select="compatibility/title"/></h1>
        <xsl:apply-templates select="compatibility/program">
          <xsl:sort select="compatibility/program/location"/>
          <xsl:sort select="compatibility/program[name]"/>
        </xsl:apply-templates>
      </body>
    </html>
  </xsl:template>

  <xsl:template name="compat">
    <xsl:variable name="name" select="name()"/>
    <xsl:variable name="descr">
      <xsl:choose>
	<xsl:when test="$name = 'clicompat'">
	  <xsl:text>Command line interface compatibility</xsl:text>
	</xsl:when>
	<xsl:when test="$name = 'funccompat'">
	  <xsl:text>Functional compatibility</xsl:text>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:value-of select="$name"/>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <div class="{$name}">
      <xsl:value-of select="$descr"/>: <xsl:value-of select="@status"/>
      <xsl:if test="text">
	<div class="text">
	  <xsl:apply-templates select="text"/>
	</div>
      </xsl:if>      
    </div>
  </xsl:template>

  <xsl:template match="clicompat|funccompat">
    <xsl:call-template name="compat" />
  </xsl:template>

  <xsl:template match="program">
    <div class="program">
      <div class="name">
        <xsl:value-of select="location"/>/<xsl:value-of select="@name"/>
        <xsl:if test="@status">
          <xsl:text> (</xsl:text>
	  <xsl:value-of select="@status"/>
          <xsl:text>)</xsl:text>
        </xsl:if>
      </div>
      <xsl:apply-templates select="clicompat|funccompat"/>
    </div>
  </xsl:template>

</xsl:stylesheet>
