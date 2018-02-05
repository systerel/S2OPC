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

/******************************************************************************

 File Name            : toolkit_header_init.c

 Date                 : 05/02/2018 16:15:30

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "toolkit_header_init.h"

/*---------------------
   List of Components
  ---------------------*/
#include "address_space.h"
#include "address_space_bs.h"
#include "address_space_it.h"
#include "channel_mgr.h"
#include "channel_mgr_1.h"
#include "channel_mgr_bs.h"
#include "channel_mgr_it.h"
#include "constants.h"
#include "constants_bs.h"
#include "io_dispatch_mgr.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "msg_browse_response_bs.h"
#include "msg_read_request.h"
#include "msg_read_request_bs.h"
#include "msg_read_response_bs.h"
#include "request_handle_bs.h"
#include "response_write_bs.h"
#include "service_browse.h"
#include "service_browse_decode_bs.h"
#include "service_browse_seq.h"
#include "service_browse_seq_it.h"
#include "service_get_endpoints_bs.h"
#include "service_mgr.h"
#include "service_read.h"
#include "service_read_it.h"
#include "service_response_cli_cb_bs.h"
#include "service_write_decode_bs.h"
#include "session_core.h"
#include "session_core_1.h"
#include "session_core_1_it.h"
#include "session_core_2.h"
#include "session_core_bs.h"
#include "session_core_it.h"
#include "session_mgr.h"
#include "session_mgr_it.h"
#include "session_request_handle_bs.h"
#include "toolkit_header.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void INITIALISATION(void)
{
    constants_bs__INITIALISATION();
    constants__INITIALISATION();
    message_in_bs__INITIALISATION();
    message_out_bs__INITIALISATION();
    session_core_1_it__INITIALISATION();
    request_handle_bs__INITIALISATION();
    channel_mgr_1__INITIALISATION();
    channel_mgr_it__INITIALISATION();
    channel_mgr_bs__INITIALISATION();
    channel_mgr__INITIALISATION();
    session_core_2__INITIALISATION();
    session_core_bs__INITIALISATION();
    session_core_1__INITIALISATION();
    session_core_it__INITIALISATION();
    session_core__INITIALISATION();
    session_mgr_it__INITIALISATION();
    session_request_handle_bs__INITIALISATION();
    session_mgr__INITIALISATION();
    msg_read_request_bs__INITIALISATION();
    address_space_bs__INITIALISATION();
    response_write_bs__INITIALISATION();
    address_space_it__INITIALISATION();
    service_write_decode_bs__INITIALISATION();
    address_space__INITIALISATION();
    msg_read_request__INITIALISATION();
    msg_read_response_bs__INITIALISATION();
    service_read_it__INITIALISATION();
    service_read__INITIALISATION();
    service_response_cli_cb_bs__INITIALISATION();
    service_get_endpoints_bs__INITIALISATION();
    msg_browse_response_bs__INITIALISATION();
    service_browse__INITIALISATION();
    service_browse_seq_it__INITIALISATION();
    service_browse_decode_bs__INITIALISATION();
    service_browse_seq__INITIALISATION();
    service_mgr__INITIALISATION();
    io_dispatch_mgr__INITIALISATION();
    toolkit_header__INITIALISATION();
}
