<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- $Id$ -->
<xsl:stylesheet version="1.0"
     xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="xml"/>

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="ProjectName">
    <xsl:copy><xsl:value-of select="$projname"/></xsl:copy>
  </xsl:template>

  <xsl:template match="SOURCEFILE">
    <xsl:copy><xsl:value-of select="$src"/></xsl:copy>
  </xsl:template>

  <xsl:template match="OUTPUTFILENAME">
    <xsl:copy><xsl:value-of select="$projname"/>.elf</xsl:copy>
  </xsl:template>

  <xsl:template match="ProjectFiles/Files/Name">
    <xsl:copy><xsl:value-of select="$src"/></xsl:copy>
  </xsl:template>

  <xsl:template match="LIB">
    <xsl:copy><xsl:value-of select="."/><xsl:value-of select="$brd"/></xsl:copy>
  </xsl:template>

  <xsl:template match="OPTIONSFORALL">
    <xsl:copy><xsl:value-of select="$ccflags"/></xsl:copy>
  </xsl:template>

  <xsl:template match="PART">
    <xsl:copy><xsl:value-of select="$part"/></xsl:copy>
  </xsl:template>

  <xsl:template match="Files/File00000/FileName">
    <xsl:copy><xsl:value-of select="$src"/></xsl:copy>
  </xsl:template>

  <xsl:template match="EXTERNALMAKEFILE">
    <xsl:copy><xsl:value-of select="$projname"/>.mk</xsl:copy>
  </xsl:template>


  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
