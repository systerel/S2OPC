const bind = require('./bind_sopc_client');
const variant = require('./variant');

class DataValue
{
    constructor() {
        this.value = new variant.Variant();
        this.status = 0;
        this.src_ts = 0;
        this.srv_ts = 0;
        this.src_ps = 0;
        this.srv_ps = 0;
    }

    FromC(value) {
        this.value.FromC(value.value);
        this.status = value.status;
        this.src_ts = value.src_ts;
        this.src_ps = value.src_ps;
        this.srv_ts = value.srv_ts;
        this.srv_ps = value.srv_ps;

        return this;
    }

    setValue(variant) {
        this.value = variant;
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

module.exports = {
    DataValue
 };
