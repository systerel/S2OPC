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
This script removes the method nodes instances (but not the method nodes in type definition)

Script use example:
saxonb-xslt -s:$S2OPC_REPO/tests/data/address_space/s2opc_origin.xml -xsl:$S2OPC_REPO/scripts/remove-methods-address-space.xslt -o:$S2OPC_REPO/tests/data/address_space/s2opc_nano_tmp.xml
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://opcfoundation.org/UA/2011/03/UANodeSet.xsd" xpath-default-namespace="http://opcfoundation.org/UA/2011/03/UANodeSet.xsd" version="2.0"  >
    <xsl:output method="xml"  encoding="UTF-8" indent="yes"/>

    <!-- delete all method with a methodDeclarationId (others are in type definition) -->
    <xsl:template match="UAMethod[not(count(@MethodDeclarationId)=0)]"/>

    <!-- delete all references to the method deleted -->
    <xsl:template match="Reference">
      <xsl:if test="not(current() = //UAMethod[not(count(@MethodDeclarationId)=0)]/@NodeId)">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
      </xsl:if>
    </xsl:template>

    <!-- delete children variables of method nodes deleted + MaxNodesPerMethodCall -->
    <xsl:template match="UAVariable">
      <xsl:if test="not(@ParentNodeId = //UAMethod[not(count(@MethodDeclarationId)=0)]/@NodeId) and not(@NodeId = 'i=11709')">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
      </xsl:if>
    </xsl:template>

    <!-- copy rule -->
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>

</xsl:stylesheet>
