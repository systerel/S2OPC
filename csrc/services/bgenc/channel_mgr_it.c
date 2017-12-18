/******************************************************************************

 File Name            : channel_mgr_it.c

 Date                 : 18/12/2017 17:24:01

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "channel_mgr_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 channel_mgr_it__channel_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_it__INITIALISATION(void)
{
    channel_mgr_it__channel_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_it__init_iter_channel(t_bool* const channel_mgr_it__p_continue)
{
    constants__get_card_t_channel(&channel_mgr_it__channel_i);
    *channel_mgr_it__p_continue = (1 <= channel_mgr_it__channel_i);
}

void channel_mgr_it__continue_iter_channel(t_bool* const channel_mgr_it__p_continue,
                                           constants__t_channel_i* const channel_mgr_it__p_channel)
{
    constants__get_cast_t_channel(channel_mgr_it__channel_i, channel_mgr_it__p_channel);
    channel_mgr_it__channel_i = channel_mgr_it__channel_i - 1;
    *channel_mgr_it__p_continue = (1 <= channel_mgr_it__channel_i);
}
