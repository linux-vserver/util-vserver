<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml"
                version="1.0">

  <xsl:output method="xml"
              indent="yes"
              doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>

  <xsl:param name="confdir"/>
  
  <xsl:template match="/">
    <html>
      <xsl:apply-templates/>
    </html>
  </xsl:template>

  <xsl:template match="database">
    <head>
      <title>The <xsl:value-of select="$confdir"/> directory</title>
      <link rel="stylesheet" title="LSD"    type="text/css" href="configuration-lsd.css"></link>
      <link rel="stylesheet" title="boring" type="text/css" href="configuration.css"></link>
    </head>
    <body>
      <h1>The content of the <xsl:value-of select="$confdir"/> directory</h1>

      <ul>
        <xsl:call-template name="collection">
          <xsl:with-param name="thisdir"><xsl:value-of select="$confdir"/></xsl:with-param>
        </xsl:call-template>
      </ul>
    </body>
  </xsl:template>

  <xsl:template name="collection">
    <xsl:param name="thisdir"/>
    <xsl:choose>
      <xsl:when test="count(scalar) + count(link) + count(program) + count(data) + count(hash) + count(list) + count(boolean) > 0">
        <ul>
          <xsl:call-template name="dir-iterate">
            <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
          </xsl:call-template>
        </ul>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="dir-iterate">
          <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="dir-iterate">
    <xsl:param name="thisdir"/>
    <xsl:apply-templates select="scalar|link|program|data|hash|list|boolean">
      <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    
    <xsl:apply-templates select="collection">
      <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="program">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">script</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>

  <xsl:template match="hash">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">hash</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>
  
  <xsl:template match="scalar">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">file</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>

  <xsl:template match="data">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">data</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>

  <xsl:template match="list">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">list</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>
  
  <xsl:template match="link">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">symlink</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>

  <xsl:template match="boolean">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">boolean</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>
    </li>
  </xsl:template>
  
  <xsl:template match="collection">
    <xsl:param name="thisdir"/>
    <li>
      <xsl:call-template name="printfullname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">directory</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>

      <xsl:call-template name="collection">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/>/<xsl:value-of select="@name"/></xsl:with-param>
      </xsl:call-template>
    </li>
  </xsl:template>

  <xsl:template match="collection" mode="printrpath">
    <xsl:text>/</xsl:text>
      <span class="{@type}">
      <xsl:value-of select="@name"/>
    </span>
  </xsl:template>
  
  <xsl:template name="printfullname">
    <xsl:param name="thisdir"/>
    <xsl:param name="style"/>
    <span class="{$style}">
      <xsl:value-of select="$confdir"/>
      <xsl:apply-templates select="ancestor-or-self::collection" mode="printrpath"/>
    </span>
  </xsl:template>

  <xsl:template name="printname">
    <xsl:param name="thisdir"/>
    <xsl:param name="style"/>
    <span class="{$style}" title="{$thisdir}/{@name}">
      <xsl:value-of select="@name"/>
    </span>
  </xsl:template>

  <xsl:template name="printcontent">
    <br/>
    <span class="description">
      <xsl:apply-templates select="description"/>
    </span>
  </xsl:template>

  <xsl:template match="ulink">
    <a href="{@url}"><xsl:apply-templates/></a>
  </xsl:template>

</xsl:stylesheet>
