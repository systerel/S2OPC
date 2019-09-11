const bind = require('./bind_sopc_client');
const ref = require('ref');

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
        switch(this.type_id)
        {
            case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
                this.value = variant.variant_value.bool;
                break;
            case bind.SOPC_BuiltinId.SOPC_SByte_Id.value:
                this.value = variant.variant_value.sbyte;
                break;
            case bind.SOPC_BuiltinId.SOPC_Byte_Id.value:
                this.value = variant.variant_value.bytev;
                break;
            case bind.SOPC_BuiltinId.SOPC_Int16_Id.value:
                this.value = variant.variant_value.int16;
                break;
            case bind.SOPC_BuiltinId.SOPC_UInt16_Id.value:
                this.value = variant.variant_value.uint16;
                break;
            case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
                this.value = variant.variant_value.int32;
                break;
            case bind.SOPC_BuiltinId.SOPC_UInt32_Id.value:
                this.value = variant.variant_value.uint32;
                break;
            case bind.SOPC_BuiltinId.SOPC_Int64_Id.value:
                this.value = variant.variant_value.int64;
                break;
            case bind.SOPC_BuiltinId.SOPC_UInt64_Id.value:
                this.value = variant.variant_value.uint64;
                break;
            case bind.SOPC_BuiltinId.SOPC_Float_Id.value:
                this.value = variant.variant_value.floatv;
                break;
            case bind.SOPC_BuiltinId.SOPC_Double_Id.value:
                this.value = variant.variant_value.doublev;
                break;
            case bind.SOPC_BuiltinId.SOPC_String_Id.value:
                this.value = ref.readCString(variant.variant_value.string.data);
                break;
            case bind.SOPC_BuiltinId.SOPC_DateTime_Id.value:
                throw Error("DateTime variant type not supported");
            case bind.SOPC_BuiltinId.SOPC_Guid_Id.value:
                throw Error("Guid variant type not supported");
            case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
                this.value = ref.readCString(variant.variant_value.bstring.data);
                break;
            case bind.SOPC_BuiltinId.SOPC_XmlElement_Id.value:
                this.value = ref.readCString(variant.variant_value.xml_elt.data);
                break;
            case bind.SOPC_BuiltinId.SOPC_NodeId_Id.value:
                if (!ref.isNull(variant.variant_value.node_id))
                {
                    var node_id = variant.variant_value.node_id.deref();
                    switch(node_id.id_type) {
                        case bind.SOPC_IdentifierType.SOPC_IdentifierType_Numeric.value:
                            this.value = `ns=${node_id.ns};i=${node_id.data.numeric}`
                            break;
                        case bind.SOPC_IdentifierType.SOPC_IdentifierType_String.value:
                            var node_id_string = ref.readCString(node_id.data.string.data);
                            this.value = `ns=${node_id.ns};s=${node_id_string}`;
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
                if(!ref.isNull(variant.variant_value.qname)) {
                    var qualified_name = variant.variant_value.qname.deref();
                    var string_value = ref.readCString(qualified_name.string.data);
                    var ns = qualified_name.ns;
                    this.value = `ns=${ns};s=${string_value}`;
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
                    this.value = `LANG=${locale};TEXT=${text}`;
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
        return this;
    }

    ToC() {
        var variant_value = ref.NULL;
        switch(this.type_id) {
            case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
                variant_value = new bind.SOPC_VariantValue({
                    int32: this.value
                });
                break;
            default:
                throw Error("Variant.setValue: TODO type not managed");
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
