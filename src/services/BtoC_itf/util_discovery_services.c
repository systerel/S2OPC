/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <assert.h>

#include "util_discovery_services.h"
#include "sopc_toolkit_constants.h"
#include "sopc_toolkit_config.h"

constants__t_StatusCode_i  build_endPoints_Descriptions(const constants__t_endpoint_config_idx_i endpoint_config_idx,
                         SOPC_String*  requestEndpointUrl,
                         uint32_t*     nbOfEndpointDescriptions,
                         OpcUa_EndpointDescription**   endpointDescriptions) {

	SOPC_Endpoint_Config* sopcEndpointConfig = SOPC_ToolkitServer_GetEndpointConfig(endpoint_config_idx);
	SOPC_StatusCode status;
	SOPC_String configEndPointURL;
        uint32_t tmpLength;
	SOPC_String_Initialize (&configEndPointURL);


	status = SOPC_String_AttachFromCstring(&configEndPointURL, sopcEndpointConfig->endpointURL);
	assert(STATUS_OK == status);

	/* We check that the endPointURL provided by the request matches
	 * the configuration endPointURL */
	int32_t endPointURLComparison = 0;
	status = SOPC_String_Compare(requestEndpointUrl,
						 &configEndPointURL,
						 1,
						 &endPointURLComparison);

	 if(status != STATUS_OK || endPointURLComparison != 0){
		*nbOfEndpointDescriptions = 0;
		return constants__e_sc_bad_invalid_argument;
	}

	uint8_t	nbSecuConfigs = sopcEndpointConfig->nbSecuConfigs;

	SOPC_SecurityPolicy* tabSecurityPolicy = sopcEndpointConfig->secuConfigurations;

	/* cf §5.6.2.2: t is recommended that Servers only include the endpointUrl, securityMode,
	securityPolicyUri, userIdentityTokens, transportProfileUri and securityLevel with all
	other parameters set to null. */
	// TODO: this code section can probably be optimized
	OpcUa_EndpointDescription My_OpcUa_EndpointDescription[3*nbSecuConfigs];
	int nbEndpointDescription = 0;
	OpcUa_EndpointDescription newEndPointDescription;
	OpcUa_EndpointDescription_Initialize(&newEndPointDescription);
	newEndPointDescription.EndpointUrl = configEndPointURL;
	/* cf §7.10 Part4 - Value 0 is for not recommended endPoint.
	   Others values corresponds to more secured endPoints.*/

	for (int iSecuConfig=0; iSecuConfig<nbSecuConfigs; iSecuConfig++){
		SOPC_SecurityPolicy currentSecurityPolicy = tabSecurityPolicy[iSecuConfig];
		uint16_t securityModes = currentSecurityPolicy.securityModes;
		newEndPointDescription.SecurityPolicyUri = currentSecurityPolicy.securityPolicy;

		// Add an endPoint description per security mode
		if((SOPC_SECURITY_MODE_NONE_MASK & securityModes) != 0){
			newEndPointDescription.SecurityMode = OpcUa_MessageSecurityMode_None;
			newEndPointDescription.SecurityLevel = 0;
			My_OpcUa_EndpointDescription[nbEndpointDescription] = newEndPointDescription;
			nbEndpointDescription++;
		}

		if((SOPC_SECURITY_MODE_SIGN_MASK & securityModes) != 0){
			newEndPointDescription.SecurityMode = OpcUa_MessageSecurityMode_Sign;
			newEndPointDescription.SecurityLevel = 1;
                        /* Copy server certificate */
                        if(sopcEndpointConfig->serverCertificate != NULL){
                            status = SOPC_KeyManager_Certificate_CopyDER(sopcEndpointConfig->serverCertificate,
                                                                    &newEndPointDescription.ServerCertificate.Data,
                                                                    &tmpLength);
                            assert(STATUS_OK == status && tmpLength <= INT32_MAX);
                            newEndPointDescription.ServerCertificate.Length = (int32_t) tmpLength;
                        }
			My_OpcUa_EndpointDescription[nbEndpointDescription] = newEndPointDescription;
			nbEndpointDescription++;
		}

		if((SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK & securityModes) != 0){
			newEndPointDescription.SecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
			newEndPointDescription.SecurityLevel = 1;
                        /* Copy server certificate */
                        if(sopcEndpointConfig->serverCertificate != NULL){
                            status = SOPC_KeyManager_Certificate_CopyDER(sopcEndpointConfig->serverCertificate,
                                                                    &newEndPointDescription.ServerCertificate.Data,
                                                                    &tmpLength);
                            assert(STATUS_OK == status && tmpLength <= INT32_MAX);
                            newEndPointDescription.ServerCertificate.Length = (int32_t) tmpLength;
                        }
			My_OpcUa_EndpointDescription[nbEndpointDescription] = newEndPointDescription;
			nbEndpointDescription++;
		}

	}

	OpcUa_EndpointDescription* final_OpcUa_EndpointDescription = malloc(nbEndpointDescription*sizeof(OpcUa_EndpointDescription));
	for (int i=0; i<nbEndpointDescription; i++){
		final_OpcUa_EndpointDescription[i] = My_OpcUa_EndpointDescription[i];
	}

	*nbOfEndpointDescriptions = nbEndpointDescription;
	*endpointDescriptions = final_OpcUa_EndpointDescription;

	return constants__e_sc_ok;

}
