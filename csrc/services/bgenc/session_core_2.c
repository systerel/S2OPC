/******************************************************************************

 File Name            : session_core_2.c

 Date                 : 18/12/2017 17:24:05

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_2.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_channel_i session_core_2__a_channel_i[constants__t_session_i_max + 1];
constants__t_channel_config_idx_i session_core_2__a_orphaned_i[constants__t_session_i_max + 1];
constants__t_sessionState session_core_2__a_state_i[constants__t_session_i_max + 1];
t_bool session_core_2__s_session_i[constants__t_session_i_max + 1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_2__INITIALISATION(void)
{
    {
        t_entier4 i;
        for (i = constants__t_session_i_max; 0 <= i; i = i - 1)
        {
            session_core_2__s_session_i[i] = false;
        }
    }
    {
        t_entier4 i;
        for (i = constants__t_session_i_max; 0 <= i; i = i - 1)
        {
            session_core_2__a_state_i[i] = constants__e_session_closed;
        }
    }
    {
        t_entier4 i;
        for (i = constants__t_session_i_max; 0 <= i; i = i - 1)
        {
            session_core_2__a_channel_i[i] = constants__c_channel_indet;
        }
    }
    {
        t_entier4 i;
        for (i = constants__t_session_i_max; 0 <= i; i = i - 1)
        {
            session_core_2__a_orphaned_i[i] = constants__c_channel_config_idx_indet;
        }
    }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_2__add_session(const constants__t_session_i session_core_2__p_session)
{
    session_core_2__s_session_i[session_core_2__p_session] = true;
}

void session_core_2__remove_session(const constants__t_session_i session_core_2__p_session)
{
    session_core_2__s_session_i[session_core_2__p_session] = false;
}

void session_core_2__reset_session_channel(const constants__t_session_i session_core_2__p_session)
{
    session_core_2__a_channel_i[session_core_2__p_session] = constants__c_channel_indet;
}

void session_core_2__reset_session_orphaned(const constants__t_session_i session_core_2__p_session)
{
    session_core_2__a_orphaned_i[session_core_2__p_session] = constants__c_channel_config_idx_indet;
}

void session_core_2__is_valid_session(const constants__t_session_i session_core_2__session,
                                      t_bool* const session_core_2__ret)
{
    *session_core_2__ret = session_core_2__s_session_i[session_core_2__session];
}

void session_core_2__get_session_state(const constants__t_session_i session_core_2__session,
                                       constants__t_sessionState* const session_core_2__state)
{
    *session_core_2__state = session_core_2__a_state_i[session_core_2__session];
}

void session_core_2__set_session_state_1(const constants__t_session_i session_core_2__p_session,
                                         const constants__t_sessionState session_core_2__p_state)
{
    session_core_2__a_state_i[session_core_2__p_session] = session_core_2__p_state;
}

void session_core_2__set_session_channel(const constants__t_session_i session_core_2__session,
                                         const constants__t_channel_i session_core_2__channel)
{
    session_core_2__a_channel_i[session_core_2__session] = session_core_2__channel;
}

void session_core_2__get_session_channel(const constants__t_session_i session_core_2__session,
                                         constants__t_channel_i* const session_core_2__channel)
{
    *session_core_2__channel = session_core_2__a_channel_i[session_core_2__session];
}

void session_core_2__set_session_orphaned_1(
    const constants__t_session_i session_core_2__p_session,
    const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx)
{
    session_core_2__a_orphaned_i[session_core_2__p_session] = session_core_2__p_channel_config_idx;
}
