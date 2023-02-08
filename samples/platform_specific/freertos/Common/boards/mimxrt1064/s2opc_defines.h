/* Defines proper to S2OPC configuration other MIMXRT1064
 *  should be adapted in regard of other boards and software */
#define USE_RTOS 1
#define SOPC_PTR_SIZE 4
#define WITH_USER_ASSERT 1

/* Defines proper to boards configuration */
#define FSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE 1
#define FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL 1
