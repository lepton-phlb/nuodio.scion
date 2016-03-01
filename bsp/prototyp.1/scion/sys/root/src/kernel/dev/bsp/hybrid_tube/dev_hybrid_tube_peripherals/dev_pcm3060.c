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
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_gpio_ex.h"

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_hybrid_tube_gpio.h"

//#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/dev_pcm3060.h"

/*============================================
| Global Declaration
==============================================*/

#define GPIO_ARGC 4

//
typedef struct{
   uchar8_t SE:1;     //b0
   uchar8_t RSV:3;    //b3,b2,b1
   uchar8_t DAPSV:1;  //b4
   uchar8_t ADPSV:1;  //b5
   uchar8_t SRST:1;   //b6
   uchar8_t MRST:1;   //b7
}pcm3060_register_x40_t;

//
typedef struct{
  uchar8_t FMT2:2;  //b1,b0
  uchar8_t RSV:2;    //b3,b2
  uchar8_t MS:3;     //b6,b5,b4 
  uchar8_t CSEL2:1;  //b7
}pcm3060_register_x43_t;

//
typedef struct{
   uchar8_t MUT2:2;     //b1,b0;
   uchar8_t DREV2:1;    //b2
   uchar8_t RSV2:3;     //b5,b4,b3
   uchar8_t OVER:1;     //b6 
   uchar8_t RSV1:1;     //b7
}pcm3060_register_x44_t;

//
typedef struct{
   uchar8_t AZRO:1;   //b0
   uchar8_t ZREV:1;   //b1 
   uchar8_t RSV:2;    //b3,b2
   uchar8_t DMC:1;    //b4 
   uchar8_t DMF0:1;   //b5  
   uchar8_t DMF1:1;   //b6
   uchar8_t FLT:1;    //b7
}pcm3060_register_x45_t;


//
typedef struct{
   uchar8_t FMT1:2;     //b1,b0 
   uchar8_t RSV:2;      //b3,b2
   uchar8_t MS:3;       //b6,b5,b4
   uchar8_t CSEL1:1;    //b7
}pcm3060_register_x48_t;


//
typedef struct{
   uchar8_t MUT1:2;     //b0,b1 
   uchar8_t DREV1:1;    //b2
   uchar8_t BYP:1;      //b3
   uchar8_t ZCDD:1;     //b4
   uchar8_t RSV:3;      //b7,b6,b5
}pcm3060_register_x49_t;



typedef struct pcm3060_info_st{
  //filled by user
  gpio_info_t pin_cs;
  gpio_info_t pin_reset;
  gpio_info_t pin_pr;
  //
  desc_t desc_spi_w;
 
}pcm3060_info_t;


static pcm3060_info_t g_pcm3060_info;

const pcm3060_register_x40_t pcm3060_register_x40_dflt={
   .MRST=1,
   .SRST=1,
   .ADPSV=0, // adc normal mode
   .DAPSV=0, // dac normal mode
   .RSV=0,
   .SE = 1 // dac output Single-ended
};

//DAC
const pcm3060_register_x43_t pcm3060_register_x43_dflt={
   .CSEL2=0, // SCKI2, BCK2, LRCK2 enabled for dac operation (default)
   .MS=0,  //to do DAC in slave mode and adc in master mode // for adc MS=010 for 24.576 MHz and 48KHz sampling rate Master mode, 512 fS
   .RSV=0,
   .FMT2=0 // 00 24-bit dac I2S format (default)
};

//ADC
const pcm3060_register_x48_t pcm3060_register_x48_dflt={
   .CSEL1=0, //SCKI1, BCK1, LRCK1 enabled for ADC operation (default)
   .MS=2,  //ADC MS=010 for 24.576 MHz and 48KHz sampling rate Master mode, 512 fS
   .FMT1=0 // 00 24-bit dac I2S format (default)
};





//
const char dev_pcm3060_name[]="pcm3060\0";

static int dev_pcm3060_load(void);
static int dev_pcm3060_open(desc_t desc, int o_flag);
static int dev_pcm3060_close(desc_t desc);
static int dev_pcm3060_isset_read(desc_t desc);
static int dev_pcm3060_isset_write(desc_t desc);
static int dev_pcm3060_read(desc_t desc, char* buf,int size);
static int dev_pcm3060_write(desc_t desc, const char* buf,int size);
static int dev_pcm3060_seek(desc_t desc,int offset,int origin);
static int dev_pcm3060_ioctl(desc_t desc,int request,va_list ap);


dev_map_t dev_pcm3060_map={
   dev_pcm3060_name,
   S_IFBLK,
   dev_pcm3060_load,
   dev_pcm3060_open,
   dev_pcm3060_close,
   dev_pcm3060_isset_read,
   dev_pcm3060_isset_write,
   dev_pcm3060_read,
   dev_pcm3060_write,
   dev_pcm3060_seek,
   dev_pcm3060_ioctl //ioctl
};


/*============================================
| Implementation
==============================================*/
/*-------------------------------------------
| Name:dev_pcm_3060_config_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm_3060_config_write(pcm3060_info_t* pcm3060_info, uchar8_t register_no, uchar8_t* value){
   //
   desc_t desc_spi_w = INVALID_DESC;
   //
   if(pcm3060_info==(pcm3060_info_t*)0){
      return -1;
   }
   //
   desc_spi_w =  pcm3060_info->desc_spi_w;
   //
   register_no = register_no & 0x7F; //D7 must be set to 0
   // set pin /CS = 0
   GPIO_WriteBit(pcm3060_info->pin_cs.gpio_bank_no,pcm3060_info->pin_cs.gpio_pin_no,Bit_RESET);
   // send register number
   ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)&register_no,1);
   // send value
   ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)value,1);
   // set pin /CS = 1
   GPIO_WriteBit(pcm3060_info->pin_cs.gpio_bank_no,pcm3060_info->pin_cs.gpio_pin_no,Bit_SET);
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_rotary_encoder_x_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_load(){
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_pcm3060_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
 
   }

   return 0;

}

/*-------------------------------------------
| Name:dev_pcm3060_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_close(desc_t desc){

   return 0;
}

/*-------------------------------------------
| Name:dev_pcm3060_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_isset_read(desc_t desc){
  return -1;
}

/*-------------------------------------------
| Name:dev_pcm3060_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_pcm3060_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_read(desc_t desc, char* buf,int size){
   return -1;
}

/*-------------------------------------------
| Name:dev_pcm3060_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_write(desc_t desc, const char* buf,int size){
   pcm3060_info_t* pcm3060_info = (pcm3060_info_t*)(ofile_lst[desc].p);
   desc_t desc_spi_w = INVALID_DESC;
   int cb =0;
   
   //
   if(pcm3060_info==(pcm3060_info_t*)0){
      return -1;
   }
   //
   desc_spi_w = ofile_lst[desc].desc_nxt[1];
   if(desc_spi_w==INVALID_DESC)
      return -1;
   //
   for(cb=0;cb<size;cb++){
      // set pin /CS = 0
      GPIO_WriteBit(pcm3060_info->pin_cs.gpio_bank_no,pcm3060_info->pin_cs.gpio_pin_no,Bit_RESET);
      // send data
      ofile_lst[desc_spi_w].pfsop->fdev.fdev_write(desc_spi_w,(const char *)&buf[cb],1);
      // set pin /CS = 1
      GPIO_WriteBit(pcm3060_info->pin_cs.gpio_bank_no,pcm3060_info->pin_cs.gpio_pin_no,Bit_SET);
   }
   //
   return cb;
}

/*-------------------------------------------
| Name:dev_pcm3060_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_seek(desc_t desc,int offset,int origin){
   return -1;
}

/*-------------------------------------------
| Name:dev_pcm3060_ioctl
| Description: ex: mount /dev/pcm3060 /dev/spi0 /dev/g_in 0 e.15 f.2 f.5 (a0, cs, shdn, pr)
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_pcm3060_ioctl(desc_t desc,int request,va_list ap){
   
   switch(request) {
     
      case I_LINK: {
         int argc;
         char** argv;
         int i;
         int fd;
         //
         pcm3060_info_t* pcm3060_info;
         GPIO_InitTypeDef GPIO_InitStructure;

         //pcm3060_info = _sys_malloc(sizeof(pcm3060_info_t));
         pcm3060_info = &g_pcm3060_info;
        
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
               case GPIO_ARGC: //cs
                  if(gpio_info_init(&pcm3060_info->pin_cs,argv[i])<0)
                    return -1;
               break;
               
               case GPIO_ARGC+1: //reset
                  if(gpio_info_init(&pcm3060_info->pin_reset,argv[i])<0)
                    return -1;
               break;
               
            }//switch
              
         }//for
         
         
         //
         GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
         GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
         GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

         //pin /CS 
         GPIO_InitStructure.GPIO_Pin = pcm3060_info->pin_cs.gpio_pin_no;
         GPIO_Init(pcm3060_info->pin_cs.gpio_bank_no, &GPIO_InitStructure);
         //not selected /cs=1
         GPIO_WriteBit(pcm3060_info->pin_cs.gpio_bank_no,pcm3060_info->pin_cs.gpio_pin_no,Bit_SET);
         
          //pin /RST
         GPIO_InitStructure.GPIO_Pin = pcm3060_info->pin_reset.gpio_pin_no;
         GPIO_Init(pcm3060_info->pin_reset.gpio_bank_no, &GPIO_InitStructure);
         // not active /RST= 1
         GPIO_WriteBit(pcm3060_info->pin_reset.gpio_bank_no,pcm3060_info->pin_reset.gpio_pin_no,Bit_SET);
         __kernel_usleep(10000); //10000 uS, 10 ms 
         // not active /RST= 0
         GPIO_WriteBit(pcm3060_info->pin_reset.gpio_bank_no,pcm3060_info->pin_reset.gpio_pin_no,Bit_RESET);
         __kernel_usleep(10000); //10000 uS, 10 ms 
          // not active /RST= 1
         GPIO_WriteBit(pcm3060_info->pin_reset.gpio_bank_no,pcm3060_info->pin_reset.gpio_pin_no,Bit_SET);
         __kernel_usleep(10000); //10000 uS, 10 ms 
         //
         pcm3060_info->desc_spi_w = ofile_lst[desc].desc_nxt[1];
         //
         if( pcm3060_info->desc_spi_w ==INVALID_DESC)
            return -1;
         //
         ofile_lst[desc].p=pcm3060_info;
         //
         //send pcm dac configuration 
         dev_pcm_3060_config_write(pcm3060_info,0x43,(uchar8_t*)&pcm3060_register_x43_dflt);
         //send pcm adc configuration 
         dev_pcm_3060_config_write(pcm3060_info,0x48,(uchar8_t*)&pcm3060_register_x48_dflt);
         //send pcm adc dac in normal mode configuration 
         dev_pcm_3060_config_write(pcm3060_info,0x40,(uchar8_t*)&pcm3060_register_x40_dflt);
         
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
| End of Source  : dev_pcm3060.c
==============================================*/
