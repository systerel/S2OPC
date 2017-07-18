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

#include "test_results.h"

#include "sopc_base_types.h"


static t_bool valid_read_result = FALSE;
static OpcUa_WriteRequest *current_pWriteRequest = NULL;


void test_results_set_read_result(t_bool res){
  valid_read_result = res;
}

void test_results_set_WriteRequest(OpcUa_WriteRequest *pWriteReq) {
  current_pWriteRequest = pWriteReq;
}


t_bool test_results_get_read_result(void){
  return valid_read_result;
}

OpcUa_WriteRequest *test_results_get_WriteRequest(void) {
  return current_pWriteRequest;
}
