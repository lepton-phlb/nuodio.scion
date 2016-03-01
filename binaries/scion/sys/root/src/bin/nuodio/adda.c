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
#include <intrinsics.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/system.h"
#include "kernel/core/signal.h"
#include "kernel/core/libstd.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"
#include "kernel/core/time.h"
#include "kernel/core/wait.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/select.h"
#include "kernel/core/stropts.h"
//
#include "kernel/core/ioctl_lcd.h"
#include "kernel/core/ioctl_fb.h"
//
#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/ioctl_hybrid_tube_gpio.h"

/*===========================================
Global Declaration
=============================================*/
#define USE_USB

//ADC DAC
#define ADC_DAC_SAMPLES_PER_BUFFER 48 //((48)*2) //32
#pragma data_alignment=4 
static int32_t audio_adc_dac_buffer[ADC_DAC_SAMPLES_PER_BUFFER]={0};

//usb
#define AUDIO_SAMPLING_FREQUENCY  (48000)
#define AUDIO_SAMPLE_BYTES        (2)  // 16 bits 2 Bytes
#define AUDIO_CHANNEL_NB          (2)  // stereo 2 channel 

#define AUDIO_IN_PACKET_SZ                            (uint32_t)(((AUDIO_SAMPLING_FREQUENCY * AUDIO_SAMPLE_BYTES *AUDIO_CHANNEL_NB)  /1000))

#pragma data_alignment=4 
static int16_t usb_audio_buffer_16bits[AUDIO_IN_PACKET_SZ]={0};

char adda_debug_flag = 0;
int32_t s_left;
int32_t s_right;
/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name:adda_main
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int adda_main(int argc,char* argv[]){
  
   int cb;
   int fd_gpio;
   int fd_i2s3;
   int fd_usbaudio;
   int i;
   int32_t* p_adda_audio_buffer_24bits= audio_adc_dac_buffer;
   int16_t* p_usb_audio_buffer_16bits= usb_audio_buffer_16bits;
  
   //
   if((fd_gpio=open("/dev/gpio",O_RDWR,0))<0)
      return -1;
   printf("gpio open\r\n");
   //
   ioctl(fd_gpio,IN2ADC_DAC2OUT);

#ifdef USE_USB
   //
   if(( fd_usbaudio = open("/dev/usbaudio",O_RDWR|O_NSYNC,0))<0)
      return -1;
   printf("usb audio open\r\n");
   
   //wait usb ok: 2s
   usleep(2000000); 
#endif
   
   //i2s3
   if((fd_i2s3=open("/dev/i2s3",O_RDWR,0))<0)
      return -1;
   printf("i2s3 open\r\n");

   //
   for(;;){
      cb = read(fd_i2s3,(char*)audio_adc_dac_buffer,sizeof(audio_adc_dac_buffer));
      //
      write(fd_i2s3,(char*)audio_adc_dac_buffer,cb);
#ifdef USE_USB
      //
      for(i=0;i<ADC_DAC_SAMPLES_PER_BUFFER;i++){
         int32_t i_left = 2*i;
         int32_t i_right = 2*i+1;
         
         int32_t s_left = *(p_adda_audio_buffer_24bits+i_left);
         int32_t s_right = *(p_adda_audio_buffer_24bits+i_right);
           
         *(p_usb_audio_buffer_16bits+i_left) = (int16_t)((s_left)&0x0000ffff);    
         *(p_usb_audio_buffer_16bits+i_right) = (int16_t)((s_right)&0x0000ffff);
      }
      //
      write(fd_usbaudio,p_usb_audio_buffer_16bits,ADC_DAC_SAMPLES_PER_BUFFER*sizeof(int16_t));
      //
#endif
   }
   
   //
   return 0;
}

  
/*===========================================
End of Source more.c
=============================================*/