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
                console.log("DataValue Boolean: TODO");
                break;
            case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
                this.value = variant.variant_value.int32;
                break;
            case bind.SOPC_BuiltinId.SOPC_String_Id.value:
                this.value = ref.readCString(variant.value.variant_value.string.data);
                break;
            case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
                console.log("DataValue ByteString: TODO");
                break;
            default:
                console.log("DataValue type", this.type_id, "not managed: TODO");
                break;
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

module.exports.Variant = Variant;
