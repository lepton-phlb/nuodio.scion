/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2015 <lepton.phlb@gmail.com>.
All Rights Reserved.

Contributor(s): Jean-Jacques Pitrolle <lepton.jjp@gmail.com>.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/


/*===========================================
Includes
=============================================*/
#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_gpio_ex.h"

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_hybrid_tube_gpio.h"

#include <ctype.h>

/*===========================================
Global Declaration
=============================================*/
// ioctl command
// true by pass
#define GPIOCMD_TRUEBYPASS         0x0001
// adc in switches a1 a2
#define GPIOCMD_ADC_INPUT_A1           0x0002
#define GPIOCMD_ADC_INPUT_A2           0x0003

// input effect (right/left) effect source switches c1 c2
#define GPIOCMD_INPUT_EFFECT_SRC_RIGHT_C1           0x0004
#define GPIOCMD_INPUT_EFFECT_SRC_LEFT_C2           0x0005

// line output (right/left) source switches d1 d2
#define GPIOCMD_EFFECT_OUTPUT_SRC_RIGHT_D1           0x0006
#define GPIOCMD_EFFECT_OUTPUT_SRC_LEFT_D2           0x0007

//
#define POWER_DISABLE (Bit_RESET)   // 
#define POWER_ENABLE  (Bit_SET)     //

//enable power supply 5V
const  gpio_info_t gpio_cmd_enable_power_5V = {
     .gpio_bank_no=GPIOB,
     .gpio_pin_no=GPIO_Pin_5
};

//enable power supply 9V
const  gpio_info_t gpio_cmd_enable_power_9V = {
     .gpio_bank_no=GPIOD,
     .gpio_pin_no=GPIO_Pin_7
};

//true by pass
const  gpio_info_t gpio_cmd_bypass = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_10
};


#define ANALOG_SWITCH_NC (Bit_RESET)   // Normaly Closed active on low (default on schematic)
#define ANALOG_SWITCH_NO (Bit_SET)     // Normally Open active on high

// adc in switches a1 a2
#define INPUT_ADC_SRC_FROM_LINEIN_RIGHT         ANALOG_SWITCH_NC
#define INPUT_ADC_SRC_FROM_EFFECTOUT_LEFT       ANALOG_SWITCH_NO
const  gpio_info_t gpio_switch_source_input_adc_a1 = {
     .gpio_bank_no=GPIOF,
     .gpio_pin_no=GPIO_Pin_12
};

#define INPUT_ADC_SRC_FROM_LINEIN_LEFT          ANALOG_SWITCH_NC
#define INPUT_ADC_SRC_FROM_EFFECTOUT_RIGHT      ANALOG_SWITCH_NO
const  gpio_info_t gpio_switch_source_input_adc_a2 = {
     .gpio_bank_no=GPIOF,
     .gpio_pin_no=GPIO_Pin_15
};

// input effect (right/left) effect source switches c1 c2
#define INPUT_EFFECT_SRC_FROM_LINEIN_RIGHT      ANALOG_SWITCH_NC
#define INPUT_EFFECT_SRC_FROM_DACOUT_RIGHT      ANALOG_SWITCH_NO

const  gpio_info_t gpio_switch_source_right_input_effect_c1 = {
     .gpio_bank_no=GPIOF,
     .gpio_pin_no=GPIO_Pin_11
};

#define INPUT_EFFECT_SRC_FROM_LINEIN_LEFT       ANALOG_SWITCH_NC
#define INPUT_EFFECT_SRC_FROM_DACOUT_LEFT       ANALOG_SWITCH_NO

const  gpio_info_t gpio_switch_source_left_input_effect_c2 = {
     .gpio_bank_no=GPIOG,
     .gpio_pin_no=GPIO_Pin_0
};

// line output (right/left) source switches d1 d2
#define LINE_OUTPUT_SRC_FROM_DACOUT_RIGHT       ANALOG_SWITCH_NC
#define LINE_OUTPUT_SRC_FROM_EFFECTOUT_RIGHT     ANALOG_SWITCH_NO

const  gpio_info_t gpio_switch_source_right_line_ouptut_d1 = {
     .gpio_bank_no=GPIOF,
     .gpio_pin_no=GPIO_Pin_14
};

#define LINE_OUTPUT_SRC_FROM_DACOUT_LEFT       ANALOG_SWITCH_NC
#define LINE_OUTPUT_SRC_FROM_EFFECTOUT_LEFT    ANALOG_SWITCH_NO
const  gpio_info_t gpio_switch_source_left_line_ouptut_d2 = {
     .gpio_bank_no=GPIOF,
     .gpio_pin_no=GPIO_Pin_13
};



const char dev_hybrid_tube_gpio_name[]="gpio\0";

int dev_hybrid_tube_gpio_load(void);
int dev_hybrid_tube_gpio_open(desc_t desc, int o_flag);
int dev_hybrid_tube_gpio_close(desc_t desc);
int dev_hybrid_tube_gpio_isset_read(desc_t desc);
int dev_hybrid_tube_gpio_isset_write(desc_t desc);
int dev_hybrid_tube_gpio_read(desc_t desc, char* buf,int size);
int dev_hybrid_tube_gpio_write(desc_t desc, const char* buf,int size);
int dev_hybrid_tube_gpio_seek(desc_t desc,int offset,int origin);
int dev_hybrid_tube_gpio_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_hybrid_tube_gpio_map={
   dev_hybrid_tube_gpio_name,
   S_IFBLK,
   dev_hybrid_tube_gpio_load,
   dev_hybrid_tube_gpio_open,
   dev_hybrid_tube_gpio_close,
   dev_hybrid_tube_gpio_isset_read,
   dev_hybrid_tube_gpio_isset_write,
   dev_hybrid_tube_gpio_read,
   dev_hybrid_tube_gpio_write,
   dev_hybrid_tube_gpio_seek,
   dev_hybrid_tube_gpio_ioctl //ioctl
};

// e.18 F.15
int gpio_info_init(gpio_info_t* gpio_info, char* str_gpio_info_line){
   int len = strlen(str_gpio_info_line);
   int i;
   char bank_no=0;
   int pin_no=-1;
   
   //
   if(str_gpio_info_line==(char*)0){
     return -1;
   }
   //
   for(i=0; i<len;i++){
      if(str_gpio_info_line[i]=='.' && i>0 && (i+1)<len){
         bank_no=tolower(str_gpio_info_line[i-1]);
         pin_no=atoi(&str_gpio_info_line[i+1]);
         break;
      }
      
   }
   //
   switch(bank_no){
#ifdef GPIOA
      case 'a':
        gpio_info->gpio_bank_no=GPIOA;
      break;
#endif
      
#ifdef GPIOB
      case 'b':
        gpio_info->gpio_bank_no=GPIOB;
      break;
#endif
#ifdef GPIOC
      case 'c':
        gpio_info->gpio_bank_no=GPIOC;
      break;
#endif
      
#ifdef GPIOD
      case 'd':
        gpio_info->gpio_bank_no=GPIOD;
      break;
#endif
#ifdef GPIOE
      case 'e':
        gpio_info->gpio_bank_no=GPIOE;
      break;
#endif
      
#ifdef GPIOF
      case 'f':
        gpio_info->gpio_bank_no=GPIOF;
      break;
#endif
#ifdef GPIOG
      case 'g':
        gpio_info->gpio_bank_no=GPIOG;
      break;
#endif
#ifdef GPIOH
      case 'h':
        gpio_info->gpio_bank_no=GPIOH;
      break;
#endif      
#ifdef GPIOI
      case 'i':
        gpio_info->gpio_bank_no=GPIOI;
      break;
#endif
#ifdef GPIOJ
      case 'j':
        gpio_info->gpio_bank_no=GPIOJ;
      break;
#endif
#ifdef GPIOK
      case 'k':
        gpio_info->gpio_bank_no=GPIOK;
      break;
#endif
      
      default:
      return -1;
   }
   
   //
   switch(pin_no){
#ifdef GPIO_Pin_0
      case 0:
        gpio_info->gpio_pin_no=GPIO_Pin_0;
      break;
#endif 
#ifdef GPIO_Pin_1
      case 1:
        gpio_info->gpio_pin_no=GPIO_Pin_1;
      break;
#endif  
#ifdef GPIO_Pin_2
      case 2:
        gpio_info->gpio_pin_no=GPIO_Pin_2;
      break;
#endif  
#ifdef GPIO_Pin_3
      case 3:
        gpio_info->gpio_pin_no=GPIO_Pin_3;
      break;
#endif  
#ifdef GPIO_Pin_4
      case 4:
        gpio_info->gpio_pin_no=GPIO_Pin_4;
      break;
#endif  
#ifdef GPIO_Pin_5
      case 5:
        gpio_info->gpio_pin_no=GPIO_Pin_5;
      break;
#endif  
#ifdef GPIO_Pin_6
      case 6:
        gpio_info->gpio_pin_no=GPIO_Pin_6;
      break;
#endif  
#ifdef GPIO_Pin_7
      case 7:
        gpio_info->gpio_pin_no=GPIO_Pin_7;
      break;
#endif  
#ifdef GPIO_Pin_8
      case 8:
        gpio_info->gpio_pin_no=GPIO_Pin_8;
      break;
#endif  
#ifdef GPIO_Pin_9
      case 9:
        gpio_info->gpio_pin_no=GPIO_Pin_9;
      break;
#endif  
#ifdef GPIO_Pin_10
      case 10:
        gpio_info->gpio_pin_no=GPIO_Pin_10;
      break;
#endif  
#ifdef GPIO_Pin_11
      case 11:
        gpio_info->gpio_pin_no=GPIO_Pin_11;
      break;
#endif  
#ifdef GPIO_Pin_12
      case 12:
        gpio_info->gpio_pin_no=GPIO_Pin_12;
      break;
#endif  
#ifdef GPIO_Pin_13
      case 13:
        gpio_info->gpio_pin_no=GPIO_Pin_13;
      break;
#endif  
#ifdef GPIO_Pin_14
      case 14:
        gpio_info->gpio_pin_no=GPIO_Pin_14;
      break;
#endif  
#ifdef GPIO_Pin_15
      case 15:
        gpio_info->gpio_pin_no=GPIO_Pin_15;
      break;
#endif  
 
      
      default:
      return -1;
        
   }
   
   //
   return 0;
}
   
/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_load(void){
   GPIO_InitTypeDef GPIO_InitStructure;
   
   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   
   //cmd power 5V 
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_enable_power_5V.gpio_pin_no;
   GPIO_Init(gpio_cmd_enable_power_5V.gpio_bank_no, &GPIO_InitStructure);
   
   //cmd power 9V 
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_enable_power_9V.gpio_pin_no;
   GPIO_Init(gpio_cmd_enable_power_9V.gpio_bank_no, &GPIO_InitStructure);
   
   
   //true bypass
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_bypass.gpio_pin_no;
   GPIO_Init(gpio_cmd_bypass.gpio_bank_no, &GPIO_InitStructure);
   
   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   //analog switches
   //
   GPIO_InitStructure.GPIO_Pin = gpio_switch_source_input_adc_a1.gpio_pin_no;
   GPIO_Init(gpio_switch_source_input_adc_a1.gpio_bank_no, &GPIO_InitStructure);
   //
   GPIO_InitStructure.GPIO_Pin = gpio_switch_source_input_adc_a2.gpio_pin_no;
   GPIO_Init(gpio_switch_source_input_adc_a2.gpio_bank_no, &GPIO_InitStructure);
   
   //
   GPIO_InitStructure.GPIO_Pin = gpio_switch_source_right_input_effect_c1.gpio_pin_no;
   GPIO_Init(gpio_switch_source_right_input_effect_c1.gpio_bank_no, &GPIO_InitStructure);
   //
   GPIO_InitStructure.GPIO_Pin = gpio_switch_source_left_input_effect_c2.gpio_pin_no;
   GPIO_Init(gpio_switch_source_left_input_effect_c2.gpio_bank_no, &GPIO_InitStructure);
   
   //
   GPIO_InitStructure.GPIO_Pin = gpio_switch_source_right_line_ouptut_d1.gpio_pin_no;
   GPIO_Init(gpio_switch_source_right_line_ouptut_d1.gpio_bank_no, &GPIO_InitStructure);
   //
   GPIO_InitStructure.GPIO_Pin = gpio_switch_source_left_line_ouptut_d2.gpio_pin_no;
   GPIO_Init(gpio_switch_source_left_line_ouptut_d2.gpio_bank_no, &GPIO_InitStructure);
   
   // delay to avoid current peak at startup.
   __kernel_usleep(500000); //500 ms 
   //power enable 5V
   GPIO_WriteBit(gpio_cmd_enable_power_5V.gpio_bank_no,gpio_cmd_enable_power_5V.gpio_pin_no,POWER_ENABLE);
   // delay to avoid current peak at startup.
   __kernel_usleep(500000); //500 ms 
   //power enable 9V
   GPIO_WriteBit(gpio_cmd_enable_power_9V.gpio_bank_no,gpio_cmd_enable_power_9V.gpio_pin_no,POWER_ENABLE);
   // delay to avoid current peak at startup.
   __kernel_usleep(500000); //500 ms 
   //TRUE BYPASS ENABLED
   GPIO_WriteBit(gpio_cmd_bypass.gpio_bank_no,gpio_cmd_bypass.gpio_pin_no,Bit_RESET);
   //TRUE BYPASS DISABLED
   GPIO_WriteBit(gpio_cmd_bypass.gpio_bank_no,gpio_cmd_bypass.gpio_pin_no,Bit_SET);
   //__kernel_usleep(500000); //500 ms 
   //Right
   //input
   GPIO_WriteBit(gpio_switch_source_right_input_effect_c1.gpio_bank_no,gpio_switch_source_right_input_effect_c1.gpio_pin_no,INPUT_EFFECT_SRC_FROM_LINEIN_RIGHT);
   //output
   GPIO_WriteBit(gpio_switch_source_right_line_ouptut_d1.gpio_bank_no,gpio_switch_source_right_line_ouptut_d1.gpio_pin_no,LINE_OUTPUT_SRC_FROM_EFFECTOUT_RIGHT);
   //Left
   //input
   GPIO_WriteBit(gpio_switch_source_left_input_effect_c2.gpio_bank_no,gpio_switch_source_left_input_effect_c2.gpio_pin_no,INPUT_EFFECT_SRC_FROM_LINEIN_LEFT);
   //output
   GPIO_WriteBit(gpio_switch_source_left_line_ouptut_d2.gpio_bank_no,gpio_switch_source_left_line_ouptut_d2.gpio_pin_no,LINE_OUTPUT_SRC_FROM_EFFECTOUT_LEFT);
   
   
   
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_write(desc_t desc, const char* buf,int size){
   //read only mode
   return -1;
}

/*-------------------------------------------
| Name:dev_a0350_gpio_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_seek(desc_t desc,int offset,int origin){

   switch(origin) {

   case SEEK_SET:
      ofile_lst[desc].offset=offset;
      break;

   case SEEK_CUR:
      ofile_lst[desc].offset+=offset;
      break;

   case SEEK_END:
      //to do: warning in SEEK_END (+ or -)????
      ofile_lst[desc].offset-=offset;
      break;
   }

   return ofile_lst[desc].offset;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_gpio_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_gpio_ioctl(desc_t desc,int request,va_list ap){

   switch(request) {

   //
   default:
      return -1;

   }

   return 0;
}

/*============================================
| End of Source  : dev_hybrid_tube_board.c
==============================================*/