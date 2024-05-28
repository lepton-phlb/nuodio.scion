/* Copyright (c) 2018 nuodio. All Rights Reserved.
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
#include "kernel/core/time.h"
#include "kernel/core/libstd.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/select.h"

#include "kernel/core/time.h"
#include "kernel/core/wait.h"

#include "lib/libc/unistd.h"
#include "lib/libc/termios/termios.h"
#include "lib/libc/stdio/stdio.h"

#include "bin/nuodio/compact-series/nu/preset/nu_preset.h"

/*============================================
| Global Declaration
==============================================*/
#define NU_PRESET_STORAGE_DIR_PATH "/sdcard/preset"
#define NU_PRESET_STORAGE_PATH "/sdcard/preset/.preset"

/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        nu_preset_prepare
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_prepare(nu_preset_storage_t* p_nu_preset_storage){
   //
   if((p_nu_preset_storage->fd=open(NU_PRESET_STORAGE_PATH,O_RDWR,0))<0){
      //
      mkdir(NU_PRESET_STORAGE_DIR_PATH,0);
      //
      if((p_nu_preset_storage->fd=open(NU_PRESET_STORAGE_PATH,O_CREAT|O_RDWR,0))<0){
         return -1;
      }
   }
   //
   return 0;                     
}

/*--------------------------------------------
| Name:        nu_preset_make
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_make(nu_preset_storage_t* p_nu_preset_storage){
   int preset_index=0;
   nu_preset_t nu_preset={0};
   int cb;
   //
   memset(p_nu_preset_storage->vector,0,sizeof(p_nu_preset_storage->vector));
   //
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   lseek(p_nu_preset_storage->fd,0,SEEK_SET);
   //
   for(preset_index=0;preset_index<NU_MIDI_PRESET_MAX;preset_index++){
      memcpy(&p_nu_preset_storage->preset_list[preset_index],&nu_preset,sizeof(nu_preset));
      cb = write(p_nu_preset_storage->fd,&nu_preset,sizeof(nu_preset));
      if(cb<sizeof(nu_preset)){
         return -1;
      }
   }
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_load
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_load(nu_preset_storage_t* p_nu_preset_storage){
   int preset_index=0;
   nu_preset_t nu_preset={0};
   int cb;
   //
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   lseek(p_nu_preset_storage->fd,0,SEEK_SET);
   //
   for(preset_index=0;preset_index<NU_MIDI_PRESET_MAX;preset_index++){
      cb = read(p_nu_preset_storage->fd,&nu_preset,sizeof(nu_preset));
      if(cb<sizeof(nu_preset)){
         return -1;
      }
      memcpy(&p_nu_preset_storage->preset_list[preset_index],&nu_preset,sizeof(nu_preset));
   }
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_sync
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_sync(nu_preset_storage_t* p_nu_preset_storage){
   int preset_index=0;
   int cb;
   //
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   lseek(p_nu_preset_storage->fd,0,SEEK_SET);
   //
   for(preset_index=0;preset_index<NU_MIDI_PRESET_MAX;preset_index++){
      cb = write(p_nu_preset_storage->fd,&p_nu_preset_storage->preset_list[preset_index],sizeof(nu_preset_t));
      if(cb<sizeof(nu_preset_t)){
         return -1;
      }
   }
   //
   close(p_nu_preset_storage->fd);
   //
   if((p_nu_preset_storage->fd=open(NU_PRESET_STORAGE_PATH,O_RDWR,0))<0){
      return -1;
   }
   
   //
   return 0;         
}

/*--------------------------------------------
| Name:        nu_preset_lookup_free
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_lookup_free(nu_preset_storage_t* p_nu_preset_storage){
   int preset_index=0;
   //
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   for(preset_index=0;preset_index<NU_MIDI_PRESET_MAX;preset_index++){
      if(p_nu_preset_storage->preset_list[preset_index].status==NU_PRESET_STATUS_FREE){
         return preset_index;
      }
   }
   //
   return -1;
}
/*--------------------------------------------
| Name:        nu_preset_is_used
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_is_used(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no){
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   if(preset_no>NU_MIDI_PRESET_MAX){
      return -1;
   }
   //
   if(p_nu_preset_storage->preset_list[preset_no].status==NU_PRESET_STATUS_USED){
      return 1;
   }
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_set_used
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_set_used(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no){
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   if(preset_no>NU_MIDI_PRESET_MAX){
      return -1;
   }
   //
   p_nu_preset_storage->preset_list[preset_no].status=NU_PRESET_STATUS_USED;
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_set_free
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_set_free(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no){
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   if(preset_no>NU_MIDI_PRESET_MAX){
      return -1;
   }
   //
   p_nu_preset_storage->preset_list[preset_no].status=NU_PRESET_STATUS_FREE;
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_get_name
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
char* nu_preset_get_name(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no){
   if(p_nu_preset_storage->fd<0){
      return (char*)0;
   }
   //
   if(preset_no>NU_MIDI_PRESET_MAX){
     return (char*)0;
   }
   //
   if(strlen(p_nu_preset_storage->preset_list[preset_no].name)<=0){
      return (char*)0;
   }
   //
   return p_nu_preset_storage->preset_list[preset_no].name;
}

/*--------------------------------------------
| Name:        nu_preset_get
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_get(nu_preset_storage_t* p_nu_preset_storage,nu_preset_t* p_preset,uint8_t preset_no){
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   if(preset_no>NU_MIDI_PRESET_MAX){
      return -1;
   }
   //
   if(p_preset==(nu_preset_t*)0){
      return -1;
   }
   //
   memcpy(p_preset,&p_nu_preset_storage->preset_list[preset_no],sizeof(nu_preset_t));
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_put
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_put(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no,nu_preset_t* p_preset){
   if(p_nu_preset_storage->fd<0){
      return -1;
   }
   //
   if(preset_no>NU_MIDI_PRESET_MAX){
      return -1;
   }
   //
   if(p_preset==(nu_preset_t*)0){
      return -1;
   }
   //
   memcpy(&p_nu_preset_storage->preset_list[preset_no],p_preset,sizeof(nu_preset_t));
   //
   return 0;
}

/*--------------------------------------------
| Name:        nu_preset_unit_test
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int nu_preset_init(nu_preset_storage_t* p_nu_preset_storage,uint8_t flag_force_make){
   //
   nu_preset_t nu_preset;
   uint8_t preset_no=0;
   
   //
   if(nu_preset_prepare(p_nu_preset_storage)<0){
      return -1;
   }
   
   //
   if(flag_force_make || nu_preset_load(p_nu_preset_storage)<0){
  
       //
      if(nu_preset_make(p_nu_preset_storage)<0){
         return -1;
      }
      //
      if(nu_preset_load(p_nu_preset_storage)<0){
         return -1;
      }
      //
      if(nu_preset_sync(p_nu_preset_storage)<0){
         return -1;
      }
#if 0
      //
      for(preset_no=0;preset_no<NU_MIDI_PRESET_MAX;preset_no++){
         snprintf(nu_preset.name,NU_PRESET_NAME_LENGTH_MAX,"my-preset%02d",preset_no);
         nu_preset.effect_parameters_index[0]=0;
         nu_preset.effect_parameters_index[1]=0+preset_no;
         nu_preset.effect_parameters_index[2]=1+preset_no;
         nu_preset.effect_parameters_index[3]=2+preset_no;
         //
         nu_preset_put(p_nu_preset_storage,preset_no,&nu_preset);
      }
      
      //
      if(nu_preset_sync(p_nu_preset_storage)<0){
         return -1;
      }
#endif
   }
   
   
   //
   for(preset_no=0;preset_no<NU_MIDI_PRESET_MAX;preset_no++){
      nu_preset_get(p_nu_preset_storage,&nu_preset,preset_no);
      //
      fprintf(stderr,"%2d: %s %2d:%2d:%2d\r\n",preset_no,
              nu_preset.name,
              nu_preset.effect_parameters_index[1],
              nu_preset.effect_parameters_index[2],
              nu_preset.effect_parameters_index[3]);
   }
   //
 
   return 0;
}


/*============================================
| End of Source  : nu.c
==============================================*/
