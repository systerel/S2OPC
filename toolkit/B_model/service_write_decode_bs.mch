MACHINE
    service_write_decode_bs

SEES
    constants

DEFINITIONS
    d_variables ==
        nb_WriteValue,
        WriteValue_NodeId,
        WriteValue_AttributeId,
        WriteValue_Value /* The field value of the DataValue */
        /* TODO: StatusCode, SourceTimestamp (determine if ServerTimestamp is useful) */
    ;
    d_inv ==
        nb_WriteValue : NAT &
        nb_WriteValue <= k_n_WriteResponse_max &
        WriteValue_NodeId : t_WriteValue +-> t_NodeId &
        WriteValue_AttributeId : t_WriteValue +-> t_AttributeId &
        WriteValue_Value : t_WriteValue +-> t_Variant
    ;
    d_init ==
        nb_WriteValue := 0 ||
        WriteValue_NodeId :: {{}} ||
        WriteValue_AttributeId :: {{}} ||
        WriteValue_Value :: {{}}

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_init

OPERATIONS
    StatusCode_service <-- decode_write_request(req_payload) =
    PRE
        req_payload : t_ByteString_i &
        req_payload : t_ByteString
    THEN
        StatusCode_service,
        d_variables
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &
            d_inv
        )
    END
    ;

    free_write_request =
    BEGIN
        d_init
    END
    ;

    nb_req <-- get_nb_WriteValue =
    BEGIN
        nb_req := nb_WriteValue
    END
    ;

    isvalid, nid, aid, value <-- getall_WriteValue(wvi) =
    PRE
        wvi : t_WriteValue_i &
        wvi : t_WriteValue
    THEN
        isvalid := bool(wvi : dom(WriteValue_NodeId) &
                        wvi : dom(WriteValue_AttributeId) &
                        wvi : dom(WriteValue_Value)) ||
        IF wvi : dom(WriteValue_NodeId) &
           wvi : dom(WriteValue_AttributeId) &
           wvi : dom(WriteValue_Value)
        THEN
            nid,
            aid,
            value
            :(
                nid : t_NodeId_i &
                nid = WriteValue_NodeId(wvi) &
                aid : t_AttributeId_i &
                aid = WriteValue_AttributeId(wvi) &
                value : t_Variant_i &
                value = WriteValue_Value(wvi)
            )
        ELSE
            nid :: t_NodeId_i ||
            aid :: t_AttributeId_i ||
            value :: t_Variant_i
        END
    END

END
