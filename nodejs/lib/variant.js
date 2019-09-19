/**
 * Variant Object
 *
 * @module variant
 */
const bind = require('./bind_sopc_client');
const ref = require('ref');

/**
 * Internal helper function used to convert single C variant value to JS
 * @param {Number} type_id type identifier
 * @param {C_Variant} variant C variant (containing a single value)
 * @return JS Variant value
 */
function valueFromCSingle(type_id, variant) {
    let value = null;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value: {
            value = variant.variant_value.bool;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value: {
            value = variant.variant_value.sbyte;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value: {
            value = variant.variant_value.bytev;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value: {
            value = variant.variant_value.int16;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value: {
            value = variant.variant_value.uint16;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value: {
            value = variant.variant_value.int32;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value: {
            value = variant.variant_value.uint32;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value: {
            value = variant.variant_value.int64;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value: {
            value = variant.variant_value.uint64;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value: {
            value = variant.variant_value.floatv;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value: {
            value = variant.variant_value.doublev;
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_String_Id.value: {
            value = ref.readCString(variant.variant_value.string.data);
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value: {
            throw Error("DateTime variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value: {
            throw Error("Guid variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value: {
            value = ref.readCString(variant.variant_value.bstring.data);
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value: {
            value = ref.readCString(variant.variant_value.xml_elt.data);
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value: {
            if (!ref.isNull(variant.variant_value.node_id)) {
                let node_id = variant.variant_value.node_id.deref();
                switch (node_id.id_type) {
                    case bind.SOPC_IdentifierType.SOPC_IdentifierType_Numeric.value: {
                        value = `ns=${node_id.ns};i=${node_id.data.numeric}`
                        break;
                    }
                    case bind.SOPC_IdentifierType.SOPC_IdentifierType_String.value: {
                        let node_id_string = ref.readCString(node_id.data.string.data);
                        value = `ns=${node_id.ns};s=${node_id_string}`;
                        break;
                    }
                    default: {
                        throw Error("NodeId id type not supported");
                    }
                }
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value: {
            throw Error("ExpandedNodeId variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value: {
            throw Error("StatusCode variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value: {
            if (!ref.isNull(variant.variant_value.qname)) {
                let qualified_name = variant.variant_value.qname.deref();
                let string_value = ref.readCString(qualified_name.string.data);
                let ns = qualified_name.ns;
                value = `ns=${ns};s=${string_value}`;
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value: {
            if (!ref.isNull(variant.variant_value.localized_text)) {
                let local_text = variant.variant_value.localized_text.deref();
                let locale = "not_defined";
                if (!ref.isNull(local_text.locale.data)) {
                    ref.readCString(local_text.locale.data);
                }
                let text = ref.readCString(local_text.text.data);
                value = `LANG=${locale};TEXT=${text}`;
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value: {
            throw Error("Extension Object variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DataValue_Id.value: {
            throw Error("Data Value variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value: {
            throw Error("Variant variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value: {
            throw Error("DiagnosticInfo type not supported");
        }
        default: {
            throw Error(`Variant type with id ${type_id} is not managed`);
        }
    }
    return value;
}

/**
 * Internal function used to convert C array variant value to JS
 * @param {Number} type_id type identifier
 * @param {C_Variant} variant C variant (containing an array)
 * @returns JS variant value
 */
function valueFromCArray(type_id, variant) {
    let value = [];
    let count = 0;
    let length = variant.variant_value.array.length;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value: {
            variant.variant_value.array.content.booleanArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.booleanArr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value: {
            variant.variant_value.array.content.sByteArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.sByteArr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value: {
            variant.variant_value.array.content.byteArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.byteArr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value: {
            variant.variant_value.array.content.int16Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.int16Arr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value: {
            variant.variant_value.array.content.uint16Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.uint16Arr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value: {
            variant.variant_value.array.content.int32Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.int32Arr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value: {
            variant.variant_value.array.content.uint32Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.uint32Arr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value: {
            variant.variant_value.array.content.int64Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.int64Arr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value: {
            variant.variant_value.array.content.uint64Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.uint64Arr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value: {
            variant.variant_value.array.content.floatVArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.floatVArr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value: {
            variant.variant_value.array.content.doubleVArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.doubleVArr[count]);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_String_Id.value: {
            variant.variant_value.array.content.stringArr.length = length;
            for (count = 0; count < length; count++){
                let s = variant.variant_value.array.content.stringArr[count].data;
                let s_read = ref.readCString(s);
                value.push(s_read);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value: {
            throw Error("DateTime variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value: {
            throw Error("Guid variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value: {
            variant.variant_value.array.content.bStringArr.length = length;
            for (count = 0; count < length; count++){
                let bs = variant.variant_value.array.content.bStringArr[count].data;
                let bs_read = ref.readCString(bs);
                value.push(bs_read);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value: {
            variant.variant_value.array.content.xmlEltArr.length = length;
            for (count = 0; count < length; count++){
                let xs = variant.variant_value.array.content.xmlEltArr[count].data;
                let xs_read = ref.readCString(xs);
                value.push(xs_read);
            }
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value: {
            throw Error("NodeId Array type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value: {
            throw Error("ExpandedNodeId variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value: {
            throw Error("StatusCode variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value: {
            throw Error("QualifiedName Array type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value: {
            throw Error("LocalizedText Array type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value: {
            throw Error("Extension Object variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DataValue_Id.value: {
            throw Error("Data Value variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value: {
            throw Error("Variant variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value: {
            throw Error("DiagnosticInfo type not supported");
        }
        default: {
            throw Error(`Variant type with id ${type_id} is not managed`);
        }
    }
    return value;
}

/**
 * Internal function used to convert JS single variant value to C variant value
 * @param {Number} type_id Type identifier
 * @param {*} value value to be set in variant C data structure
 * @returns C Variant value
 */
function ValueToCSingle(type_id, value) {
    let variant_value = null;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                bool: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                sbyte: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                bytev: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                int16: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                uint16: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                int32: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                uint32: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                int64: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                uint64: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                floatv: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value: {
            variant_value = new bind.SOPC_VariantValue({
                doublev: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_String_Id.value: {
            let buffer = new Buffer(value.length + 1);
            ref.writeCString(buffer, 0, value);
            variant_value = new bind.SOPC_VariantValue({
                string: new bind.SOPC_String({
                    length: value.length,
                    do_not_clear: true,
                    data: buffer
                })
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value: {
            throw Error("DateTime variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value: {
            throw Error("Guid variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value: {
            let bs_buffer = new Buffer(value.length + 1);
            ref.writeCString(bs_buffer, 0, value);
            variant_value = new bind.SOPC_VariantValue({
                string: new bind.SOPC_String({
                    length: value.length,
                    do_not_clear: true,
                    data: bs_buffer
                })
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value: {
            let xml_buffer = new Buffer(value.length + 1);
            ref.writeCString(xml_buffer, 0, value);
            variant_value = new bind.SOPC_VariantValue({
                string: new bind.SOPC_String({
                    length: value.length,
                    do_not_clear: true,
                    data: xml_buffer
                })
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value: {
            throw Error("Node Id type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value: {
            throw Error("ExpandedNodeId variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value: {
            throw Error("StatusCode variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value: {
            throw Error("QualifiedName type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value: {
            throw Error("LocalizedText type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value: {
            throw Error("Extension Object variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DataValue_Id.value: {
            throw Error("Data Value variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value: {
            throw Error("Variant variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value: {
            throw Error("DiagnosticInfo type not supported");
        }
        default: {
            throw Error(`Variant type with id ${type_id} is not managed`);
        }
    }
    return variant_value;
}

/**
 * Internal function used to convert JS array variant value to C variant value
 * @param {Number} type_id Type identifier
 * @param {*[]} value array of values
 * @returns C variant value
 */
function ValueToCArray(type_id, value) {
    let variant_value = null;
    let array_content = null;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                booleanArr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                sByteArr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                byteArr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                int16Arr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                uint16Arr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                int32Arr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                uint32Arr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                int64Arr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                uint64Arr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                floatVArr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value: {
            array_content = new bind.SOPC_VariantArrayValue({
                doubleVArr: value
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_String_Id.value: {
            let string_array = new bind.SOPC_String_Array(value.length);
            for (let count=0; count < value.length; count++) {
                let buffer = new Buffer(value[count].length + 1);
                ref.writeCString(buffer, 0, value[count]);
                let string_value = new bind.SOPC_String({
                    length: value[count].length,
                    do_not_clear: true,
                    data: buffer
                });
                string_array[count] = string_value;
            }
            array_content = new bind.SOPC_VariantArrayValue({
                stringArr: string_array
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value: {
            throw Error("DateTime variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value: {
            throw Error("Guid variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value: {
            let b_string_array = new bind.SOPC_String_Array(value.length);
            for (let b_count=0; b_count < value.length; b_count++) {
                let bs_buffer = new Buffer(value[b_count].length + 1);
                ref.writeCString(bs_buffer, 0, value[b_count]);
                let b_string_value = new bind.SOPC_String({
                    length: value[b_count].length,
                    do_not_clear: true,
                    data: bs_buffer
                });
                b_string_array[b_count] = b_string_value;
            }
            array_content = new bind.SOPC_VariantArrayValue({
                bStringArr: b_string_array
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value: {
            let x_string_array = new bind.SOPC_String_Array(value.length);
            for (let x_count=0; x_count < value.length; x_count++) {
                let x_buffer = new Buffer(value[x_count].length + 1);
                ref.writeCString(x_buffer, 0, value[x_count]);
                let x_string_value = new bind.SOPC_String({
                    length: value[x_count].length,
                    do_not_clear: true,
                    data: x_buffer
                });
                x_string_array[x_count] = x_string_value;
            }
            array_content = new bind.SOPC_VariantArrayValue({
                xmlEltArr: x_string_array
            });
            break;
        }
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value: {
            throw Error("Node Id type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value: {
            throw Error("ExpandedNodeId variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value: {
            throw Error("StatusCode variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value: {
            throw Error("QualifiedName type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value: {
            throw Error("LocalizedText type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value: {
            throw Error("Extension Object variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DataValue_Id.value: {
            throw Error("Data Value variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value: {
            throw Error("Variant variant type not supported");
        }
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value: {
            throw Error("DiagnosticInfo type not supported");
        }
        default: {
            throw Error(`Variant type with id ${type_id} is not managed`);
        }
    }
    if (null !== array_content) {
            let array_value = new bind.SOPC_VariantArray({
                length : value.length,
                content : array_content
            });
            variant_value = new bind.SOPC_VariantValue({
                array: array_value
            });
    }
    return variant_value;
}

/** Class representing a Variant
 *
 * Supported array types: Single, Array
 *
 * Supported "Read" types:
 * - Single: Bool, Signed Byte, Byte, Int16, UInt16, Int32, UInt32, Int64, UInt64,
 *           Float, Double, String, ByteString, XmlElement, NodeId, LocalizedText,
 *           QualifiedName
 * - Array: Bool, Signed Byte, Byte, Int16, UInt16, Int32, UInt32, Int64, UInt64,
 *          Float, Double, String, ByteString, XmlElement
 * Supported "Write" types:
 * - Single: Bool, Signed Byte, Byte, Int16, UInt16, Int32, UInt32, Int64, UInt64,
 *           Float, Double, String, ByteString, XmlElement
 * - Array: Bool, Signed Byte, Byte, Int16, UInt16, Int32, UInt32, Int64, UInt64,
 *          Float, Double, String, ByteString, XmlElement
 *
 * Any unsupported type identifier or array type will cause an error
 * The variant value set by the user shall match the type identifier,
 * no type verification is made. Any incorrect type will result in an
 * unpredicted behavior.
 *
 * Note: UInt64 and Int64 value may be stored in a string (Number JS type is too small
 * to store some values)
 */
class Variant
{
    /**
     * Create an empty variant
     */
    constructor() {
        this.type_id = 0;
        this.array_type = 0;
        this.value = 0;
    }

    /**
     * set the variant value
     * @param {Number} type_id Type identifier
     * @param {Number} array_type Array Type
     * @param {*} value variant value
     * @returns this
     */
    setValue(type_id, array_type, value) {
        this.type_id = type_id;
        this.array_type = array_type;
        this.value = value;
        return this;
    }

    /**
     * Internal function used to convert C Variant to JS Variant
     * @param {C_Variant} variant C variant
     * @returns this
     */
    FromC(variant) {
        this.type_id = variant.built_in_type_id;
        this.array_type = variant.variant_array_type;
        this.value = null;
        if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_SingleValue.value === this.array_type) {
            this.value = valueFromCSingle(this.type_id, variant);
        }
        else if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_Array.value === this.array_type)
        {
            this.value = valueFromCArray(this.type_id, variant);
        }
        else
        {
            throw Error("Unsupported VariantArrayType");
        }
        return this;
    }

    /**
     * Internal function used to convert JS Variant to C Variant
     * @returns C_Variant
     */
    ToC() {
        let variant_value = ref.NULL;
        if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_SingleValue.value === this.array_type) {
            variant_value = ValueToCSingle(this.type_id, this.value);
        }
        else if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_Array.value === this.array_type)
        {
            variant_value = ValueToCArray(this.type_id, this.value);
        }
        else {
            Error(`VariantArrayType ${this.array_type} not supported`);
        }
        return new bind.SOPC_Variant({
            do_not_clear : false,
            built_in_type_id : this.type_id,
            variant_array_type : this.array_type,
            variant_value : variant_value
        });
    }
}

module.exports = {
    Variant
};
