/**
 * BrowseResult Object
 * @module browse_result
 * @see module:browse_result_reference
 */
const browse_result_reference = require('./browse_result_reference');

/**
 * Class representing a BrowseResult
 */
class BrowseResult {
    /**
     * Creates an empty BrowseResult
     */
    constructor() {
        this.status = 0;
        this.references = [];
    }

    /**
     * Internal function used to convert C ReadValue to JS ReadValue
     * @param {C_BrowseResult} value C BrowseResult
     * @returns this
     */
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
