const bind = require('./bind_sopc_client');
const ref = require('ref');

function valueFromCSingle(type_id, variant) {
    var value = null;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
            value = variant.variant_value.bool;
            break;
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value:
            value = variant.variant_value.sbyte;
            break;
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value:
            value = variant.variant_value.bytev;
            break;
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value:
            value = variant.variant_value.int16;
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value:
            value = variant.variant_value.uint16;
            break;
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
            value = variant.variant_value.int32;
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value:
            value = variant.variant_value.uint32;
            break;
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value:
            value = variant.variant_value.int64;
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value:
            value = variant.variant_value.uint64;
            break;
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value:
            value = variant.variant_value.floatv;
            break;
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value:
            value = variant.variant_value.doublev;
            break;
        case bind.SOPC_BuiltinId.SOPC_String_Id.value:
            value = ref.readCString(variant.variant_value.string.data);
            break;
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value:
            throw Error("DateTime variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value:
            throw Error("Guid variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
            value = ref.readCString(variant.variant_value.bstring.data);
            break;
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value:
            value = ref.readCString(variant.variant_value.xml_elt.data);
            break;
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value:
            if (!ref.isNull(variant.variant_value.node_id)) {
                var node_id = variant.variant_value.node_id.deref();
                switch (node_id.id_type) {
                    case bind.SOPC_IdentifierType.SOPC_IdentifierType_Numeric.value:
                        value = `ns=${node_id.ns};i=${node_id.data.numeric}`
                        break;
                    case bind.SOPC_IdentifierType.SOPC_IdentifierType_String.value:
                        var node_id_string = ref.readCString(node_id.data.string.data);
                        value = `ns=${node_id.ns};s=${node_id_string}`;
                        break;
                    default:
                        throw Error("NodeId id type not supported");
                }
                break;
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value:
            throw Error("ExpandedNodeId variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value:
            throw Error("StatusCode variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value:
            if (!ref.isNull(variant.variant_value.qname)) {
                var qualified_name = variant.variant_value.qname.deref();
                var string_value = ref.readCString(qualified_name.string.data);
                var ns = qualified_name.ns;
                value = `ns=${ns};s=${string_value}`;
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value:
            if (!ref.isNull(variant.variant_value.localized_text)) {
                var local_text = variant.variant_value.localized_text.deref();
                var locale = "not_defined";
                if (!ref.isNull(local_text.locale.data)) {
                    ref.readCString(local_text.locale.data);
                }
                var text = ref.readCString(local_text.text.data);
                value = `LANG=${locale};TEXT=${text}`;
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value:
            throw Error("Extension Object variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DataValueId.value:
            throw Error("Data Value variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value:
            throw Error("Variant variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value:
            throw Error("DiagnosticInfo type not supported");
        default:
            throw Error("Variant type with id ", this.type_id, " is not managed");
    }
    return value;
}

function valueFromCArray(type_id, variant) {
    var value = [];
    var count = 0;
    var length = variant.variant_value.array.length;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
            variant.variant_value.array.content.booleanArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.booleanArr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value:
            variant.variant_value.array.content.sByteArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.sByteArr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value:
            variant.variant_value.array.content.byteArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.byteArr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value:
            variant.variant_value.array.content.int16Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.int16Arr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value:
            variant.variant_value.array.content.uint16Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.uint16Arr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
            variant.variant_value.array.content.int32Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.int32Arr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value:
            variant.variant_value.array.content.uint32Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.uint32Arr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value:
            variant.variant_value.array.content.int64Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.int64Arr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value:
            variant.variant_value.array.content.uint64Arr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.uint64Arr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value:
            variant.variant_value.array.content.floatVArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.floatVArr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value:
            variant.variant_value.array.content.doubleVArr.length = length;
            for (count = 0; count < length; count++){
                value.push(variant.variant_value.array.content.doubleVArr[count]);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_String_Id.value:
            variant.variant_value.array.content.stringArr.length = length;
            for (count = 0; count < length; count++){
                var s = variant.variant_value.array.content.stringArr[count].data;
                var s_read = ref.readCString(s);
                value.push(s_read);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value:
            throw Error("DateTime variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value:
            throw Error("Guid variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
            variant.variant_value.array.content.bStringArr.length = length;
            for (count = 0; count < length; count++){
                var bs = variant.variant_value.array.content.bStringArr[count].data;
                var bs_read = ref.readCString(bs);
                value.push(bs_read);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value:
            variant.variant_value.array.content.xmlEltArr.length = length;
            for (count = 0; count < length; count++){
                var xs = variant.variant_value.array.content.xmlEltArr[count].data;
                var xs_read = ref.readCString(xs);
                value.push(xs_read);
            }
            break;
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value:
            throw Error("NodeId Array type not supported");
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value:
            throw Error("ExpandedNodeId variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value:
            throw Error("StatusCode variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value:
            throw Error("QualifiedName Array type not supported");
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value:
            throw Error("LocalizedText Array type not supported");
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value:
            throw Error("Extension Object variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DataValueId.value:
            throw Error("Data Value variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value:
            throw Error("Variant variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value:
            throw Error("DiagnosticInfo type not supported");
        default:
            throw Error("Variant type with id ", this.type_id, " is not managed");
    }
    return value;
}

function ValueToCSingle(type_id, value) {
    var variant_value = null;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                bool: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                sbyte: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                bytev: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                int16: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                uint16: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                int32: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                uint32: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                int64: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                uint64: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                floatv: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value:
            variant_value = new bind.SOPC_VariantValue({
                doublev: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_String_Id.value:
            var buffer = new Buffer(value.length + 1);
            ref.writeCString(buffer, 0, value);
            variant_value = new bind.SOPC_VariantValue({
                string: new bind.SOPC_String({
                    length: value.length,
                    do_not_clear: true,
                    data: buffer
                })
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value:
            throw Error("DateTime variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value:
            throw Error("Guid variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
            var bs_buffer = new Buffer(value.length + 1);
            ref.writeCString(bs_buffer, 0, value);
            variant_value = new bind.SOPC_VariantValue({
                string: new bind.SOPC_String({
                    length: value.length,
                    do_not_clear: true,
                    data: bs_buffer
                })
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value:
            var xml_buffer = new Buffer(value.length + 1);
            ref.writeCString(xml_buffer, 0, value);
            variant_value = new bind.SOPC_VariantValue({
                string: new bind.SOPC_String({
                    length: value.length,
                    do_not_clear: true,
                    data: xml_buffer
                })
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value:
            throw Error("Node Id type not supported");
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value:
            throw Error("ExpandedNodeId variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value:
            throw Error("StatusCode variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value:
            throw Error("QualifiedName type not supported");
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value:
            throw Error("LocalizedText type not supported");
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value:
            throw Error("Extension Object variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DataValueId.value:
            throw Error("Data Value variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value:
            throw Error("Variant variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value:
            throw Error("DiagnosticInfo type not supported");
        default:
            throw Error("Variant type with id ", type_id, " is not managed");
    }
    return variant_value;
}

function ValueToCArray(type_id, value) {
    var variant_value = null;
    var array_content = null;
    switch (type_id) {
        case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                booleanArr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_SByte_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                sByteArr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Byte_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                byteArr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Int16_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                int16Arr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                uint16Arr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                int32Arr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                uint32Arr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Int64_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                int64Arr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                uint64Arr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Float_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                floatVArr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_Double_Id.value:
            array_content = new bind.SOPC_VariantArrayValue({
                doubleVArr: value
            });
            break;
        case bind.SOPC_BuiltinId.SOPC_String_Id.value:
            var string_array = new bind.SOPC_String_Array(value.length);
            for (var count=0; count < value.length; count++) {
                var buffer = new Buffer(value[count].length + 1);
                ref.writeCString(buffer, 0, value[count]);
                var string_value = new bind.SOPC_String({
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
        case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value:
            throw Error("DateTime variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Guid_Id.value:
            throw Error("Guid variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
            var b_string_array = new bind.SOPC_String_Array(value.length);
            for (var b_count=0; b_count < value.length; b_count++) {
                var bs_buffer = new Buffer(value[b_count].length + 1);
                ref.writeCString(bs_buffer, 0, value[b_count]);
                var b_string_value = new bind.SOPC_String({
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
        case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value:
            var x_string_array = new bind.SOPC_String_Array(value.length);
            for (var x_count=0; x_count < value.length; x_count++) {
                var x_buffer = new Buffer(value[x_count].length + 1);
                ref.writeCString(x_buffer, 0, value[x_count]);
                var x_string_value = new bind.SOPC_String({
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
        case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value:
            throw Error("Node Id type not supported");
        case bind.SOPC_BuiltinId.SOPC_ExpandedNodeId_Id.value:
            throw Error("ExpandedNodeId variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_StatusCode_Id.value:
            throw Error("StatusCode variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_QualifiedName_Id.value:
            throw Error("QualifiedName type not supported");
        case bind.SOPC_BuiltinId.SOPC_LocalizedText_Id.value:
            throw Error("LocalizedText type not supported");
        case bind.SOPC_BuiltinId.SOPC_ExtensionObject_Id.value:
            throw Error("Extension Object variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DataValueId.value:
            throw Error("Data Value variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_Variant_Id.value:
            throw Error("Variant variant type not supported");
        case bind.SOPC_BuiltinId.SOPC_DiagnosticInfo_Id.value:
            throw Error("DiagnosticInfo type not supported");
        default:
            throw Error("Variant type with id ", type_id, " is not managed");
    }
    if (null != array_content) {
            var array_value = new bind.SOPC_VariantArray({
                length : value.length,
                content : array_content
            });
            variant_value = new bind.SOPC_VariantValue({
                array: array_value
            });
    }
    return variant_value;
}

class Variant
{
    constructor() {
        this.type_id = 0;
        this.array_type = 0;
        this.value = 0;
    }

    setValue(type_id, array_type, value) {
        this.type_id = type_id;
        this.array_type = array_type;
        this.value = value;
        return this;
    }

    FromC(variant) {
        this.type_id = variant.built_in_type_id;
        this.array_type = variant.variant_array_type;
        this.value = null;
        if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_SingleValue.value == this.array_type) {
            this.value = valueFromCSingle(this.type_id, variant);
        }
        else if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_Array.value == this.array_type)
        {
            this.value = valueFromCArray(this.type_id, variant);
        }
        else
        {
            throw Error("Unsupported VariantArrayType");
        }
        return this;
    }

    ToC() {
        var variant_value = ref.NULL;
        if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_SingleValue.value == this.array_type) {
            variant_value = ValueToCSingle(this.type_id, this.value);
        }
        else if (bind.SOPC_VariantArrayType.SOPC_VariantArrayType_Array.value == this.array_type)
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
