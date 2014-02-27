<?xml version='1.0'?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:doc="http://www.freedesktop.org/dbus/1.0/doc.dtd"
  exclude-result-prefixes="doc">
<!--
     Convert D-Bus Glib xml into DocBook refentries
     Copyright (C) 2007 William Jon McCann
     License: GPL
-->
<xsl:output method="xml" indent="yes" encoding="UTF-8"/>

<xsl:template match="/">

<xsl:variable name="interface" select="//interface/@name"/>
<xsl:variable name="basename">
  <xsl:call-template name="interface-basename">
    <xsl:with-param name="str" select="$interface"/>
  </xsl:call-template>
</xsl:variable>

<refentry><xsl:attribute name="id"><xsl:value-of select="$basename"/></xsl:attribute>
  <refmeta>
    <refentrytitle role="top_of_page"><xsl:value-of select="//interface/@name"/></refentrytitle>
  </refmeta>

  <refnamediv>
    <refname><xsl:value-of select="//interface/@name"/></refname>
    <refpurpose><xsl:value-of select="$basename"/> interface</refpurpose>
  </refnamediv>

  <refsynopsisdiv role="synopsis">
    <title role="synopsis.title">Methods</title>
    <synopsis>
  <xsl:call-template name="methods-synopsis">
    <xsl:with-param name="basename" select="$basename"/>
  </xsl:call-template>
    </synopsis>
  </refsynopsisdiv>

  <xsl:choose>
    <xsl:when test="count(///signal) > 0">
      <refsect1 role="signal_proto">
        <title role="signal_proto.title">Signals</title>
        <synopsis>
          <xsl:call-template name="signals-synopsis">
            <xsl:with-param name="basename" select="$basename"/>
          </xsl:call-template>
        </synopsis>
      </refsect1>
    </xsl:when>
  </xsl:choose>

  <refsect1 role="impl_interfaces">
    <title role="impl_interfaces.title">Implemented Interfaces</title>
    <para>
    Objects implementing <xsl:value-of select="$interface"/> also implements
    org.freedesktop.DBus.Introspectable,
    org.freedesktop.DBus.Properties
    </para>
  </refsect1>

  <xsl:choose>
    <xsl:when test="count(///property) > 0">
      <refsect1 role="properties">
        <title role="properties.title">Properties</title>
        <synopsis>
          <xsl:call-template name="properties-synopsis">
            <xsl:with-param name="basename" select="$basename"/>
          </xsl:call-template>
        </synopsis>
      </refsect1>
    </xsl:when>
  </xsl:choose>

  <refsect1 role="desc">
    <title role="desc.title">Description</title>
    <para>
      <xsl:apply-templates select="//interface/doc:doc"/>
    </para>
  </refsect1>

  <refsect1 role="details">
    <title role="details.title">Details</title>
    <xsl:call-template name="method-details">
      <xsl:with-param name="basename" select="$basename"/>
    </xsl:call-template>
  </refsect1>

  <xsl:choose>
    <xsl:when test="count(///signal) > 0">
      <refsect1 role="signals">
        <title role="signals.title">Signal Details</title>
        <xsl:call-template name="signal-details">
          <xsl:with-param name="basename" select="$basename"/>
        </xsl:call-template>
      </refsect1>
    </xsl:when>
  </xsl:choose>

  <xsl:choose>
    <xsl:when test="count(///property) > 0">
      <refsect1 role="property_details">
        <title role="property_details.title">Property Details</title>
        <xsl:call-template name="property-details">
          <xsl:with-param name="basename" select="$basename"/>
        </xsl:call-template>
      </refsect1>
    </xsl:when>
  </xsl:choose>

</refentry>
</xsl:template>


<xsl:template name="property-doc">
  <xsl:apply-templates select="doc:doc/doc:description"/>

  <variablelist role="params">
    <xsl:for-each select="arg">
<varlistentry><term><parameter><xsl:value-of select="@name"/></parameter>:</term>
<listitem><simpara><xsl:value-of select="doc:doc/doc:summary"/></simpara></listitem>
</varlistentry>
    </xsl:for-each>
  </variablelist>

  <xsl:apply-templates select="doc:doc/doc:since"/>
  <xsl:apply-templates select="doc:doc/doc:deprecated"/>
  <xsl:apply-templates select="doc:doc/doc:permission"/>
  <xsl:apply-templates select="doc:doc/doc:seealso"/>
</xsl:template>


<xsl:template name="property-details">
  <xsl:param name="basename"/>
  <xsl:variable name="longest">
    <xsl:call-template name="find-longest">
      <xsl:with-param name="set" select="@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="///property">
  <refsect2>
    <title><anchor role="function"><xsl:attribute name="id"><xsl:value-of select="$basename"/>:<xsl:value-of select="@name"/></xsl:attribute></anchor>The "<xsl:value-of select="@name"/>" property</title>
<indexterm><primary><xsl:value-of select="@name"/></primary><secondary><xsl:value-of select="$basename"/></secondary></indexterm>
<programlisting>'<xsl:value-of select="@name"/>'<xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="2"/></xsl:call-template>
<xsl:call-template name="property-args"><xsl:with-param name="indent" select="string-length(@name) + 2"/></xsl:call-template></programlisting>
  </refsect2>

  <xsl:call-template name="property-doc"/>

  </xsl:for-each>
</xsl:template>

<xsl:template name="signal-doc">
  <xsl:apply-templates select="doc:doc/doc:description"/>

  <variablelist role="params">
    <xsl:for-each select="arg">
<varlistentry><term><parameter><xsl:value-of select="@name"/></parameter>:</term>
<listitem><simpara><xsl:value-of select="doc:doc/doc:summary"/></simpara></listitem>
</varlistentry>
    </xsl:for-each>
  </variablelist>

  <xsl:apply-templates select="doc:doc/doc:since"/>
  <xsl:apply-templates select="doc:doc/doc:deprecated"/>
  <xsl:apply-templates select="doc:doc/doc:permission"/>
  <xsl:apply-templates select="doc:doc/doc:seealso"/>
</xsl:template>

<xsl:template name="signal-details">
  <xsl:param name="basename"/>
  <xsl:variable name="longest">
    <xsl:call-template name="find-longest">
      <xsl:with-param name="set" select="@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="///signal">
  <refsect2>
    <title><anchor role="function"><xsl:attribute name="id"><xsl:value-of select="$basename"/>::<xsl:value-of select="@name"/></xsl:attribute></anchor>The <xsl:value-of select="@name"/> signal</title>
<indexterm><primary><xsl:value-of select="@name"/></primary><secondary><xsl:value-of select="$basename"/></secondary></indexterm>
<programlisting><xsl:value-of select="@name"/> (<xsl:call-template name="signal-args"><xsl:with-param name="indent" select="string-length(@name) + 2"/><xsl:with-param name="prefix" select="."/></xsl:call-template>)</programlisting>
  </refsect2>

  <xsl:call-template name="signal-doc"/>

  </xsl:for-each>
</xsl:template>

<xsl:template match="doc:code">
<programlisting>
<xsl:apply-templates />
</programlisting>
</xsl:template>

<xsl:template match="doc:tt">
  <literal>
    <xsl:apply-templates />
  </literal>
</xsl:template>

<xsl:template match="doc:i">
  <emphasis>
    <xsl:apply-templates />
  </emphasis>
</xsl:template>

<xsl:template match="doc:b">
  <emphasis role="bold">
    <xsl:apply-templates />
  </emphasis>
</xsl:template>

<xsl:template match="doc:ulink">
  <ulink>
    <xsl:attribute name="url"><xsl:value-of select="@url"/></xsl:attribute>
    <xsl:value-of select="."/>
  </ulink>
</xsl:template>

<xsl:template match="doc:summary">
  <xsl:apply-templates />
</xsl:template>

<xsl:template match="doc:example">
<informalexample>
<xsl:apply-templates />
</informalexample>
</xsl:template>

<xsl:template name="listitems-do-term">
  <xsl:param name="str"/>
  <xsl:choose>
    <xsl:when test="string-length($str) > 0">
      <emphasis role="bold"><xsl:value-of select="$str"/>: </emphasis>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template name="do-listitems">
  <xsl:for-each select="doc:item">
    <listitem>
      <xsl:call-template name="listitems-do-term"><xsl:with-param name="str" select="doc:term"/></xsl:call-template>
      <xsl:apply-templates select="doc:definition"/>
    </listitem>
  </xsl:for-each>
</xsl:template>

<xsl:template match="doc:list">
  <para>
    <xsl:choose>
      <xsl:when test="contains(@type,'number')">
        <orderedlist>
          <xsl:call-template name="do-listitems"/>
        </orderedlist>
      </xsl:when>
      <xsl:otherwise>
        <itemizedlist>
          <xsl:call-template name="do-listitems"/>
        </itemizedlist>
      </xsl:otherwise>
    </xsl:choose>
  </para>
</xsl:template>

<xsl:template match="doc:para">
<para>
<xsl:apply-templates />
</para>
</xsl:template>

<xsl:template match="doc:description">
<xsl:apply-templates />
</xsl:template>

<xsl:template match="doc:since">
<para role="since">Since <xsl:value-of select="@version"/>
</para>
</xsl:template>

<xsl:template match="doc:deprecated">
  <xsl:variable name="name" select="../../@name"/>
  <xsl:variable name="parent">
    <xsl:call-template name="interface-basename">
      <xsl:with-param name="str" select="../../../@name"/>/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="type" select="name(../..)"/>

  <para role="deprecated">
  <warning><para><literal><xsl:value-of select="$name"/></literal> is deprecated since version <xsl:value-of select="@version"/> and should not be used in newly-written code. Use

  <xsl:variable name="to">
  <xsl:choose>
    <xsl:when test="contains($type,'property')">
      <xsl:value-of select="$parent"/>:<xsl:value-of select="@instead"/>
    </xsl:when>
    <xsl:when test="contains($type,'signal')">
      <xsl:value-of select="$parent"/>::<xsl:value-of select="@instead"/>
    </xsl:when>
    <xsl:when test="contains($type,'method')">
      <xsl:value-of select="$parent"/>.<xsl:value-of select="@instead"/>
    </xsl:when>
    <xsl:when test="contains($type,'interface')">
      <xsl:value-of select="@instead"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="@instead"/>
    </xsl:otherwise>
  </xsl:choose>
  </xsl:variable>

  <xsl:call-template name="create-link">
    <xsl:with-param name="type" select="$type"/>
    <xsl:with-param name="to" select="$to"/>
    <xsl:with-param name="val" select="@instead"/>
  </xsl:call-template>
instead.</para></warning>
</para>
</xsl:template>

<xsl:template match="doc:permission">
<para role="permission">
<xsl:apply-templates />
</para>
</xsl:template>

<xsl:template match="doc:errors">
<para role="errors">
<xsl:apply-templates />
</para>
</xsl:template>

<xsl:template match="doc:seealso">
<para>
See also:
<xsl:apply-templates />

</para>
</xsl:template>

<xsl:template name="create-link">
  <xsl:param name="type"/>
  <xsl:param name="to"/>
  <xsl:param name="val"/>

  <xsl:choose>
    <xsl:when test="contains($type,'property')">
      <link><xsl:attribute name="linkend"><xsl:value-of select="$to"/></xsl:attribute><literal><xsl:value-of select="$val"/></literal></link>
    </xsl:when>
    <xsl:when test="contains($type,'signal')">
      <link><xsl:attribute name="linkend"><xsl:value-of select="$to"/></xsl:attribute><literal><xsl:value-of select="$val"/></literal></link>
    </xsl:when>
    <xsl:when test="contains($type,'method')">
      <link><xsl:attribute name="linkend"><xsl:value-of select="$to"/></xsl:attribute><function><xsl:value-of select="$val"/></function></link>
    </xsl:when>
    <xsl:when test="contains($type,'interface')">
      <link><xsl:attribute name="linkend"><xsl:value-of select="$to"/></xsl:attribute><xsl:value-of select="$val"/></link>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template match="doc:ref">
  <xsl:call-template name="create-link">
    <xsl:with-param name="type" select="@type"/>
    <xsl:with-param name="to" select="@to"/>
    <xsl:with-param name="val" select="."/>
  </xsl:call-template>
</xsl:template>

<xsl:template name="method-doc">
  <xsl:apply-templates select="doc:doc/doc:description"/>

  <variablelist role="params">
    <xsl:for-each select="arg">
<varlistentry><term><parameter><xsl:value-of select="@name"/></parameter>:</term>
<listitem><simpara><xsl:apply-templates select="doc:doc/doc:summary"/></simpara></listitem>
</varlistentry>
    </xsl:for-each>
  </variablelist>

  <xsl:apply-templates select="doc:doc/doc:since"/>
  <xsl:apply-templates select="doc:doc/doc:deprecated"/>

  <xsl:choose>
    <xsl:when test="count(doc:doc/doc:errors) > 0">
      <refsect3>
        <title>Errors</title>
        <variablelist role="errors">
          <xsl:for-each select="doc:doc/doc:errors/doc:error">
            <varlistentry>
              <term><parameter><xsl:value-of select="@name"/></parameter>:</term>
              <listitem><simpara><xsl:apply-templates select="."/></simpara></listitem>
            </varlistentry>
          </xsl:for-each>
        </variablelist>
      </refsect3>
    </xsl:when>
  </xsl:choose>

  <xsl:choose>
    <xsl:when test="count(doc:doc/doc:permission) > 0">
      <refsect3>
        <title>Permissions</title>
        <xsl:apply-templates select="doc:doc/doc:permission"/>
      </refsect3>
    </xsl:when>
  </xsl:choose>

  <xsl:apply-templates select="doc:doc/doc:seealso"/>
</xsl:template>

<xsl:template name="method-details">
  <xsl:param name="basename"/>
  <xsl:variable name="longest">
    <xsl:call-template name="find-longest">
      <xsl:with-param name="set" select="@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="///method">
    <refsect2>
    <title><anchor role="function"><xsl:attribute name="id"><xsl:value-of select="$basename"/>.<xsl:value-of select="@name"/></xsl:attribute></anchor><xsl:value-of select="@name"/> ()</title>
<indexterm><primary><xsl:value-of select="@name"/></primary><secondary><xsl:value-of select="$basename"/></secondary></indexterm>
<programlisting><xsl:value-of select="@name"/> (<xsl:call-template name="method-args"><xsl:with-param name="indent" select="string-length(@name) + 2"/><xsl:with-param name="prefix" select="."/></xsl:call-template>)</programlisting>
    </refsect2>

    <xsl:call-template name="method-doc"/>

  </xsl:for-each>
</xsl:template>


<xsl:template name="properties-synopsis">
  <xsl:param name="basename"/>
  <xsl:variable name="longest">
    <xsl:call-template name="find-longest">
      <xsl:with-param name="set" select="///property/@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="///property">
<link><xsl:attribute name="linkend"><xsl:value-of select="$basename"/>:<xsl:value-of select="@name"/></xsl:attribute>'<xsl:value-of select="@name"/>'</link><xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$longest - string-length(@name) + 1"/></xsl:call-template> <xsl:call-template name="property-args"><xsl:with-param name="indent" select="$longest + 2"/></xsl:call-template>
</xsl:for-each>
</xsl:template>


<xsl:template name="signals-synopsis">
  <xsl:param name="basename"/>
  <xsl:variable name="longest">
    <xsl:call-template name="find-longest">
      <xsl:with-param name="set" select="///signal/@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="///signal">
<link><xsl:attribute name="linkend"><xsl:value-of select="$basename"/>::<xsl:value-of select="@name"/></xsl:attribute><xsl:value-of select="@name"/></link><xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$longest - string-length(@name) + 1"/></xsl:call-template>(<xsl:call-template name="signal-args"><xsl:with-param name="indent" select="$longest + 2"/><xsl:with-param name="prefix" select="///signal"/></xsl:call-template>)
</xsl:for-each>
</xsl:template>


<xsl:template name="methods-synopsis">
  <xsl:param name="basename"/>
  <xsl:variable name="longest">
    <xsl:call-template name="find-longest">
      <xsl:with-param name="set" select="///method/@name"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:for-each select="///method">
<link><xsl:attribute name="linkend"><xsl:value-of select="$basename"/>.<xsl:value-of select="@name"/></xsl:attribute><xsl:value-of select="@name"/></link><xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$longest - string-length(@name) + 1"/></xsl:call-template>(<xsl:call-template name="method-args"><xsl:with-param name="indent" select="$longest + 2"/><xsl:with-param name="prefix" select="///method"/></xsl:call-template>)
</xsl:for-each>
</xsl:template>


<xsl:template name="method-args"><xsl:param name="indent"/><xsl:param name="prefix"/><xsl:variable name="longest"><xsl:call-template name="find-longest"><xsl:with-param name="set" select="$prefix/arg/@type"/></xsl:call-template></xsl:variable><xsl:for-each select="arg"><xsl:value-of select="@direction"/>
<xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="4 - string-length(@direction)"/></xsl:call-template>'<xsl:value-of select="@type"/>'<xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$longest - string-length(@type) + 1"/></xsl:call-template>
<xsl:value-of select="@name"/><xsl:if test="not(position() = last())">,
<xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$indent"/></xsl:call-template></xsl:if>
</xsl:for-each>
</xsl:template>


<xsl:template name="signal-args"><xsl:param name="indent"/><xsl:param name="prefix"/><xsl:variable name="longest"><xsl:call-template name="find-longest"><xsl:with-param name="set" select="$prefix/arg/@type"/></xsl:call-template></xsl:variable><xsl:for-each select="arg">'<xsl:value-of select="@type"/>'<xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$longest - string-length(@type) + 1"/></xsl:call-template>
<xsl:value-of select="@name"/><xsl:if test="not(position() = last())">,
<xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="$indent"/></xsl:call-template></xsl:if>
</xsl:for-each>
</xsl:template>


<xsl:template name="property-args"><xsl:param name="indent"/>
<xsl:value-of select="@access"/><xsl:call-template name="pad-spaces"><xsl:with-param name="width" select="9 - string-length(@access) + 1"/></xsl:call-template>'<xsl:value-of select="@type"/>'
</xsl:template>


<xsl:template name="pad-spaces">
  <xsl:param name="width"/>
  <xsl:variable name="spaces" select="'                                                                        '" ></xsl:variable>
  <xsl:value-of select="substring($spaces,1,$width)"/>
</xsl:template>


<xsl:template name="find-longest">
  <xsl:param name="set"/>
  <xsl:param name="index" select="1"/>
  <xsl:param name="longest" select="0"/>

  <xsl:choose>
    <xsl:when test="$index > count($set)">
      <!--finished looking-->
      <xsl:value-of select="$longest"/>
    </xsl:when>
    <xsl:when test="string-length($set[$index])>$longest">
      <!--found new longest-->
      <xsl:call-template name="find-longest">
        <xsl:with-param name="set" select="$set"/>
        <xsl:with-param name="index" select="$index + 1"/>
        <xsl:with-param name="longest" select="string-length($set[$index])"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <!--this isn't any longer-->
      <xsl:call-template name="find-longest">
        <xsl:with-param name="set" select="$set"/>
        <xsl:with-param name="index" select="$index + 1"/>
        <xsl:with-param name="longest" select="$longest"/>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="interface-basename">
  <xsl:param name="str"/>
  <xsl:choose>
    <xsl:when test="contains($str,'.')">
      <xsl:call-template name="interface-basename">
	<xsl:with-param name="str" select="substring-after($str,'.')"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$str"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>

