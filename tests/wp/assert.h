/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * frama_c_assert.h
 *
 *  Created on: 25 juin 2018
 *      Author: simon
 */

#ifndef FRAMA_C_ASSERT_H_
#define FRAMA_C_ASSERT_H_

#ifdef __FRAMAC__
// Let us use asserts for defensive programming, but make them invisible to
// FramaC.
#undef assert
#define assert(x) ;
#endif // __FRAMAC__

#endif /* FRAMA_C_ASSERT_H_ */
