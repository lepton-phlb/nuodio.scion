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
#include "kernel/core/signal.h"
#include "kernel/core/libstd.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"
#include "kernel/core/devio.h"
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
#include "lib/libc/stdio/stdio.h"

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/ioctl_rotary_encoder.h"
#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_peripherals/ioctl_hybrid_tube_gpio.h"

/*===========================================
Global Declaration
=============================================*/

static  unsigned char buf[256*64/2];
/*===========================================
Implementation
=============================================*/
#define DEFAULT_GAIN_IN    255 
#define DEFAULT_GAIN_OUT   255
/*-------------------------------------------
| Name:tstboard_main
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int tstboard_main(int argc,char* argv[]){
   int fd_rotary1;
   int fd_rotary2;
   int fd_rotary3;
   int fd_rotary4;
   
   
   int fd_g_inr;
   int fd_g_inl;
   int fd_g_outr;
   int fd_g_outl;
   
   int fd_g_sat;
   int fd_r_tone;
   int fd_r_lvl;
   
   int fd_gpio;
   int fd_i2s3;
   
   
   uchar8_t rotary_switch1=0;
   uchar8_t rotary_switch2=0;
   uchar8_t rotary_switch3=0;
   uchar8_t rotary_switch4=0;
   
   int timeout=1000;
   struct timeval time_out;
   fd_set readfs;
   
   uchar8_t u8_resistor;
   
   
   //
#if 0
   FILE* fs;
   int fd_tty;
   int fd_lcd;
   //
   if((fd_tty=open("/dev/tty0",O_WRONLY,0))<0)
      return -1;
   // 
   if((fd_lcd=open("/dev/lcd0.0",O_WRONLY,0))<0)
      return -1;
   //
   ioctl(fd_tty,I_LINK,fd_lcd);
   //
   memset((unsigned char*)buf,0,sizeof(buf));
   write(fd_lcd,buf,sizeof(buf));
   //
   write(fd_tty,"           ",10);
   write(fd_tty,"E",1);
   write(fd_tty,"E",1);
#endif
   
   //rotary encoder
   if((fd_rotary1=open("/dev/rotry1",O_RDONLY|O_NONBLOCK,0))<0)
      return -1;
   printf("rotary1 open\r\n");
   
   if((fd_rotary2=open("/dev/rotry2",O_RDONLY|O_NONBLOCK,0))<0)
      return -1;
   printf("rotary2 open\r\n");
   
   if((fd_rotary3=open("/dev/rotry3",O_RDONLY|O_NONBLOCK,0))<0)
      return -1;
   printf("rotary3 open\r\n");
    
   if((fd_rotary4=open("/dev/rotry4",O_RDONLY|O_NONBLOCK,0))<0)
      return -1;
   printf("rotary4 open\r\n");
   
   //gain in
   if((fd_g_inr=open("/dev/g_inr",O_WRONLY,0))<0)
      return -1;
   printf("g in r open\r\n");
   
   if((fd_g_inl=open("/dev/g_inl",O_WRONLY,0))<0)
      return -1;
   printf("g in l open\r\n");
   
   //gain out
   if((fd_g_outr=open("/dev/g_outr",O_WRONLY,0))<0)
      return -1;
   printf("g out r open\r\n");
   
   if((fd_g_outl=open("/dev/g_outl",O_WRONLY,0))<0)
      return -1;
   printf("g out l open\r\n");
   
   //gain saturation
   if((fd_g_sat=open("/dev/g_sat",O_WRONLY,0))<0)
      return -1;
   printf("g sat open\r\n");
   
    //r tone
   if((fd_r_tone=open("/dev/r_tone",O_WRONLY,0))<0)
      return -1;
   printf("r tone open\r\n");
   
   //
   //init gain on line in (Right/Left) and line out (Right/Left)
   //line in
   u8_resistor = DEFAULT_GAIN_IN;
   write(fd_g_inr,&u8_resistor,1);
   write(fd_g_inl,&u8_resistor,1);
   //line out
   u8_resistor = DEFAULT_GAIN_OUT;
   write(fd_g_outr,&u8_resistor,1);
   write(fd_g_outl,&u8_resistor,1);
   
   //gpio audio routing
   if((fd_gpio=open("/dev/gpio",O_RDWR,0))<0)
      return -1;
   printf("gpio open\r\n");
   //
   ioctl(fd_gpio,IN2ADC_DAC2OUT);
   
   //i2s3
   if((fd_i2s3=open("/dev/i2s3",O_RDWR,0))<0)
      return -1;
   printf("i2s3 open\r\n");
   
   
   //
   FD_ZERO(&readfs);
   //
   for(;;){
     
       //
      time_out.tv_sec = timeout/1000;
      time_out.tv_usec = (timeout%1000)*1000;
      //
      FD_SET(fd_rotary1,&readfs);
      FD_SET(fd_rotary2,&readfs);
      FD_SET(fd_rotary3,&readfs);
      FD_SET(fd_rotary4,&readfs);
      //

      switch( select(fd_rotary4+1,&readfs,0,0,&time_out) ) {
         case 0:
            //if(!(timeout%100))
            //printf("elapse time:%d s.\r\n",timeout/100);
            ioctl(fd_rotary1,ROTRYSWTCH,&rotary_switch1);
            ioctl(fd_rotary2,ROTRYSWTCH,&rotary_switch2);
            ioctl(fd_rotary3,ROTRYSWTCH,&rotary_switch3);
            ioctl(fd_rotary4,ROTRYSWTCH,&rotary_switch4);
            printf("switch 1:%d 2:%d 3:%d 4:%d\r\n",rotary_switch1,rotary_switch2,rotary_switch3,rotary_switch4);
            
         break;
         
         default:
            //
            if(FD_ISSET(fd_rotary1,&readfs)) {   //rotary 1
               int cb; 
               int32_t counter=0;
      
               if((cb=read(fd_rotary1,&counter,sizeof(counter)))==sizeof(counter)){
                  printf("rotary1 =%d\r\n",counter);
                  //
                  u8_resistor = counter;
                  write(fd_g_inr,&u8_resistor,1);
                  write(fd_g_inl,&u8_resistor,1);
               }
            }
            if(FD_ISSET(fd_rotary2,&readfs)) {   //rotary 2
               int cb; 
               int32_t counter=0;
      
               if((cb=read(fd_rotary2,&counter,sizeof(counter)))==sizeof(counter)){
                  printf("rotary2 =%d\r\n",counter);
                  //
                  u8_resistor = counter;
                  write(fd_g_outr,&u8_resistor,1);
                  write(fd_g_outl,&u8_resistor,1);
               }
            }
            if(FD_ISSET(fd_rotary3,&readfs)) {   //rotary 3
               int cb; 
               int32_t counter=0;
      
               if((cb=read(fd_rotary3,&counter,sizeof(counter)))==sizeof(counter)){
                  printf("rotary3 =%d\r\n",counter);
                  //
                  u8_resistor = counter;
                  write(fd_g_sat,&u8_resistor,1);
               }
            }
            if(FD_ISSET(fd_rotary4,&readfs)) {   //rotary 4
               int cb; 
               int32_t counter=0;
      
               if((cb=read(fd_rotary4,&counter,sizeof(counter)))==sizeof(counter)){
                  printf("rotary4 =%d\r\n",counter);
                  //
                  u8_resistor = counter;
                  write(fd_r_tone,&u8_resistor,1);
               }
            }
         break;
      }
      
     

   }
   
   //
   return 0;
}

  
/*===========================================
End of Source more.c
=============================================*/