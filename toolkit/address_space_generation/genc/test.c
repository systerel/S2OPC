
#include <stdio.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_types.h"
#include "sopc_base_types.h"
#include "sopc_user_app_itf.h"

extern SOPC_AddressSpace addressSpace;
#define DisplayName addressSpace.displayNameArray
#define DisplayName_begin addressSpace.displayNameIdxArray_begin
#define DisplayName_end addressSpace.displayNameIdxArray_end
#define Description addressSpace.descriptionArray
#define Description_begin addressSpace.descriptionIdxArray_begin
#define Description_end addressSpace.descriptionIdxArray_end
#define DEFAULT_VARIANT  {SOPC_Null_Id, SOPC_VariantArrayType_SingleValue,{0}}

void test_browsename(){
    printf ("test BrowseName\n");
    const char *var;
    if (addressSpace.browseNameArray[1].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "VariablesFolderBn";if (strcmp((char*)addressSpace.browseNameArray[1].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[2].NamespaceIndex != 261) {printf("invalid BrowseName ") ;}  
var = "15361";if (strcmp((char*)addressSpace.browseNameArray[2].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[3].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SIGNALs";if (strcmp((char*)addressSpace.browseNameArray[3].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[4].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SWITCHs";if (strcmp((char*)addressSpace.browseNameArray[4].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[5].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "TRACKs";if (strcmp((char*)addressSpace.browseNameArray[5].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[6].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_G019";if (strcmp((char*)addressSpace.browseNameArray[6].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[7].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[7].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[8].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RC";if (strcmp((char*)addressSpace.browseNameArray[8].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[9].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_G025";if (strcmp((char*)addressSpace.browseNameArray[9].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[10].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[10].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[11].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RC";if (strcmp((char*)addressSpace.browseNameArray[11].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[12].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_G026";if (strcmp((char*)addressSpace.browseNameArray[12].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[13].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[13].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[14].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_W1";if (strcmp((char*)addressSpace.browseNameArray[14].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[15].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[15].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[16].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RC";if (strcmp((char*)addressSpace.browseNameArray[16].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[17].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_026TK";if (strcmp((char*)addressSpace.browseNameArray[17].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[18].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[18].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[19].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_OSTK";if (strcmp((char*)addressSpace.browseNameArray[19].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[20].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[20].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[21].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_P500";if (strcmp((char*)addressSpace.browseNameArray[21].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[22].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "BALA_RDLS_WBK_RDLN_EBK";if (strcmp((char*)addressSpace.browseNameArray[22].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[23].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RM";if (strcmp((char*)addressSpace.browseNameArray[23].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[24].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "Int64";if (strcmp((char*)addressSpace.browseNameArray[24].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[25].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "Uint32";if (strcmp((char*)addressSpace.browseNameArray[25].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[26].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "Double";if (strcmp((char*)addressSpace.browseNameArray[26].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[27].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "String";if (strcmp((char*)addressSpace.browseNameArray[27].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[28].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "ByteString";if (strcmp((char*)addressSpace.browseNameArray[28].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[29].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XmlElement";if (strcmp((char*)addressSpace.browseNameArray[29].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[30].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "GK";if (strcmp((char*)addressSpace.browseNameArray[30].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[31].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "ASK";if (strcmp((char*)addressSpace.browseNameArray[31].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[32].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBKK";if (strcmp((char*)addressSpace.browseNameArray[32].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[33].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBZCRQ";if (strcmp((char*)addressSpace.browseNameArray[33].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[34].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBZ-AK";if (strcmp((char*)addressSpace.browseNameArray[34].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[35].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBZCRQ-AK";if (strcmp((char*)addressSpace.browseNameArray[35].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[36].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SendCommand";if (strcmp((char*)addressSpace.browseNameArray[36].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[37].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "OffBlocking-K";if (strcmp((char*)addressSpace.browseNameArray[37].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[38].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "OffBlocking-CC";if (strcmp((char*)addressSpace.browseNameArray[38].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[39].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "GZ";if (strcmp((char*)addressSpace.browseNameArray[39].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[40].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SZ";if (strcmp((char*)addressSpace.browseNameArray[40].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[41].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBZ-CC";if (strcmp((char*)addressSpace.browseNameArray[41].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[42].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBZCRQ-CC";if (strcmp((char*)addressSpace.browseNameArray[42].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[43].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "XBZC-CC";if (strcmp((char*)addressSpace.browseNameArray[43].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[44].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "GK";if (strcmp((char*)addressSpace.browseNameArray[44].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[45].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "ASK";if (strcmp((char*)addressSpace.browseNameArray[45].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[46].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SendCommand";if (strcmp((char*)addressSpace.browseNameArray[46].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[47].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "OffBlocking-K";if (strcmp((char*)addressSpace.browseNameArray[47].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[48].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "OffBlocking-CC";if (strcmp((char*)addressSpace.browseNameArray[48].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[49].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SZ";if (strcmp((char*)addressSpace.browseNameArray[49].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[50].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "GZ";if (strcmp((char*)addressSpace.browseNameArray[50].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[51].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "GK";if (strcmp((char*)addressSpace.browseNameArray[51].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[52].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "NWK";if (strcmp((char*)addressSpace.browseNameArray[52].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[53].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RWK";if (strcmp((char*)addressSpace.browseNameArray[53].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[54].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "LK";if (strcmp((char*)addressSpace.browseNameArray[54].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[55].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "SendCommand";if (strcmp((char*)addressSpace.browseNameArray[55].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[56].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "OffBlocking-K";if (strcmp((char*)addressSpace.browseNameArray[56].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[57].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "OffBlocking-CC";if (strcmp((char*)addressSpace.browseNameArray[57].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[58].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "NWZ";if (strcmp((char*)addressSpace.browseNameArray[58].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[59].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "RWZ";if (strcmp((char*)addressSpace.browseNameArray[59].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[60].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "TK";if (strcmp((char*)addressSpace.browseNameArray[60].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[61].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "TK";if (strcmp((char*)addressSpace.browseNameArray[61].Name.Data, var)) {printf("invalid BrowseName ") ;}  
if (addressSpace.browseNameArray[62].NamespaceIndex != 0) {printf("invalid BrowseName ") ;}  
var = "TK";if (strcmp((char*)addressSpace.browseNameArray[62].Name.Data, var)) {printf("invalid BrowseName ") ;}  

}

void test_value(){
    printf ("test Value\n");
    int pos;
    const char* nodeid;
    SOPC_Byte builtInTypeId;
    const char *value;
    const char *val;
    
  pos = 1;
  
  nodeid = "s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
  printf("test Value for nodeid %s\n", nodeid);
  builtInTypeId = SOPC_Boolean_Id;

  if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  pos = 2;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 3;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 4;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 5;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 6;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 7;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 8;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 9;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 10;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 11;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 12;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 13;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 14;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 15;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 16;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 17;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 18;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 19;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 20;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 21;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 22;
  nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 23;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 24;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 25;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 26;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 27;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 28;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 29;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 30;
  nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
  printf("test Value for nodeid %s\n", nodeid);
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Null_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  if (!(addressSpace.valueArray[pos].Value.Boolean == 0)) {printf("invalid Value ") ;}

  pos = 31;
  
  nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
  printf("test Value for nodeid %s\n", nodeid);
  builtInTypeId = SOPC_Boolean_Id;

  if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  pos = 32;
  
  nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
  printf("test Value for nodeid %s\n", nodeid);
  builtInTypeId = SOPC_Boolean_Id;

  if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  pos = 33;
  
  nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
  printf("test Value for nodeid %s\n", nodeid);
  builtInTypeId = SOPC_Boolean_Id;

  if (!(addressSpace.valueArray[pos].BuiltInTypeId == builtInTypeId)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

  pos = 34;
  
  nodeid = "ns=1;i=2003";
  printf("test Value for nodeid %s\n", nodeid);  
  if (!(addressSpace.valueArray[pos].BuiltInTypeId == SOPC_Float_Id)) {printf("invalid BuiltInTypeId ") ;}

  if (!(addressSpace.valueArray[pos].ArrayType == SOPC_VariantArrayType_SingleValue)) {printf("invalid Arraytype ") ;}

}

void test_reference(){
    printf("test reference \n");
    int pos;
    int res;
    int exp;
    const char* nodeid;

    
    nodeid = "i=1000";
    pos = 1;
    exp = 8;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 85){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 85, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1001){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1001, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1002){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1002, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1003){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1003, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1004){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1004, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   7, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1005){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1005, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   8, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 8 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 8 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1006){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1006, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;i=15361";
    pos = 2;
    exp = 5;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 85){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 85, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs";
    pos = 3;
    exp = 5;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 15361){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026 for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs";
    pos = 4;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 15361){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs";
    pos = 5;
    exp = 6;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 15361){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 15361, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_P500")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_P500 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019";
    pos = 6;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   7, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM";
    pos = 7;
    exp = 8;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   7, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   8, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 8 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 8 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC";
    pos = 8;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   7, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025";
    pos = 9;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   7, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM";
    pos = 10;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC";
    pos = 11;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026";
    pos = 12;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026.RM for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM";
    pos = 13;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1";
    pos = 14;
    exp = 7;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   6, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 6 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   7, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 7 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM";
    pos = 15;
    exp = 5;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   5, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 5 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC";
    pos = 16;
    exp = 4;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   4, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 4 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK";
    pos = 17;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK.RM for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM";
    pos = 18;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK";
    pos = 19;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK.RM for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM";
    pos = 20;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500";
    pos = 21;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK";
    pos = 22;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM for %s \n" , nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM";
    pos = 23;
    exp = 3;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 35){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 35, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 61){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 61, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   3, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 3 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK for %s \n" , nodeid);}
            
}

    }


    nodeid = "i=1001";
    pos = 24;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1000){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "i=1002";
    pos = 25;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1000){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "i=1003";
    pos = 26;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1000){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "i=1004";
    pos = 27;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1000){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "i=1005";
    pos = 28;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1000){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "i=1006";
    pos = 29;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 1000){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 1000, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    pos = 30;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    pos = 31;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    pos = 32;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    pos = 33;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    pos = 34;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    pos = 35;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    pos = 36;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    pos = 37;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    pos = 38;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    pos = 39;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    pos = 40;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    pos = 41;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    pos = 42;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    pos = 43;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G019.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G019.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    pos = 44;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    pos = 45;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    pos = 46;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    pos = 47;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    pos = 48;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    pos = 49;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    pos = 50;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G025.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G025.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    pos = 51;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SIGNALs.BALA_RDLS_G026.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SIGNALs.BALA_RDLS_G026.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    pos = 52;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    pos = 53;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    pos = 54;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    pos = 55;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    pos = 56;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    pos = 57;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1 for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    pos = 58;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    pos = 59;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.SWITCHs.BALA_RDLS_W1.RC")!=0) {printf ("Invalid nodeId expected Objects.15361.SWITCHs.BALA_RDLS_W1.RC for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    pos = 60;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_026TK.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_026TK.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    pos = 61;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_OSTK.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_OSTK.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    pos = 62;
    exp = 2;
    res = addressSpace.referenceIdxArray_end[pos] - addressSpace.referenceIdxArray_begin[pos] + 1 ;
    if (res != exp) {
        printf("Invalid number of reference expected %d result %d : nodeid %s\n", exp, res, nodeid);
    }
    else{
        
            printf("test reference %d node %d nodeid %s\n",   1, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 47){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 47, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 1 -1 ])->NodeId;

    
int nsIndex = 261;
        
    if (res-> IdentifierType != IdentifierType_String ||
        res-> Namespace != nsIndex ||
        strcmp((char*)res-> Data.String.Data, "Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM")!=0) {printf ("Invalid nodeId expected Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM for %s \n" , nodeid);}
            
}

            printf("test reference %d node %d nodeid %s\n",   2, pos, nodeid);
            
{
SOPC_NodeId* res = addressSpace.referenceTypeArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ];

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 40){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 40, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

{
SOPC_NodeId* res = &(addressSpace.referenceTargetArray[addressSpace.referenceIdxArray_begin[pos] + 2 -1 ])->NodeId;

    
int nsIndex = 0;
        
    if (res-> IdentifierType != IdentifierType_Numeric ||
        res-> Namespace != nsIndex ||
        res-> Data.Numeric != 63){printf ("Invalid nodeId expected (%d, i, %d) result (%d,%d,%d) for %s \n", nsIndex, 63, res-> Namespace, res-> IdentifierType, res-> Data.Numeric, nodeid);}
            
}

    }


}

int compareLocalizedText(SOPC_LocalizedText LText, const char *text, const char *locale){
    int i = 0;
    if (strcmp((char*)LText.Locale.Data, locale) != 0){
        printf("locale KO");
        i++;
    }
    else{
        printf("locale ok");
    }
    if (strcmp((char*)LText.Text.Data, text) != 0){
        printf(", text KO");
        i++;
    }
    else{
        printf(", text ok");
    }
    return i;
}


void test_Description(){
    printf("test Description\n");
    int pos;
    int res;
    int exp;
    const char* nodeid;
    
    nodeid = "i=1000";
    pos = 1;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "VariablesFolderDescObj1d2", "");
            printf("\n");
        
    }


    nodeid = "ns=261;i=15361";
    pos = 2;
    exp = 2;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
            printf("test Description %d node %d nodeid %s : ", 2, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 2 -1], "NoName", "en");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs";
    pos = 3;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "en");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs";
    pos = 4;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs";
    pos = 5;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019";
    pos = 6;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM";
    pos = 7;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC";
    pos = 8;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025";
    pos = 9;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM";
    pos = 10;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC";
    pos = 11;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026";
    pos = 12;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM";
    pos = 13;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1";
    pos = 14;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM";
    pos = 15;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC";
    pos = 16;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK";
    pos = 17;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM";
    pos = 18;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK";
    pos = 19;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM";
    pos = 20;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500";
    pos = 21;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK";
    pos = 22;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM";
    pos = 23;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "NoName", "");
            printf("\n");
        
    }


    nodeid = "i=1001";
    pos = 24;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Int64_1d", "");
            printf("\n");
        
    }


    nodeid = "i=1002";
    pos = 25;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "UInt32_1d", "");
            printf("\n");
        
    }


    nodeid = "i=1003";
    pos = 26;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Double_1d", "");
            printf("\n");
        
    }


    nodeid = "i=1004";
    pos = 27;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "String_1d", "");
            printf("\n");
        
    }


    nodeid = "i=1005";
    pos = 28;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "ByteString_1d", "");
            printf("\n");
        
    }


    nodeid = "i=1006";
    pos = 29;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "XmlElement_1d", "");
            printf("\n");
        
    }


    nodeid = "s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    pos = 30;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Permissive Signal Status, ~é€bla", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    pos = 31;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Signal Approach Locking", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    pos = 32;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit blocking is in effect for the signal", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    pos = 33;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit Block Removal Request", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    pos = 34;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit blocking is rejected for the signal", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    pos = 35;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit block removal is rejected for the signal", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    pos = 36;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    pos = 37;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    pos = 38;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    pos = 39;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Signal request", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    pos = 40;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Signal cancel", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    pos = 41;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit Block Application Request", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    pos = 42;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit Block Removal Request", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    pos = 43;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Exit Block Removal - Acknowledge", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    pos = 44;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Permissive Signal Status", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    pos = 45;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Signal Approach Locking", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    pos = 46;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    pos = 47;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    pos = 48;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    pos = 49;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Signal cancel", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    pos = 50;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Signal request", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    pos = 51;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Permissive Signal Status", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    pos = 52;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Switch Detected Normal", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    pos = 53;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Switch Detected Reverse", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    pos = 54;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Switch Locally Locked", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    pos = 55;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    pos = 56;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    pos = 57;
    exp = 0;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    pos = 58;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Switch Calling in Normal Direction", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    pos = 59;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Switch Calling in Reverse Direction", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    pos = 60;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Secondary Detection Status", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    pos = 61;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Secondary Detection Status", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    pos = 62;
    exp = 1;
    res = Description_end[pos] - Description_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of Description expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test Description %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(Description[Description_begin[pos] + 1 -1], "Secondary Detection Status", "");
            printf("\n");
        
    }


}


void test_DisplayName(){
    printf("test DisplayName\n");
    int pos;
    int res;
    int exp;
    const char* nodeid;
    
    nodeid = "i=1000";
    pos = 1;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "VariablesFolderDn", "");
            printf("\n");
        
    }


    nodeid = "ns=261;i=15361";
    pos = 2;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "15361", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs";
    pos = 3;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SIGNALs", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs";
    pos = 4;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SWITCHs", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs";
    pos = 5;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "TRACKs", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019";
    pos = 6;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_G019", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM";
    pos = 7;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC";
    pos = 8;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025";
    pos = 9;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_G025", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM";
    pos = 10;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC";
    pos = 11;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026";
    pos = 12;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_G026", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM";
    pos = 13;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1";
    pos = 14;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_W1", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM";
    pos = 15;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC";
    pos = 16;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK";
    pos = 17;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_026TK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM";
    pos = 18;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK";
    pos = 19;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_OSTK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM";
    pos = 20;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_P500";
    pos = 21;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_P500", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK";
    pos = 22;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "BALA_RDLS_WBK_RDLN_EBK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM";
    pos = 23;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RM", "");
            printf("\n");
        
    }


    nodeid = "i=1001";
    pos = 24;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "Int64_1dn", "");
            printf("\n");
        
    }


    nodeid = "i=1002";
    pos = 25;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "UInt32_1dn", "");
            printf("\n");
        
    }


    nodeid = "i=1003";
    pos = 26;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "Double_1dn", "");
            printf("\n");
        
    }


    nodeid = "i=1004";
    pos = 27;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "String_1dn", "");
            printf("\n");
        
    }


    nodeid = "i=1005";
    pos = 28;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "ByteString_1dn", "");
            printf("\n");
        
    }


    nodeid = "i=1006";
    pos = 29;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XmlElement_1dn", "");
            printf("\n");
        
    }


    nodeid = "s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.GK";
    pos = 30;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "GK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK";
    pos = 31;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "ASK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBKK";
    pos = 32;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBKK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ";
    pos = 33;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBZCRQ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZ-AK";
    pos = 34;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBZ-AK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.XBZCRQ-AK";
    pos = 35;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBZCRQ-AK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand";
    pos = 36;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SendCommand", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K";
    pos = 37;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "OffBlocking-K", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC";
    pos = 38;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "OffBlocking-CC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.GZ";
    pos = 39;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "GZ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.SZ";
    pos = 40;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SZ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZ-CC";
    pos = 41;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBZ-CC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZCRQ-CC";
    pos = 42;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBZCRQ-CC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC.XBZC-CC";
    pos = 43;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "XBZC-CC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.GK";
    pos = 44;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "GK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RM.ASK";
    pos = 45;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "ASK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.SendCommand";
    pos = 46;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SendCommand", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-K";
    pos = 47;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "OffBlocking-K", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.OffBlocking-CC";
    pos = 48;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "OffBlocking-CC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.SZ";
    pos = 49;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SZ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G025.RC.GZ";
    pos = 50;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "GZ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G026.RM.GK";
    pos = 51;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "GK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.NWK";
    pos = 52;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "NWK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.RWK";
    pos = 53;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RWK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RM.LK";
    pos = 54;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "LK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.SendCommand";
    pos = 55;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "SendCommand", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-K";
    pos = 56;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "OffBlocking-K", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.OffBlocking-CC";
    pos = 57;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "OffBlocking-CC", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.NWZ";
    pos = 58;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "NWZ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.SWITCHs.BALA_RDLS_W1.RC.RWZ";
    pos = 59;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "RWZ", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_026TK.RM.TK";
    pos = 60;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "TK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_OSTK.RM.TK";
    pos = 61;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "TK", "");
            printf("\n");
        
    }


    nodeid = "ns=261;s=Objects.15361.TRACKs.BALA_RDLS_WBK_RDLN_EBK.RM.TK";
    pos = 62;
    exp = 1;
    res = DisplayName_end[pos] - DisplayName_begin[pos] + 1;
    if (res != exp){
        printf("Invalid number of DisplayName expected %d result %d : pos %d nodeid %s\n", exp, res, pos, nodeid);
    }
    else{
        
            printf("test DisplayName %d node %d nodeid %s : ", 1, pos, nodeid);
            compareLocalizedText(DisplayName[DisplayName_begin[pos] + 1 -1], "TK", "");
            printf("\n");
        
    }


}


int main (){
    test_browsename();
    test_value();
    test_reference();
    test_Description();
    test_DisplayName();
    printf("%zu %s\n", strlen((char *)Description[1].Text.Data), Description[1].Text.Data);
    return 0;
}

