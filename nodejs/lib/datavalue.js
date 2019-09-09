const bind = require('./bind_sopc_client');
const ref = require('ref');

class VariantValue
{
    constructor() {
        this.value = 0;
    }
}

class Variant
{
    constructor() {
        this.type_id = 0;
        this.array_type = 0;
        this.do_not_clear = false;
        this.variant_value = new VariantValue()
    }

    ToC() {
        throw Error("TODO implement");
    }
}

class DataValue
{
    constructor() {
        this.value = new Variant();
        this.status = 0;
        this.src_ts = 0;
        this.srv_ts = 0;
        this.src_ps = 0;
        this.srv_ps = 0;
    }

    FromC(value) {
        const type_id = value.value.built_in_type_id;
        //TODO check variant array type
        this.value = null;
        switch(type_id)
        {
            case bind.SOPC_BuiltinId.SOPC_Boolean_Id.value:
                console.log("DataValue Boolean: TODO");
                break;
            case bind.SOPC_BuiltinId.SOPC_Int32_Id.value:
                this.value = value.value.variant_value.int32;
                break;
            case bind.SOPC_BuiltinId.SOPC_String_Id.value:
                this.value = ref.readCString(value.value.variant_value.string.data);
                break;
            case bind.SOPC_BuiltinId.SOPC_ByteString_Id.value:
                console.log("DataValue ByteString: TODO");
                break;
            default:
                console.log("DataValue type", type_id, "not managed: TODO");
                break;
        }
        this.status = value.status;
        this.src_ts = value.src_ts;
        this.src_ps = value.src_ps;
        this.srv_ts = value.srv_ts;
        this.srv_ps = value.srv_ps;

        return this;
    }

    setValue(type_id, value) {
        this.value.setValue(type_id, value);
        return this;
    }

    ToC()
    {
        var data_value = new bind.SOPC_DataValue({
            value : this.value.ToC(),
            status : this.status,
            src_ts : this.src_ts,
            srv_ts : this.srv_ts,
            src_ps : this.src_ps,
            srv_ps : this.srv_ps
        });
        return data_value;
    }
}


module.exports.DataValue = DataValue;
