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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
version="2.0">
<xsl:output method="text" encoding="UTF-8"/>

<xsl:template match="/">

<!-- Filter intermediate nodes: only keep result associated to a test case. -->
<xsl:for-each select="//ResultNode/*">
    <xsl:variable name="description" select="@description"/>
    <xsl:variable name="testresult" select="@testresult"/>
    <xsl:variable name="testname" select="@name"/>
    <xsl:analyze-string select="$testname" regex="(\d+|initialize).js">
        <xsl:matching-substring>
        <xsl:choose>
        <!-- Test results codes are the followings
            0: Fail/Error
            1: Warning
            2: Not Implemented
            3: Skipped
            4: Not Supported
            5: OK/Log
            6: Back-trace
        -->
<xsl:when test="starts-with($testresult, '0')">Error|<xsl:value-of select="regex-group(1)" />|<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '1')">Warning|<xsl:value-of select="regex-group(1)" />|<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '2')">Not implemented|<xsl:value-of select="regex-group(1)" />|<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '3')">Skipped|<xsl:value-of select="regex-group(1)" />|<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '4')">Not supported|<xsl:value-of select="regex-group(1)" />|<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '5')">Ok|<xsl:value-of select="regex-group(1)" />|<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '6')"><xsl:message>Unsupported testresult code <xsl:value-of select="regex-group(1)"/> for test <xsl:value-of select="$testname"/> </xsl:message></xsl:when>
        </xsl:choose>
        </xsl:matching-substring>
    </xsl:analyze-string><xsl:text/>
</xsl:for-each>

</xsl:template>

</xsl:stylesheet>


