const bind = require('./bind_sopc_client');
const ref = require('ref');

class BrowseRequest {
    constructor() {
        this.nodeId = null;
        this.direction = 0;
        this.referenceTypeId = null;
        this.includeSubtypes = false;
    }

    setNodeId(nodeId) {
        this.nodeId = nodeId;
        return this;
    }

    setDirection(direction) {
        this.direction = direction;
        return this;
    }

    setReferenceTypeId(referenceTypeId) {
        this.referenceTypeId = referenceTypeId;
        return this;
    }

    setIncludeSubtypes(includeSubtypes) {
        this.includeSubtypes = includeSubtypes;
        return this;
    }

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
