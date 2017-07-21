/* Copyright (c) 2017 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __NU_H__
#define __NU_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/
#define __RSW_OFF       __ROTARY_0
#define __RSW_CONT      __ROTARY_1
#define __RSW_MGOHM     __ROTARY_2
#define __RSW_3P        __ROTARY_3
#define __RSW_ZLOOP     __ROTARY_4
#define __RSW_ZLINE     __ROTARY_5
#define __RSW_RCDIDN    __ROTARY_6
#define __RSW_RCDDT     __ROTARY_7
#define __RSW_CLAMP     __ROTARY_8
#define __RSW_ROTPHAS   __ROTARY_9
#define __RSW_WATT      __ROTARY_10
#define __RSW_HARM      __ROTARY_11
#define __RSW_SETUP     __ROTARY_12



//
typedef struct nuboot_xgui_st {
   //
   int fd_keybfootswitch;
  
} nuboot_xgui_t, *p_nuboot_xgui_t;


#endif
