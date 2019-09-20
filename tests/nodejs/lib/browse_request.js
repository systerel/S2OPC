/**
 * BrowseRequest Object
 * @module browse_request
 */
const bind = require('./bind_sopc_client');
const ref = require('ref');

/**
 * Class representing a BrowseRequest
 */
class BrowseRequest {
    /**
     * Creates an empty BrowseRequest
     */
    constructor() {
        this.nodeId = null;
        this.direction = 0;
        this.referenceTypeId = null;
        this.includeSubtypes = false;
    }

    /**
     * Set the node id
     * @param {String} nodeId Node idenitifier
     * @returns this
     */
    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }

    /**
     * Set the browse direction
     * 0: Forward
     * 1: Inverse
     * 2: Both
     * @param {Number} direction direction to browse
     * @returns this
     */
    setDirection(direction) {
        this.direction = direction;
        return this;
    }

    /**
     * Set the reference type id
     * Null by default
     * @param {String} referenceTypeId reference type id
     * @returns this
     */
    setReferenceTypeId(referenceTypeId) {
        this.referenceTypeId = referenceTypeId;
        return this;
    }

    /**
     * Set the include subtypes flag
     * false by default
     * @param {Boolean} includeSubtypes flag to include subtypes
     * @returns this
     */
    setIncludeSubtypes(includeSubtypes) {
        this.includeSubtypes = includeSubtypes;
        return this;
    }

    /**
     * Internal function used to convert JS BrowseRequest to C BrowseRequest
     * @returns C_BrowseRequest
     */
    ToC() {
        return new bind.SOPC_ClientHelper_BrowseRequest({
            nodeId : (this.nodeId === null) ? ref.NULL_POINTER : this.nodeId,
            direction : this.direction,
            referenceTypeId : (this.referenceTypeId === null) ? ref.NULL_POINTER : this.referenceTypeId,
            includeSubtypes : this.includeSubtypes
        })
    }
}

module.exports = {
    BrowseRequest
};
