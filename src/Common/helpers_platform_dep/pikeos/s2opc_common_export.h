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

#ifndef S2OPC_COMMON_EXPORT_H
#define S2OPC_COMMON_EXPORT_H

#ifdef S2OPC_COMMON_STATIC_DEFINE
#define S2OPC_COMMON_EXPORT
#define S2OPC_COMMON_NO_EXPORT
#else
#ifndef S2OPC_COMMON_EXPORT
#ifdef s2opc_common_EXPORTS
/* We are building this library */
#define S2OPC_COMMON_EXPORT
#else
/* We are using this library */
#define S2OPC_COMMON_EXPORT
#endif
#endif

#ifndef S2OPC_COMMON_NO_EXPORT
#define S2OPC_COMMON_NO_EXPORT
#endif
#endif

#ifndef S2OPC_COMMON_DEPRECATED
#define S2OPC_COMMON_DEPRECATED __attribute__((__deprecated__))
#endif

#ifndef S2OPC_COMMON_DEPRECATED_EXPORT
#define S2OPC_COMMON_DEPRECATED_EXPORT S2OPC_COMMON_EXPORT S2OPC_COMMON_DEPRECATED
#endif

#ifndef S2OPC_COMMON_DEPRECATED_NO_EXPORT
#define S2OPC_COMMON_DEPRECATED_NO_EXPORT S2OPC_COMMON_NO_EXPORT S2OPC_COMMON_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef S2OPC_COMMON_NO_DEPRECATED
#define S2OPC_COMMON_NO_DEPRECATED
#endif
#endif

#endif /* S2OPC_COMMON_EXPORT_H */
