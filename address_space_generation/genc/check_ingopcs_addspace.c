/*
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma GCC diagnostic ignored "-Wunused-variable"

#include <stdio.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

extern SOPC_AddressSpace addressSpace;
#define DisplayName addressSpace.displayNameArray
#define DisplayName_begin addressSpace.displayNameIdxArray_begin
#define DisplayName_end addressSpace.displayNameIdxArray_end
#define Description addressSpace.descriptionArray
#define Description_begin addressSpace.descriptionIdxArray_begin
#define Description_end addressSpace.descriptionIdxArray_end
#define DEFAULT_VARIANT                                        \
    {                                                          \
        SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, { 0 } \
    }

bool test_browsename(void)
{
    bool bres = true;
    printf("test BrowseName\n");
    const char* var;
    if (addressSpace.browseNameArray[1].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Root";
    if (strcmp((char*) addressSpace.browseNameArray[1].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[2].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Objects";
    if (strcmp((char*) addressSpace.browseNameArray[2].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[3].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Types";
    if (strcmp((char*) addressSpace.browseNameArray[3].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[4].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Views";
    if (strcmp((char*) addressSpace.browseNameArray[4].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[5].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "ObjectTypes";
    if (strcmp((char*) addressSpace.browseNameArray[5].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[6].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "VariableTypes";
    if (strcmp((char*) addressSpace.browseNameArray[6].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[7].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "DataTypes";
    if (strcmp((char*) addressSpace.browseNameArray[7].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[8].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "ReferenceTypes";
    if (strcmp((char*) addressSpace.browseNameArray[8].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[9].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder1";
    if (strcmp((char*) addressSpace.browseNameArray[9].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[10].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder2";
    if (strcmp((char*) addressSpace.browseNameArray[10].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[11].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder3";
    if (strcmp((char*) addressSpace.browseNameArray[11].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[12].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder4";
    if (strcmp((char*) addressSpace.browseNameArray[12].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[13].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder5";
    if (strcmp((char*) addressSpace.browseNameArray[13].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[14].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder6";
    if (strcmp((char*) addressSpace.browseNameArray[14].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[15].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder7";
    if (strcmp((char*) addressSpace.browseNameArray[15].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[16].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder8";
    if (strcmp((char*) addressSpace.browseNameArray[16].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[17].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder9";
    if (strcmp((char*) addressSpace.browseNameArray[17].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[18].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder10";
    if (strcmp((char*) addressSpace.browseNameArray[18].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[19].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder11";
    if (strcmp((char*) addressSpace.browseNameArray[19].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[20].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder12";
    if (strcmp((char*) addressSpace.browseNameArray[20].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[21].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder13";
    if (strcmp((char*) addressSpace.browseNameArray[21].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[22].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder14";
    if (strcmp((char*) addressSpace.browseNameArray[22].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[23].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder15";
    if (strcmp((char*) addressSpace.browseNameArray[23].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[24].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder16";
    if (strcmp((char*) addressSpace.browseNameArray[24].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[25].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder17";
    if (strcmp((char*) addressSpace.browseNameArray[25].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[26].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder18";
    if (strcmp((char*) addressSpace.browseNameArray[26].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[27].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder19";
    if (strcmp((char*) addressSpace.browseNameArray[27].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[28].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SessionPlaceHolder20";
    if (strcmp((char*) addressSpace.browseNameArray[28].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[29].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "VariablesFolderBn";
    if (strcmp((char*) addressSpace.browseNameArray[29].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[30].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "15361";
    if (strcmp((char*) addressSpace.browseNameArray[30].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[31].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SIGNALs";
    if (strcmp((char*) addressSpace.browseNameArray[31].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[32].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SWITCHs";
    if (strcmp((char*) addressSpace.browseNameArray[32].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[33].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "TRACKs";
    if (strcmp((char*) addressSpace.browseNameArray[33].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[34].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_G019";
    if (strcmp((char*) addressSpace.browseNameArray[34].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[35].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[35].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[36].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RC";
    if (strcmp((char*) addressSpace.browseNameArray[36].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[37].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_G025";
    if (strcmp((char*) addressSpace.browseNameArray[37].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[38].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[38].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[39].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RC";
    if (strcmp((char*) addressSpace.browseNameArray[39].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[40].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_G026";
    if (strcmp((char*) addressSpace.browseNameArray[40].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[41].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[41].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[42].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_W1";
    if (strcmp((char*) addressSpace.browseNameArray[42].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[43].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[43].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[44].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RC";
    if (strcmp((char*) addressSpace.browseNameArray[44].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[45].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_026TK";
    if (strcmp((char*) addressSpace.browseNameArray[45].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[46].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[46].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[47].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_OSTK";
    if (strcmp((char*) addressSpace.browseNameArray[47].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[48].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[48].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[49].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_P500";
    if (strcmp((char*) addressSpace.browseNameArray[49].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[50].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "BALA_RDLS_WBK_RDLN_EBK";
    if (strcmp((char*) addressSpace.browseNameArray[50].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[51].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RM";
    if (strcmp((char*) addressSpace.browseNameArray[51].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[52].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "LocaleIdArray";
    if (strcmp((char*) addressSpace.browseNameArray[52].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[53].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "State";
    if (strcmp((char*) addressSpace.browseNameArray[53].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[54].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "EnabledFlag";
    if (strcmp((char*) addressSpace.browseNameArray[54].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[55].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Int64";
    if (strcmp((char*) addressSpace.browseNameArray[55].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[56].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInt32";
    if (strcmp((char*) addressSpace.browseNameArray[56].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[57].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Double";
    if (strcmp((char*) addressSpace.browseNameArray[57].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[58].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "String";
    if (strcmp((char*) addressSpace.browseNameArray[58].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[59].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "ByteString";
    if (strcmp((char*) addressSpace.browseNameArray[59].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[60].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "XmlElement";
    if (strcmp((char*) addressSpace.browseNameArray[60].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[61].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SByte";
    if (strcmp((char*) addressSpace.browseNameArray[61].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[62].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Byte";
    if (strcmp((char*) addressSpace.browseNameArray[62].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[63].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Int16";
    if (strcmp((char*) addressSpace.browseNameArray[63].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[64].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInt16";
    if (strcmp((char*) addressSpace.browseNameArray[64].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[65].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Int32";
    if (strcmp((char*) addressSpace.browseNameArray[65].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[66].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInt64";
    if (strcmp((char*) addressSpace.browseNameArray[66].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[67].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Float";
    if (strcmp((char*) addressSpace.browseNameArray[67].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[68].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "GK";
    if (strcmp((char*) addressSpace.browseNameArray[68].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[69].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "ASK";
    if (strcmp((char*) addressSpace.browseNameArray[69].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[70].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBKK";
    if (strcmp((char*) addressSpace.browseNameArray[70].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[71].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBZCRQ";
    if (strcmp((char*) addressSpace.browseNameArray[71].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[72].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBZ-AK";
    if (strcmp((char*) addressSpace.browseNameArray[72].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[73].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBZCRQ-AK";
    if (strcmp((char*) addressSpace.browseNameArray[73].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[74].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SendCommand";
    if (strcmp((char*) addressSpace.browseNameArray[74].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[75].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "OffBlocking-K";
    if (strcmp((char*) addressSpace.browseNameArray[75].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[76].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "OffBlocking-CC";
    if (strcmp((char*) addressSpace.browseNameArray[76].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[77].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "GZ";
    if (strcmp((char*) addressSpace.browseNameArray[77].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[78].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SZ";
    if (strcmp((char*) addressSpace.browseNameArray[78].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[79].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBZ-CC";
    if (strcmp((char*) addressSpace.browseNameArray[79].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[80].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBZCRQ-CC";
    if (strcmp((char*) addressSpace.browseNameArray[80].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[81].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "XBZC-CC";
    if (strcmp((char*) addressSpace.browseNameArray[81].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[82].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "GK";
    if (strcmp((char*) addressSpace.browseNameArray[82].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[83].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "ASK";
    if (strcmp((char*) addressSpace.browseNameArray[83].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[84].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SendCommand";
    if (strcmp((char*) addressSpace.browseNameArray[84].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[85].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "OffBlocking-K";
    if (strcmp((char*) addressSpace.browseNameArray[85].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[86].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "OffBlocking-CC";
    if (strcmp((char*) addressSpace.browseNameArray[86].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[87].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SZ";
    if (strcmp((char*) addressSpace.browseNameArray[87].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[88].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "GZ";
    if (strcmp((char*) addressSpace.browseNameArray[88].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[89].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "GK";
    if (strcmp((char*) addressSpace.browseNameArray[89].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[90].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "NWK";
    if (strcmp((char*) addressSpace.browseNameArray[90].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[91].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RWK";
    if (strcmp((char*) addressSpace.browseNameArray[91].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[92].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "LK";
    if (strcmp((char*) addressSpace.browseNameArray[92].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[93].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "SendCommand";
    if (strcmp((char*) addressSpace.browseNameArray[93].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[94].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "OffBlocking-K";
    if (strcmp((char*) addressSpace.browseNameArray[94].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[95].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "OffBlocking-CC";
    if (strcmp((char*) addressSpace.browseNameArray[95].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[96].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "NWZ";
    if (strcmp((char*) addressSpace.browseNameArray[96].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[97].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "RWZ";
    if (strcmp((char*) addressSpace.browseNameArray[97].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[98].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "TK";
    if (strcmp((char*) addressSpace.browseNameArray[98].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[99].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "TK";
    if (strcmp((char*) addressSpace.browseNameArray[99].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[100].NamespaceIndex != 261)
    {
        printf("invalid BrowseName ");
    }
    var = "TK";
    if (strcmp((char*) addressSpace.browseNameArray[100].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[101].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "BaseVariableType";
    if (strcmp((char*) addressSpace.browseNameArray[101].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[102].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "BaseDataVariableType";
    if (strcmp((char*) addressSpace.browseNameArray[102].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[103].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "BaseObjectType";
    if (strcmp((char*) addressSpace.browseNameArray[103].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[104].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "FolderType";
    if (strcmp((char*) addressSpace.browseNameArray[104].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[105].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "References";
    if (strcmp((char*) addressSpace.browseNameArray[105].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[106].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "BaseDataType";
    if (strcmp((char*) addressSpace.browseNameArray[106].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[107].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Number";
    if (strcmp((char*) addressSpace.browseNameArray[107].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[108].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Integer";
    if (strcmp((char*) addressSpace.browseNameArray[108].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[109].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInteger";
    if (strcmp((char*) addressSpace.browseNameArray[109].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[110].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Enumeration";
    if (strcmp((char*) addressSpace.browseNameArray[110].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[111].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Boolean";
    if (strcmp((char*) addressSpace.browseNameArray[111].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[112].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "SByte";
    if (strcmp((char*) addressSpace.browseNameArray[112].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[113].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Byte";
    if (strcmp((char*) addressSpace.browseNameArray[113].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[114].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Int16";
    if (strcmp((char*) addressSpace.browseNameArray[114].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[115].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInt16";
    if (strcmp((char*) addressSpace.browseNameArray[115].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[116].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Int32";
    if (strcmp((char*) addressSpace.browseNameArray[116].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[117].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInt32";
    if (strcmp((char*) addressSpace.browseNameArray[117].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[118].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Int64";
    if (strcmp((char*) addressSpace.browseNameArray[118].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[119].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "UInt64";
    if (strcmp((char*) addressSpace.browseNameArray[119].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[120].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Float";
    if (strcmp((char*) addressSpace.browseNameArray[120].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[121].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Double";
    if (strcmp((char*) addressSpace.browseNameArray[121].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[122].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "String";
    if (strcmp((char*) addressSpace.browseNameArray[122].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[123].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "DateTime";
    if (strcmp((char*) addressSpace.browseNameArray[123].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[124].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Guid";
    if (strcmp((char*) addressSpace.browseNameArray[124].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[125].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "ByteString";
    if (strcmp((char*) addressSpace.browseNameArray[125].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[126].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "XmlElement";
    if (strcmp((char*) addressSpace.browseNameArray[126].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[127].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "NodeId";
    if (strcmp((char*) addressSpace.browseNameArray[127].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[128].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "ExpandedNodeId";
    if (strcmp((char*) addressSpace.browseNameArray[128].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[129].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "StatusCode";
    if (strcmp((char*) addressSpace.browseNameArray[129].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[130].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "QualifiedName";
    if (strcmp((char*) addressSpace.browseNameArray[130].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[131].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "LocalizedText";
    if (strcmp((char*) addressSpace.browseNameArray[131].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[132].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Structure";
    if (strcmp((char*) addressSpace.browseNameArray[132].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[133].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "DataValue";
    if (strcmp((char*) addressSpace.browseNameArray[133].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[134].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "DiagnosticInfo";
    if (strcmp((char*) addressSpace.browseNameArray[134].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[135].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Image";
    if (strcmp((char*) addressSpace.browseNameArray[135].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[136].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "LocaleId";
    if (strcmp((char*) addressSpace.browseNameArray[136].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }
    if (addressSpace.browseNameArray[137].NamespaceIndex != 0)
    {
        printf("invalid BrowseName ");
    }
    var = "Decimal128";
    if (strcmp((char*) addressSpace.browseNameArray[137].Name.Data, var))
    {
        bres = false;
        printf("invalid BrowseName ");
    }

    return bres;
}

bool test_value(void)
{
    printf("test Value\n");
    bool bres = true;
    int pos;
    const char* nodeid;
    SOPC_Byte builtInTypeId;
    char* value_node;
    bool bool_value;
    signed char int8_value;
    unsigned char uint8_value;
    signed short int16_value;
    unsigned short uint16_value;
    signed long int32_value;
    unsigned long uint32_value;
    int long long int64_value;
    unsigned int long long uint64_value;
    double double_value;
    unsigned long date_time_value_32_lsb;
    unsigned long date_time_value_32_msb;
    float float_value;
    int status_value;

    pos = 1;
    nodeid = "i=2271";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 2;

    nodeid = "i=2259";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "0";
    builtInTypeId = SOPC_Int32_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 3;

    nodeid = "i=3114";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 4;

    nodeid = "i=1001";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "-1000";
    builtInTypeId = SOPC_Int64_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 5;

    nodeid = "i=1002";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "1000";
    sscanf(value_node, "%lu", &uint32_value);

    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_UInt32_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Uint32 == uint32_value))
    {
        printf("invalid Value \n");
    }

    pos = 6;

    nodeid = "i=1003";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "2.0";
    builtInTypeId = SOPC_Double_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 7;

    nodeid = "i=1004";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "String:INGOPCS";
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_String_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (strcmp((char*) addressSpace.valueArray[pos].Value.String.Data, value_node) != 0)
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 8;

    nodeid = "i=1005";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "ByteString:INGOPCS";
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_ByteString_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (strcmp((char*) addressSpace.valueArray[pos].Value.Bstring.Data, value_node) != 0)
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 9;

    nodeid = "i=1006";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "XmlElement:INGOPCS";

    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_XmlElement_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (strcmp((char*) addressSpace.valueArray[pos].Value.XmlElt.Data, value_node) != 0)
    {
        printf("invalid Value \n");
    }

    pos = 10;

    nodeid = "i=1007";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "-127";
    builtInTypeId = SOPC_SByte_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 11;

    nodeid = "i=1008";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "255";
    builtInTypeId = SOPC_Byte_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 12;

    nodeid = "i=1009";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "-32767";
    builtInTypeId = SOPC_Int16_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 13;

    nodeid = "i=1010";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "65535";
    sscanf(value_node, "%hu", &uint16_value);

    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_UInt16_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Uint16 == uint16_value))
    {
        printf("invalid Value \n");
    }

    pos = 14;

    nodeid = "i=1011";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "-2147483647";
    builtInTypeId = SOPC_Int32_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 15;

    nodeid = "i=1012";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "1844674407370955";
    sscanf(value_node, "%llu", &uint64_value);

    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_UInt64_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Uint64 == uint64_value))
    {
        printf("invalid Value \n");
    }

    pos = 16;

    nodeid = "i=1013";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "109517.875";
    sscanf(value_node, "%f", &float_value);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Float_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype  \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Floatv == float_value))
    {
        bres = false;
        printf("invalid Value  \n");
    }

    pos = 17;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 18;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 19;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 20;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 21;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 22;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 23;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 24;
    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 25;
    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 26;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 27;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 28;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 29;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 30;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 31;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 32;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 33;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 34;
    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 35;
    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 36;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 37;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 38;

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 39;

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 40;

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 41;

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 42;

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 43;
    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 44;
    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 45;

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 46;

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 47;

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "true";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 48;

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "true";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 49;

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    printf("test Value for nodeid %s\n", nodeid);
    value_node = "false";
    builtInTypeId = SOPC_Boolean_Id;
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (builtInTypeId == SOPC_Boolean_Id)
    {
        if (!strcmp(value_node, "true"))
        {
            bool_value = true;
        }
        else
        {
            bool_value = false;
        }
        if (!(addressSpace.valueArray[pos].Value.Boolean == bool_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_SByte_Id)
    {
        sscanf(value_node, "%hhi", &int8_value);
        if (!(addressSpace.valueArray[pos].Value.Sbyte == int8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Byte_Id)
    {
        sscanf(value_node, "%hhu", &uint8_value);
        if (!(addressSpace.valueArray[pos].Value.Byte == uint8_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int16_Id)
    {
        sscanf(value_node, "%hi", &int16_value);
        if (!(addressSpace.valueArray[pos].Value.Int16 == int16_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int32_Id)
    {
        sscanf(value_node, "%li", &int32_value);
        if (!(addressSpace.valueArray[pos].Value.Int32 == int32_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Int64_Id)
    {
        sscanf(value_node, "%lld", &int64_value);
        if (!(addressSpace.valueArray[pos].Value.Int64 == int64_value))
        {
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_Double_Id)
    {
        sscanf(value_node, "%lf", &double_value);
        if (!(addressSpace.valueArray[pos].Value.Doublev == double_value))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }
    else if (builtInTypeId == SOPC_NodeId_Id)
    {
        if (!strcmp((char*) addressSpace.valueArray[pos].Value.NodeId->Data.String.Data, value_node))
        {
            bres = false;
            printf("invalid Value \n");
        }
    }

    pos = 50;
    nodeid = "i=62";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    pos = 51;
    nodeid = "i=63";
    printf("test Value for nodeid %s\n", nodeid);
    if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id))
    {
        bres = false;
        printf("invalid BuiltInTypeId \n");
    }

    if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue))
    {
        bres = false;
        printf("invalid Arraytype \n");
    }

    if (!(addressSpace.valueArray[pos].Value.Boolean == 0))
    {
        bres = false;
        printf("invalid Value \n");
    }

    return bres;
}

bool test_reference(void)
{
    printf("test reference \n");
    bool bres = true;
    int pos;
    int res;
    int exp;
    const char* nodeid;

    nodeid = "i=84";
    pos = 1;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 85)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 85, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 86)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 86, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 87)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 87, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=85";
    pos = 2;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 15361)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361,
                       res->Namespace, res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 84)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 84, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=86";
    pos = 3;
    exp = 6;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 84)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 84, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 88)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 88, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 89)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 89, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 90)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 90, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 91)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 91, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=87";
    pos = 4;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 84)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 84, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=88";
    pos = 5;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 86)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 86, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 58)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 58, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=89";
    pos = 6;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 86)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 86, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 62)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 62, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=90";
    pos = 7;
    exp = 30;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 86)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 86, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 26)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 26, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 27)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 27, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 28)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 28, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 29)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 29, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 8, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 8 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 8 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 2)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 2, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 9, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 9 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 9 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 3)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 3, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 10, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 10 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 10 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 4)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 4, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 11, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 11 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 11 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 5)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 5, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 12, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 12 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 12 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 6)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 6, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 13, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 13 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 13 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 7)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 7, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 14, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 14 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 14 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 8)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 8, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 15, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 15 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 15 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 9)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 9, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 16, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 16 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 16 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 10)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 10, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 17, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 17 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 17 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 11)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 11, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 18, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 18 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 18 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 12)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 12, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 19, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 19 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 19 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 13)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 13, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 20, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 20 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 20 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 14)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 14, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 21, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 21 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 21 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 15)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 22, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 22 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 22 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 16)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 16, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 23, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 23 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 23 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 17)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 17, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 24, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 24 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 24 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 18)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 18, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 25, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 25 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 25 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 19)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 19, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 26, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 26 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 26 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 20)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 20, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 27, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 27 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 27 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 21)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 21, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 28, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 28 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 28 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 22)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 22, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 29, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 29 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 29 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 23)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 23, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 30, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 30 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 30 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=91";
    pos = 8;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 86)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 86, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 31)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 31, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=10001";
    pos = 9;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10002";
    pos = 10;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10003";
    pos = 11;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10004";
    pos = 12;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10005";
    pos = 13;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10006";
    pos = 14;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10007";
    pos = 15;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10008";
    pos = 16;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10009";
    pos = 17;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10010";
    pos = 18;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10011";
    pos = 19;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10012";
    pos = 20;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10013";
    pos = 21;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10014";
    pos = 22;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10015";
    pos = 23;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10016";
    pos = 24;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10017";
    pos = 25;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10018";
    pos = 26;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10019";
    pos = 27;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=10020";
    pos = 28;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=1000";
    pos = 29;
    exp = 15;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 85)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 85, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1001)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1001, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1002)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1002, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1003)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1003, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1004)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1004, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1005)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1005, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 8, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 8 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 8 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1006)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1006, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 9, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 9 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 9 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1007)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1007, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 10, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 10 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 10 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1008)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1008, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 11, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 11 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 11 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1009)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1009, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 12, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 12 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 12 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1010)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1010, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 13, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 13 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 13 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1011)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1011, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 14, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 14 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 14 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1012)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1012, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 15, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 15 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 15 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1013)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1013, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;i=15361";
    pos = 30;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs";
    pos = 31;
    exp = 5;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 15361)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361,
                       res->Namespace, res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026 for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs";
    pos = 32;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 15361)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361,
                       res->Namespace, res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs";
    pos = 33;
    exp = 6;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 15361)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361,
                       res->Namespace, res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_P500") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_P500 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019";
    pos = 34;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM";
    pos = 35;
    exp = 8;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 8, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 8 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 8 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC";
    pos = 36;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025";
    pos = 37;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM";
    pos = 38;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC";
    pos = 39;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026";
    pos = 40;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026.RM for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM";
    pos = 41;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1";
    pos = 42;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 6, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 7, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM";
    pos = 43;
    exp = 5;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 5, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC";
    pos = 44;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 4, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK";
    pos = 45;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK.RM for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM";
    pos = 46;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK";
    pos = 47;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK.RM for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM";
    pos = 48;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500";
    pos = 49;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK";
    pos = 50;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM for %s \n", nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM";
    pos = 51;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 61)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 3, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK for %s \n", nodeid);
            }
        }
    }

    nodeid = "i=2271";
    pos = 52;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 68)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 68, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 46)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 46, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 2268)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 2268, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=2259";
    pos = 53;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=3114";
    pos = 54;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=1001";
    pos = 55;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1002";
    pos = 56;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1003";
    pos = 57;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1004";
    pos = 58;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1005";
    pos = 59;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1006";
    pos = 60;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1007";
    pos = 61;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1008";
    pos = 62;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1009";
    pos = 63;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1010";
    pos = 64;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1011";
    pos = 65;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1012";
    pos = 66;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1013";
    pos = 67;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 1000)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    pos = 68;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    pos = 69;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    pos = 70;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    pos = 71;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    pos = 72;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    pos = 73;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    pos = 74;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    pos = 75;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    pos = 76;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    pos = 77;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    pos = 78;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    pos = 79;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    pos = 80;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    pos = 81;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    pos = 82;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    pos = 83;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    pos = 84;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    pos = 85;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    pos = 86;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    pos = 87;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    pos = 88;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    pos = 89;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    pos = 90;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    pos = 91;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    pos = 92;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    pos = 93;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    pos = 94;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    pos = 95;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    pos = 96;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    pos = 97;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    pos = 98;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    pos = 99;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    pos = 100;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 47)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 261;

            if (res->IdentifierType != SOPC_IdentifierType_String || res->Namespace != nsIndex ||
                strcmp((char*) res->Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM") != 0)
            {
                printf("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM for %s \n", nodeid);
            }
        }

        printf("test reference %d node %d nodeid %s\n", 2, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 40)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 63)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=62";
    pos = 101;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=63";
    pos = 102;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 62)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 62, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=58";
    pos = 103;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=61";
    pos = 104;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 58)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 58, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=31";
    pos = 105;
    exp = 0;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
    }

    nodeid = "i=24";
    pos = 106;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 35)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 86)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 86, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=26";
    pos = 107;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=27";
    pos = 108;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 26)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 26, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=28";
    pos = 109;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 26)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 26, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=29";
    pos = 110;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=1";
    pos = 111;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=2";
    pos = 112;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 27)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 27, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=3";
    pos = 113;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 28)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 28, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=4";
    pos = 114;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 27)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 27, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=5";
    pos = 115;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 28)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 28, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=6";
    pos = 116;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 27)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 27, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=7";
    pos = 117;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 28)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 28, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=8";
    pos = 118;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 27)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 27, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=9";
    pos = 119;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 28)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 28, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=10";
    pos = 120;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 26)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 26, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=11";
    pos = 121;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 26)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 26, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=12";
    pos = 122;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=13";
    pos = 123;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=14";
    pos = 124;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=15";
    pos = 125;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=16";
    pos = 126;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=17";
    pos = 127;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=18";
    pos = 128;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=19";
    pos = 129;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=20";
    pos = 130;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=21";
    pos = 131;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=22";
    pos = 132;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=23";
    pos = 133;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=25";
    pos = 134;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 24)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 24, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=30";
    pos = 135;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 15)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=295";
    pos = 136;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 12)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 12, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    nodeid = "i=121";
    pos = 137;
    exp = 1;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else
    {
        printf("test reference %d node %d nodeid %s\n", 1, pos, nodeid);

        {
            SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1];

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 45)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 45, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }

        {
            SOPC_NodeId* res =
                &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 - 1])->NodeId;

            int nsIndex = 0;

            if (res->IdentifierType != SOPC_IdentifierType_Numeric || res->Namespace != nsIndex ||
                res->Data.Numeric != 26)
            {
                printf("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 26, res->Namespace,
                       res->IdentifierType, res->Data.Numeric, nodeid);
            }
        }
    }

    return bres;
}

int compareLocalizedText(SOPC_LocalizedText LText, const char* text, const char* locale)
{
    int i = 0;
    if (strcmp((char*) LText.Locale.Data, locale) != 0)
    {
        printf("locale KO");
        i++;
    }
    else
    {
        printf("locale ok");
    }
    if (strcmp((char*) LText.Text.Data, text) != 0)
    {
        printf(", text KO");
        i++;
    }
    else
    {
        printf(", text ok");
    }
    return i;
}

bool test_Description(void)
{
    printf("test Description\n");
    bool bres = true;
    int pos;
    int res;
    int exp;
    const char* nodeid;

    nodeid = "i=84";
    pos = 1;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "The root of the server address space.", "");
        printf("\n");
    }

    nodeid = "i=85";
    pos = 2;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for objects in the server address space.", "");
        printf("\n");
    }

    nodeid = "i=86";
    pos = 3;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for types in the server address space.", "");
        printf("\n");
    }

    nodeid = "i=87";
    pos = 4;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for views in the server address space.", "");
        printf("\n");
    }

    nodeid = "i=88";
    pos = 5;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for object types in the server address space.", "");
        printf("\n");
    }

    nodeid = "i=89";
    pos = 6;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for variable types in the server address space.", "");
        printf("\n");
    }

    nodeid = "i=90";
    pos = 7;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for data types in the server address space.", "");
        printf("\n");
    }

    nodeid = "i=91";
    pos = 8;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The browse entry point when looking for reference types in the server address space.",
                             "");
        printf("\n");
    }

    nodeid = "i=10001";
    pos = 9;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10002";
    pos = 10;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10003";
    pos = 11;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10004";
    pos = 12;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10005";
    pos = 13;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10006";
    pos = 14;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10007";
    pos = 15;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10008";
    pos = 16;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10009";
    pos = 17;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10010";
    pos = 18;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10011";
    pos = 19;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10012";
    pos = 20;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10013";
    pos = 21;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10014";
    pos = 22;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10015";
    pos = 23;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10016";
    pos = 24;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10017";
    pos = 25;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10018";
    pos = 26;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10019";
    pos = 27;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=10020";
    pos = 28;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=1000";
    pos = 29;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "VariablesFolderDescObj1d2", "");
        printf("\n");
    }

    nodeid = "ns=261;i=15361";
    pos = 30;
    exp = 2;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");

        printf("test Description %d node %d nodeid %s : ", 2, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 2 - 1], "NoName", "en");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs";
    pos = 31;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs";
    pos = 32;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs";
    pos = 33;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019";
    pos = 34;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM";
    pos = 35;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC";
    pos = 36;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025";
    pos = 37;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM";
    pos = 38;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC";
    pos = 39;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026";
    pos = 40;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM";
    pos = 41;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1";
    pos = 42;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM";
    pos = 43;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC";
    pos = 44;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK";
    pos = 45;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM";
    pos = 46;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK";
    pos = 47;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM";
    pos = 48;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500";
    pos = 49;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK";
    pos = 50;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM";
    pos = 51;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "NoName", "");
        printf("\n");
    }

    nodeid = "i=2271";
    pos = 52;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "A list of locales supported by the server.",
                             "");
        printf("\n");
    }

    nodeid = "i=2259";
    pos = 53;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "i=3114";
    pos = 54;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "If TRUE the diagnostics collection is enabled.", "");
        printf("\n");
    }

    nodeid = "i=1001";
    pos = 55;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Int64_1d", "");
        printf("\n");
    }

    nodeid = "i=1002";
    pos = 56;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "UInt32_1d", "");
        printf("\n");
    }

    nodeid = "i=1003";
    pos = 57;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Double_1d", "");
        printf("\n");
    }

    nodeid = "i=1004";
    pos = 58;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "String_1d", "");
        printf("\n");
    }

    nodeid = "i=1005";
    pos = 59;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "ByteString_1d", "");
        printf("\n");
    }

    nodeid = "i=1006";
    pos = 60;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "XmlElement_1d", "");
        printf("\n");
    }

    nodeid = "i=1007";
    pos = 61;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "SByte_1d", "");
        printf("\n");
    }

    nodeid = "i=1008";
    pos = 62;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Byte_1d", "");
        printf("\n");
    }

    nodeid = "i=1009";
    pos = 63;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Int16_1d", "");
        printf("\n");
    }

    nodeid = "i=1010";
    pos = 64;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "UInt16_1d", "");
        printf("\n");
    }

    nodeid = "i=1011";
    pos = 65;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Int32_1d", "");
        printf("\n");
    }

    nodeid = "i=1012";
    pos = 66;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "UInt64_1d", "");
        printf("\n");
    }

    nodeid = "i=1013";
    pos = 67;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Float_1d", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    pos = 68;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Permissive Signal Status, ~é€bla", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    pos = 69;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Signal Approach Locking", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    pos = 70;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Exit blocking is in effect for the signal",
                             "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    pos = 71;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Exit Block Removal Request", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    pos = 72;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Exit blocking is rejected for the signal",
                             "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    pos = 73;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Exit block removal is rejected for the signal", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    pos = 74;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    pos = 75;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    pos = 76;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    pos = 77;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Signal request", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    pos = 78;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Signal cancel", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    pos = 79;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Exit Block Application Request", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    pos = 80;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Exit Block Removal Request", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    pos = 81;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Exit Block Removal - Acknowledge", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    pos = 82;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Permissive Signal Status", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    pos = 83;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Signal Approach Locking", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    pos = 84;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    pos = 85;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    pos = 86;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    pos = 87;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Signal cancel", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    pos = 88;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Signal request", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    pos = 89;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Permissive Signal Status", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    pos = 90;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Switch Detected Normal", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    pos = 91;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Switch Detected Reverse", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    pos = 92;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Switch Locally Locked", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    pos = 93;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    pos = 94;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    pos = 95;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    pos = 96;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Switch Calling in Normal Direction", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    pos = 97;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Switch Calling in Reverse Direction", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    pos = 98;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Secondary Detection Status", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    pos = 99;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Secondary Detection Status", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    pos = 100;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Secondary Detection Status", "");
        printf("\n");
    }

    nodeid = "i=62";
    pos = 101;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The abstract base type for all variable nodes.", "");
        printf("\n");
    }

    nodeid = "i=63";
    pos = 102;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The type for variable that represents a process value.", "");
        printf("\n");
    }

    nodeid = "i=58";
    pos = 103;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "The base type for all object nodes.", "");
        printf("\n");
    }

    nodeid = "i=61";
    pos = 104;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "The type for objects that organize other nodes.", "");
        printf("\n");
    }

    nodeid = "i=31";
    pos = 105;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "The abstract base type for all references.",
                             "");
        printf("\n");
    }

    nodeid = "i=24";
    pos = 106;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that can have any valid DataType.", "");
        printf("\n");
    }

    nodeid = "i=26";
    pos = 107;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that can have any numeric DataType.", "");
        printf("\n");
    }

    nodeid = "i=27";
    pos = 108;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that can have any integer DataType.", "");
        printf("\n");
    }

    nodeid = "i=28";
    pos = 109;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that can have any unsigned integer DataType.", "");
        printf("\n");
    }

    nodeid = "i=29";
    pos = 110;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an enumerated DataType.", "");
        printf("\n");
    }

    nodeid = "i=1";
    pos = 111;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is either TRUE or FALSE.", "");
        printf("\n");
    }

    nodeid = "i=2";
    pos = 112;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between -128 and 127.", "");
        printf("\n");
    }

    nodeid = "i=3";
    pos = 113;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between 0 and 255.", "");
        printf("\n");
    }

    nodeid = "i=4";
    pos = 114;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between −32,768 and 32,767.", "");
        printf("\n");
    }

    nodeid = "i=5";
    pos = 115;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between 0 and 65535.", "");
        printf("\n");
    }

    nodeid = "i=6";
    pos = 116;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between −2,147,483,648  and 2,147,483,647.", "");
        printf("\n");
    }

    nodeid = "i=7";
    pos = 117;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between 0 and 4,294,967,295.", "");
        printf("\n");
    }

    nodeid = "i=8";
    pos = 118;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(
            Description[Description_begin[pos] + 1 - 1],
            "Describes a value that is an integer between −9,223,372,036,854,775,808 and 9,223,372,036,854,775,807.",
            "");
        printf("\n");
    }

    nodeid = "i=9";
    pos = 119;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an integer between 0 and 18,446,744,073,709,551,615.", "");
        printf("\n");
    }

    nodeid = "i=10";
    pos = 120;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an IEEE 754-1985 single precision floating point number.", "");
        printf("\n");
    }

    nodeid = "i=11";
    pos = 121;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an IEEE 754-1985 double precision floating point number.", "");
        printf("\n");
    }

    nodeid = "i=12";
    pos = 122;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a sequence of printable Unicode characters.", "");
        printf("\n");
    }

    nodeid = "i=13";
    pos = 123;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a Gregorian calender date and time.", "");
        printf("\n");
    }

    nodeid = "i=14";
    pos = 124;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a 128-bit globally unique identifier.", "");
        printf("\n");
    }

    nodeid = "i=15";
    pos = 125;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a sequence of bytes.", "");
        printf("\n");
    }

    nodeid = "i=16";
    pos = 126;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Describes a value that is an XML element.",
                             "");
        printf("\n");
    }

    nodeid = "i=17";
    pos = 127;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an identifier for a node within a Server address space.", "");
        printf("\n");
    }

    nodeid = "i=18";
    pos = 128;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an absolute identifier for a node.", "");
        printf("\n");
    }

    nodeid = "i=19";
    pos = 129;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a code representing the outcome of an operation by a Server.",
                             "");
        printf("\n");
    }

    nodeid = "i=20";
    pos = 130;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a name qualified by a namespace.", "");
        printf("\n");
    }

    nodeid = "i=21";
    pos = 131;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is human readable Unicode text with a locale identifier.", "");
        printf("\n");
    }

    nodeid = "i=22";
    pos = 132;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(
            Description[Description_begin[pos] + 1 - 1],
            "Describes a value that is any type of structure that can be described with a data encoding.", "");
        printf("\n");
    }

    nodeid = "i=23";
    pos = 133;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is a structure containing a value, a status code and timestamps.",
                             "");
        printf("\n");
    }

    nodeid = "i=25";
    pos = 134;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(
            Description[Description_begin[pos] + 1 - 1],
            "Describes a value that is a structure containing diagnostics associated with a StatusCode.", "");
        printf("\n");
    }

    nodeid = "i=30";
    pos = 135;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1],
                             "Describes a value that is an image encoded as a string of bytes.", "");
        printf("\n");
    }

    nodeid = "i=295";
    pos = 136;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "An identifier for a user locale.", "");
        printf("\n");
    }

    nodeid = "i=121";
    pos = 137;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(Description[Description_begin[pos] + 1 - 1], "Describes a 128-bit decimal value.", "");
        printf("\n");
    }

    return bres;
}

bool test_DisplayName(void)
{
    printf("test DisplayName\n");
    bool bres = true;
    int pos;
    int res;
    int exp;
    const char* nodeid;

    nodeid = "i=84";
    pos = 1;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Root", "");
        printf("\n");
    }

    nodeid = "i=85";
    pos = 2;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Objects", "");
        printf("\n");
    }

    nodeid = "i=86";
    pos = 3;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Types", "");
        printf("\n");
    }

    nodeid = "i=87";
    pos = 4;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Views", "");
        printf("\n");
    }

    nodeid = "i=88";
    pos = 5;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ObjectTypes", "");
        printf("\n");
    }

    nodeid = "i=89";
    pos = 6;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "VariableTypes", "");
        printf("\n");
    }

    nodeid = "i=90";
    pos = 7;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "DataTypes", "");
        printf("\n");
    }

    nodeid = "i=91";
    pos = 8;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ReferenceTypes", "");
        printf("\n");
    }

    nodeid = "i=10001";
    pos = 9;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 1", "");
        printf("\n");
    }

    nodeid = "i=10002";
    pos = 10;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 2", "");
        printf("\n");
    }

    nodeid = "i=10003";
    pos = 11;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 3", "");
        printf("\n");
    }

    nodeid = "i=10004";
    pos = 12;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 4", "");
        printf("\n");
    }

    nodeid = "i=10005";
    pos = 13;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 5", "");
        printf("\n");
    }

    nodeid = "i=10006";
    pos = 14;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 6", "");
        printf("\n");
    }

    nodeid = "i=10007";
    pos = 15;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 7", "");
        printf("\n");
    }

    nodeid = "i=10008";
    pos = 16;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 8", "");
        printf("\n");
    }

    nodeid = "i=10009";
    pos = 17;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 9", "");
        printf("\n");
    }

    nodeid = "i=10010";
    pos = 18;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 10", "");
        printf("\n");
    }

    nodeid = "i=10011";
    pos = 19;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 11", "");
        printf("\n");
    }

    nodeid = "i=10012";
    pos = 20;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 12", "");
        printf("\n");
    }

    nodeid = "i=10013";
    pos = 21;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 13", "");
        printf("\n");
    }

    nodeid = "i=10014";
    pos = 22;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 14", "");
        printf("\n");
    }

    nodeid = "i=10015";
    pos = 23;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 15", "");
        printf("\n");
    }

    nodeid = "i=10016";
    pos = 24;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 16", "");
        printf("\n");
    }

    nodeid = "i=10017";
    pos = 25;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 17", "");
        printf("\n");
    }

    nodeid = "i=10018";
    pos = 26;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 18", "");
        printf("\n");
    }

    nodeid = "i=10019";
    pos = 27;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 19", "");
        printf("\n");
    }

    nodeid = "i=10020";
    pos = 28;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SessionPlaceHolder 20", "");
        printf("\n");
    }

    nodeid = "i=1000";
    pos = 29;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "VariablesFolderDn", "");
        printf("\n");
    }

    nodeid = "ns=261;i=15361";
    pos = 30;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "15361", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs";
    pos = 31;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SIGNALs", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs";
    pos = 32;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SWITCHs", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs";
    pos = 33;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "TRACKs", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019";
    pos = 34;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_G019", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM";
    pos = 35;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC";
    pos = 36;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025";
    pos = 37;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_G025", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM";
    pos = 38;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC";
    pos = 39;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026";
    pos = 40;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_G026", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM";
    pos = 41;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1";
    pos = 42;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_W1", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM";
    pos = 43;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC";
    pos = 44;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK";
    pos = 45;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_026TK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM";
    pos = 46;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK";
    pos = 47;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_OSTK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM";
    pos = 48;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500";
    pos = 49;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_P500", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK";
    pos = 50;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BALA_RDLS_WBK_RDLN_EBK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM";
    pos = 51;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RM", "");
        printf("\n");
    }

    nodeid = "i=2271";
    pos = 52;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "LocaleIdArray", "");
        printf("\n");
    }

    nodeid = "i=2259";
    pos = 53;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "State", "");
        printf("\n");
    }

    nodeid = "i=3114";
    pos = 54;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "EnabledFlag", "");
        printf("\n");
    }

    nodeid = "i=1001";
    pos = 55;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Int64_1dn", "");
        printf("\n");
    }

    nodeid = "i=1002";
    pos = 56;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInt32_1dn", "");
        printf("\n");
    }

    nodeid = "i=1003";
    pos = 57;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Double_1dn", "");
        printf("\n");
    }

    nodeid = "i=1004";
    pos = 58;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "String_1dn", "");
        printf("\n");
    }

    nodeid = "i=1005";
    pos = 59;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ByteString_1dn", "");
        printf("\n");
    }

    nodeid = "i=1006";
    pos = 60;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XmlElement_1dn", "");
        printf("\n");
    }

    nodeid = "i=1007";
    pos = 61;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SByte_1dn", "");
        printf("\n");
    }

    nodeid = "i=1008";
    pos = 62;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Byte_1dn", "");
        printf("\n");
    }

    nodeid = "i=1009";
    pos = 63;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Int16_1dn", "");
        printf("\n");
    }

    nodeid = "i=1010";
    pos = 64;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInt16_1dn", "");
        printf("\n");
    }

    nodeid = "i=1011";
    pos = 65;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Int32_1dn", "");
        printf("\n");
    }

    nodeid = "i=1012";
    pos = 66;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInt64_1dn", "");
        printf("\n");
    }

    nodeid = "i=1013";
    pos = 67;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Float_1dn", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    pos = 68;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "GK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    pos = 69;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ASK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    pos = 70;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBKK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    pos = 71;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBZCRQ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    pos = 72;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBZ-AK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    pos = 73;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBZCRQ-AK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    pos = 74;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SendCommand", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    pos = 75;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "OffBlocking-K", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    pos = 76;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "OffBlocking-CC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    pos = 77;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "GZ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    pos = 78;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SZ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    pos = 79;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBZ-CC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    pos = 80;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBZCRQ-CC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    pos = 81;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XBZC-CC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    pos = 82;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "GK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    pos = 83;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ASK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    pos = 84;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SendCommand", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    pos = 85;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "OffBlocking-K", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    pos = 86;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "OffBlocking-CC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    pos = 87;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SZ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    pos = 88;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "GZ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    pos = 89;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "GK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    pos = 90;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "NWK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    pos = 91;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RWK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    pos = 92;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "LK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    pos = 93;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SendCommand", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    pos = 94;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "OffBlocking-K", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    pos = 95;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "OffBlocking-CC", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    pos = 96;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "NWZ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    pos = 97;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "RWZ", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    pos = 98;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "TK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    pos = 99;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "TK", "");
        printf("\n");
    }

    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    pos = 100;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "TK", "");
        printf("\n");
    }

    nodeid = "i=62";
    pos = 101;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BaseVariableType", "");
        printf("\n");
    }

    nodeid = "i=63";
    pos = 102;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BaseDataVariableType", "");
        printf("\n");
    }

    nodeid = "i=58";
    pos = 103;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BaseObjectType", "");
        printf("\n");
    }

    nodeid = "i=61";
    pos = 104;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "FolderType", "");
        printf("\n");
    }

    nodeid = "i=31";
    pos = 105;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "References", "");
        printf("\n");
    }

    nodeid = "i=24";
    pos = 106;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "BaseDataType", "");
        printf("\n");
    }

    nodeid = "i=26";
    pos = 107;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Number", "");
        printf("\n");
    }

    nodeid = "i=27";
    pos = 108;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Integer", "");
        printf("\n");
    }

    nodeid = "i=28";
    pos = 109;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInteger", "");
        printf("\n");
    }

    nodeid = "i=29";
    pos = 110;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Enumeration", "");
        printf("\n");
    }

    nodeid = "i=1";
    pos = 111;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Boolean", "");
        printf("\n");
    }

    nodeid = "i=2";
    pos = 112;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "SByte", "");
        printf("\n");
    }

    nodeid = "i=3";
    pos = 113;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Byte", "");
        printf("\n");
    }

    nodeid = "i=4";
    pos = 114;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Int16", "");
        printf("\n");
    }

    nodeid = "i=5";
    pos = 115;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInt16", "");
        printf("\n");
    }

    nodeid = "i=6";
    pos = 116;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Int32", "");
        printf("\n");
    }

    nodeid = "i=7";
    pos = 117;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInt32", "");
        printf("\n");
    }

    nodeid = "i=8";
    pos = 118;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Int64", "");
        printf("\n");
    }

    nodeid = "i=9";
    pos = 119;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "UInt64", "");
        printf("\n");
    }

    nodeid = "i=10";
    pos = 120;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Float", "");
        printf("\n");
    }

    nodeid = "i=11";
    pos = 121;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Double", "");
        printf("\n");
    }

    nodeid = "i=12";
    pos = 122;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "String", "");
        printf("\n");
    }

    nodeid = "i=13";
    pos = 123;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "DateTime", "");
        printf("\n");
    }

    nodeid = "i=14";
    pos = 124;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Guid", "");
        printf("\n");
    }

    nodeid = "i=15";
    pos = 125;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ByteString", "");
        printf("\n");
    }

    nodeid = "i=16";
    pos = 126;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "XmlElement", "");
        printf("\n");
    }

    nodeid = "i=17";
    pos = 127;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "NodeId", "");
        printf("\n");
    }

    nodeid = "i=18";
    pos = 128;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "ExpandedNodeId", "");
        printf("\n");
    }

    nodeid = "i=19";
    pos = 129;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "StatusCode", "");
        printf("\n");
    }

    nodeid = "i=20";
    pos = 130;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "QualifiedName", "");
        printf("\n");
    }

    nodeid = "i=21";
    pos = 131;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "LocalizedText", "");
        printf("\n");
    }

    nodeid = "i=22";
    pos = 132;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Structure", "");
        printf("\n");
    }

    nodeid = "i=23";
    pos = 133;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "DataValue", "");
        printf("\n");
    }

    nodeid = "i=25";
    pos = 134;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "DiagnosticInfo", "");
        printf("\n");
    }

    nodeid = "i=30";
    pos = 135;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Image", "");
        printf("\n");
    }

    nodeid = "i=295";
    pos = 136;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "LocaleId", "");
        printf("\n");
    }

    nodeid = "i=121";
    pos = 137;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp)
    {
        bres = false;
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else
    {
        printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
        compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 - 1], "Decimal128", "");
        printf("\n");
    }

    return bres;
}

int main(void)
{
    bool bres = true;
    bres = test_browsename();
    bres = bres && test_value();
    bres = bres && test_reference();
    bres = bres && test_Description();
    bres = bres && test_DisplayName();
    if (bres == false)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
