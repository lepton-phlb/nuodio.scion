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
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/core/dirent.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_gpio_ex.h"

#include "kernel/dev/bsp/compact_series/dev_compact_series/dev_compact_series_gpio.h"
#include "kernel/dev/bsp/compact_series/dev_compact_series/ioctl_compact_series_gpio.h"


#include <ctype.h>

/*===========================================
Global Declaration
=============================================*/

//
#define POWER_DISABLE (Bit_RESET)   // 
#define POWER_ENABLE  (Bit_SET)     //


//true by pass
const  gpio_info_t gpio_cmd_bypass = {
     .gpio_bank_no=GPIOC,
     .gpio_pin_no=GPIO_Pin_6
};

//20V ANALOGIC POWER
const  gpio_info_t gpio_cmd_analogic_power = {
     .gpio_bank_no=GPIOD,
     .gpio_pin_no=GPIO_Pin_7
};

//MIDI MODE (out or thru)
const  gpio_info_t gpio_cmd_midi_mode = {
     .gpio_bank_no=GPIOA,
     .gpio_pin_no=GPIO_Pin_3
};

//fxio 0
const  gpio_info_t gpio_cmd_fxio_0 = {
     .gpio_bank_no=GPIOB,
     .gpio_pin_no=GPIO_Pin_5
};

//fxio 1
const  gpio_info_t gpio_cmd_fxio_1 = {
     .gpio_bank_no=GPIOB,
     .gpio_pin_no=GPIO_Pin_7
};

//fxio 2
const  gpio_info_t gpio_cmd_fxio_2 = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_0
};
//fxio 3
const  gpio_info_t gpio_cmd_fxio_3 = {
     .gpio_bank_no=GPIOB,
     .gpio_pin_no=GPIO_Pin_9
};
//fxio 4
const  gpio_info_t gpio_cmd_fxio_4 = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_1
};
//fxio 5
const  gpio_info_t gpio_cmd_fxio_5 = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_6
};
//fxio 6
const  gpio_info_t gpio_cmd_fxio_6 = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_2
};
//fxio 7
const  gpio_info_t gpio_cmd_fxio_7 = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_5
};


#define ANALOG_SWITCH_NC (Bit_RESET)   // Normaly Closed active on low (default on schematic)
#define ANALOG_SWITCH_NO (Bit_SET)     // Normally Open active on high





const char dev_compact_series_gpio_name[]="gpio\0";

int dev_compact_series_gpio_load(void);
int dev_compact_series_gpio_open(desc_t desc, int o_flag);
int dev_compact_series_gpio_close(desc_t desc);
int dev_compact_series_gpio_isset_read(desc_t desc);
int dev_compact_series_gpio_isset_write(desc_t desc);
int dev_compact_series_gpio_read(desc_t desc, char* buf,int size);
int dev_compact_series_gpio_write(desc_t desc, const char* buf,int size);
int dev_compact_series_gpio_seek(desc_t desc,int offset,int origin);
int dev_compact_series_gpio_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_compact_series_gpio_map={
   dev_compact_series_gpio_name,
   S_IFBLK,
   dev_compact_series_gpio_load,
   dev_compact_series_gpio_open,
   dev_compact_series_gpio_close,
   dev_compact_series_gpio_isset_read,
   dev_compact_series_gpio_isset_write,
   dev_compact_series_gpio_read,
   dev_compact_series_gpio_write,
   dev_compact_series_gpio_seek,
   dev_compact_series_gpio_ioctl //ioctl
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
| Name:dev_compact_series_gpio_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_load(void){
   GPIO_InitTypeDef GPIO_InitStructure;
   
   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   
   //true bypass
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_bypass.gpio_pin_no;
   GPIO_Init(gpio_cmd_bypass.gpio_bank_no, &GPIO_InitStructure);
   //TRUE BYPASS ENABLED
   GPIO_WriteBit(gpio_cmd_bypass.gpio_bank_no,gpio_cmd_bypass.gpio_pin_no,Bit_RESET);
   //TRUE BYPASS DISABLED
   //GPIO_WriteBit(gpio_cmd_bypass.gpio_bank_no,gpio_cmd_bypass.gpio_pin_no,Bit_SET);
   //__kernel_usleep(500000); //500 ms 
   
   //disabled 20V analogic power
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_analogic_power.gpio_pin_no;
   GPIO_Init(gpio_cmd_analogic_power.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_analogic_power.gpio_bank_no,gpio_cmd_analogic_power.gpio_pin_no,Bit_RESET);
   
   //set midi mode to thru (default position)
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_midi_mode.gpio_pin_no;
   GPIO_Init(gpio_cmd_midi_mode.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_midi_mode.gpio_bank_no,gpio_cmd_midi_mode.gpio_pin_no,Bit_SET);
   
   //fxio_0
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_0.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_0.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_0.gpio_bank_no,gpio_cmd_fxio_0.gpio_pin_no,Bit_RESET);
   
   //fxio_1
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_1.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_1.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_1.gpio_bank_no,gpio_cmd_fxio_1.gpio_pin_no,Bit_RESET);
   
   //fxio_2
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_2.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_2.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_2.gpio_bank_no,gpio_cmd_fxio_2.gpio_pin_no,Bit_RESET);
   
   //fxio_3
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_3.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_3.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_3.gpio_bank_no,gpio_cmd_fxio_3.gpio_pin_no,Bit_RESET);
   
   //fxio_4
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_4.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_4.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_4.gpio_bank_no,gpio_cmd_fxio_4.gpio_pin_no,Bit_RESET);
   
   //fxio_5
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_5.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_5.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_5.gpio_bank_no,gpio_cmd_fxio_5.gpio_pin_no,Bit_RESET);
   
   //fxio_6
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_6.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_6.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_6.gpio_bank_no,gpio_cmd_fxio_6.gpio_pin_no,Bit_RESET);
   
   //fxio_7
   GPIO_InitStructure.GPIO_Pin = gpio_cmd_fxio_7.gpio_pin_no;
   GPIO_Init(gpio_cmd_fxio_7.gpio_bank_no, &GPIO_InitStructure);
   GPIO_WriteBit(gpio_cmd_fxio_7.gpio_bank_no,gpio_cmd_fxio_7.gpio_pin_no,Bit_RESET);
   
  
   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_gpio_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_gpio_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_close(desc_t desc){
   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_gpio_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_gpio_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_compact_series_gpio_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_gpio_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_write(desc_t desc, const char* buf,int size){
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
int dev_compact_series_gpio_seek(desc_t desc,int offset,int origin){

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
| Name:dev_compact_series_gpio_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_compact_series_gpio_ioctl(desc_t desc,int request,va_list ap){

   switch(request) {
      //
      case GPIOCMD_TRUEBYPASS:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off==1){
            //TRUE BYPASS ENABLED
            GPIO_WriteBit(gpio_cmd_bypass.gpio_bank_no,gpio_cmd_bypass.gpio_pin_no,Bit_RESET);
         }else if (on_off==0){
            //TRUE BYPASS DISABLED
            GPIO_WriteBit(gpio_cmd_bypass.gpio_bank_no,gpio_cmd_bypass.gpio_pin_no,Bit_SET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_ANALOGICPOWER:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off==1){
            //20V ANALOGIC POWER ENABLED
            GPIO_WriteBit(gpio_cmd_analogic_power.gpio_bank_no,gpio_cmd_analogic_power.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            //20V ANALOGIC POWER DISABLED
            GPIO_WriteBit(gpio_cmd_analogic_power.gpio_bank_no,gpio_cmd_analogic_power.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_0:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_0.gpio_bank_no,gpio_cmd_fxio_0.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_0.gpio_bank_no,gpio_cmd_fxio_0.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_1:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_1.gpio_bank_no,gpio_cmd_fxio_1.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_1.gpio_bank_no,gpio_cmd_fxio_1.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_2:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_2.gpio_bank_no,gpio_cmd_fxio_2.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_2.gpio_bank_no,gpio_cmd_fxio_2.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_3:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_3.gpio_bank_no,gpio_cmd_fxio_3.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_3.gpio_bank_no,gpio_cmd_fxio_3.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_4:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_4.gpio_bank_no,gpio_cmd_fxio_4.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_4.gpio_bank_no,gpio_cmd_fxio_4.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_5:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_5.gpio_bank_no,gpio_cmd_fxio_5.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_5.gpio_bank_no,gpio_cmd_fxio_5.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_6:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_6.gpio_bank_no,gpio_cmd_fxio_6.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_6.gpio_bank_no,gpio_cmd_fxio_6.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
      
      //
      case GPIOCMD_FXIO_7:{
         uint32_t on_off = va_arg( ap, int);
         //
         if(on_off>0){
            GPIO_WriteBit(gpio_cmd_fxio_7.gpio_bank_no,gpio_cmd_fxio_7.gpio_pin_no,Bit_SET);
         }else if (on_off==0){
            GPIO_WriteBit(gpio_cmd_fxio_7.gpio_bank_no,gpio_cmd_fxio_7.gpio_pin_no,Bit_RESET);
         }else{
            return -1;
         }
      }
      break;
     
      
      //
      default:
      return -1;

   }

   return 0;
}

/*============================================
| End of Source  : dev_compact_series_board.c
==============================================*/