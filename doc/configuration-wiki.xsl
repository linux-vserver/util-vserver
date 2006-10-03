<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" version="2.0">
	<xsl:output method="html" indent="yes" omit-xml-declaration="yes" encoding="ISO-8859-1"/>
	<xsl:param name="confdir" select="'/etc/vservers'"/>
	<!-- set following parameter to anything else than 'true' to use a symbolic character to mark up file types -->
	<xsl:param name="printstylename" select="'true'"/>

	<xsl:template match="/database">
		<h1>The contents of the <xsl:value-of select="$confdir"/> directory</h1>
		This page is automatically created by XSL transformation of configuration.xml. Additions, corrections, etc. should be made in that file, not in the Wiki page.
		<xsl:choose>
			<xsl:when test="$printstylename = 'true'">
				Directory names are written in <b>bold</b> letters.
				
				Boolean files are files without content. Only existence matters.
				
				Scalar files are files with only one line (one value).
				
				List files are files with multiple values. One value per line.
				
				Hash files are files with multiple key/value pairs. One key/value pair per line.

				Symbolic names (variables) are written in <i>italic</i> letters.
			</xsl:when>
			<xsl:otherwise>
				Directories are marked with a trailing / and <b>bold</b> letters.
		
				Scripts are marked with a trailing superscript <sup>*</sup>.
		
				Hash files (with key/value pairs) are marked with a trailing superscript <sup>#</sup>.
		
				Boolean files (without content) are marked with a trailing superscript <sup>0</sup>.
		
				Scalar files (with only one line) are marked with a trailing superscript <sup>1</sup>.
		
				List files (with multiple lines) are marked with a trailing superscript <sup>min - max</sup> sequence, where min is the minimum and max the maximum number of lines.
		
				Symlinks are marked with a trailing superscript <sup>@</sup>.
		
				Data files have no special mark up.
		
				Symbolic names (variables) are written in <i>italic</i> letters.
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:call-template name="collection">
			<xsl:with-param name="thisdir"><xsl:value-of select="$confdir"/></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

  <xsl:template name="collection">
    <xsl:param name="thisdir"/>
    <xsl:if test="count(scalar) + count(link) + count(program) + count(data) + count(hash) + count(list) + count(boolean) + count(collection)>0">
      <ul>
        <xsl:if test="@id"><span><xsl:attribute name="id"><xsl:value-of select="@id" /></xsl:attribute></span></xsl:if>
        <xsl:call-template name="dir-iterate">
          <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        </xsl:call-template>
      </ul>
    </xsl:if>
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
      <xsl:call-template name="printname">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/></xsl:with-param>
        <xsl:with-param name="style">directory</xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="printcontent"/>

      <xsl:call-template name="collection">
        <xsl:with-param name="thisdir"><xsl:value-of select="$thisdir"/>/<xsl:call-template name="printdirname"/></xsl:with-param>
      </xsl:call-template>
    </li>
  </xsl:template>

  <xsl:template match="collection" mode="printrpath">
    <xsl:text>/</xsl:text>
      <span class="{@type}">
      <xsl:value-of select="@name"/>
    </span>
  </xsl:template>

	<xsl:template name="printname">
		<xsl:param name="thisdir"/>
		<xsl:param name="style"/>
		<xsl:if test="@id"><span><xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute></span></xsl:if>
		<xsl:if test="$style = 'directory'">'''</xsl:if>
		<xsl:value-of select="$thisdir"/>/<xsl:if test="@type = 'symbolic'">''</xsl:if><xsl:value-of select="@name"/><xsl:if test="@type = 'symbolic'">''</xsl:if>
		<xsl:choose>
			<xsl:when test="$printstylename = 'true'">
				<xsl:if test="$style = 'directory'">'''</xsl:if>
				<xsl:value-of select="concat(' [',$style,']')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="$style = 'directory'">/'''</xsl:when>
					<xsl:when test="$style = 'symlink'"><sup>@</sup></xsl:when>
					<xsl:when test="$style = 'boolean'"><sup>0</sup></xsl:when>
					<xsl:when test="$style = 'list'"><sup><xsl:choose>
						<xsl:when test="minElements"><xsl:value-of select="minElements"/></xsl:when>
						<xsl:otherwise>0</xsl:otherwise>
					</xsl:choose><xsl:value-of select="0 + minElements"/>-<xsl:choose>
						<xsl:when test="maxElements"><xsl:value-of select="maxElements"/></xsl:when>
						<xsl:otherwise>&#8734;</xsl:otherwise>
					</xsl:choose></sup></xsl:when>
					<xsl:when test="$style = 'file'"><sup>1</sup></xsl:when>
					<xsl:when test="$style = 'script'"><sup>*</sup></xsl:when>
					<xsl:when test="$style = 'hash'"><sup>#</sup></xsl:when>
					<xsl:when test="$style = 'data'"></xsl:when>
					<xsl:otherwise><sup>???</sup></xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name="printcontent">
		<br/>
		<xsl:apply-templates select="description"/>
		<xsl:call-template name="since"/>
		<xsl:apply-templates select="default"/>
		<xsl:call-template name="default"/>
		<xsl:if test="$printstylename = 'true'">
			<xsl:apply-templates select="minElements"/>
			<xsl:apply-templates select="maxElements"/>
		</xsl:if>
		<xsl:apply-templates select="restriction"/>
		<xsl:apply-templates select="elements"/>
		<xsl:apply-templates select="parameterList"/>
		<xsl:apply-templates select="keys"/>
	</xsl:template>
	
	<!-- Extra content for scalar nodes -->
	
	<xsl:template match="restriction">
		<ul>
			<ul>
				Allowed values:
				<xsl:apply-templates select="enumeration">
					<xsl:sort select="@value"/>
				</xsl:apply-templates>
			</ul>
		</ul>
	</xsl:template>
	
	<xsl:template match="restriction/enumeration">
		<xsl:value-of select="concat('&#13;; ',@value)"/>
	</xsl:template>

	<!-- Extra content for hash nodes -->
	
	<xsl:template match="keys">
		<ul>
			<ul>
				<xsl:apply-templates select="key">
					<xsl:sort select="@name"/>
				</xsl:apply-templates>
			</ul>
		</ul>
	</xsl:template>
	
	<xsl:template match="keys/key">
		<xsl:value-of select="concat('&#13;; ',@name,' : ',replace(description,'[\r\n]+',' '))"/>
	</xsl:template>

	<!-- Extra content for list nodes -->
	<xsl:template match="minElements">
		<ul>
			<ul>
				<xsl:value-of select="concat('&#13;; minimum number of Elements : ',replace('[\r\n]+',' '))"/>
			</ul>
		</ul>
	</xsl:template>
	
	<xsl:template match="maxElements">
		<ul>
			<ul>
				<xsl:value-of select="concat('&#13;; maximum number of Elements : ',replace('[\r\n]+',' '))"/>
			</ul>
		</ul>
	</xsl:template>

	<xsl:template match="elements">
		<ul>
			<ul>
				Possible values:
				<xsl:apply-templates select="element">
					<xsl:sort select="@name"/>
				</xsl:apply-templates>
			</ul>
		</ul>
	</xsl:template>
	
	<xsl:template match="elements/element">
		<xsl:value-of select="concat('&#13;; ',@name,' : ',replace(description,'[\r\n]+',' '))"/>
	</xsl:template>
	
	<!-- Extra content for program nodes -->
	
	<xsl:template match="parameterList">
		<ul>
			<ul>
				Will be called with the following parameters:
				<xsl:apply-templates select="parameter"/>
			</ul>
		</ul>
	</xsl:template>
	
	<xsl:template match="parameterList/parameter">
		<xsl:value-of select="concat('&#13;; ',@name,' : ',replace(description,'[\r\n]+',' '))"/>
	</xsl:template>
	
	<!-- Extra content for all nodes -->
	
	<xsl:template match="default">
		<ul><ul><xsl:value-of select="concat('&#13;; Default : ',replace(.,'[\r\n]+',' '))"/></ul></ul>
	</xsl:template>

	<xsl:template name="default"><xsl:if test="@default">
		<ul><ul>&#13;; Default : <xsl:value-of select="@default"/></ul></ul>
	</xsl:if></xsl:template>"
  
	<xsl:template name="since"><xsl:if test="@since">
		<ul><ul>&#13;; Since Version : <xsl:value-of select="@since"/></ul></ul>
	</xsl:if></xsl:template>
	
	<!-- -->
	
	<xsl:template name="printdirname"><xsl:if test="@type='symbolic'">''</xsl:if><xsl:value-of select="@name"/><xsl:if test="@type='symbolic'">''</xsl:if></xsl:template>
	<xsl:template match="description"><xsl:apply-templates/></xsl:template>
	<xsl:template match="ulink">[<xsl:value-of select="@url"/><xsl:value-of select="concat(' ',replace(.,'[\r\n]+',' '))"/>]</xsl:template>
	<xsl:template match="br"><br /></xsl:template>
	<xsl:template match="p"><div><xsl:apply-templates/></div></xsl:template>
	<xsl:template match="tool"><code><xsl:apply-templates/></code></xsl:template>
	<xsl:template match="command"><code><xsl:apply-templates/></code></xsl:template>
	<xsl:template match="directory"><xsl:apply-templates/></xsl:template>
	<xsl:template match="optionref">[[#<xsl:choose>
		<xsl:when test="@ref"><xsl:value-of select="@ref"/></xsl:when>
		<xsl:otherwise><xsl:apply-templates/></xsl:otherwise>
	</xsl:choose>|<xsl:apply-templates/>]]</xsl:template>
	
	<xsl:template match="filename">[file://<xsl:value-of select="text()"/>]</xsl:template>
  
</xsl:stylesheet>
