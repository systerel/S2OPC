/**
 * EndpointDescription object
 *
 * @module endpoint_description
 */

/** Class representing a user identity token */
class UserIdentityToken
{
    /**
     * Create an empty UserIdentityToken
     */
    constructor() {
        this.policyId = "";
        this.tokenType = "";
        this.issuedTokenType = "";
        this.issuerEndpointUrl = "";
        this.securityPolicyUri = "";
    }

    /**
     * Internal function used to convert C user identity token to JS
     * @param {C_UserIdentityToken} value S2OPC library C value to be converted
     * @return this
     */
    FromC(value) {
        this.policyId = value.policyId;
        switch(value.tokenType) {
            case 0:
                this.tokenType = "Anonymous";
                break;
            case 1:
                this.tokenType = "Username";
                break;
            case 2:
                this.tokenType = "Certificate";
                break;
            case 3:
                this.tokenType = "IssuedToken";
                break;
            case 4:
                this.tokenType = "Kerberos";
                break;
            default:
                this.tokenType = "Unknown";
                break;
        }
        this.issuedTokenType = value.issuedTokenType;
        this.issuerEndpointUrl = value.issuerEndpointUrl;
        this.securityPolicyUri = value.securityPolicyUri;

        return this;
    }
}


/** Class representing a EndpointDescription */
class EndpointDescription
{
    /**
     * Create an empty EndpointDescription
     */
    constructor() {
        this.endpointUrl = "";
        this.security_mode = 0;
        this.security_policyUri = "";
        this.userIdentityTokens = [];
        this.transportProfileUri = "";
        this.securityLevel = 0;
    }

    /**
     * Internal function used to convert C endpoint description to JS
     * @param {C_EndpointDescription} value S2OPC library C endpoint description to be converted
     * @return this
     */
    FromC(value) {
        if (value.nbOfUserIdentityTokens != 0) {
            value.userIdentityTokens.length = value.nbOfUserIdentityTokens;
            for (let i = 0; i < value.nbOfUserIdentityTokens; i++)
            {
                let userIdentityToken = new UserIdentityToken().FromC(value.userIdentityTokens[i]);
                this.userIdentityTokens.push(userIdentityToken);
            }
        }

        this.endpointUrl = value.endpointUrl;
        this.security_mode = value.security_mode;
        this.security_policyUri = value.security_policyUri;
        this.transportProfileUri = value.transportProfileUri;
        this.securityLevel = value.securityLevel;

        return this;
    }
}

module.exports = {
    EndpointDescription
 };
