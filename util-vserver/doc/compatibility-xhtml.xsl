<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/TR/xhtml1/strict"
                version="1.0">

  <xsl:output method="xml"
              indent="yes"
              doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>

  <xsl:template match="/">
    <html>
      <head>
        <title><xsl:apply-templates select="compatibility/title"/></title>
      </head>
      <body>
        <xsl:apply-templates select="compatibility/program">
          <xsl:sort select="compatibility/program/location"/>
          <xsl:sort select="compatibility/program[name]"/>
        </xsl:apply-templates>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="program">
    <div class="program">
      <h2>
        <xsl:if test="@status">
          <xsl:attribute name="class"><xsl:value-of select="@status"/></xsl:attribute>
        </xsl:if>
        <xsl:value-of select="location"/>/<xsl:value-of select="@name"/>
      </h2>
    </div>
  </xsl:template>

</xsl:stylesheet>
