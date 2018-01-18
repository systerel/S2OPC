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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
version="2.0">
<xsl:output method="text" encoding="UTF-8"/>

<xsl:template match="/">

<!-- Filter intermediate nodes: only keep result associated to a test case. -->
<xsl:for-each select="//ResultNode/*">
    <xsl:variable name="description" select="@description"/>
    <xsl:variable name="testresult" select="@testresult"/>
    <xsl:variable name="testname" select="@name"/>
    <xsl:analyze-string select="$testname" regex="(\d+).js">
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
<xsl:when test="starts-with($testresult, '0')">Error-<xsl:value-of select="regex-group(1)" />-<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '1')">Warning-<xsl:value-of select="regex-group(1)" />-<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '2')">Not implemented-<xsl:value-of select="regex-group(1)" />-<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '3')">Skipped-<xsl:value-of select="regex-group(1)" />-<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '4')">Not supported-<xsl:value-of select="regex-group(1)" />-<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '5')">Ok-<xsl:value-of select="regex-group(1)" />-<xsl:value-of select="$description"/><xsl:text>
</xsl:text></xsl:when>
<xsl:when test="starts-with($testresult, '6')"><xsl:message>Unsupported testresult code <xsl:value-of select="regex-group(1)"/> for test <xsl:value-of select="$testname"/> </xsl:message></xsl:when>
        </xsl:choose>
        </xsl:matching-substring>
    </xsl:analyze-string><xsl:text/>
</xsl:for-each>

</xsl:template>

</xsl:stylesheet>


