#ifndef SOPC_MONITORED_ITEM_POINTER_IMPL_H_
#define SOPC_MONITORED_ITEM_POINTER_IMPL_H_

#include "constants.h"

typedef struct SOPC_InternalMontitoredItem
{
    uint32_t monitoredItemId;
    constants__t_subscription_i subId;
    constants__t_NodeId_i nid;
    constants__t_AttributeId_i aid;
    constants__t_TimestampsToReturn_i timestampToReturn;
    constants__t_monitoringMode_i monitoringMode;
    constants__t_client_handle_i clientHandle;
} SOPC_InternalMontitoredItem;

#endif /* SOPC_MONITORED_ITEM_POINTER_IMPL_H_ */
