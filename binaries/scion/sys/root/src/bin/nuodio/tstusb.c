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
#include <math.h>

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
#define AUDIO_SAMPLING_FREQUENCY  (48000)
#define AUDIO_SAMPLE_BYTES        (2)  // 16 bits 2 Bytes
#define AUDIO_CHANNEL_NB          (2)  // stereo 2 channel 

#define AUDIO_IN_PACKET_SZ                            (uint32_t)(((AUDIO_SAMPLING_FREQUENCY * AUDIO_SAMPLE_BYTES *AUDIO_CHANNEL_NB)  /1000))

#pragma data_alignment=4 
static int16_t audio_isoc_buffer_16bits[AUDIO_IN_PACKET_SZ]={0};


/*===========================================
Implementation
=============================================*/
/*-------------------------------------------
| Name: init_sinus_16_bit_48KHz_ex
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static void init_sinus_16_bit_48KHz_ex(int16_t *p_16bits_buffer,int32_t fs){
  int i;
   float PI=3.14159265;
   int32_t Amax= (0x7FFF>>2); //8388607/10;
   int32_t Nsamples=(48000/fs);
   //
   for(i=0;i<48;i++){
      int16_t s;
      s=Amax*sin(2.0*PI*((float)(fs))*(float)i*1/(48000.0));
      #if AUDIO_CHANNEL_NB==1
         //mono
         p_16bits_buffer[i] = s;    
      #elif AUDIO_CHANNEL_NB==2
         //stereo
         p_16bits_buffer[2*i] = s;    
         p_16bits_buffer[2*i+1] = s; 
      #endif
   }
}


/*-------------------------------------------
| Name: tstusb_main
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int tstusb_main(int argc,char* argv[]){
   int fd_usbaudio;
   
   //
   if(( fd_usbaudio = open("/dev/usbaudio",O_RDWR,0))<0)
      return -1;
   printf("usb audio open\r\n");
   //
   init_sinus_16_bit_48KHz_ex(audio_isoc_buffer_16bits,1000);
   //
   for(;;){
      write(fd_usbaudio,audio_isoc_buffer_16bits,AUDIO_IN_PACKET_SZ);
   }
   //
   return 0;
}

  
/*===========================================
End of Source more.c
=============================================*/