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

#ifndef _sopc_toolkit_build_info_h
#define _sopc_toolkit_build_info_h
#include "sopc_toolkit_constants.h"
#include "sopc_user_app_itf.h"
const SOPC_Build_Info toolkit_build_info = {
    .toolkitVersion = SOPC_TOOLKIT_VERSION,
    .toolkitSrcCommit = "9dd1618e5e49f15498b6ecba4002c52810b16e5d",
    .toolkitDockerId = "sha256:e6ef818bf9d8a244afdf2558bac405719a2b73ca1e154de635502988ab48f49c",
    .toolkitBuildDate = "2018-03-29",
};
#endif
