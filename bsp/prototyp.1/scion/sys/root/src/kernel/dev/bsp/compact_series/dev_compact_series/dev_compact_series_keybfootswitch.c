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
#include <stdlib.h>
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/dirent.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_keyb.h"
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


/*===========================================
Global Declaration
=============================================*/
static const char dev_compact_series_keybfootswitch_name[]="kbftsw";

static int dev_compact_series_keybfootswitch_load(void);
static int dev_compact_series_keybfootswitch_open(desc_t desc, int o_flag);
static int dev_compact_series_keybfootswitch_close(desc_t desc);
static int dev_compact_series_keybfootswitch_isset_read(desc_t desc);
static int dev_compact_series_keybfootswitch_isset_write(desc_t desc);
static int dev_compact_series_keybfootswitch_read(desc_t desc, char* buf,int size);
static int dev_compact_series_keybfootswitch_write(desc_t desc, const char* buf,int size);
static int dev_compact_series_keybfootswitch_seek(desc_t desc,int offset,int origin);
static int dev_compact_series_keybfootswitch_ioctl(desc_t desc,int request,va_list ap);

//
dev_map_t dev_compact_series_keybfootswitch_map={
   dev_compact_series_keybfootswitch_name,
   S_IFCHR,
   dev_compact_series_keybfootswitch_load,
   dev_compact_series_keybfootswitch_open,
   dev_compact_series_keybfootswitch_close,
   dev_compact_series_keybfootswitch_isset_read,
   dev_compact_series_keybfootswitch_isset_write,
   dev_compact_series_keybfootswitch_read,
   dev_compact_series_keybfootswitch_write,
   dev_compact_series_keybfootswitch_seek,
   dev_compact_series_keybfootswitch_ioctl
};


#define FOOTSWITCH_BYPASS_MASK   ((uint8_t)(0x01<<1))

#define ROTARY_1_PUSH_MASK       ((uint8_t)(0x01<<4))
#define ROTARY_2_PUSH_MASK       ((uint8_t)(0x01<<5))
#define ROTARY_3_PUSH_MASK       ((uint8_t)(0x01<<6))


//footswtich bypass
static const  gpio_info_t gpio_footswitch_bypass = {
     .gpio_bank_no=GPIOE,
     .gpio_pin_no=GPIO_Pin_4
};


//rotary 1 push 
static const  gpio_info_t gpio_rotary_1_push = {
     .gpio_bank_no=GPIOG,
     .gpio_pin_no=GPIO_Pin_4
};

//rotary 2 push 
static const  gpio_info_t gpio_rotary_2_push = {
     .gpio_bank_no=GPIOG,
     .gpio_pin_no=GPIO_Pin_7
};

//rotary 3 push
static const  gpio_info_t gpio_rotary_3_push = {
     .gpio_bank_no=GPIOH,
     .gpio_pin_no=GPIO_Pin_13
};



static uint8_t g_footswitch_state_memory=0;
/*===========================================
Implementation
=============================================*/
/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_read_gpio
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static uint8_t dev_compact_series_keybfootswitch_read_gpio(void){
   uint8_t byte;
   
   //
   byte=0;
      
   //bypass
   if(!GPIO_ReadInputDataBit(gpio_footswitch_bypass.gpio_bank_no,gpio_footswitch_bypass.gpio_pin_no)){
       byte|=FOOTSWITCH_BYPASS_MASK;
   }
  
   //rotary button 1
   if(!GPIO_ReadInputDataBit(gpio_rotary_1_push.gpio_bank_no,gpio_rotary_1_push.gpio_pin_no)){
       byte|=ROTARY_1_PUSH_MASK;
   }
   //rotary button 2
   if(!GPIO_ReadInputDataBit(gpio_rotary_2_push.gpio_bank_no,gpio_rotary_2_push.gpio_pin_no)){
       byte|=ROTARY_2_PUSH_MASK;
   }
   //rotary button 3
   if(!GPIO_ReadInputDataBit(gpio_rotary_3_push.gpio_bank_no,gpio_rotary_3_push.gpio_pin_no)){
       byte|=ROTARY_3_PUSH_MASK;
   }
   
   //
   return byte;
}


/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_load(void){
   
   GPIO_InitTypeDef GPIO_InitStructure;
   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
   //footswitch bypass
   GPIO_InitStructure.GPIO_Pin = gpio_footswitch_bypass.gpio_pin_no;
   GPIO_Init(gpio_footswitch_bypass.gpio_bank_no, &GPIO_InitStructure);
  
   //
   //note : rotary S pin already initialized in rotary encoder driver.
   
   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {

   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_close(desc_t desc){

   if(ofile_lst[desc].oflag & O_RDONLY) {
      if(!ofile_lst[desc].nb_reader) {
      }
   }

   if(ofile_lst[desc].oflag & O_WRONLY) {
      if(!ofile_lst[desc].nb_writer) {
      }
   }

   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_isset_read(desc_t desc){
   uint8_t byte = dev_compact_series_keybfootswitch_read_gpio();
   //
   if(byte!=g_footswitch_state_memory){
      g_footswitch_state_memory = byte;
      return 0;
   }
   //
   g_footswitch_state_memory = byte;
   //
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_isset_write(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/


static int dev_compact_series_keybfootswitch_read(desc_t desc, char* buf,int size){
   //
   uint8_t byte = dev_compact_series_keybfootswitch_read_gpio();
   //
   if(!buf)
      return -1;
   if(size<1)
      return -1;
   
   //
   dev_compact_series_keybfootswitch_read_gpio();
   //
   buf[0]=byte;
   //
   g_footswitch_state_memory = byte;
   //
   return 1;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_write(desc_t desc, const char* buf,int size){
   return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_compact_series_keybfootswitch_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_keybfootswitch_ioctl(desc_t desc,int request,va_list ap){
   switch(request) {
   case IOCTL_MULTIBOOT_GETVAL:{
      int* kbval_p = va_arg( ap, int*);
      //
      uint8_t byte = dev_compact_series_keybfootswitch_read_gpio();
      if(byte==0){ //no footswtich pressed. force to 0x0F for boot option and avoid collision with 0x00 (default case). 
         byte=0x0F;
      }
      //
      *kbval_p=  byte;
      //
   }
   break;
   
   case KBGETVAL: {  //get current footswitch value
      int* kbval_p = va_arg( ap, int*);
      if(!kbval_p)
         return -1;
      //
      uint8_t byte = dev_compact_series_keybfootswitch_read_gpio();
      *kbval_p=  byte;
   }
   break;
   //
   default:
      return -1;

   }

   return 0;
}

/*===========================================
End of Sourcedev_compact_series_keybfootswitch.c
=============================================*/