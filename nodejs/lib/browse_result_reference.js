class BrowseResultReference {
    constructor() {
        this.referenceTypeId = null;
        this.isForward = false;
        this.nodeId = null;
        this.browseName = null;
        this.displayName = null;
        this.nodeClass = 0;
    }

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
