/*
 *  Copyright (C) 2018 Systerel and others.
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

/** \file
 *
 * \brief A client library that supports and automates the subscription.
 *
 */

#include "libs2opc_client.h"
#include "state_machine.h"

cst_string_t s2opc_client_getVersion(void);
s2opc_client_result_t s2opc_client_initialize(const s2opc_client_static_cfg_t* pCfg);
s2opc_client_result_t s2opc_client_configure_connection(const s2opc_client_connect_cfg_t* pCfg,
                                                        s2opc_client_connection_id_t* c_id);
s2opc_client_result_t s2opc_client_connect(const s2opc_client_connection_id_t c_id,
                                           const int64_t publish_period_ms,
                                           data_change_callback_t data_change_callback);
s2opc_client_result_t s2opc_client_add_to_subscription(const s2opc_client_connection_id_t c_id,
                                                       s2opc_client_data_id_t* d_id);
s2opc_client_result_t s2opc_client_disconnect(const s2opc_client_connection_id_t c_id);
