/**
 * DataValue object
 *
 * @module datavalue
 * @see module:variant
 */
const bind = require('./bind_sopc_client');
const variant = require('./variant');

/** Class representing a DataValue */
class DataValue
{
    /**
     * Create an empty DataValue
     */
    constructor() {
        this.value = new variant.Variant();
        this.status = 0;
        this.src_ts = 0;
        this.srv_ts = 0;
        this.src_ps = 0;
        this.srv_ps = 0;
    }

    /**
     * Internal function used to convert C data value to JS
     * @param {C_DataValue} value S2OPC library C data value to be converted
     * @return this
     */
    FromC(value) {
        if (value.status === 0) {
            this.value.FromC(value.value);
        }
        this.status = value.status;
        this.src_ts = value.src_ts;
        this.src_ps = value.src_ps;
        this.srv_ts = value.srv_ts;
        this.srv_ps = value.srv_ps;

        return this;
    }

    /**
     * Set the DataValue Value (Variant)
     * @param {Variant} variant variant to be set as value
     * @see module:variant
     */
    setValue(variant) {
        this.value = variant;
        return this;
    }

    /**
     * Internal function used to convert JS DataValue to C
     * @return C_DataValue
     */
    ToC()
    {
        let data_value = new bind.SOPC_DataValue({
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

module.exports = {
    DataValue
 };
