const browse_result_reference = require('./browse_result_reference');

class BrowseResult {
    constructor() {
        this.status = 0;
        this.references = [];
    }

    FromC(value) {
        this.status = value.statusCode;
        this.references = [];
        value.references.length = value.nbOfReferences;
        for (let i = 0; i < value.nbOfReferences; i++) {
            let browse_result_ref = new browse_result_reference.BrowseResultReference()
                                                               .FromC(value.references[i]);
            this.references.push(browse_result_ref);
        }
        return this;
    }
}

module.exports = {
    BrowseResult
};
