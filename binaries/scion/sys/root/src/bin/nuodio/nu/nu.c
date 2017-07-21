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

#include "kernel/core/ioctl_lcd.h"

#include "lib/libc/unistd.h"
#include "lib/libc/termios/termios.h"
#include "lib/libc/stdio/stdio.h"

#include "lib/graphic/xlcd/xlcd.h"
#include "lib/graphic/xlcd/xlcd_layout.h"
#include "lib/graphic/xgui/xgui_keyboard.h"
#include "lib/graphic/xgui/xgui_system.h"
#include "lib/graphic/xgui/segger_glib/src/gui/gui.h"

#include "bin/nuodio/nu/nu.h"
#include "bin/nuodio/nu/displays/nu_displays_definition.h"

/*============================================
| Global Declaration
==============================================*/

const int nu_display_main_definition_size = sizeof(nu_display_main_definition) / sizeof(DISPLAY_DEFINITION);

 DISPLAY_LIST  nu_display_list[] = {
   { NU_DISPLAY_MAIN_DEFINITION_SIZE,
   (DISPLAY_DEFINITION*)&nu_display_main_definition,
   &nu_display_main_app
   }
};

extern const GUI_BITMAP bmlabel_interface_full_fat;
extern const GUI_BITMAP bmlabel_interface_full_thin;


/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        callback_xgui
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
extern const GUI_BITMAP bmselect_arrow;
static int callback_xgui(struct xlcd_context_st* const xlcd_context,
                  char key, unsigned int status,
                  unsigned int xtime) {
   //
   struct nu_xgui_st *p_nu_xgui = (struct nu_xgui_st*)(xlcd_context->p);
   //
   switch ( status ) {
      case (__XLCD_INIT) :{
         //default initialization
         p_nu_xgui->distortion_type = DIST_TYPE_DFLT;
         p_nu_xgui->sat_value = SAT_VALUE_DFLT;
         p_nu_xgui->tone_value = TONE_VALUE_DFLT;
         p_nu_xgui->level_value = LEVEL_VALUE_DFLT;
         //
         guiSetDisplayList((DISPLAY_LIST*)&nu_display_list);
         guiManagerStart((struct xlcd_core_context_st*)xlcd_context);
        
         GUI_DrawBitMap(0,0,(GUI_BITMAP*)&bmlabel_interface_full_fat,DRAWMODE_WRITE);
         LCDGRefreshSim();
         ioctl(STDOUT_FILENO, LCDFLSBUF, NULL);
         
         GUI_DrawBitMap(0,0,(GUI_BITMAP*)&bmlabel_interface_full_thin,DRAWMODE_WRITE);
         LCDGRefreshSim();
         ioctl(STDOUT_FILENO, LCDFLSBUF, NULL);
        
      }
      break;
      //
      default:
         xgui_engine((struct xlcd_core_context_st*)xlcd_context, key, status, xtime);
      break;
   }

   return 0;
}
/*--------------------------------------------
| Name:        nu_main
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_main(int argc, char* argv[]) {
   struct xlcd_attr_st attr;
   struct xlcd_core_context_st xlcd_core_context;
   struct nu_xgui_st nu_xgui;
   //
   memset (&xlcd_core_context,     0, sizeof(xlcd_core_context));
  
   //
   if ((nu_xgui.fd_keybfootswitch = open("/dev/kbftsw", O_RDONLY|O_NONBLOCK,0)) < 0) {
      printf("warning: cannot open /dev/ftswtch\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_1 = open("/dev/rotry1", O_RDONLY|O_NONBLOCK,0)) < 0) {
      printf("warning: cannot open /dev/rotry1\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_2 = open("/dev/rotry2", O_RDONLY|O_NONBLOCK,0)) < 0) {
      printf("warning: cannot open /dev/rotry2\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_3 = open("/dev/rotry3", O_RDONLY|O_NONBLOCK,0)) < 0) {
      printf("warning: cannot open /dev/rotry3\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_4 = open("/dev/rotry4", O_RDONLY|O_NONBLOCK,0)) < 0) {
      printf("warning: cannot open /dev/rotry4\r\n");
   }

   //
   xlcd_core_context.xlcd_context.p = &nu_xgui;
   attr.callback = callback_xgui;
   attr.argc = argc;
   attr.argv = argv;
   //
   attr.layout.kb_layout = (void*)0;

   attr.fdin_server = -1;
   attr.fdout_server = -1;
   //
   if (xlcd_create(&xlcd_core_context, &attr) < 0)
      return -1;
   //
   if(nu_xgui.fd_rotary_1!=-1){
      xlcd_select_fd(&xlcd_core_context, nu_xgui.fd_rotary_1);
   }
   //
   if(nu_xgui.fd_keybfootswitch!=-1){
      xlcd_select_fd(&xlcd_core_context, nu_xgui.fd_keybfootswitch);
   }
   if(nu_xgui.fd_rotary_2!=-1){
      xlcd_select_fd(&xlcd_core_context, nu_xgui.fd_rotary_2);
   }
   if(nu_xgui.fd_rotary_3!=-1){
      xlcd_select_fd(&xlcd_core_context, nu_xgui.fd_rotary_3);
   }
   if(nu_xgui.fd_rotary_4!=-1){
      xlcd_select_fd(&xlcd_core_context, nu_xgui.fd_rotary_4);
   }
   //
   xlcd_engine(&xlcd_core_context);  // forever
   return 0;
}

/*============================================
| End of Source  : nu.c
==============================================*/
