/* Copyright (c) 2017 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/



/*============================================
| Includes
==============================================*/
#include <stdint.h>
#include <stdlib.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/errno.h"
#include "kernel/core/dirent.h"
#include "kernel/core/system.h"
#include "kernel/core/time.h"
#include "kernel/core/libstd.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/select.h"

#include "lib/libc/unistd.h"
#include "lib/libc/termios/termios.h"
#include "lib/libc/stdio/stdio.h"

#include "kernel/core/time.h"
#include "kernel/core/wait.h"

#include "kernel/core/ioctl_lcd.h"
#include "kernel/core/ioctl_encoder.h"

#include "lib/libc/misc/ltostr.h"

#include "lib/graphic/xlcd/xlcd.h"
#include "lib/graphic/xlcd/xlcd_layout.h"
#include "lib/graphic/xgui/xgui_keyboard.h"
#include "lib/graphic/xgui/xgui_system.h"
#include "lib/graphic/xgui/segger_glib/src/gui/gui.h"

#include "bin/nuodio/nu/nu.h"

/*============================================
| Global Declaration
==============================================*/

extern const GUI_FONT GUI_Font8BITWONDER8;
extern const GUI_FONT GUI_FontGrixelKyrou7WideXtnd14;

extern const GUI_BITMAP bmlabel_sat;
extern const GUI_BITMAP bmlabel_tone;
extern const GUI_BITMAP bmlabel_level;

extern const GUI_BITMAP bmlabel_tube;
extern const GUI_BITMAP bmlabel_silicium;
extern const GUI_BITMAP bmlabel_germanium;
extern const GUI_BITMAP bmlabel_si_tube;
extern const GUI_BITMAP bmlabel_ge_tube;
extern const GUI_BITMAP bmlabel_tube_si;
extern const GUI_BITMAP bmlabel_tube_ge;
extern const GUI_BITMAP bmlabel_fuzz;



const GUI_BITMAP * const distorsion_type_label_image_list[]={
   &bmlabel_germanium
   ,&bmlabel_silicium
   ,&bmlabel_tube
   ,&bmlabel_si_tube
   ,&bmlabel_ge_tube
   ,&bmlabel_tube_ge
   ,&bmlabel_tube_si
   ,&bmlabel_fuzz
};

extern const GUI_BITMAP bmlabel_0;
extern const GUI_BITMAP bmlabel_1;
extern const GUI_BITMAP bmlabel_2;
extern const GUI_BITMAP bmlabel_3;
extern const GUI_BITMAP bmlabel_4;
extern const GUI_BITMAP bmlabel_5;
extern const GUI_BITMAP bmlabel_6;
extern const GUI_BITMAP bmlabel_7;
extern const GUI_BITMAP bmlabel_8;
extern const GUI_BITMAP bmlabel_9;


const GUI_BITMAP * const distorsion_type_label_number_list[] = {
   &bmlabel_0
   ,&bmlabel_1
   ,&bmlabel_2
   ,&bmlabel_3
   ,&bmlabel_4
   ,&bmlabel_5
   ,&bmlabel_6
   ,&bmlabel_7
   ,&bmlabel_8
   ,&bmlabel_9
};

const char* footswitch_status_name[] ={
   "idle",
   "prev",
   "bypass",
   "next",
   "rotary1",
   "rotary2",
   "rotary3",
   "rotary4"
};

//
const char* distorsion_type_name[DIST_TYPE_MAX+1] = {
   "Germanium",
   "Silicium",
   "Tube",
   "Ge+Tube",
   "Si+tube",
   "Tube+Ge",
   "Tube+Si",
   "Fuzz"
};

/*
label Sat            x=64px  y=0 
label tone           x=128px y=0
label level          x=192px y=0

label type           x=0   y=16
label 1er chiffre    x=64  y=16
label 2eme chiffre   x=128 y=16
label 3eme chiffre   x=192 y=16
label BLE            x=208 Y=48
label batterie       x=224 y=48

*/

//zonning
#define X_LABEL_SAT (64)
#define Y_LABEL_SAT (0)
//
#define X_LABEL_TONE (128)
#define Y_LABEL_TONE (0)

#define X_LABEL_LEVEL (192)
#define Y_LABEL_LEVEL (0)

#define X_LABEL_TYPE (0)
#define Y_LABEL_TYPE (16)

#define X_VALUE_SAT (64)
#define Y_VALUE_SAT (16)

#define X_VALUE_TONE (128)
#define Y_VALUE_TONE (16)

#define X_VALUE_LEVEL (192)
#define Y_VALUE_LEVEL (16)

#define X_LABEL_FOOTSWITCH (10)
#define Y_LABEL_FOOTSWITCH (48)

//
#define ID_STATICTEXT_SAT     121
#define ID_STATICTEXT_TONE    122
#define ID_STATICTEXT_LEVEL   123

#define ID_IMAGELABEL_SAT     121
#define ID_IMAGELABEL_TONE    122
#define ID_IMAGELABEL_LEVEL   123

#define ID_SATURATION_TYPE    124
#define ID_SATURATION_SAT     125
#define ID_SATURATION_TONE    126
#define ID_SATURATION_LEVEL   127

#define ID_STATICTEXT_FOOTSWITCH 150


//label
static const ITEM_TEXT_PARAMETER text_label_sat = { ID_STATICTEXT_SAT,
ITEM_TYPE_TEXT,
X_LABEL_SAT,
Y_LABEL_SAT,
"SAT",
(void*)&GUI_Font6x9,
};

static const ITEM_TEXT_PARAMETER text_label_tone = { ID_STATICTEXT_TONE,
ITEM_TYPE_TEXT,
X_LABEL_TONE,
Y_LABEL_TONE,
"TONE",
(void*)&GUI_Font6x9,
};

static const ITEM_TEXT_PARAMETER text_label_level = { ID_STATICTEXT_LEVEL,
ITEM_TYPE_TEXT,
X_LABEL_LEVEL,
Y_LABEL_LEVEL,
"LEVEL",
(void*)&GUI_Font6x9,
};

//label
void* p_image_label_sat=(void*)&bmlabel_sat;
static const ITEM_DYNAMICIMAGE_PARAMETER image_label_sat = { ID_IMAGELABEL_SAT,
ITEM_TYPE_DYNAMICIMAGE,
X_LABEL_SAT,
Y_LABEL_SAT,
(void**)&p_image_label_sat,
0,0,NULL, NULL,
NULL , 150 };
//
void* p_image_label_tone=(void*)&bmlabel_tone;
static const ITEM_DYNAMICIMAGE_PARAMETER image_label_tone = { ID_IMAGELABEL_TONE,
ITEM_TYPE_DYNAMICIMAGE,
X_LABEL_TONE,
Y_LABEL_TONE,
(void**)&p_image_label_tone,
0,0,NULL, NULL,
NULL , 150 };
//
void* p_image_label_level=(void*)&bmlabel_level;
static const ITEM_DYNAMICIMAGE_PARAMETER image_label_level = { ID_IMAGELABEL_LEVEL,
ITEM_TYPE_DYNAMICIMAGE,
X_LABEL_LEVEL,
Y_LABEL_LEVEL,
(void**)&p_image_label_level,
0,0,NULL, NULL,
NULL , 150 };

//parameters
void* p_saturation_type_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER saturation_parameter_type = { ID_SATURATION_TYPE,
ITEM_TYPE_DYNAMICIMAGE,
X_LABEL_TYPE,
Y_LABEL_TYPE,
(void**)&p_saturation_type_label_image,
0,0,NULL, NULL,
NULL , 150 };

void* p_saturation_sat_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER saturation_parameter_sat = { ID_SATURATION_SAT,
ITEM_TYPE_DYNAMICIMAGE,
X_VALUE_SAT,
Y_VALUE_SAT,
(void**)&p_saturation_sat_label_image,
0,0,NULL, NULL,
NULL , 150 };

void* p_saturation_tone_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER saturation_parameter_tone = { ID_SATURATION_TONE,
ITEM_TYPE_DYNAMICIMAGE,
X_VALUE_TONE,
Y_VALUE_TONE,
(void**)&p_saturation_tone_label_image,
0,0,NULL, NULL,
NULL , 150 };


void* p_saturation_level_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER saturation_parameter_level = { ID_SATURATION_LEVEL,
ITEM_TYPE_DYNAMICIMAGE,
X_VALUE_LEVEL,
Y_VALUE_LEVEL,
(void**)&p_saturation_level_label_image,
0,0,NULL, NULL,
NULL , 150 };

//
static char display_footswitch_state[32]="undef";
static const ITEM_TEXT_PARAMETER text_label_footswitch = { ID_STATICTEXT_FOOTSWITCH,
ITEM_TYPE_TEXT,
X_LABEL_FOOTSWITCH,
Y_LABEL_FOOTSWITCH,
display_footswitch_state,
(void*)&GUI_Font6x9,
};

const DISPLAY_DEFINITION  nu_display_main_definition[] = {
   /*&text_label_sat,
   &text_label_tone,
   &text_label_level,*/
   &image_label_sat,
   &image_label_tone,
   &image_label_level,
   &saturation_parameter_type,
   &saturation_parameter_sat,
   &saturation_parameter_tone,
   &saturation_parameter_level,
   &text_label_footswitch
};


/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        nu_display_main_app
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_display_main_app(struct xlcd_context_st* xlcd_context,
   MSG_ID msgId,
   char key,
   unsigned int status,
   unsigned int xtime){
   
    struct nu_xgui_st *p_nu_xgui = (struct nu_xgui_st*)(xlcd_context->p);
   //char sk1_txt[6];

   switch (msgId) {
      case MSG_DISPLAY_INIT: {
        
         //distorsion_type_label_list
         //type
         guiModifyBitmap(ID_SATURATION_TYPE, (void*)distorsion_type_label_image_list[p_nu_xgui->distortion_type], NULL);
         //sat
         guiModifyBitmap(ID_SATURATION_SAT, (void*)distorsion_type_label_number_list[p_nu_xgui->sat_value], NULL);
         //tone
         guiModifyBitmap(ID_SATURATION_TONE, (void*)distorsion_type_label_number_list[p_nu_xgui->tone_value], NULL);
         //level
         guiModifyBitmap(ID_SATURATION_LEVEL, (void*)distorsion_type_label_number_list[p_nu_xgui->level_value], NULL);
      }
         break;

      case MSG_SOFTKEY_1:
         // guiSetDisplay(xlcd_context, ID_DISPLAY_MOHM_UN);
         break;

      case MSG_SOFTKEY_2:
         //
         break;

      case MSG_SOFTKEY_3:
         break;

      case MSG_SOFTKEY_4:
         break;

      case MSG_KEY_TEST_DOWN:   // appui de la touche
         break;

      case MSG_KEY_INFO:
         //guiSetDisplayPopup(xlcd_context, ID_DISPLAY_POPUP_HELP, POPUP_TYPE_NOREFRESH, 0);
         break;

      default:
         break;
   }

   //
   switch (status) {
      case __XLCD_IO_EVENT: {
         int cb;
         int32_t counter = 0;
         
         //footswitch
         if (FD_ISSET(p_nu_xgui->fd_keybfootswitch, &xlcd_context->readfs)) {
            uint8_t footswitch_state=0;
            if ((cb = read(p_nu_xgui->fd_keybfootswitch, &footswitch_state, sizeof(footswitch_state))) == sizeof(footswitch_state)) {
               switch(footswitch_state){
                  case 0x00://idle
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[0]);
                  break;
                  
                  case 0x01://prev 0001
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[1]);
                  break;
                  
                  
                  case 0x02://bypass 0010
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[2]);
                  break;
                  
                  case 0x04://next 0100
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[3]);
                  break;
                  
                  case 0x10://rotary1 0001 0000
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[4]);
                  break;
                  
                  case 0x20://rotary2 0010 0000
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[5]);
                  break;
                  
                  case 0x40://rotary3 0100 0000
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[6]);
                  break;
                  
                  case 0x80://rotary3 1000 0000
                     guiSetText(ID_STATICTEXT_FOOTSWITCH,(char*)footswitch_status_name[7]);
                  break;
                  
               }
            }  
         }

         //
      }
      
   }

   return 0;
}