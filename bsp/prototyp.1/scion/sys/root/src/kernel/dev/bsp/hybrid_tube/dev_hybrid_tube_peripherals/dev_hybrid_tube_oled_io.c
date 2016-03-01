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


/*============================================
| Includes
==============================================*/

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
#include "kernel/fs/vfs/vfsdev.h"

#include "kernel/dev/arch/all/lcd/dev_oled_ssd1322.h"

#define UNUSE_STM32F_CUBEMX_HAL

#ifdef USE_STM32F_CUBEMX_HAL
   #include "kernel/dev/bsp/hybrid_tube/cubemx_hal/gpio.h"
   //extern SPI_HandleTypeDef hspi5;
   #define __RES(__v__) if(__v__==1) HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_SET); else HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET)
   #define __CS(__v__) if(__v__==1) HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_SET); else HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_RESET)
   #define __DC(__v__) if(__v__==1) HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET); else HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET)

#else
   #include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
   #include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
   #include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
   #include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
   #include "kernel/dev/arch/cortexm/stm32f4xx/spi.h"

   #include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_spi_x.h"

   #include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_board/dev_hybrid_tube_board.h"
#endif


/*============================================
| Global Declaration
==============================================*/


const char dev_hybrid_tube_oled_io_name[]="oledio\0hybrid_tube_oled_io\0";

int dev_hybrid_tube_oled_io_load(void);
int dev_hybrid_tube_oled_io_open(desc_t desc, int o_flag);
int dev_hybrid_tube_oled_io_close(desc_t desc);
int dev_hybrid_tube_oled_io_isset_read(desc_t desc);
int dev_hybrid_tube_oled_io_isset_write(desc_t desc);
int dev_hybrid_tube_oled_io_read(desc_t desc, char* buf,int size);
int dev_hybrid_tube_oled_io_write(desc_t desc, const char* buf,int size);
int dev_hybrid_tube_oled_io_seek(desc_t desc,int offset,int origin);
int dev_hybrid_tube_oled_io_ioctl(desc_t desc,int request,va_list ap);

dev_map_t dev_hybrid_tube_oled_io_map={
   dev_hybrid_tube_oled_io_name,
   S_IFBLK,
   dev_hybrid_tube_oled_io_load,
   dev_hybrid_tube_oled_io_open,
   dev_hybrid_tube_oled_io_close,
   __fdev_not_implemented,
   __fdev_not_implemented,
   dev_hybrid_tube_oled_io_read,
   dev_hybrid_tube_oled_io_write,
   dev_hybrid_tube_oled_io_seek,
   dev_hybrid_tube_oled_io_ioctl //ioctl
};

/*============================================
| Implementation
==============================================*/

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_load(void){
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_close(desc_t desc){

   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_read(desc_t desc, char* buf,int size){
   desc_t desc_link;
   //
   if(ofile_lst[desc].oflag & O_RDONLY) {
     desc_link= ofile_lst[desc].desc_nxt[0];
   }
   //
   if(desc_link<0) 
     return -1;
   //
   return ofile_lst[desc_link].pfsop->fdev.fdev_read(desc_link,buf,size);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_write(desc_t desc, const char* buf,int size){
   desc_t desc_link;
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
     desc_link= ofile_lst[desc].desc_nxt[1];
   }
   //
   if(desc_link<0) 
     return -1;
   //
   return ofile_lst[desc_link].pfsop->fdev.fdev_write(desc_link,buf,size);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_oled_io_seek(desc_t desc,int offset,int origin){
   desc_t desc_link;
   
   if(ofile_lst[desc].oflag & O_RDONLY) {
     desc_link= ofile_lst[desc].desc_nxt[0];
   }
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
     desc_link= ofile_lst[desc].desc_nxt[1];
   }
   //
   if(desc_link<0) 
     return -1;
   //
   return ofile_lst[desc_link].pfsop->fdev.fdev_seek(desc_link,offset,origin);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_oled_io_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
#ifdef USE_STM32F_CUBEMX_HAL
int dev_hybrid_tube_oled_io_ioctl(desc_t desc,int request,va_list ap){

   //to do support I_LINK Command
    
   switch(request) {
      case I_LINK:{
        GPIO_InitTypeDef GPIO_InitStruct;

        /*Configure GPIO pins : PF4 PF5 PF6 */
        GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
        
        /*Configure GPIO pins : PE13 PE14 PE15 */
        GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
        
        //RESET=0;
        __RES(0);
        __kernel_usleep(10000); //10000 uS, 10 ms 
        //RESET=1 
        __RES(1);
      }
      break;
      //
      case I_UNLINK:{
      }
      break;
      //
      case  IOCTL_OLED_SSD1322_IO_CMD_ENTER:{
         //CS=0
         __CS(0);
         //DC=0;
         __DC(0);
      }
      break;
      //
      case  IOCTL_OLED_SSD1322_IO_DATA_ENTER:{
         //CS=0 
         __CS(0);
         //DC=1;
         __DC(1);
      }
      break;
      //
      case  IOCTL_OLED_SSD1322_IO_LEAVE:{
         //DC=1;
         __DC(1);
         //CS=1 
         __CS(1);
      }
      break;
      //
      case IOCTL_OLED_SSD1322_IO_RESET:
       //RESET=0;
        __RES(0);
        __kernel_usleep(10000); //10000 uS, 10 ms 
        //RESET=1 
        __RES(1);
      break;
      //
      default:
         return -1;
   }

   return 0;
}
#else
int dev_hybrid_tube_oled_io_ioctl(desc_t desc,int request,va_list ap){

   //to do support I_LINK Command
    
   switch(request) {
      case I_LINK:{
        //RESET=0;
        gpio_reset(GPIO_OLED_RESET); 
        __kernel_usleep(10000); //10000 uS, 10 ms 
        //RESET=1 
        gpio_set(GPIO_OLED_RESET);
        //
      }
      break;
      //
      case I_UNLINK:{
      }
      break;
      //
      case  IOCTL_OLED_SSD1322_IO_CMD_ENTER:{
         //CS=0
         gpio_reset(GPIO_OLED_CS);
         //DC=0;
         gpio_reset(GPIO_OLED_DC);
      }
      break;
      //
      case  IOCTL_OLED_SSD1322_IO_DATA_ENTER:{
         //CS=0 
         gpio_reset(GPIO_OLED_CS);
         //DC=1;
         gpio_set(GPIO_OLED_DC);
      }
      break;
      //
      case  IOCTL_OLED_SSD1322_IO_LEAVE:{
        //DC=1;
        gpio_set(GPIO_OLED_DC); 
        //CS=1 
        gpio_set(GPIO_OLED_CS);
        
      }
      break;
      //
      case IOCTL_OLED_SSD1322_IO_RESET:
        //RESET=0;
        gpio_reset(GPIO_OLED_RESET); 
        __kernel_usleep(10000); //10000 uS, 10 ms 
        //RESET=1 
        gpio_set(GPIO_OLED_RESET);
      break;
      //
      default:
         return -1;
   }

   return 0;
}
#endif


/*============================================
| End of Source  : dev_hybrid_tube_oled_io.c
==============================================*/
