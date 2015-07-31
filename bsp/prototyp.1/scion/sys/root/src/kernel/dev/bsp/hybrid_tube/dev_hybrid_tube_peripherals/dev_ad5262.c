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
#include "kernel/core/malloc.h"
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

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_ad5262.h"

/*============================================
| Global Declaration
==============================================*/

#define GPIO_ARGC 4

typedef struct ad5262_info_st{
  //filled by user
  gpio_info_t pin_cs;
  gpio_info_t pin_shdn;
  gpio_info_t pin_pr;
  //
  uchar8_t a0; //0: RDAC1, 1:RDAC2 
  //
  int16_t resistor_value;

}ad5262_info_t;

const char dev_ad5262_name[]="ad5262\0";

int dev_ad5262_load(void);
int dev_ad5262_open(desc_t desc, int o_flag);
int dev_ad5262_close(desc_t desc);
int dev_ad5262_isset_read(desc_t desc);
int dev_ad5262_isset_write(desc_t desc);
int dev_ad5262_read(desc_t desc, char* buf,int size);
int dev_ad5262_write(desc_t desc, const char* buf,int size);
int dev_ad5262_seek(desc_t desc,int offset,int origin);
int dev_ad5262_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_ad5262_map={
   dev_ad5262_name,
   S_IFBLK,
   dev_ad5262_load,
   dev_ad5262_open,
   dev_ad5262_close,
   dev_ad5262_isset_read,
   dev_ad5262_isset_write,
   dev_ad5262_read,
   dev_ad5262_write,
   dev_ad5262_seek,
   dev_ad5262_ioctl //ioctl
};


/*============================================
| Implementation
==============================================*/

/*-------------------------------------------
| Name:dev_rotary_encoder_x_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_load(){
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_ad5262_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
 
   }

   return 0;

}

/*-------------------------------------------
| Name:dev_ad5262_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_close(desc_t desc){

   return 0;
}

/*-------------------------------------------
| Name:dev_ad5262_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_isset_read(desc_t desc){
  return -1;
}

/*-------------------------------------------
| Name:dev_ad5262_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_ad5262_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_ad5262_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_write(desc_t desc, const char* buf,int size){
   ad5262_info_t* ad5262_info = (ad5262_info_t*)(ofile_lst[desc].p);
   desc_t desc_spi_w = INVALID_DESC;
   int cb =0;
   
   //
   if(ad5262_info==(ad5262_info_t*)0){
      return -1;
   }
   //
   desc_spi_w = ofile_lst[desc].desc_nxt[1];
   if(desc_spi_w==INVALID_DESC)
      return -1;
   //
   for(cb=0;cb<size;cb++){
      // set pin /CS = 0
      GPIO_WriteBit(ad5262_info->pin_cs.gpio_bank_no,ad5262_info->pin_cs.gpio_pin_no,Bit_RESET);
      //send A0 bit
      ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)&ad5262_info->a0,1);
      // send 8 bits of resistor value
      ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)&buf[cb],1);
      // set pin /CS = 1
      GPIO_WriteBit(ad5262_info->pin_cs.gpio_bank_no,ad5262_info->pin_cs.gpio_pin_no,Bit_SET);
   }
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_ad5262_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_ad5262_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_ad5262_ioctl
| Description: ex: mount /dev/ad5262 /dev/spi0 /dev/g_in 0 e.15 f.2 f.5 (a0, cs, shdn, pr)
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/

int dev_ad5262_ioctl(desc_t desc,int request,va_list ap){
   switch(request) {
     
      case I_LINK: {
         int argc;
         char** argv;
         int i;
         int fd;
         int rdac_a0=-1;
         //
         ad5262_info_t* ad5262_info;
         GPIO_InitTypeDef GPIO_InitStructure;

         ad5262_info = _sys_malloc(sizeof(ad5262_info_t));
        
         //
         fd=va_arg(ap, int);//not used
         //
         argc=va_arg(ap, int);
         argv=va_arg(ap, char**);
         //
         if(argc<(GPIO_ARGC+1))
           return -1;
         //
         for(i=GPIO_ARGC; i<argc; i++) {
           
            switch(i){
               case GPIO_ARGC:
                  rdac_a0=atoi(argv[i]);
                          //
                  if(rdac_a0<0 || rdac_a0>1)
                     return -1;
                  //
                  ad5262_info->a0=(uchar8_t)rdac_a0;
               break;
               
               case GPIO_ARGC+1: //cs
                  if(gpio_info_init(&ad5262_info->pin_cs,argv[i])<0)
                    return -1;
               break;
               
               case GPIO_ARGC+2: //shdn
                  if(gpio_info_init(&ad5262_info->pin_shdn,argv[i])<0)
                    return -1;
               break;
               
               case GPIO_ARGC+3: //pr
                  if(gpio_info_init(&ad5262_info->pin_pr,argv[i])<0)
                    return -1;
               break;
               
            }//switch
              
         }//for
         
         
         //
         GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
         GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
         GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

         //pin /CS 
         GPIO_InitStructure.GPIO_Pin = ad5262_info->pin_cs.gpio_pin_no;
         GPIO_Init(ad5262_info->pin_cs.gpio_bank_no, &GPIO_InitStructure);
         //not selected /cs=1
         GPIO_WriteBit(ad5262_info->pin_cs.gpio_bank_no,ad5262_info->pin_cs.gpio_pin_no,Bit_SET);
         
          //pin /SHDN
         GPIO_InitStructure.GPIO_Pin = ad5262_info->pin_shdn.gpio_pin_no;
         GPIO_Init(ad5262_info->pin_shdn.gpio_bank_no, &GPIO_InitStructure);
         // not active /SHDN = 1
         GPIO_WriteBit(ad5262_info->pin_shdn.gpio_bank_no,ad5262_info->pin_shdn.gpio_pin_no,Bit_SET);
         
         //pin PR
         GPIO_InitStructure.GPIO_Pin = ad5262_info->pin_pr.gpio_pin_no;
         GPIO_Init(ad5262_info->pin_pr.gpio_bank_no, &GPIO_InitStructure);
         //positive edge set PR=0
         GPIO_WriteBit(ad5262_info->pin_pr.gpio_bank_no,ad5262_info->pin_pr.gpio_pin_no,Bit_RESET);
         __kernel_usleep(10000); //10000 uS, 10 ms 
         //positive edge set PR=1
         GPIO_WriteBit(ad5262_info->pin_pr.gpio_bank_no,ad5262_info->pin_pr.gpio_pin_no,Bit_SET);
         //
         ad5262_info->resistor_value=0x80; //128 mid range /pr
         //
         ofile_lst[desc].p=ad5262_info;
      }
      break;

      //
      case I_UNLINK: {
      }
      break;
      
      //
      default:
         return -1;

   }
   //
   return 0;
}


/*============================================
| End of Source  : dev_ad5262.c
==============================================*/
