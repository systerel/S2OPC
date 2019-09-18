/**
 * BrowseResultReference Object
 * @module browse_result_reference
 */

 /**
  * Class representing a BrowseResultReference
  */
class BrowseResultReference {
    /**
     * Creates an empty BrowseResultReference
     */
    constructor() {
        this.referenceTypeId = null;
        this.isForward = false;
        this.nodeId = null;
        this.browseName = null;
        this.displayName = null;
        this.nodeClass = 0;
    }

    /**
     * Internal function used to convert the C BrowseResultReference to JS
     * @param {C_BrowseResultReference} value C BrowseResultReference
     * @returns this
     */
    FromC(value) {
        this.referenceTypeId = value.referenceTypeId;
        this.isForward = value.isForward;
        this.nodeId = value.nodeId;
        this.browseName = value.browseName;
        this.displayName = value.displayName;
        this.nodeClass = value.nodeClass;
        return this;
    }
}

module.exports = {
    BrowseResultReference
};
