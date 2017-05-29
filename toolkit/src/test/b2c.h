/*?***************************************************************************

 Fichier : b2c.h

 Auteur(s) : F. Badeau (Systerel)

 Date de creation : 25/09/2009

 Version de creation : V1

 ******************************************************************************

 FONCTIONS :

 DESCRIPTION :
 Parties communes pour le traducteur C et diversification pour les operateurs
 elementaires

 ***************************************************************************?*/
#ifndef _b2c_h
#define _b2c_h
#include <stdint.h>
#include <stdbool.h>


/* t_bool */
#ifndef _t_bool_
#define _t_bool_
typedef bool t_bool;
#endif /* t_bool */

/* t_entier4 */
#ifndef _t_entier4_
#define _t_entier4_
typedef int32_t t_entier4;
#endif  /* t_entier4 */

/* MAXINT */
#ifndef MAXINT
#define MAXINT (2147483647)
#endif /* MAXINT */

/* MININT */
#ifndef MININT
#define MININT (-MAXINT)
#endif /* MININT */


#endif /* _b2c_h */
