MACHINE
    response_write_bs

SEES
    constants

ABSTRACT_VARIABLES
    ResponseWrite_StatusCode

INVARIANT
    ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode

INITIALISATION
    ResponseWrite_StatusCode :: {{}}

OPERATIONS
    ResponseWrite_allocated <-- alloc_write_request_responses_malloc(nb_req) =
    PRE
        nb_req : NAT &
        nb_req <= k_n_WriteResponse_max
    THEN
        ResponseWrite_allocated :: BOOL
    END
    ;

    set_ResponseWrite_StatusCode(wvi, sc) =
    PRE
        wvi : t_WriteValue_i &
        wvi : t_WriteValue &
        sc : t_StatusCode_i &
        sc : t_StatusCode
    THEN
        ResponseWrite_StatusCode(wvi) := sc
    END
    ;

    isvalid, sc <-- getall_ResponseWrite_StatusCode(wvi) =
    PRE
        wvi : t_WriteValue_i &
        wvi : t_WriteValue
    THEN
        isvalid := bool(wvi : dom(ResponseWrite_StatusCode)) ||
        IF wvi : dom(ResponseWrite_StatusCode) THEN
            sc
            :(
                sc : t_StatusCode_i &
                sc = ResponseWrite_StatusCode(wvi)
            )
        ELSE
            sc :: t_StatusCode_i
        END
    END
END
