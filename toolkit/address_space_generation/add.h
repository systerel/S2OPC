/* Maybe temp */
/* Declares the variables produced by the AddS generator */
/* This is not easy, because the #defines are in the same file
 * Maybe you should #include "add.c" but ... Meh.
 * Maybe arrays should be defined with [] instead of [NB]
 * But the problem still applies to number of variables...
 */


#ifndef _add_h
#define _add_h


#include <stdbool.h>

#include "sopc_builtintypes.h"


extern SOPC_QualifiedName BrowseName[];
extern SOPC_LocalizedText Description[];
extern int Description_begin[];
extern int Description_end[];
extern SOPC_LocalizedText DisplayName[];
extern int DisplayName_begin[];
extern int DisplayName_end[];
extern int reference_begin[];
extern int reference_end[];
extern SOPC_NodeId* reference_type[];
extern SOPC_ExpandedNodeId* reference_target[];
extern bool reference_isForward[];
extern SOPC_NodeId* NodeId[];
extern OpcUa_NodeClass NodeClass[];
extern SOPC_ByteString *Value[];
extern SOPC_StatusCode status_code[];
extern SOPC_SByte AccessLevel[];


#endif /* _add_h */
