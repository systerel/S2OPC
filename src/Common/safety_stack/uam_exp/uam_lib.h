/**
 * OPC Foundation OPC UA Safety Mapper
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Tianzhe Yu
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA Mapper Compile Macros
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Contains all Macros required for OPC UA Mapper public Interface.
 *
 * Safety-Related: no
 */

/* interface attribute*/
#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILD_UAM_DLL
#ifdef __GNUC__
#define UAM_API __attribute__((dllexport))
#define UAM_API_TYPE __attribute__((dllexport))
#else
#define UAM_API __declspec(dllexport)
#define UAM_API_TYPE __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define UAM_API __attribute__((dllimport))
#define UAM_API_TYPE __attribute__((dllimport))
#else
#define UAM_API __declspec(dllimport)
#define UAM_API_TYPE __declspec(dllimport)
#endif
#endif
#define LOCAL_API
#define WINAPI __stdcall
#else
#if __GNUC__ >= 4
#define UAM_API __attribute__((visibility("default")))
#define LOCAL_API __attribute__((visibility("hidden")))
#define UAM_API_TYPE
#else
#define UAM_API
#define LOCAL_API
#endif
#define WINAPI
#endif
