/**
 * C Bindings for the S2OPC Client Wrapper library
 *
 * Shall not be used directly by JS user
 * use higher level functions instead
 *
 * C-JS translation made using nodeFFI and ref packages
 *
 * @module:bind_sopc_client
 */
const ffi = require('ffi');
const ref = require('ref');
const StructType = require('ref-struct');
const ArrayType = require('ref-array');
const UnionType = require('ref-union');
const Enum = require('enum');

const Bool_Array = ArrayType('bool');
const Int8_Array = ArrayType('int8');
const Int16_Array = ArrayType('int16');
const Int32_Array = ArrayType('int32');
const Int64_Array = ArrayType('int64');
const Byte_Array = ArrayType('uint8');
const Uint16_Array = ArrayType('uint16');
const Uint32_Array = ArrayType('uint32');
const Uint64_Array = ArrayType('uint64');
const Float_Array = ArrayType('float');
const Double_Array = ArrayType('double');
const CStringArray = ArrayType(ref.types.CString);

const uint8_ptr = ref.refType('uint8');
const int32_ptr = ref.refType('int32');
const uint32_ptr = ref.refType('uint32');
const int64_ptr = ref.refType('int64');

const SOPC_ClientHelper_SecurityCfg = StructType({
    security_policy: 'CString',
    security_mode: 'int32',
    path_cert_auth: 'CString',
    path_cert_srv: 'CString',
    path_cert_cli: 'CString',
    path_key_cli: 'CString',
    policyId: 'CString',
    username: 'CString',
    password: 'CString'
});

const SOPC_BuiltinId = new Enum({
    'SOPC_Null_Id': 0,
    'SOPC_Boolean_Id': 1,
    'SOPC_SByte_Id': 2,
    'SOPC_Byte_Id': 3,
    'SOPC_Int16_Id': 4,
    'SOPC_UInt16_Id': 5,
    'SOPC_Int32_Id': 6,
    'SOPC_UInt32_Id': 7,
    'SOPC_Int64_Id': 8,
    'SOPC_UInt64_Id': 9,
    'SOPC_Float_Id': 10,
    'SOPC_Double_Id': 11,
    'SOPC_String_Id': 12,
    'SOPC_DateTime_Id': 13,
    'SOPC_Guid_Id': 14,
    'SOPC_ByteString_Id': 15,
    'SOPC_XmlElement_Id': 16,
    'SOPC_NodeId_Id': 17,
    'SOPC_ExpandedNodeId_Id': 18,
    'SOPC_StatusCode_Id': 19,
    'SOPC_QualifiedName_Id': 20,
    'SOPC_LocalizedText_Id': 21,
    'SOPC_ExtensionObject_Id': 22,
    'SOPC_DataValue_Id': 23,
    'SOPC_Variant_Id': 24,
    'SOPC_DiagnosticInfo_Id': 25
});

const SOPC_VariantArrayType = new Enum({
    'SOPC_VariantArrayType_SingleValue': 0,
    'SOPC_VariantArrayType_Array': 1,
    'SOPC_VariantArrayType_Matrix': 2,
});

const SOPC_String = StructType({
    length: 'int32',
    do_not_clear: 'bool',
    data: uint8_ptr
});

const SOPC_String_Array = ArrayType(SOPC_String);

const SOPC_Guid = StructType({
    data1: 'uint32',
    data2: 'uint16',
    data3: 'uint16',
    data4: uint8_ptr
});

const SOPC_GuidPtr = ref.refType(SOPC_Guid);

const SOPC_IdentifierType = new Enum({
    'SOPC_IdentifierType_Numeric': 0,
    'SOPC_IdentifierType_String': 1,
    'SOPC_IdentifierType_Guid': 2,
    'SOPC_IdentifierType_ByteString': 3,
});

const SOPC_NodeIdData = UnionType({
    numeric: 'uint32',
    string : SOPC_String,
    guid : SOPC_GuidPtr,
    bstring : SOPC_String
});

const SOPC_NodeId = StructType({
    id_type : 'int32',
    ns : 'uint16',
    data : SOPC_NodeIdData
});
const SOPC_NodeIdPtr = ref.refType(SOPC_NodeId);

const SOPC_QualifiedName = StructType({
    ns: 'uint16',
    string: SOPC_String
});
const SOPC_QualifiedNamePtr = ref.refType(SOPC_QualifiedName);

const SOPC_LocalizedText = StructType({
    locale: SOPC_String,
    text: SOPC_String
});

const SOPC_LocalizedTextPtr = ref.refType(SOPC_LocalizedText);

const SOPC_VariantArrayValue = UnionType({
    //TODO to fill
    booleanArr : Bool_Array,
    sByteArr : Int8_Array,
    byteArr : Byte_Array,
    int16Arr : Int16_Array,
    uint16Arr : Uint16_Array,
    int32Arr : Int32_Array,
    uint32Arr : Uint32_Array,
    int64Arr : Int64_Array,
    uint64Arr : Uint64_Array,
    floatVArr : Float_Array,
    doubleVArr : Double_Array,
    stringArr : SOPC_String_Array,
    dateArr : int64_ptr,
    //guidArr : SOPC_Guid_ptr,
    bStringArr : SOPC_String_Array,
    xmlEltArr : SOPC_String_Array,
    nodeIdArr : SOPC_NodeIdPtr,
    //expNodeIdArr : SOPC_ExpandedNodeId_ptr,
    statusArr : uint32_ptr,
    //qNameArr : SOPC_QualifiedName_ptr,
    //localizedTextArr : SOPC_LocalizedText_ptr,
    //extObjectArr : SOPC_ExtensionObject_ptr,
    //dataValueArr : SOPC_DataValue_ptr,
    //variantArr : SOPC_Variant_ptr,
    //diagInfoArr : SOPC_DiagnosticInfo_ptr
});

const SOPC_VariantArray = StructType({
    length: 'int32',
    content: SOPC_VariantArrayValue
});

const SOPC_VariantMatrix = StructType({
    dimensions : 'int32',
    arrayDimensions : int32_ptr,
    content : SOPC_VariantArrayValue
});

const SOPC_VariantValue = UnionType({
    //TODO to fill
    bool: 'bool',
    sbyte: 'int8',
    bytev: 'uint8',
    int16: 'int16',
    uint16: 'uint16',
    int32: 'int32',
    uint32: 'uint32',
    int64: 'int64',
    uint64: 'uint64',
    floatv: 'float',
    doublev: 'double',
    string: SOPC_String,
    date_time: 'int64',
    //guid: SOPC_GuidPtr,
    bstring: SOPC_String,
    xml_elt: SOPC_String,
    node_id: SOPC_NodeIdPtr,
    //exp_node_id: SOPC_ExpandedNodeIdPtr,
    status: 'uint32',
    qname: SOPC_QualifiedNamePtr,
    localized_text: SOPC_LocalizedTextPtr,
    //extension_obj: SOPC_ExtensionObjectPtr,
    //data_value: SOPC_DataValuePtr,
    //diag_info: SOPC_DiagnosticInfoPtr,
    array: SOPC_VariantArray,
    matrix: SOPC_VariantMatrix
});

const SOPC_Variant = StructType({
    do_not_clear: 'bool',
    built_in_type_id: 'int32',
    variant_array_type: 'int32',
    variant_value: SOPC_VariantValue
});

const SOPC_DataValue = StructType({
    value: SOPC_Variant,
    status: 'uint32',
    src_ts: 'int64',
    srv_ts: 'int64',
    src_ps: 'uint16',
    srv_ps: 'uint16'
});

const SOPC_DataValue_ptr = ref.refType(SOPC_DataValue);
const SOPC_DataValuePtrArray = ArrayType(SOPC_DataValue_ptr);

const SOPC_ClientHelper_WriteValue = StructType({
    nodeId : 'CString',
    indexRange : 'CString',
    value : SOPC_DataValue_ptr
});

const SOPC_ClientHelper_WriteValueArray = ArrayType(SOPC_ClientHelper_WriteValue);
const UInt32Array = ArrayType('uint32');

const SOPC_ClientHelper_ReadValue = StructType({
    nodeId : 'CString',
    attributeId : 'uint32',
    indexRange : 'CString'
});

const SOPC_ClientHelper_ReadValueArray = ArrayType(SOPC_ClientHelper_ReadValue);

const SOPC_ClientHelper_BrowseRequest = StructType({
    nodeId : 'CString',
    direction : 'int32',
    referenceTypeId : 'CString',
    includeSubtypes : 'bool'
});

const SOPC_ClientHelper_BrowseRequestArray = ArrayType(SOPC_ClientHelper_BrowseRequest);

const SOPC_ClientHelper_BrowseResultReference = StructType({
    referenceTypeId : 'CString',
    isForward : 'bool',
    nodeId : 'CString',
    browseName : 'CString',
    displayName : 'CString',
    nodeClass : 'int32'
});
const SOPC_ClientHelper_BrowseResultReferenceArray = ArrayType(SOPC_ClientHelper_BrowseResultReference);
const SOPC_ClientHelper_BrowseResult = StructType({
    statusCode : 'uint32',
    nbOfReferences : 'int32',
    references : SOPC_ClientHelper_BrowseResultReferenceArray
});
const SOPC_ClientHelper_BrowseResultArray = ArrayType(SOPC_ClientHelper_BrowseResult);

const sopc_client = ffi.Library('libclient_subscription', {
    'SOPC_ClientHelper_Initialize': ['int32', ['CString', 'int32']],
    'SOPC_ClientHelper_Finalize': ['void', []],
    'SOPC_ClientHelper_Connect': ['int32', ['CString', SOPC_ClientHelper_SecurityCfg]],
    'SOPC_ClientHelper_Disconnect': ['int32', ['int32']],
    'SOPC_ClientHelper_CreateSubscription': ['int32', ['int32', 'pointer']],
    'SOPC_ClientHelper_AddMonitoredItems': ['int32', ['int32', CStringArray, 'size_t']],
    'SOPC_ClientHelper_Unsubscribe': ['int32', ['int32']],
    'SOPC_ClientHelper_Write': ['int32', ['int32', SOPC_ClientHelper_WriteValueArray, 'size_t', UInt32Array]],
    'SOPC_ClientHelper_Read': ['int32', ['int32', SOPC_ClientHelper_ReadValueArray, 'size_t', SOPC_DataValuePtrArray]],
    'SOPC_ClientHelper_Browse': ['int32', ['int32', SOPC_ClientHelper_BrowseRequestArray, 'size_t', SOPC_ClientHelper_BrowseResultArray]]
});

module.exports = {
    sopc_client,
    CStringArray,
    UInt32Array,
    security_cfg : SOPC_ClientHelper_SecurityCfg,
    SOPC_String,
    SOPC_String_Array,
    SOPC_DataValue,
    SOPC_DataValuePtrArray,
    SOPC_BuiltinId,
    SOPC_IdentifierType,
    SOPC_VariantArrayType,
    SOPC_VariantValue,
    SOPC_VariantArrayValue,
    SOPC_VariantArray,
    SOPC_Variant,
    SOPC_ClientHelper_WriteValue,
    SOPC_ClientHelper_ReadValue,
    SOPC_ClientHelper_BrowseRequest,
    SOPC_ClientHelper_BrowseResultReference,
    SOPC_ClientHelper_BrowseResult,
    SOPC_ClientHelper_BrowseResultArray
};
