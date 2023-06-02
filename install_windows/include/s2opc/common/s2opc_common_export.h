
#ifndef S2OPC_COMMON_EXPORT_H
#define S2OPC_COMMON_EXPORT_H

#ifdef S2OPC_COMMON_STATIC_DEFINE
#  define S2OPC_COMMON_EXPORT
#  define S2OPC_COMMON_NO_EXPORT
#else
#  ifndef S2OPC_COMMON_EXPORT
#    ifdef s2opc_common_EXPORTS
        /* We are building this library */
#      define S2OPC_COMMON_EXPORT 
#    else
        /* We are using this library */
#      define S2OPC_COMMON_EXPORT 
#    endif
#  endif

#  ifndef S2OPC_COMMON_NO_EXPORT
#    define S2OPC_COMMON_NO_EXPORT 
#  endif
#endif

#ifndef S2OPC_COMMON_DEPRECATED
#  define S2OPC_COMMON_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef S2OPC_COMMON_DEPRECATED_EXPORT
#  define S2OPC_COMMON_DEPRECATED_EXPORT S2OPC_COMMON_EXPORT S2OPC_COMMON_DEPRECATED
#endif

#ifndef S2OPC_COMMON_DEPRECATED_NO_EXPORT
#  define S2OPC_COMMON_DEPRECATED_NO_EXPORT S2OPC_COMMON_NO_EXPORT S2OPC_COMMON_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef S2OPC_COMMON_NO_DEPRECATED
#    define S2OPC_COMMON_NO_DEPRECATED
#  endif
#endif

#endif
