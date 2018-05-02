/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    monitored_item_pointer_bs

SEES
    constants
    
ABSTRACT_VARIABLES
    s_monitoredItemPointer
    
INVARIANT
    
    s_monitoredItemPointer <: t_monitoredItemPointer
    
INITIALISATION
    
    s_monitoredItemPointer :(s_monitoredItemPointer <: t_monitoredItemPointer & s_monitoredItemPointer = {})

OPERATIONS
    
    /* Needs UNINIT to deallocate dict and list */
    UNINITIALISATION =
    BEGIN
        skip
    END
    ;
    
    bres, monitoredItemPointer <-- create_monitored_item_pointer (p_subscription,
                                                                  p_nid,
                                                                  p_aid,
                                                                  p_timestampToReturn,
                                                                  p_monitoringMode,
                                                                  p_clientHandle) =
    PRE
        p_subscription : t_subscription_i &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_monitoringMode : t_monitoringMode_i &
        p_monitoringMode : t_monitoringMode &
        p_clientHandle : t_client_handle_i
    THEN
        IF t_monitoredItemPointer = s_monitoredItemPointer
        THEN
            bres := FALSE ||
            monitoredItemPointer := c_monitoredItemPointer_indet
        ELSE
            ANY l_monitoredItem WHERE
                l_monitoredItem : t_monitoredItemPointer - s_monitoredItemPointer
            THEN
                bres := TRUE ||
                s_monitoredItemPointer := s_monitoredItemPointer \/ {l_monitoredItem} ||
                monitoredItemPointer := l_monitoredItem
            END
        END
    END
    ;
    
    delete_monitored_item_pointer (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer
    THEN
        s_monitoredItemPointer := s_monitoredItemPointer - {p_monitoredItemPointer}
    END
    ;
    
    p_monitoredItemId,
    p_subscription,
    p_nid,
    p_aid,
    p_timestampToReturn,
    p_monitoringMode,
    p_clientHandle 
      <--getall_monitoredItemPointer (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer
    THEN
        p_monitoredItemId,
        p_subscription,
        p_nid,
        p_aid,
        p_timestampToReturn,
        p_monitoringMode,
        p_clientHandle
        :(p_monitoredItemId : t_monitoredItemId_i &
          p_monitoredItemId : t_monitoredItemId &
          p_subscription : t_subscription_i &
          p_subscription : t_subscription &
          p_nid : t_NodeId_i &
          p_nid : t_NodeId &        
          p_aid : t_AttributeId_i &
          p_aid : t_AttributeId &
          p_timestampToReturn : t_TimestampsToReturn_i &
          p_timestampToReturn : t_TimestampsToReturn &
          p_monitoringMode : t_monitoringMode_i &
          p_monitoringMode : t_monitoringMode &
          p_clientHandle : t_client_handle_i
        )
    END
    
    
    /*
    is_valid <-- monitored_item_pointer_is_valid (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i
    THEN
        is_valid := bool(p_monitoredItemPointer : s_monitoredItemPointer)
    END
    ;
    */
    
    
    
END
