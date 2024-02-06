<?xml version="1.0" encoding="UTF-8"?>
<!--
Licensed to Systerel under one or more contributor license
agreements. See the NOTICE file distributed with this work
for additional information regarding copyright ownership.
Systerel licenses this file to you under the Apache
License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License. You may obtain
a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-->
<!--
This script generates the reciprocal references for all the references defined in the address space.
Forward references are generated for available Backward references and reciprocally. 
Both backward and forward references shall be defined in address space used with S2OPC toolkit.

Script use example:
saxonb-xslt -s:$S2OPC_REPO/samples/ClientServer/data/address_space/s2opc_no_reciprocal_refs.xml -xsl:$S2OPC_REPO/scripts/gen-reciprocal-refs-address-space.xslt -o:$S2OPC_REPO/samples/ClientServer/data/address_space/s2opc.xml -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://opcfoundation.org/UA/2011/03/UANodeSet.xsd" xpath-default-namespace="http://opcfoundation.org/UA/2011/03/UANodeSet.xsd" version="2.0"  >
    <xsl:output method="xml"  encoding="UTF-8" indent="yes"/>

    <xsl:template match="References">
        <References>
            <xsl:apply-templates select="@*|node()"/>

            <!-- node id of the current node evaluated. -->
            <xsl:variable name="NodeId" select="../@NodeId"/>

            <!-- get list of nodes it references -->
            <xsl:variable name="refNodeIdList" select="Reference/node()"/>
            <!-- Select all backward references that have current node as target and that are not defined in current node -->
            <xsl:variable name='refListForward' select="//Reference[@IsForward='false' and text()=$NodeId and not(../../@NodeId = $refNodeIdList)]"/>
            <!-- Select all forward references that have current node as target and that are not defined in current node -->
            <xsl:variable name='refListBack' select="//Reference[count(@IsForward)=0 and text()=$NodeId and not(../../@NodeId = $refNodeIdList)]"/>

            <!-- Add missing forward references in current node -->
            <xsl:for-each select="$refListForward">
                <Reference ReferenceType='{@ReferenceType}'><xsl:value-of select="../../@NodeId"/></Reference>

            </xsl:for-each>
            <!-- Add missing backward references in current node -->
            <xsl:for-each select="$refListBack">
                <Reference ReferenceType='{@ReferenceType}' IsForward='false'><xsl:value-of select="../../@NodeId"/></Reference>
            </xsl:for-each>
        </References>
    </xsl:template>

    <!-- copy rule -->
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>

</xsl:stylesheet>
