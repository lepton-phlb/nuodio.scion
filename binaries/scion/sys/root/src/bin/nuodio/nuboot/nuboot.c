/* Copyright (c) 2017 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/


/*============================================
| Includes
==============================================*/
#include <stdint.h>
#include <stdlib.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/errno.h"
#include "kernel/core/dirent.h"
#include "kernel/core/system.h"
#include "kernel/core/libstd.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/time.h"
#include "kernel/core/wait.h"
#include "kernel/core/select.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_hd.h"
#include "kernel/core/ioctl_lcd.h"

#include "kernel/fs/vfs/vfstypes.h"

#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"

#include "lib/libc/unistd.h"
#include "lib/libc/termios/termios.h"
#include "lib/libc/stdio/stdio.h"

#include "lib/graphic/xlcd/xlcd.h"
#include "lib/graphic/xlcd/xlcd_layout.h"
#include "lib/graphic/xgui/xgui_keyboard.h"
#include "lib/graphic/xgui/xgui_system.h"
#include "lib/graphic/xgui/segger_glib/src/gui/gui.h"

#include "bin/nuodio/nuboot/nuboot.h"

/*============================================
| Global Declaration
==============================================*/
#define NUODIO_FIRMWARE_FILEPATH "/sdcard/firmware/nuodio-nu.tube-firmware.bin"
//
#define APPLICATION_ADDRESS   (uint32_t)0x08100000 
typedef  void (*pFunction)(void);

//
extern const GUI_BITMAP bmlabel_bootloader_nb_message_copy;
extern const GUI_BITMAP bmlabel_bootloader_nb_icons_copy;
//
extern const GUI_BITMAP bmlabel_bootloader_nb_message_update;
extern const GUI_BITMAP bmlabel_bootloader_nb_progress_bar;
extern const GUI_BITMAP bmlabel_bootloader_nb_progress_element;
#define PROGRESS_BAR_ELEMENT_START_X_PIXEL (67)
#define PROGRESS_BAR_ELEMENT_START_Y_PIXEL (35)
#define PROGRESS_BAR_ELEMENT_WIDTH_PIXEL (6)
#define PROGRESS_BAR_ELEMENT_MAX (20)

/*============================================
| Implementation
==============================================*/

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
      fprintf(stderr,"error: cannot open: /dev/sdio0!\r\n");
      return -1;
   }
   //
   fd_usbmsd= open("/dev/usbmsd",O_RDWR,0);
   if(fd_usbmsd<0){
      fprintf(stderr,"error: cannot open: /dev/usbmsd!\r\n");
      return -1;
   }
   //
   if(ioctl(fd_usbmsd,I_LINK,fd_sdio)<0){
      fprintf(stderr,"error: cannot mount: /dev/usbmsd on /dev/sdio0!\r\n");
      return -1;
   }
   //
   if(fattach(fd_usbmsd,"/dev/usbsd0")<0){
      fprintf(stderr,"error: cannot fattach: /dev/usbsd0\r\n");
      return -1;
   }
   //
   return 0;
}

/*-------------------------------------------
| Name: mount_sdcard_storage
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int mount_sdcard_storage(void){
   //
   if(mkdir("/sdcard",0)<0){
      return -1;
   }
   //
   return mount(fs_fatfs,"/dev/null","/sdcard");
}

/*-------------------------------------------
| Name: mount_sdcard_storage
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int is_firmware_file_exist(void){
   int fd=-1;
   //
   fd=open(NUODIO_FIRMWARE_FILEPATH,O_RDONLY,0);
   if(fd<0){
      return 0;
   }
   //
   close(fd);
   //
   return 1;
}

/*-------------------------------------------
| Name: mount_sdcard_storage
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int flash_firmware_file(void){
   int fd=-1;
   int fd_cpu_flash;
   int cb=0;
   int cb_flashed=0;
   struct stat stat_info;
   static uint8_t buffer[1024]; 
   static uint32_t nb_progress_element;
   //
   if(stat(NUODIO_FIRMWARE_FILEPATH,&stat_info)<0){
      return -1;
   }
   //
   if(!S_ISREG(stat_info.st_mode)){
      return -1;
   }
   //
   fd_cpu_flash=open("/dev/flash0",O_RDWR,0);
   if(fd_cpu_flash<0){
      return -1;
   }
   //
   fd=open(NUODIO_FIRMWARE_FILEPATH,O_RDONLY,0);
   if(fd<0){
      close(fd_cpu_flash);
      return -1;
   }
   //erase flash
   if(ioctl(fd_cpu_flash,HDCLRDSK,(void*)0)<0){
      close(fd);
      close(fd_cpu_flash);
      return -1;
   }
   //
   cb=read(fd,buffer,sizeof(buffer));
   while(cb>0){
      //write buffer in flash
      if(write(fd_cpu_flash,buffer,cb)<0){
         close(fd);
         close(fd_cpu_flash);
         return -1;
      }
      //
      cb_flashed+=cb;
      //
      nb_progress_element =((cb_flashed*100)/stat_info.st_size)/((100)/(PROGRESS_BAR_ELEMENT_MAX));
      if(nb_progress_element>0){
         //
         GUI_DrawBitMap(PROGRESS_BAR_ELEMENT_START_X_PIXEL+(nb_progress_element-1)*PROGRESS_BAR_ELEMENT_WIDTH_PIXEL,
                        PROGRESS_BAR_ELEMENT_START_Y_PIXEL,
                        (GUI_BITMAP*)&bmlabel_bootloader_nb_progress_element,
                        DRAWMODE_WRITE);
         //
         LCDGRefreshSim();
         ioctl(STDOUT_FILENO, LCDFLSBUF, NULL);
      }
      //
      cb=read(fd,buffer,sizeof(buffer));
   }
   //
   GUI_DrawBitMap(PROGRESS_BAR_ELEMENT_START_X_PIXEL+(PROGRESS_BAR_ELEMENT_MAX-1)*PROGRESS_BAR_ELEMENT_WIDTH_PIXEL,
                  PROGRESS_BAR_ELEMENT_START_Y_PIXEL,
                  (GUI_BITMAP*)&bmlabel_bootloader_nb_progress_element,
                  DRAWMODE_WRITE);
   //
   LCDGRefreshSim();
   ioctl(STDOUT_FILENO, LCDFLSBUF, NULL);
   //
   close(fd);
   close(fd_cpu_flash);
   //
   remove(NUODIO_FIRMWARE_FILEPATH);
   //
   return 1;
}

/*--------------------------------------------
| Name:        callback_xgui
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
static int callback_xgui(struct xlcd_context_st* const xlcd_context,
                  char key, unsigned int status,
                  unsigned int xtime) {
   //
   struct nuboot_xgui_st *p_nuboot_xgui = (struct nuboot_xgui_st*)(xlcd_context->p);
   //
   switch ( status ) {
      case (__XLCD_INIT) :{
         //
         xgui_init();
         //
         if(xlcd_context->argc>1){
            if(xlcd_context->argv[1]!=(char*)0 && !strcmp(xlcd_context->argv[1],"storage")){
               //
               GUI_DrawBitMap(32,16,(GUI_BITMAP*)&bmlabel_bootloader_nb_message_copy,DRAWMODE_WRITE);
               GUI_DrawBitMap(32,32,(GUI_BITMAP*)&bmlabel_bootloader_nb_icons_copy,DRAWMODE_WRITE);
               //
               LCDGRefreshSim();
               ioctl(STDOUT_FILENO, LCDFLSBUF, NULL);   
            }else if(xlcd_context->argv[1]!=(char*)0 && !strcmp(xlcd_context->argv[1],"jumpflash")){
               if(!is_firmware_file_exist()){
                  //panic
                  //to remove: force jump
                  exit(-1);
               } 
               //
               GUI_DrawBitMap(32,16,(GUI_BITMAP*)&bmlabel_bootloader_nb_message_update,DRAWMODE_WRITE);
               GUI_DrawBitMap(32,32,(GUI_BITMAP*)&bmlabel_bootloader_nb_progress_bar,DRAWMODE_WRITE);
               //
               LCDGRefreshSim();
               ioctl(STDOUT_FILENO, LCDFLSBUF, NULL);   
               //
               // to do check firmware intergrity
               // if firmware check integrity failed add message "invalid firmware signature"
               //
               if(flash_firmware_file()<0){
                   exit(-1);
               }
               //
               exit(0);
            }
         }
         //
      }
      break;
      //
      default:
         //xgui_engine((struct xlcd_core_context_st*)xlcd_context, key, status, xtime);
      break;
   }

   return 0;
}
/*--------------------------------------------
| Name:        nuboot_main
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nuboot_main(int argc, char* argv[]) {
   struct xlcd_attr_st attr;
   struct xlcd_core_context_st xlcd_core_context;
   struct nuboot_xgui_st nuboot_xgui;
   //
   memset (&xlcd_core_context,0, sizeof(xlcd_core_context));
   //
   if(argc>1){
      if(argv[1]!=(char*)0 && !strcmp(argv[1],"storage")){
         //
         if(mount_usb_storage()<0){
            return -1;
         }
      }else if(argv[1]!=(char*)0 && !strcmp(argv[1],"jumpflash")){
         //
         if(mount_sdcard_storage()<0){
            return -1;
         }
         //
         if(!is_firmware_file_exist()){
            //
            jump_to_firmware();
         }
      }
   }else{
       //
       jump_to_firmware();
   }
  
   //
   if ((nuboot_xgui.fd_keybfootswitch = open("/dev/kbftsw", O_RDONLY|O_NONBLOCK,0)) < 0) {
      printf("warning: cannot open /dev/ftswtch\r\n");
   }
  
   //
   xlcd_core_context.xlcd_context.p = &nuboot_xgui;
   attr.callback = callback_xgui;
   attr.argc = argc;
   attr.argv = argv;
   //
   attr.layout.kb_layout = (void*)0;

   attr.fdin_server = -1;
   attr.fdout_server = -1;
   //
   if (xlcd_create(&xlcd_core_context, &attr) < 0){
      return -1;
   }
   //
   if(nuboot_xgui.fd_keybfootswitch!=-1){
      xlcd_select_fd(&xlcd_core_context, nuboot_xgui.fd_keybfootswitch);
   }
  
   //
   xlcd_engine(&xlcd_core_context);  // forever
   return 0;
}

/*============================================
| End of Source  : nu.c
==============================================*/
