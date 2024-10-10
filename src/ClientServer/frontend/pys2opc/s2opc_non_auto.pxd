# import C bool representation
from libcpp cimport bool

# patch to avoid compilation problems (do not use it)
ctypedef char* va_list

# autopxd ignores empty struct definitions
cdef struct SOPC_SLinkedList
cdef struct SOPC_SecretBuffer
cdef struct SOPC_User
cdef struct _SOPC_AddressSpace
cdef struct SOPC_KeyCertPair
cdef struct SOPC_CryptoProvider
cdef struct SOPC_PKIProvider
cdef struct SOPC_PKI_Profile
cdef struct SOPC_PKI_ChainProfile
cdef struct SOPC_PKI_LeafProfile
cdef struct SOPC_UserWithAuthorization
cdef struct timespec
cdef struct time_t
ctypedef struct tm
ctypedef struct _SOPC_Looper
cdef struct _SOPC_EventHandler
cdef struct _SOPC_AddressSpaceAccess
cdef struct SOPC_SLinkedList_Elt
cdef struct _SOPC_Dict
cdef struct _SOPC_Event
ctypedef struct SOPC_CircularLogFile
ctypedef struct SOPC_Log_Instance
ctypedef struct SOPC_ConfigClientXML_Custom
ctypedef struct SOPC_ConfigServerXML_Custom