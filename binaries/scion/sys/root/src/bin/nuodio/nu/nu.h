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
typedef enum {
   germanium,
   silicium,
   tube,
   germanium_tube,
   silicium_tube,
   tube_germanium,
   tube_silicium,
   fuzz
}enum_distortion_type_t;

#define DIST_TYPE_GERMANIUM      ((uint8_t)0)
#define DIST_TYPE_SILICIUM       ((uint8_t)1)
#define DIST_TYPE_TUBE           ((uint8_t)2)
#define DIST_TYPE_GERMANIUM_TUBE ((uint8_t)3)
#define DIST_TYPE_SILICIUM_TUBE  ((uint8_t)4)
#define DIST_TYPE_TUBE_GERMANIUM ((uint8_t)5)
#define DIST_TYPE_TUBE_SILICIUM  ((uint8_t)6)
#define DIST_TYPE_TUBE_FUZZ      ((uint8_t)7)
//
#define DIST_TYPE_DFLT  DIST_TYPE_GERMANIUM
#define DIST_TYPE_MIN   DIST_TYPE_GERMANIUM
#define DIST_TYPE_MAX   DIST_TYPE_TUBE_FUZZ 

//
#define SAT_VALUE_DFLT  ((uint8_t)5)
#define SAT_VALUE_MIN   ((uint8_t)0)
#define SAT_VALUE_MAX   ((uint8_t)9)
//
#define TONE_VALUE_DFLT ((uint8_t)5)
#define TONE_VALUE_MIN  ((uint8_t)0)
#define TONE_VALUE_MAX  ((uint8_t)9)
//
#define LEVEL_VALUE_DFLT   ((uint8_t)5)
#define LEVEL_VALUE_MIN    ((uint8_t)0)
#define LEVEL_VALUE_MAX    ((uint8_t)9)

//
typedef struct nu_xgui_st {
   //
   uint8_t distortion_type;
   uint8_t sat_value;
   uint8_t tone_value;
   uint8_t level_value;
   //
   int fd_keybfootswitch;
   int fd_rotary_1;
   int fd_rotary_2;
   int fd_rotary_3;
   int fd_rotary_4;
} nu_xgui_t, *P_nu_xgui_t;


#endif
