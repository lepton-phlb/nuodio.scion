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

#include "bin/nuodio/compact-series/nu/nu_effect_parameters.h"
#include "bin/nuodio/compact-series/nu/preset/nu_preset.h"
#include "bin/nuodio/compact-series/nu/nu.h"

/*============================================
| Global Declaration
==============================================*/
//
// ROTARY 3   |   ROTARY 2   |   ROTARY 1 

#define NU_DISPLAY_LONG_ROTARY_PRESSED_TIMEOUT   3 //second

//
extern const GUI_FONT GUI_Font8BITWONDER8;
extern const GUI_FONT GUI_FontGrixelKyrou7WideXtnd14;
extern const GUI_FONT GUI_FontCoquette6x626;
extern const GUI_FONT GUI_Font5x5Pixel5 ;

//
extern const GUI_BITMAP bmlabel_nb_compact_0;
extern const GUI_BITMAP bmlabel_nb_compact_1;
extern const GUI_BITMAP bmlabel_nb_compact_2;
extern const GUI_BITMAP bmlabel_nb_compact_3;
extern const GUI_BITMAP bmlabel_nb_compact_4;
extern const GUI_BITMAP bmlabel_nb_compact_5;
extern const GUI_BITMAP bmlabel_nb_compact_6;
extern const GUI_BITMAP bmlabel_nb_compact_7;
extern const GUI_BITMAP bmlabel_nb_compact_8;
extern const GUI_BITMAP bmlabel_nb_compact_9;
extern const GUI_BITMAP bmlabel_nb_compact_midi;
extern const GUI_BITMAP bmlabel_nb_compact_bluetooth;

//
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m15;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m14;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m13;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m12;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m11;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m10;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m9;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m8;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m7;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m6;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m5;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m4;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m3;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m2;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_m1;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_0;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p1;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p2;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p3;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p4;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p5;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p6;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p7;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p8;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p9;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p10;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p11;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p12;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p13;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p14;
extern const GUI_BITMAP bmlabel_nb_compact_signed_m15_0_p15_p15;

//
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_0;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_1;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_2;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_3;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_4;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_5;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_6;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_7;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_8;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_9;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_10;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_11;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_12;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_13;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_14;
extern const GUI_BITMAP bmlabel_nb_compact_unsigned_0_15_15;

//
const GUI_BITMAP * const effect_parameter_label_number_unsigned_0_9_list[] = {
    &bmlabel_nb_compact_0
   ,&bmlabel_nb_compact_1
   ,&bmlabel_nb_compact_2
   ,&bmlabel_nb_compact_3
   ,&bmlabel_nb_compact_4
   ,&bmlabel_nb_compact_5
   ,&bmlabel_nb_compact_6
   ,&bmlabel_nb_compact_7
   ,&bmlabel_nb_compact_8
   ,&bmlabel_nb_compact_9
};
   
//to do
#if 1
const GUI_BITMAP * const effect_parameter_label_number_unsigned_0_15_list[] = {
   &bmlabel_nb_compact_unsigned_0_15_0,
   &bmlabel_nb_compact_unsigned_0_15_1,
   &bmlabel_nb_compact_unsigned_0_15_2,
   &bmlabel_nb_compact_unsigned_0_15_3,
   &bmlabel_nb_compact_unsigned_0_15_4,
   &bmlabel_nb_compact_unsigned_0_15_5,
   &bmlabel_nb_compact_unsigned_0_15_6,
   &bmlabel_nb_compact_unsigned_0_15_7,
   &bmlabel_nb_compact_unsigned_0_15_8,
   &bmlabel_nb_compact_unsigned_0_15_9,
   &bmlabel_nb_compact_unsigned_0_15_10,
   &bmlabel_nb_compact_unsigned_0_15_11,
   &bmlabel_nb_compact_unsigned_0_15_12,
   &bmlabel_nb_compact_unsigned_0_15_13,
   &bmlabel_nb_compact_unsigned_0_15_14,
   &bmlabel_nb_compact_unsigned_0_15_15,
   
};

const GUI_BITMAP * const effect_parameter_label_number_signed_15_0_15_list[] = {
   &bmlabel_nb_compact_signed_m15_0_p15_m15,
   &bmlabel_nb_compact_signed_m15_0_p15_m14,
   &bmlabel_nb_compact_signed_m15_0_p15_m13,
   &bmlabel_nb_compact_signed_m15_0_p15_m12,
   &bmlabel_nb_compact_signed_m15_0_p15_m11,
   &bmlabel_nb_compact_signed_m15_0_p15_m10,
   &bmlabel_nb_compact_signed_m15_0_p15_m9,
   &bmlabel_nb_compact_signed_m15_0_p15_m8,
   &bmlabel_nb_compact_signed_m15_0_p15_m7,
   &bmlabel_nb_compact_signed_m15_0_p15_m6,
   &bmlabel_nb_compact_signed_m15_0_p15_m5,
   &bmlabel_nb_compact_signed_m15_0_p15_m4,
   &bmlabel_nb_compact_signed_m15_0_p15_m3,
   &bmlabel_nb_compact_signed_m15_0_p15_m2,
   &bmlabel_nb_compact_signed_m15_0_p15_m1,
   &bmlabel_nb_compact_signed_m15_0_p15_0,
   &bmlabel_nb_compact_signed_m15_0_p15_p1,
   &bmlabel_nb_compact_signed_m15_0_p15_p2,
   &bmlabel_nb_compact_signed_m15_0_p15_p3,
   &bmlabel_nb_compact_signed_m15_0_p15_p4,
   &bmlabel_nb_compact_signed_m15_0_p15_p5,
   &bmlabel_nb_compact_signed_m15_0_p15_p6,
   &bmlabel_nb_compact_signed_m15_0_p15_p7,
   &bmlabel_nb_compact_signed_m15_0_p15_p8,
   &bmlabel_nb_compact_signed_m15_0_p15_p9,
   &bmlabel_nb_compact_signed_m15_0_p15_p10,
   &bmlabel_nb_compact_signed_m15_0_p15_p11,
   &bmlabel_nb_compact_signed_m15_0_p15_p12,
   &bmlabel_nb_compact_signed_m15_0_p15_p13,
   &bmlabel_nb_compact_signed_m15_0_p15_p14,
   &bmlabel_nb_compact_signed_m15_0_p15_p15
};
#endif


//
const GUI_BITMAP * const effect_parameter_label_number_list[] = {
    &bmlabel_nb_compact_0
   ,&bmlabel_nb_compact_1
   ,&bmlabel_nb_compact_2
   ,&bmlabel_nb_compact_3
   ,&bmlabel_nb_compact_4
   ,&bmlabel_nb_compact_5
   ,&bmlabel_nb_compact_6
   ,&bmlabel_nb_compact_7
   ,&bmlabel_nb_compact_8
   ,&bmlabel_nb_compact_9
};


//
typedef struct display_parameter_label_number_st{
    const GUI_BITMAP * const* p_parameter_label_number_list;
    uint8_t offset;
}display_parameter_label_number_t;
//
display_parameter_label_number_t display_parameter_label_number[NU_EFFECT_PARAMETERS_MAX];
  
#define __display_parameter_value(__parameter_no__,__value__) (void*)display_parameter_label_number[__parameter_no__].p_parameter_label_number_list[__value__+display_parameter_label_number[__parameter_no__].offset]

//MIDI preset operation
extern const GUI_BITMAP bmlabel_nb_compact_lookup;

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


/*
label valeur 1er chiffre    x=64  y=16
label valeur 2eme chiffre   x=128 y=16
label valeur 3eme chiffre   x=192 y=16
label BLE            x=208 Y=48
*/

//zonning
#define X_VALUE_PARAMETER_1 (108)
#define Y_VALUE_PARAMETER_1 (0)

#define X_VALUE_PARAMETER_2 (57)
#define Y_VALUE_PARAMETER_2 (0)

#define X_VALUE_PARAMETER_3 (6)
#define Y_VALUE_PARAMETER_3  (0)

//
#define X_LABEL_MIDI (0)
#define Y_LABEL_MIDI (17)
//
#define X_LABEL_CURRENT_PRESET_NO (18)
#define Y_LABEL_CURRENT_PRESET_NO (18)
//
#define X_MIDI_PRESET_OPERATION (25)
#define Y_MIDI_PRESET_OPERATION (18)
//
#define X_LABEL_LOOKUP_PRESET_NO (45)
#define Y_LABEL_LOOKUP_PRESET_NO (18)

//   
#define X_LABEL_PRESET_NAME (15)
#define Y_LABEL_PRESET_NAME (27)
//
#define X_LABEL_BLUETOOTH_MIDI (0)
#define Y_LABEL_BLUETOOTH_MIDI (25)

//
#define ID_EFFECT_PARAMETER_1    120
#define ID_EFFECT_PARAMETER_2    121
#define ID_EFFECT_PARAMETER_3    122
//
#define ID_STATICTEXT_CURRENT_PRESET_NO         150 
#define ID_STATICTEXT_MIDI_PRESET_OPERATION     151
#define ID_STATICTEXT_LOOKUP_PRESET_NO          152
#define ID_STATICTEXT_PRESET_NAME               153
//
#define ID_MIDI_MESSAGE                   160
#define ID_BLUETOOTH_MIDI_MESSAGE         161

//parameters
void* p_effect_parameter_1_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER item_effect_parameter_1 = { ID_EFFECT_PARAMETER_1,
ITEM_TYPE_DYNAMICIMAGE,
X_VALUE_PARAMETER_1,
Y_VALUE_PARAMETER_1,
(void**)&p_effect_parameter_1_label_image,
0,0,NULL, NULL,
NULL , 150 };

void* p_effect_parameter_2_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER item_effect_parameter_2 = { ID_EFFECT_PARAMETER_2,
ITEM_TYPE_DYNAMICIMAGE,
X_VALUE_PARAMETER_2,
Y_VALUE_PARAMETER_2,
(void**)&p_effect_parameter_2_label_image,
0,0,NULL, NULL,
NULL , 150 };

void* p_effect_parameter_3_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER item_effect_parameter_3 = { ID_EFFECT_PARAMETER_3,
ITEM_TYPE_DYNAMICIMAGE,
X_VALUE_PARAMETER_3,
Y_VALUE_PARAMETER_3,
(void**)&p_effect_parameter_3_label_image,
0,0,NULL, NULL,
NULL , 150 };

//MIDI
static int midi_onoff=0;
static const ITEM_IMAGE_PARAMETER item_midi_message= { ID_MIDI_MESSAGE,
ITEM_TYPE_IMAGE,
X_LABEL_MIDI,
Y_LABEL_MIDI,
(void*)&bmlabel_nb_compact_midi,
&midi_onoff,50};

//
static char display_current_preset_no_buffer[3]="02";
static int current_preset_no_preset_no_onoff=0;
static const ITEM_TEXT_PARAMETER text_label_current_preset_no = { ID_STATICTEXT_CURRENT_PRESET_NO,
ITEM_TYPE_TEXT,
X_LABEL_CURRENT_PRESET_NO,
Y_LABEL_CURRENT_PRESET_NO,
display_current_preset_no_buffer,
(void*)&GUI_Font5x5Pixel5,
&current_preset_no_preset_no_onoff,20
};

//
void* p_midi_preset_operation_label_image;
static const ITEM_DYNAMICIMAGE_PARAMETER item_midi_preset_operation = { ID_STATICTEXT_MIDI_PRESET_OPERATION,
ITEM_TYPE_DYNAMICIMAGE,
X_MIDI_PRESET_OPERATION,
Y_MIDI_PRESET_OPERATION,
(void**)&p_midi_preset_operation_label_image,
0,0,NULL, NULL,
NULL , 150 };

//
static char display_lookup_preset_no_buffer[3]="05";
static int lookup_preset_no_preset_no_onoff=0;
static const ITEM_TEXT_PARAMETER text_label_lookup_preset_no = { ID_STATICTEXT_LOOKUP_PRESET_NO,
ITEM_TYPE_TEXT,
X_LABEL_LOOKUP_PRESET_NO,
Y_LABEL_LOOKUP_PRESET_NO,
display_lookup_preset_no_buffer,
(void*)&GUI_Font5x5Pixel5,
&lookup_preset_no_preset_no_onoff,20
};


//
static const char display_preset_name_format[]="%s";
static char display_preset_name_buffer[32]="no preset";
static int preset_name_onoff=0;
static const ITEM_TEXT_PARAMETER text_label_preset_name = { ID_STATICTEXT_PRESET_NAME,
ITEM_TYPE_TEXT,
X_LABEL_PRESET_NAME,
Y_LABEL_PRESET_NAME,
display_preset_name_buffer,
(void*)&GUI_Font5x5Pixel5,
&preset_name_onoff,20
};

//bluetooth
static int bluetooth_midi_onoff=0;
static const ITEM_IMAGE_PARAMETER bluetooth_midi_message= { ID_BLUETOOTH_MIDI_MESSAGE,
ITEM_TYPE_IMAGE,
X_LABEL_BLUETOOTH_MIDI,
Y_LABEL_BLUETOOTH_MIDI,
(void*)&bmlabel_nb_compact_bluetooth,
&bluetooth_midi_onoff,50};

//display defintion
const DISPLAY_DEFINITION  nu_display_main_definition[] = {
   &item_effect_parameter_1,
   &item_effect_parameter_2,
   &item_effect_parameter_3,
   &item_midi_message,
   &text_label_current_preset_no,
   &item_midi_preset_operation,
   &text_label_lookup_preset_no,
   &text_label_preset_name,
   &bluetooth_midi_message  
};


/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        nu_display_preload_preset_and_refresh
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_display_preload_preset_and_refresh( struct nu_xgui_st *p_nu_xgui,int preset_no){
   char  str_buffer[32]={0};
   
   //load preset parameter value
   nu_load_preset_parameter_value(p_nu_xgui,preset_no); 
   
   //set index from preset
   for(int i=0;i<NU_EFFECT_PARAMETERS_MAX;i++){
       p_nu_xgui->effect_parameters_list[i].index = p_nu_xgui->nu_preset.effect_parameters_index[i];
   }
   
   //parameters 1
   guiModifyBitmap(ID_EFFECT_PARAMETER_1, __display_parameter_value(1,p_nu_xgui->effect_parameters_list[1].index), NULL);
   //parameters 2
   guiModifyBitmap(ID_EFFECT_PARAMETER_2, __display_parameter_value(2,p_nu_xgui->effect_parameters_list[2].index), NULL);
   //parameters 3
   guiModifyBitmap(ID_EFFECT_PARAMETER_3, __display_parameter_value(3,p_nu_xgui->effect_parameters_list[3].index), NULL);

   //preset name
   snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format," -- -- ");
   //
   if(nu_preset_is_used(&p_nu_xgui->nu_preset_storage,preset_no)){
      //
      char* preset_name = nu_preset_get_name(&p_nu_xgui->nu_preset_storage,preset_no);
      //
      if(preset_name){
          snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,preset_name);
      }
         
   }
   //
   guiClearText(ID_STATICTEXT_PRESET_NAME);
   //
   guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
   
   //
   snprintf(str_buffer,sizeof(str_buffer),"%02d",preset_no);
   guiClearText(ID_STATICTEXT_LOOKUP_PRESET_NO);
   guiSetText(ID_STATICTEXT_LOOKUP_PRESET_NO,(char*)str_buffer);
   
   //
   return 0;
}
/*--------------------------------------------
| Name:        nu_display_load_preset_and_refresh
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_display_load_preset_and_refresh( struct nu_xgui_st *p_nu_xgui,int preset_no){
   char  str_buffer[32]={0};
   
   //load preset parameter value
   nu_load_preset_parameter_value(p_nu_xgui,preset_no); 
   
   //set index from preset
   for(int i=0;i<NU_EFFECT_PARAMETERS_MAX;i++){
      p_nu_xgui->effect_parameters_list[i].index = p_nu_xgui->nu_preset.effect_parameters_index[i];
      nu_effect_parameters_set_value(i,p_nu_xgui->effect_parameters_list[i].index);
      //
      if(p_nu_xgui->effect_parameters_list[i].index_min<0){
         //
         display_parameter_label_number[i].p_parameter_label_number_list= effect_parameter_label_number_signed_15_0_15_list;
         display_parameter_label_number[i].offset = 15;
      }else{
         //
         display_parameter_label_number[i].p_parameter_label_number_list= effect_parameter_label_number_unsigned_0_15_list;
         display_parameter_label_number[i].offset = 0;
      }
   }
   
   //parameters 1
   ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[1].index_min,   p_nu_xgui->effect_parameters_list[1].index_max);
   ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[1].index);
   //parameters 2
   ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[2].index_min,   p_nu_xgui->effect_parameters_list[2].index_max);
   ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[2].index);
   //parameters 3
   ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[3].index_min,  p_nu_xgui->effect_parameters_list[3].index_max);
   ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[3].index);
   
   
   //parameters 1
   guiModifyBitmap(ID_EFFECT_PARAMETER_1, __display_parameter_value(1,p_nu_xgui->effect_parameters_list[1].index), NULL);
   //parameters 2
   guiModifyBitmap(ID_EFFECT_PARAMETER_2, __display_parameter_value(2,p_nu_xgui->effect_parameters_list[2].index), NULL);
   //parameters 3
   guiModifyBitmap(ID_EFFECT_PARAMETER_3, __display_parameter_value(3,p_nu_xgui->effect_parameters_list[3].index), NULL);

   //preset name
   snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,"-- --");
   //
   if(nu_preset_is_used(&p_nu_xgui->nu_preset_storage,preset_no)){
      //
      char* preset_name = nu_preset_get_name(&p_nu_xgui->nu_preset_storage,preset_no);
      //
      if(preset_name){
          snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,preset_name);
      }
         
   }
   //
   p_nu_xgui->current_midi_preset_is_modified=0;
   
   //
   guiClearText(ID_STATICTEXT_PRESET_NAME);
   //
   guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
   
   //
   snprintf(str_buffer,sizeof(str_buffer),"%02d",preset_no);
   guiClearText(ID_STATICTEXT_CURRENT_PRESET_NO);
   guiSetText(ID_STATICTEXT_CURRENT_PRESET_NO,(char*)str_buffer);
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_display_save_preset_and_refresh
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_display_save_preset_and_refresh( struct nu_xgui_st *p_nu_xgui,int preset_no){
   char  str_buffer[32]={0};
   
   //set preset from index
   for(int i=0;i<NU_EFFECT_PARAMETERS_MAX;i++){
        p_nu_xgui->nu_preset.effect_parameters_index[i] = p_nu_xgui->effect_parameters_list[i].index;
   }
   
   //save preset parameter value
   nu_save_preset_parameter_value(p_nu_xgui,preset_no); 
   
   //preset name
   snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format," -- -- ");
   //
   if(nu_preset_is_used(&p_nu_xgui->nu_preset_storage,preset_no)){
      //
      char* preset_name = nu_preset_get_name(&p_nu_xgui->nu_preset_storage,preset_no);
      //
      if(preset_name){
          snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,preset_name);
      }
         
   }
   //
   p_nu_xgui->current_midi_preset_is_modified=0;
   
   //
   guiClearText(ID_STATICTEXT_PRESET_NAME);
   //
   guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
   
   //
   snprintf(str_buffer,sizeof(str_buffer),"%02d",preset_no);
   guiClearText(ID_STATICTEXT_LOOKUP_PRESET_NO);
   guiSetText(ID_STATICTEXT_LOOKUP_PRESET_NO,(char*)str_buffer);
   
   //
   return 0;
}

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
   char  str_buffer[32]={0};
   //
   struct timespec  curtime={0};
   
   //
   kernel_clock_gettime(CLOCK_MONOTONIC, &curtime);
   //
   switch (msgId) {
      case MSG_DISPLAY_INIT: {
         //
         p_nu_xgui->rotary_1_is_pressed=0;
         p_nu_xgui->rotary_2_is_pressed=0;
         p_nu_xgui->rotary_3_is_pressed=0;
         p_nu_xgui->rotary_4_is_pressed=0;
         //
         p_nu_xgui->rotary_1_is_long_pressed=0;
         p_nu_xgui->rotary_2_is_long_pressed=0;
         p_nu_xgui->rotary_3_is_long_pressed=0;
         p_nu_xgui->rotary_4_is_long_pressed=0;
         //
         p_nu_xgui->rotary_1_is_turned=0;
         p_nu_xgui->rotary_2_is_turned=0;
         p_nu_xgui->rotary_3_is_turned=0;
         p_nu_xgui->rotary_4_is_turned=0;
         
         //midi nrpn
         p_nu_xgui->midi_nrpn_msb_parameter=NU_MIDI_NRPN_PARAMETER_MSB_INVALID;
         p_nu_xgui->midi_nrpn_lsb_parameter=NU_MIDI_NRPN_PARAMETER_LSB_INVALID;
         p_nu_xgui->midi_nrpn_msb_data=0;
         p_nu_xgui->midi_nrpn_lsb_data=0;
         
         //
         p_nu_xgui->midi_preset_no=1;
         p_nu_xgui->current_midi_preset_no=p_nu_xgui->midi_preset_no;
         //
         nu_display_load_preset_and_refresh(p_nu_xgui,p_nu_xgui->current_midi_preset_no);
          
         //midi operation
         //guiModifyBitmap(ID_STATICTEXT_MIDI_PRESET_OPERATION, (void*)&bmlabel_nb_compact_lookup, NULL);
         
         //bluetooth midi
         guiSetItemVisible(ID_BLUETOOTH_MIDI_MESSAGE,0);
         //
         guiSetItemVisible(ID_STATICTEXT_MIDI_PRESET_OPERATION,0);
         //guiSetItemVisible(ID_STATICTEXT_PRESET_NAME,1);
         //guiSetItemBlink(ID_STATICTEXT_PRESET_NAME,1);
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
         
         uint8_t footswitch_state=0;
         if ((cb = read(p_nu_xgui->fd_keybfootswitch, &footswitch_state, sizeof(footswitch_state))) == sizeof(footswitch_state)) {
#if 0
            switch(footswitch_state){
               case 0x00://idle
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[0]);
               break;
               
               case 0x01://prev 0001
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[1]);
               break;
               
               
               case 0x02://bypass 0010
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[2]);
               break;
               
               case 0x04://next 0100
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[3]);
               break;
               
               case 0x10://rotary1 0001 0000
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[4]);
               break;
               
               case 0x20://rotary2 0010 0000
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[5]);
               break;
               
               case 0x40://rotary3 0100 0000
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[6]);
               break;
               
               case 0x80://rotary4 1000 0000
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)footswitch_status_name[4]);
               break;
               
            }
#endif
         }  
         
         //footswitch
         /*
         if (FD_ISSET(p_nu_xgui->fd_keybfootswitch, &xlcd_context->readfs)) {
            
         }
         */
         
         //rotary 1 //load or clone
         if(footswitch_state&0x40 && p_nu_xgui->rotary_1_is_pressed==0){
            //rotary is pressed
            p_nu_xgui->rotary_1_is_pressed=1;
            //
            p_nu_xgui->rotary_1_start_time_pressed=curtime.tv_sec;
            //MIDI Preset index
            p_nu_xgui->midi_preset_no=p_nu_xgui->current_midi_preset_no;
            //
            p_nu_xgui->rotary_1_is_turned=0;
            ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTERLIMIT, NU_MIDI_PRESET_MIN, NU_MIDI_PRESET_MAX);
            ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTER, p_nu_xgui->midi_preset_no);
           
         }else if( !(footswitch_state&0x40) && p_nu_xgui->rotary_1_is_pressed==1){
            //rotary is released
            p_nu_xgui->rotary_1_is_pressed=0;
            p_nu_xgui->rotary_1_is_long_pressed=0;
            //short press
            if(p_nu_xgui->rotary_1_is_turned==1){
               //load operation
               if(p_nu_xgui->current_midi_preset_no!= p_nu_xgui->midi_preset_no){
                  p_nu_xgui->current_midi_preset_no = p_nu_xgui->midi_preset_no;
                  //
                  nu_display_load_preset_and_refresh(p_nu_xgui,p_nu_xgui->current_midi_preset_no); 
               }
            }else{
               //clone operation
               // to do find the first free preset no. if no free preset display error message.  
            }
           
            //restore parameters 1 value
            p_nu_xgui->rotary_1_is_turned=0;
            ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[1].index_min,   p_nu_xgui->effect_parameters_list[1].index_max);
            ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[1].index);
         }
         
         //rotary 2 //parameter 2 or parameter 6 (pushed)
         if(footswitch_state&0x20 && p_nu_xgui->rotary_2_is_pressed==0){
            //rotary is pressed
            p_nu_xgui->rotary_2_is_pressed=1;
            p_nu_xgui->rotary_2_is_turned=0;
            //restore parameters 6 value (see EFFECT_PARAMETER_ROTARY_2_PUSHED_INDEX)
            ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[6].index_min,  p_nu_xgui->effect_parameters_list[6].index_max);
            ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[6].index);
            //
            guiModifyBitmap(ID_EFFECT_PARAMETER_2,  __display_parameter_value(6,counter), NULL);
            //
         }else if( !(footswitch_state&0x20) && p_nu_xgui->rotary_2_is_pressed==1){
         p_nu_xgui->rotary_2_is_pressed=0;
            //
            p_nu_xgui->rotary_2_is_long_pressed=0;
             p_nu_xgui->rotary_2_is_turned=0;
            //restore parameters 2 value
            ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[2].index_min,  p_nu_xgui->effect_parameters_list[2].index_max);
            ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[2].index);
            //
            guiModifyBitmap(ID_EFFECT_PARAMETER_2,  __display_parameter_value(2,counter), NULL);
         }
         
         //rotary 3 //lookup preset/save/save as
         if(footswitch_state&0x10 && p_nu_xgui->rotary_3_is_pressed==0){
            //rotary is pressed
            p_nu_xgui->rotary_3_is_pressed=1;
            //
            p_nu_xgui->rotary_3_start_time_pressed=curtime.tv_sec;;
            //
            p_nu_xgui->rotary_3_is_turned=0;
            ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTERLIMIT, NU_MIDI_PRESET_MIN, NU_MIDI_PRESET_MAX);
            ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTER, p_nu_xgui->midi_preset_no);
           
         }else if( !(footswitch_state&0x10) && p_nu_xgui->rotary_3_is_pressed==1){
            //rotary is released
            p_nu_xgui->rotary_3_is_pressed=0;
           
            //save
            if(p_nu_xgui->rotary_3_is_turned==0 && p_nu_xgui->rotary_3_is_long_pressed==0){
               if(p_nu_xgui->current_midi_preset_no!= p_nu_xgui->midi_preset_no){
               #if 0
                  //to do switch parameters from p_nu_xgui->current_midi_preset_no to p_nu_xgui->midi_preset_no
                  //switch and keep the current changed parameters
                  
                  //
                  p_nu_xgui->current_midi_preset_no = p_nu_xgui->midi_preset_no;
                  
                  //
                  snprintf(str_buffer,sizeof(str_buffer),"%02d", p_nu_xgui->current_midi_preset_no);
                  guiClearText(ID_STATICTEXT_CURRENT_PRESET_NO);
                  guiSetText(ID_STATICTEXT_CURRENT_PRESET_NO,(char*)str_buffer);
                    
                  //to remove: just for test
                  guiClearText(ID_STATICTEXT_PRESET_NAME);
                  snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,"switch to preset name");
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
               #endif
               }else{
                  //check if parameters is changed. if yes save.
                  //
                  snprintf(str_buffer,sizeof(str_buffer),"%02d", p_nu_xgui->current_midi_preset_no);
                  guiClearText(ID_STATICTEXT_CURRENT_PRESET_NO);
                  guiSetText(ID_STATICTEXT_CURRENT_PRESET_NO,(char*)str_buffer);
                  //
                  guiClearText(ID_STATICTEXT_PRESET_NAME);
                  snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,"save");
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
                  
                  nu_display_save_preset_and_refresh(p_nu_xgui,p_nu_xgui->current_midi_preset_no);
               }
            }
            
            //
            p_nu_xgui->rotary_3_is_long_pressed=0;
            //restore parameters 3 value
            p_nu_xgui->rotary_3_is_turned=0;
            ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTERLIMIT, p_nu_xgui->effect_parameters_list[3].index_min,  p_nu_xgui->effect_parameters_list[3].index_max);
            ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[3].index);
         }
         

         //parameter 1
         if (FD_ISSET(p_nu_xgui->fd_rotary_1, &xlcd_context->readfs)) {
            if ((cb = read(p_nu_xgui->fd_rotary_1, &counter, sizeof(counter))) == sizeof(counter)) {  
               //
               p_nu_xgui->rotary_1_is_turned=1;
               //
               if(p_nu_xgui->rotary_1_is_pressed==1){
                  //
                  p_nu_xgui->midi_preset_no = counter;
                  //
                  nu_display_preload_preset_and_refresh(p_nu_xgui,p_nu_xgui->midi_preset_no); 
               }else{
                  p_nu_xgui->effect_parameters_list[1].index = counter;
                  //
                  nu_effect_parameters_set_value(1,counter);
                  //
                  guiModifyBitmap(ID_EFFECT_PARAMETER_1,  __display_parameter_value(1,counter), NULL);
               }
            }
         }
         
         //parameter 2
         if (FD_ISSET(p_nu_xgui->fd_rotary_2, &xlcd_context->readfs)) {
            if ((cb = read(p_nu_xgui->fd_rotary_2, &counter, sizeof(counter))) == sizeof(counter)) {
               if(p_nu_xgui->rotary_2_is_pressed==0){
                  p_nu_xgui->effect_parameters_list[2].index = counter;
                  //
                  nu_effect_parameters_set_value(2,counter);
                  //
                  guiModifyBitmap(ID_EFFECT_PARAMETER_2,  __display_parameter_value(2,counter), NULL);
               }else{
                  p_nu_xgui->effect_parameters_list[6].index = counter;
                  //
                  nu_effect_parameters_set_value(6,counter);
                  //
                  guiModifyBitmap(ID_EFFECT_PARAMETER_2,  __display_parameter_value(6,counter), NULL);
               }
            }
         }
         
         //parameter 3
         if (FD_ISSET(p_nu_xgui->fd_rotary_3, &xlcd_context->readfs) &&  p_nu_xgui->rotary_3_is_long_pressed==0) {
            if ((cb = read(p_nu_xgui->fd_rotary_3, &counter, sizeof(counter))) == sizeof(counter)) { 
               //
               p_nu_xgui->rotary_3_is_turned=1;
               //
               if(p_nu_xgui->rotary_3_is_pressed==1){
               #if 0
                  p_nu_xgui->midi_preset_no = counter;
                  //
                  //preset name
                  snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format," -- -- ");
                  //
                  if(nu_preset_is_used(&p_nu_xgui->nu_preset_storage,p_nu_xgui->midi_preset_no)){
                     //
                     char* preset_name = nu_preset_get_name(&p_nu_xgui->nu_preset_storage,p_nu_xgui->midi_preset_no);
                     //
                     if(preset_name){
                         snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,preset_name);
                     }
                        
                  }
                  //
                  guiClearText(ID_STATICTEXT_PRESET_NAME);
                  //
                  guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
                  
                  //
                  snprintf(str_buffer,sizeof(str_buffer),"%02d",p_nu_xgui->midi_preset_no);
                  guiClearText(ID_STATICTEXT_LOOKUP_PRESET_NO);
                  guiSetText(ID_STATICTEXT_LOOKUP_PRESET_NO,(char*)str_buffer);
               #endif             
               }else{
                  p_nu_xgui->effect_parameters_list[3].index = counter;
                  //
                  nu_effect_parameters_set_value(3,counter);
                  //
                  guiModifyBitmap(ID_EFFECT_PARAMETER_3, __display_parameter_value(3,counter), NULL);
               }
            }
         }
        
         
         //midi
         if (FD_ISSET(p_nu_xgui->fd_mem, &xlcd_context->readfs)) {
            //
            lseek(p_nu_xgui->fd_mem,SHARED_MEM_ADDRESS_MIDI,SEEK_SET);
            if(read(p_nu_xgui->fd_mem,&p_nu_xgui->shared_memory_midi,sizeof(shared_memory_midi_t))>0){
               //
               guiSetItemVisible(ID_BLUETOOTH_MIDI_MESSAGE,1);
               //guiRefreshCurrentDisplay((xlcd_core_context_t*)xlcd_context);
               guiSetItemBlink(ID_BLUETOOTH_MIDI_MESSAGE,1);
               //
               switch(p_nu_xgui->shared_memory_midi.status){
                  //
                  case 0xB0:{ //Control Change
                     switch(p_nu_xgui->shared_memory_midi.data1){
                        //1:MSB, 2:LSB
                        case 0x62:
                           //98: Non-Registered Parameter Number (NRPN) - LSB
                           if(p_nu_xgui->shared_memory_midi.data2>=NU_MIDI_NRPN_PARAMETER_LSB_FIRST
                              && p_nu_xgui->shared_memory_midi.data2<=NU_MIDI_NRPN_PARAMETER_LSB_LAST){
                                 p_nu_xgui->midi_nrpn_lsb_parameter= p_nu_xgui->shared_memory_midi.data2;
                              }else{
                                  p_nu_xgui->midi_nrpn_lsb_parameter=NU_MIDI_NRPN_PARAMETER_LSB_INVALID;
                              }
                        break;
                        
                        //
                        case 0x63:
                           //99: Non-Registered Parameter Number (NRPN) - MSB
                           if(p_nu_xgui->shared_memory_midi.data2>=NU_MIDI_NRPN_PARAMETER_MSB_FIRST
                              && p_nu_xgui->shared_memory_midi.data2<=NU_MIDI_NRPN_PARAMETER_MSB_LAST){
                                 p_nu_xgui->midi_nrpn_msb_parameter= p_nu_xgui->shared_memory_midi.data2;
                           }else{
                              p_nu_xgui->midi_nrpn_msb_parameter=NU_MIDI_NRPN_PARAMETER_MSB_INVALID;
                           }
                        break;
                        
                        //
                        case 0x26://optional
                           //38: LSB for Control 6 (Data Entry)
                            p_nu_xgui->midi_nrpn_lsb_data = 0;
                        break;
                        
                        //
                        case 0x06:
                           //06: Data Entry MSB
                           p_nu_xgui->midi_nrpn_msb_data =  p_nu_xgui->shared_memory_midi.data2;
                           //
                           if( p_nu_xgui->midi_nrpn_msb_parameter!=NU_MIDI_NRPN_PARAMETER_MSB_INVALID
                              && p_nu_xgui->midi_nrpn_lsb_parameter!=NU_MIDI_NRPN_PARAMETER_LSB_INVALID){
                                 uint8_t parameter_no  = p_nu_xgui->midi_nrpn_msb_parameter;
                                 uint8_t parameter_value =  p_nu_xgui->midi_nrpn_msb_data;
                                 if(parameter_value >=p_nu_xgui->effect_parameters_list[parameter_no].index_min 
                                    && parameter_value <=p_nu_xgui->effect_parameters_list[parameter_no].index_max){
                                      
                                    
                                    //
                                    p_nu_xgui->effect_parameters_list[parameter_no].index = parameter_value;
                                    //
                                    switch(parameter_no){
                                       case 1:
                                          ioctl(p_nu_xgui->fd_rotary_1, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[parameter_no].index);
                                          guiModifyBitmap(ID_EFFECT_PARAMETER_1, (void*)effect_parameter_label_number_list[parameter_value], NULL);
                                       break;
                                       
                                       case 2:
                                          ioctl(p_nu_xgui->fd_rotary_2, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[parameter_no].index);
                                          guiModifyBitmap(ID_EFFECT_PARAMETER_2, (void*)effect_parameter_label_number_list[parameter_value], NULL);
                                       break;
                                       
                                       case 3:
                                          ioctl(p_nu_xgui->fd_rotary_3, ENCODERSETCOUNTER, p_nu_xgui->effect_parameters_list[parameter_no].index);
                                          guiModifyBitmap(ID_EFFECT_PARAMETER_3, (void*)effect_parameter_label_number_list[parameter_value], NULL);
                                       break;
                                    }
                                    
                                 }
                                 
                           }
                           //
                        break;
                     }
                        
                  }
                  break;
                  
                  //
                  case 0xC0:{ //Program Change
                     p_nu_xgui->midi_preset_no = p_nu_xgui->shared_memory_midi.data1;
                     p_nu_xgui->current_midi_preset_no = p_nu_xgui->midi_preset_no;
                     //
                     guiClearText(ID_STATICTEXT_PRESET_NAME);
                     snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,"change preset");
                     guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);

                     //
                     snprintf(str_buffer,sizeof(str_buffer),"%02d", p_nu_xgui->current_midi_preset_no);
                     guiClearText(ID_STATICTEXT_CURRENT_PRESET_NO);
                     guiSetText(ID_STATICTEXT_CURRENT_PRESET_NO,(char*)str_buffer);
                  }
                  break;
               }
            }
         }
           
      }
      break;
     
      //
      default:{
         uint8_t footswitch_state=0;
         int cb;
         //
         if(!xtime){
            break;
         }
         //
         if ((cb = read(p_nu_xgui->fd_keybfootswitch, &footswitch_state, sizeof(footswitch_state))) != sizeof(footswitch_state)){
            break;
         }
         //
         #if 0
         if(footswitch_state&0x10 
            && p_nu_xgui->rotary_3_is_pressed==1 
            && p_nu_xgui->rotary_3_is_turned==0
            && p_nu_xgui->rotary_3_is_long_pressed==0 
            && p_nu_xgui->current_midi_preset_no!= p_nu_xgui->midi_preset_no
            && (curtime.tv_sec - p_nu_xgui->rotary_3_start_time_pressed)>=NU_DISPLAY_LONG_ROTARY_PRESSED_TIMEOUT){
               
            //rotary 3 long pressed
            p_nu_xgui->rotary_3_is_long_pressed=1;
            
            //to do delete
            
            //return to current preset
            p_nu_xgui->midi_preset_no = p_nu_xgui->current_midi_preset_no;
            //
            guiClearText(ID_STATICTEXT_PRESET_NAME);
            snprintf(str_buffer,sizeof(str_buffer),display_preset_name_format,"delete this preset name");
            guiSetText(ID_STATICTEXT_PRESET_NAME,(char*)str_buffer);
            //
            snprintf(str_buffer,sizeof(str_buffer),"%02d", p_nu_xgui->midi_preset_no);
            guiClearText(ID_STATICTEXT_LOOKUP_PRESET_NO);
            guiSetText(ID_STATICTEXT_LOOKUP_PRESET_NO,(char*)str_buffer);
            
         }
         #endif
      }
      break;
      
   }

   return 0;
}