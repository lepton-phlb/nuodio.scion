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

#define NUODIO_FIRMWARE_DIRECTORY_PATH "/sdcard/firmware"
#define NUODIO_FIRMWARE_FILENAME "nuodio-nu.tube-firmware.bin"

#define APPLICATION_ADDRESS   (uint32_t)0x08100000 
typedef  void (*pFunction)(void);

/*===========================================
Implementation
=============================================*/

/*-------------------------------------------
| Name: jump_to_firmware
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int jump_to_firmware(void){
  
   pFunction Jump_To_Application;
   uint32_t JumpAddress;
   //
   if (((*(volatile uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000){ 
      //
      __stop_sched();
      __disable_interrupt_section_in();
         
      /* Jump to user application */
      JumpAddress = *(volatile uint32_t*) (APPLICATION_ADDRESS + 4);
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(volatile uint32_t*) APPLICATION_ADDRESS);
      Jump_To_Application();
   }else{
     printf("error: no valid firmware!\r\n");
     return -1;
   }
   //
   return 0;
}

/*-------------------------------------------
| Name: mount_usb_storage
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int mount_usb_storage(void){
   int fd_sdio;
   int fd_usbmsd;
   //
   fd_sdio= open("/dev/sdio0",O_RDWR,0);
   if(fd_sdio<0){
      printf("error: cannot open: /dev/sdio0!\r\n");
      return -1;
   }
   //
   fd_usbmsd= open("/dev/usbmsd",O_RDWR,0);
   if(fd_usbmsd<0){
      printf("error: cannot open: /dev/usbmsd!\r\n");
      return -1;
   }
   //
   if(ioctl(fd_usbmsd,I_LINK,fd_sdio)<0){
      printf("error: cannot mount: /dev/usbmsd on /dev/sdio0!\r\n");
      return -1;
   }
   //
   if(fattach(fd_usbmsd,"/dev/usbsd0")<0){
      printf("error: cannot fattach: /dev/usbsd0\r\n");
      return -1;
   }
   //
   printf("usb storage mounted\r\n");
   //
   return 0;
}

/*-------------------------------------------
| Name: flash_firmware
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int flash_firmware(void){
   int fd;
#if 0   
    mount(fstype,argv[3],argv[4]);
   
   //find firmware
   fd=open("/sdcard/firmware/nuodio-nu.tube-firmware.bin",O_RDONLY,0);
   if(fd<0)
      return -1;
   //
   
#endif
   
}

/*-------------------------------------------
| Name: boot_main
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int boot_main(int argc,char* argv[]){
   int i;
   //unsigned int opt=0;
  
   //
   for(i=1; i<argc; i++) {
      if(!argv[i])
         break;
      //
      if(strcmp(argv[i],"jump")==0){
         jump_to_firmware();
      }else if(strcmp(argv[i],"storage")==0){
        mount_usb_storage();
      }else if(strcmp(argv[i],"flasher")==0){
      }
      //
   }
    
   //for(;;){
   //usleep(10000);
   //}
 
   //
   return 0;
}



