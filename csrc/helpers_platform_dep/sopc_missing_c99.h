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

/* Copy-paste from
 * mcuxpressoide-10.3.1_2233/ide/plugins/com.nxp.mcuxpresso.tools.linux_10.3.0.201811011841/tools/arm-none-eabi/include/inttypes.h
 * as there seems to be a bug: __int64_t_defined is defined,
 * but only after the corresponding section is included,
 * so the PRI*64 directives are not defined...
 */

/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef SOPC_MISSING_C99_H_
#define SOPC_MISSING_C99_H_

#ifndef PRIi64
#define __STRINGIFY(a) #a

#define __PRI64(x) __INT64 __STRINGIFY(x)
#define __SCN64(x) __INT64 __STRINGIFY(x)
#define __PRI64LEAST(x) __LEAST64 __STRINGIFY(x)
#define __SCN64LEAST(x) __LEAST64 __STRINGIFY(x)
#define __PRI64FAST(x) __FAST64 __STRINGIFY(x)
#define __SCN64FAST(x) __FAST64 __STRINGIFY(x)

#define PRId64 __PRI64(d)
#define PRIi64 __PRI64(i)
#define PRIo64 __PRI64(o)
#define PRIu64 __PRI64(u)
#define PRIx64 __PRI64(x)
#define PRIX64 __PRI64(X)

#define SCNd64 __SCN64(d)
#define SCNi64 __SCN64(i)
#define SCNo64 __SCN64(o)
#define SCNu64 __SCN64(u)
#define SCNx64 __SCN64(x)

#define PRIdLEAST64 __PRI64LEAST(d)
#define PRIiLEAST64 __PRI64LEAST(i)
#define PRIoLEAST64 __PRI64LEAST(o)
#define PRIuLEAST64 __PRI64LEAST(u)
#define PRIxLEAST64 __PRI64LEAST(x)
#define PRIXLEAST64 __PRI64LEAST(X)

#define SCNdLEAST64 __SCN64LEAST(d)
#define SCNiLEAST64 __SCN64LEAST(i)
#define SCNoLEAST64 __SCN64LEAST(o)
#define SCNuLEAST64 __SCN64LEAST(u)
#define SCNxLEAST64 __SCN64LEAST(x)

#define PRIdFAST64 __PRI64FAST(d)
#define PRIiFAST64 __PRI64FAST(i)
#define PRIoFAST64 __PRI64FAST(o)
#define PRIuFAST64 __PRI64FAST(u)
#define PRIxFAST64 __PRI64FAST(x)
#define PRIXFAST64 __PRI64FAST(X)

#define SCNdFAST64 __SCN64FAST(d)
#define SCNiFAST64 __SCN64FAST(i)
#define SCNoFAST64 __SCN64FAST(o)
#define SCNuFAST64 __SCN64FAST(u)
#define SCNxFAST64 __SCN64FAST(x)

#endif /* PRIi64 */

#endif /* SOPC_MISSING_C99_H_ */
