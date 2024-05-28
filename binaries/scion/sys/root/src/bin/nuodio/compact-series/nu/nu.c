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
#include "kernel/core/ioctl_lcd.h"

#include "kernel/dev/dev_mem/dev_mem.h"

#include "lib/libc/unistd.h"
#include "lib/libc/termios/termios.h"
#include "lib/libc/stdio/stdio.h"

#include "lib/graphic/xlcd/xlcd.h"
#include "lib/graphic/xlcd/xlcd_layout.h"
#include "lib/graphic/xgui/xgui_keyboard.h"
#include "lib/graphic/xgui/xgui_system.h"
#include "lib/graphic/xgui/segger_glib/src/gui/gui.h"

#include "bin/nuodio/compact-series/nu/nu_effect_parameters.h"
#include "bin/nuodio/compact-series/nu/parser/numl_parser.h"
#include "bin/nuodio/compact-series/nu/preset/nu_preset.h"
#include "bin/nuodio/compact-series/nu/nu.h"
#include "bin/nuodio/compact-series/nu/displays/nu_displays_definition.h"

/*============================================
| Global Declaration
==============================================*/

//shared memory
static shared_memory_midi_t g_shared_memory_midi;

//
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
| Name:        nu_switch_preset
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_switch_preset(nu_xgui_t *p_nu_xgui,int from_preset_no,int to_preset_no){
   nu_preset_t  nu_preset_to_backup;
   
   //save destination preset 'to_preset_no'
   if(nu_preset_get(&p_nu_xgui->nu_preset_storage,&nu_preset_to_backup,to_preset_no)<0){
      return -1;
   } 
   //copy current preset to 'to_preset_no' 
   if(nu_preset_put(&p_nu_xgui->nu_preset_storage,to_preset_no,&p_nu_xgui->nu_preset)<0){
      return -1;
   } 
   //copy destination preset to source preset
   if(nu_preset_put(&p_nu_xgui->nu_preset_storage,from_preset_no,&nu_preset_to_backup)<0){
      return -1;
   } 
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_move_preset
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_move_preset(nu_xgui_t *p_nu_xgui,int from_preset_no,int to_preset_no){
   //
   if(nu_preset_put(&p_nu_xgui->nu_preset_storage,to_preset_no,&p_nu_xgui->nu_preset)<0){
      return -1;
   } 
   //
   nu_preset_set_free(&p_nu_xgui->nu_preset_storage,to_preset_no);
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_save_preset
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_save_preset_parameter_value(nu_xgui_t *p_nu_xgui,int preset_no){
   nu_preset_t  nu_preset_to_backup;
   
   //
   nu_preset_set_used(&p_nu_xgui->nu_preset_storage,preset_no);
   
   //save current preset preset_no
   if(nu_preset_put(&p_nu_xgui->nu_preset_storage,preset_no,&p_nu_xgui->nu_preset)<0){
      return -1;
   } 
   
   //store on sdcard
   if(nu_preset_sync(&p_nu_xgui->nu_preset_storage)<0){
         return -1;
   }
 
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_load_preset_parameter_value
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_load_preset_parameter_value(nu_xgui_t *p_nu_xgui,int preset_no){
   //
   if(nu_preset_get(&p_nu_xgui->nu_preset_storage,&p_nu_xgui->nu_preset,preset_no)<0){
      return -1;
   }
   //
   if(!nu_preset_is_used(&p_nu_xgui->nu_preset_storage,preset_no)){
      //
      for(int i=0; i<NU_EFFECT_PARAMETERS_MAX; i++){
         p_nu_xgui->effect_parameters_list[i].index = (p_nu_xgui->effect_parameters_list[i].index_min+p_nu_xgui->effect_parameters_list[i].index_max)/2;
      }
      //
      return 0;
   }
   
   //
   for(int i=0; i<NU_EFFECT_PARAMETERS_MAX; i++){
      p_nu_xgui->effect_parameters_list[i].index = p_nu_xgui->nu_preset.effect_parameters_index[i];
   }
   //
   return 0;
}

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
         numl_parser_context_t numl_parser_context;
         //
         nu_effect_parameters_init();
         //
         if(numl_parser_open(&numl_parser_context, nu_effect_parameters_get_list(),"/usr/etc/compact.numl")<0){
            return -1;
         }
         numl_parser_run(&numl_parser_context);
         numl_parser_close(&numl_parser_context);
         //
         p_nu_xgui->effect_parameters_list = nu_effect_parameters_get_list();
         //
         nu_preset_init(&p_nu_xgui->nu_preset_storage,0);
         //
         guiSetDisplayList((DISPLAY_LIST*)&nu_display_list);
         guiManagerStart((struct xlcd_core_context_st*)xlcd_context);
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
   pid_t pid_midid;
   
   //
   memset (&xlcd_core_context,     0, sizeof(xlcd_core_context));
  
   //
   if ((nu_xgui.fd_keybfootswitch = open("/dev/kbftsw", O_RDONLY|O_NONBLOCK,0)) < 0) {
      fprintf(stderr,"warning: cannot open /dev/ftswtch\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_1 = open("/dev/rotry1", O_RDONLY|O_NONBLOCK,0)) < 0) {
      fprintf(stderr,"warning: cannot open /dev/rotry1\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_2 = open("/dev/rotry2", O_RDONLY|O_NONBLOCK,0)) < 0) {
      fprintf(stderr,"warning: cannot open /dev/rotry2\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_3 = open("/dev/rotry3", O_RDONLY|O_NONBLOCK,0)) < 0) {
      fprintf(stderr,"warning: cannot open /dev/rotry3\r\n");
   }
   //
   if ((nu_xgui.fd_rotary_4 = open("/dev/rotry4", O_RDONLY|O_NONBLOCK,0)) < 0) {
      fprintf(stderr,"warning: cannot open /dev/rotry4\r\n");
   }
   //
   if((nu_xgui.fd_mem =  open("/dev/mem",O_RDWR,0))<0){
      fprintf(stderr,"warning: cannot open /dev/mem\r\n");
   }
   //
   ioctl(nu_xgui.fd_mem,MEMADD,SHARED_MEM_ADDRESS_MIDI,sizeof(shared_memory_midi_t),&g_shared_memory_midi);
   //
   ioctl(nu_xgui.fd_mem,MEMREG,SHARED_MEM_ADDRESS_MIDI);
   //
   int on=1;
   ioctl(nu_xgui.fd_mem,FIONBIO,&on);
   
   //
   if((pid_midid=vfork())==0){
      execl("/usr/bin/midid","/usr/bin/midid","dev-in","/dev/ttys1","dev-out","/dev/ttys1",NULL);
      exit(-1);
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
   if(nu_xgui.fd_mem!=-1){
      xlcd_select_fd(&xlcd_core_context, nu_xgui.fd_mem);
   }
  
   //
   xlcd_engine(&xlcd_core_context);  // forever
   return 0;
}

/*============================================
| End of Source  : nu.c
==============================================*/
