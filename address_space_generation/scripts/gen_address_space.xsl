<?xml version="1.0" encoding="UTF-8"?>
<!--
Copyright (C) 2018 Systerel and others.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="2.0" xmlns:ua="http://opcfoundation.org/UA/2011/03/UANodeSet.xsd"  xmlns:uax="http://opcfoundation.org/UA/2008/02/Types.xsd"  xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:sys="http://www.systerel.fr" >
    <xsl:output method="text"  encoding="UTF-8" />

<!--

Nodes order
===========
Each node is associated to an integer corresponding to a B type.
However, in order to optimize memory management, nodes are numbered, grouped by and sorted according to their corresponding classes (see 'classes' list below).
Eventually, nodes are copied into a variable: 'ua_nodes'.
Templates used to generate data are applied on this variable in 'copy' mode.

Alias
======
The reference type of a reference can be an alias.
For exemple, the following alias is defined:
     <Alias Alias="HasComponent">i=47</Alias>

And a reference can be defined thanks to this alias for example:
     <Reference IsForward="false" ReferenceType="HasComponent">ns=261;s=Objec ...

When nodes are copied, templates allow to replace alias by their values.

NodeId
=======
For each nodeId, a variable named 'nodeid_{indice}' is created.
The arrays then contain pointers on these variables.
Connection between nodeIds and variables names is encoded with nodes as follows:
    <n id="string corresponding to the node id" vn="nodeid_{position()}"/>
These nodes are contained into the variable nodeid_var_names.

Arrays indexes:
==============
Values of arrays at index 0 is non significant. Significant values start from index 1.

Fonction
========
In order to encode a function, an array of values is created. Values are sorted according to node order.
Eitheir the value is a direct access, or otherwise, the value is computed for example
in case of the function NodeId where the value is a pointer on the corresponding variable.

Relation
========
Relations are encoded with three arrays:
1/ An array containing the liste of elements values.

For example for the relation:
    - {1|-> 2, 1|->3, 3|->2}

The array is:
    - {0, 2, 3, 2}

2/ Two arrays corresponding to the starting and ending indexes.
In the previous example we have:
    - deb = {0, 1, 1, 3}
    - fin = {0, 2, 0, 3}

Please note that for an element without an image, the ending index is stricly less that the starting index.

-->

<%
classes = ('View', 'Object', 'Variable', 'VariableType', 'ObjectType', 'ReferenceType', 'DataType', 'Method')
%>

<!-- variable containing alias -->
<xsl:variable name="alias" select="//ua:Alias"/>

<!-- variable containing a copy of nodes in order to sort them. -->
<xsl:variable name="ua_nodes">
    <xsl:apply-templates select="${ '|'.join(['*/ua:UA' + s for s in classes])}" mode="copy">
        <xsl:sort select="sys:ord_class(.)"/>
    </xsl:apply-templates>
</xsl:variable>

<!-- Table associating a string identifying a node and a variable name -->
<xsl:variable name="nodeid_var_name">
    <xsl:for-each select="distinct-values($ua_nodes//@NodeId|$ua_nodes//ua:Reference/text()|$ua_nodes//ua:Reference/@ReferenceType)">
        <n id="{.}" vn="nodeid_{position()}"/>
    </xsl:for-each>
</xsl:variable>

<!-- Variable containing variable and variabletype nodes -->
<xsl:variable name="var_vartype" select="$ua_nodes/ua:UAVariable|$ua_nodes/ua:UAVariableType"/>

<xsl:template match="/">/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see &lt;http://www.gnu.org/licenses/>.
 */

#include "sopc_address_space.h"

#include &lt;stdio.h>
#include &lt;stdbool.h>

#include "opcua_statuscodes.h"

#include "sopc_toolkit_constants.h"
#include "sopc_builtintypes.h"
#include "sopc_types.h"

% for i in range(1, 9):
#define NB_${i} <xsl:value-of select="count(//ua:UA${classes[i-1]})"/><xsl:text>    /* ${classes[i-1]} */</xsl:text>
% endfor

#define NB (${' + '.join( [ 'NB_%d' % i for i in range(1,9)])})

#define toSOPC_String(s) ((SOPC_Byte*)s)

#define DEFAULT_VARIANT  {true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue,{0}}

<!-- check if address space contains duplicated nodes ids -->
<xsl:for-each-group select="$ua_nodes//@NodeId" group-by=".">
  <xsl:if test="count(current-group()) ne 1">
    <xsl:message terminate="yes">NodeId value <xsl:value-of select="current-grouping-key()"/> is duplicated in address space</xsl:message>
  </xsl:if>
</xsl:for-each-group>

<!-- Create variables for each node id -->
<xsl:for-each select="$nodeid_var_name/*">
static SOPC_NodeId <xsl:value-of select="@vn"/> = <xsl:apply-templates select="@id" mode="nodeId"/>;<xsl:text/>
static SOPC_ExpandedNodeId ex_<xsl:value-of select="@vn"/> = {<xsl:apply-templates select="@id" mode="nodeId"/>, {0,0, NULL}, 0};<xsl:text/>
</xsl:for-each>

<!-- Avoid unused variable warning -->
void* avoid_unused_nodes_var[] = {<xsl:for-each select="$nodeid_var_name/*">&amp;<xsl:value-of select="@vn"/>,
&amp;ex_<xsl:value-of select="@vn"/>,</xsl:for-each>};

<!-- BrowseName -->
static SOPC_QualifiedName BrowseName[NB + 1] = {{0, {0, 0, NULL}}
<xsl:apply-templates select = "$ua_nodes/*/@BrowseName" mode="qName"/>
};

<!-- Description, DisplayName-->
% for s in ['Description', 'DisplayName']:
static SOPC_LocalizedText ${s}[] = {{{0, 0, NULL}, {0, 0, NULL}}
<xsl:apply-templates select = "$ua_nodes/*/ua:${s}" mode="localized_text"/>
};
static int ${s}_begin[] = {0, <xsl:value-of select = "for $n in $ua_nodes/* return count($n/preceding-sibling::*/ua:${s}) + 1" separator=", "/>};
static int ${s}_end[] = {-1, <xsl:value-of select = "for $n in $ua_nodes/* return count($n/preceding-sibling::*/ua:${s}) +  count($n/ua:${s})" separator=", "/>};
% endfor

<!-- Reference -->
static int reference_begin[] = {0, <xsl:value-of select = "for $n in $ua_nodes/* return count($n/preceding-sibling::*/ua:References/*) + 1" separator=", "/>};
static int reference_end[] = {-1,&#10;<xsl:value-of select = "for $n in $ua_nodes/* return concat(count($n/preceding-sibling::*/ua:References/*), '+',  count($n/ua:References/*), ' /* ', $n/@NodeId, ' */')" separator=",&#10;"/>};
static SOPC_NodeId* reference_type[] = {NULL,  <xsl:value-of select="for $n in $ua_nodes/*/ua:References/* return concat('&amp;', $nodeid_var_name/*[@id = $n/@ReferenceType]/@vn)" separator=", "/>};
static SOPC_ExpandedNodeId* reference_target[] = {NULL, <xsl:value-of select="for $n in $ua_nodes/*/ua:References/* return concat('&amp;ex_', $nodeid_var_name/*[@id = $n/text()]/@vn)" separator=", "/>};
static bool reference_isForward[]={false, <xsl:value-of select="for $n in $ua_nodes/*/ua:References/* return if ($n/@IsForward = 'false') then 'false' else 'true' " separator=", "/>};

<!-- NodeId -->
static SOPC_NodeId* NodeId[NB+1] = {NULL,&#10;<xsl:value-of select="for $n in $ua_nodes/* return concat('&amp;', $nodeid_var_name/*[@id = $n/@NodeId]/@vn)" separator=",&#10;"/>};


<!-- NodeClass -->
static OpcUa_NodeClass NodeClass[NB+1] = {OpcUa_NodeClass_Unspecified,
    <xsl:value-of select="for $n in $ua_nodes/* return sys:get_enum_value($n)" separator=", "/>
};

<!-- Value -->
SOPC_Variant Value[NB_3+NB_4+1] = {DEFAULT_VARIANT<xsl:apply-templates select="$var_vartype" mode="value"/>};

<!-- StatusCode -->
static SOPC_StatusCode status_code[] = {OpcUa_BadDataUnavailable, <xsl:value-of select = "for $n in $var_vartype return if ($n/ua:Value) then '0x00000000' else 'OpcUa_BadDataUnavailable'" separator=", "/>};

<!-- Access level -->
static SOPC_Byte AccessLevel[] = {0, <xsl:value-of select = "for $n in $ua_nodes/ua:UAVariable return if ($n/@AccessLevel) then $n/@AccessLevel else 1" separator=", "/>};

SOPC_AddressSpace addressSpace = {
    .nbVariables = NB_3,
    .nbVariableTypes = NB_4,
    .nbObjectTypes = NB_5,
    .nbReferenceTypes = NB_6,
    .nbDataTypes = NB_7,
    .nbMethods = NB_8,
    .nbObjects = NB_2,
    .nbViews = NB_1,
    .nbNodesTotal = NB,

    .browseNameArray = BrowseName,
    .descriptionIdxArray_begin = Description_begin,
    .descriptionIdxArray_end = Description_end,
    .descriptionArray = Description,
    .displayNameIdxArray_begin = DisplayName_begin,
    .displayNameIdxArray_end = DisplayName_end,
    .displayNameArray = DisplayName,
    .nodeClassArray = NodeClass,
    .nodeIdArray = NodeId,
    .referenceIdxArray_begin = reference_begin,
    .referenceIdxArray_end = reference_end,
    .referenceTypeArray = reference_type,
    .referenceTargetArray = reference_target,
    .referenceIsForwardArray = reference_isForward,
    .valueArray = Value,
    .valueStatusArray = status_code,
    .accessLevelArray = AccessLevel,
};
</xsl:template>

<!-- Create variant for each variable -->
<xsl:template match="ua:UAVariable[ua:Value]|ua:UAVariableType[ua:Value]" mode="value"><xsl:apply-templates select="ua:Value/*" mode="value"/></xsl:template>
<xsl:template match="ua:UAVariableType[not(ua:Value)]" mode="value">, DEFAULT_VARIANT
    <xsl:message>VariableType without value</xsl:message>
</xsl:template>

<xsl:template match="ua:UAVariable[not(ua:Value)]" mode="value">, DEFAULT_VARIANT</xsl:template>


% for s in ['Boolean', 'Byte', 'Int16', 'Int32', 'Int64', 'Guid', 'NodeId']:
<xsl:template match="uax:${s}" mode="value">,{true, SOPC_${s}_Id, SOPC_VariantArrayType_SingleValue, {.${s}=<xsl:value-of select="."/>}}</xsl:template>
% endfor

<xsl:template match="uax:SByte" mode="value">,{true, SOPC_SByte_Id, SOPC_VariantArrayType_SingleValue, {.Sbyte=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:UInt16" mode="value">,{true, SOPC_UInt16_Id, SOPC_VariantArrayType_SingleValue, {.Uint16=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:UInt32" mode="value">,{true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:UInt64" mode="value">,{true, SOPC_UInt64_Id, SOPC_VariantArrayType_SingleValue, {.Uint64=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:Float" mode="value">,{true, SOPC_Float_Id, SOPC_VariantArrayType_SingleValue, {.Floatv=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:Double" mode="value">,{true, SOPC_Double_Id, SOPC_VariantArrayType_SingleValue, {.Doublev=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:String" mode="value"><xsl:variable name="st" select="translate(.,'&quot;','')"/>,{true, SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {.String=${write_string("$st")}}}</xsl:template>
<xsl:template match="uax:ByteString" mode="value"><xsl:variable name="st" select="translate(.,'&quot;','')"/>,{true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {.Bstring=${write_string("$st")}}}</xsl:template>
<xsl:template match="uax:XmlElement" mode="value"><xsl:variable name="st" select="translate(.,'&quot;','')"/>,{true, SOPC_XmlElement_Id, SOPC_VariantArrayType_SingleValue, {.XmlElt=${write_string("$st")}}}</xsl:template>
<xsl:template match="uax:NodeId" mode="value">,{true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {.NodeId=<xsl:value-of select="."/>}}</xsl:template>
<xsl:template match="uax:DateTime" mode="value">,{true, SOPC_DateTime_Id, SOPC_VariantArrayType_SingleValue, {.Date.Low32=<xsl:value-of select="."/>,.Date.High32=<xsl:value-of select="."/>}}</xsl:template>

<xsl:template match="*" mode="value">
<xsl:message terminate="yes"> unknown type <xsl:value-of select="local-name(.)"/>
</xsl:message>
</xsl:template>

<!-- generation of node id -->
<xsl:template match="@*" mode="nodeId">
    <xsl:variable name="NodeId" select="."/>
    <xsl:analyze-string select="$NodeId" regex="(ns=\d+;)?(i=\d+|s=.+)">
        <xsl:matching-substring>
        <xsl:variable name="nsIndex" select="if (regex-group(1)) then substring-after(substring-before(regex-group(1),';'),'=') else 0"/>
        <xsl:variable name="ident" select="regex-group(2)"/>{<xsl:choose>
            <xsl:when test="starts-with($ident, 'i')">SOPC_IdentifierType_Numeric, <xsl:value-of select="$nsIndex"/>, .Data.Numeric = <xsl:value-of select="substring-after($ident,'=')"/></xsl:when>
            <xsl:when test="starts-with($ident, 's')">SOPC_IdentifierType_String, <xsl:value-of select="$nsIndex"/>, .Data.String = ${write_string("substring-after($ident,'=')")}</xsl:when>
            <xsl:when test="starts-with($ident, 'g')">SOPC_IdentifierType_Guid, <xsl:value-of select="$nsIndex"/>, <xsl:value-of select="substring-after($ident,'=')"/></xsl:when>
            <xsl:when test="starts-with($ident, 'b')">SOPC_IdentifierType_ByteString,  <xsl:value-of select="$nsIndex"/>, <xsl:value-of select="substring-after($ident,'=')"/></xsl:when>
            <xsl:otherwise>
                <xsl:message terminate="yes">Unknown identifier type : '<xsl:value-of select="$ident"/>'.</xsl:message>
            </xsl:otherwise>
        </xsl:choose>
        </xsl:matching-substring>
        <xsl:non-matching-substring>
            <xsl:message  terminate="yes">'<xsl:value-of select="$NodeId"/>' invalid node id</xsl:message>
        </xsl:non-matching-substring>
    </xsl:analyze-string>}<xsl:text/>
</xsl:template>

<xsl:template match="@*" mode="qName">
    <xsl:variable name="bn" select="."/>
    <xsl:variable name="pos" select="position()"/>
    <xsl:variable name="NodeId" select="../@NodeId"/>
    <xsl:analyze-string select="$bn" regex="(\d+:)?(.+)">
        <xsl:matching-substring>
        <xsl:variable name="nsIndex" select="if (regex-group(1)) then substring-before(regex-group(1),':') else 0"/>
${print_value(',{%s,{%s,1,toSOPC_String("%s")}}/* %s*/',"$nsIndex", "string-length(regex-group(2))", "regex-group(2)", "$NodeId")}<xsl:text>
</xsl:text>
        </xsl:matching-substring>
        <xsl:non-matching-substring>
            <xsl:message><xsl:value-of select="$bn"/> invalid qualified string format</xsl:message>
        </xsl:non-matching-substring>
    </xsl:analyze-string>
</xsl:template>

<xsl:template match="*" mode="localized_text">, {${write_string("@Locale")},${write_string("./text()")}}
</xsl:template>

<xsl:template name="write-string">
    <xsl:param name="value"/>${print_value('{%s,1,toSOPC_String("%s")}',"string-length($value)","$value")}
</xsl:template>

<!-- templates to copy nodes -->
<xsl:template match="@ReferenceType" mode="copy">
    <xsl:variable name="type" select="."/>
    <xsl:attribute name="ReferenceType"><xsl:value-of select="$alias[@Alias = $type]"/></xsl:attribute>
</xsl:template>

<xsl:template match="@* | node()" mode="copy">
    <xsl:copy>
        <xsl:apply-templates select="@* | node()" mode="copy"/>
    </xsl:copy>
</xsl:template>

<!-- generate two functions with domain is node classes:
'ord_class' which associate to a class an integer to sort nodes by class
'get_enum_value' returning the enumerate C corresponding to a class. -->
% for (n, f, type) in [('ord_class', lambda x: x, 'integer'), ('get_enum_value', lambda x : 'OpcUa_NodeClass_'+ classes[x-1], 'string')]:
  <xsl:function name="sys:${n}" as="xsd:${type}">
    <xsl:param name="e"/>
    <xsl:variable name='ln' select="local-name($e)"/>
    <xsl:choose>
%   for i in range(1, 9):
    <xsl:when test="$ln='UA${classes[i-1]}'">${f(i)}</xsl:when>
%   endfor
    <xsl:otherwise>
        <xsl:message terminate="yes">Unknown class : '<xsl:value-of select="$ln"/>'.</xsl:message>
    </xsl:otherwise>
    </xsl:choose>
  </xsl:function>
% endfor

<%def name="write_string(value)"><xsl:call-template name="write-string"><xsl:with-param name="value" select="${value}"/></xsl:call-template></%def>

<%def name="print_value(patt, *args)">
    <%doc>
    Function that apply the given template string to the
    result of the XPath queries and returns an output formatted string.
    @patt str: a format string
    @args str: a set of XPath expressions
    </%doc>
${("<xsl:text>"+patt+"</xsl:text>") % tuple(map(lambda x : '</xsl:text><xsl:value-of select="%s"/><xsl:text>' % x, args))}</%def>
</xsl:stylesheet>
