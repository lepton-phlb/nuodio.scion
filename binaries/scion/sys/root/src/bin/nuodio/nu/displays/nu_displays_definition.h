/* Copyright (c) 2017 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __NU_DISPLAY_DEFINITION_H__
#define __NU_DISPLAY_DEFINITION_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/

//main display
extern const DISPLAY_DEFINITION  nu_display_main_definition;
//
extern int nu_display_main_app(struct xlcd_context_st* xlcd_context,
   MSG_ID msgId,
   char key,
   unsigned int status,
   unsigned int xtime);

#define NU_DISPLAY_MAIN_DEFINITION_SIZE 8

#endif
