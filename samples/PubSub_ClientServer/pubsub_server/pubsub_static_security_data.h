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

#ifndef PUBSUB_STATIC_SECURITY_DATA_H_
#define PUBSUB_STATIC_SECURITY_DATA_H_

/* Note: following command might be used to extract the certificate content:
 * hexdump -v -e '/1 "0x%02X,"'
 */

static const unsigned char pubSub_keySign[] = {0x59, 0x05, 0x53, 0xA2, 0x5B, 0x53, 0x4E, 0x87, 0xBB, 0x38, 0x20,
                                               0x6A, 0xBE, 0x16, 0x3B, 0x7C, 0x42, 0x66, 0xAB, 0x25, 0xD4, 0xB5,
                                               0xAD, 0xE4, 0xAC, 0x12, 0x00, 0x17, 0x1A, 0xA6, 0x84, 0x4E};

static const unsigned char pubSub_keyEncrypt[] = {0xEE, 0x9F, 0x32, 0x4D, 0xCA, 0xB0, 0x5F, 0x1D, 0xB4, 0x35, 0x2B,
                                                  0x97, 0x96, 0x66, 0xBA, 0xF5, 0xE9, 0x27, 0x50, 0x8A, 0x16, 0x61,
                                                  0xFE, 0x8A, 0xE8, 0xD8, 0x36, 0xA4, 0x46, 0x05, 0x9F, 0x79};

static const unsigned char pubSub_keyNonce[] = {0xC8, 0x38, 0x8C, 0x6C};

#endif
