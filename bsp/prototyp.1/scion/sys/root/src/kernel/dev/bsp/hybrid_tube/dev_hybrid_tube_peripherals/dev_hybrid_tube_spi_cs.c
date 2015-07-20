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
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/spi.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_spi_x.h"

#include "kernel/dev/bsp/hybrid_tube/dev_hybrid_tube_board/dev_hybrid_tube_board.h"
#include "dev_hybrid_tube_spi_cs.h"

/*============================================
| Global Declaration
==============================================*/
const spi_chip_cs_info_t spi_chip_cs_list[MAX_SPI_CHIP]={
  {0},
  {1}
};


const char dev_hybrid_tube_spi_cs_name[]="spics\0hybrid_tube_spi_cs\0";

int dev_hybrid_tube_spi_cs_load(void);
int dev_hybrid_tube_spi_cs_open(desc_t desc, int o_flag);
int dev_hybrid_tube_spi_cs_close(desc_t desc);
int dev_hybrid_tube_spi_cs_isset_read(desc_t desc);
int dev_hybrid_tube_spi_cs_isset_write(desc_t desc);
int dev_hybrid_tube_spi_cs_read(desc_t desc, char* buf,int size);
int dev_hybrid_tube_spi_cs_write(desc_t desc, const char* buf,int size);
int dev_hybrid_tube_spi_cs_seek(desc_t desc,int offset,int origin);
int dev_hybrid_tube_spi_cs_ioctl(desc_t desc,int request,va_list ap);

dev_map_t dev_hybrid_tube_spi_cs_map={
   dev_hybrid_tube_spi_cs_name,
   S_IFBLK,
   dev_hybrid_tube_spi_cs_load,
   dev_hybrid_tube_spi_cs_open,
   dev_hybrid_tube_spi_cs_close,
   __fdev_not_implemented,
   __fdev_not_implemented,
   dev_hybrid_tube_spi_cs_read,
   dev_hybrid_tube_spi_cs_write,
   dev_hybrid_tube_spi_cs_seek,
   dev_hybrid_tube_spi_cs_ioctl //ioctl
};

/*============================================
| Implementation
==============================================*/

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_load(void){
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_open(desc_t desc, int o_flag){

   //
   if(o_flag & O_RDONLY) {
   }

   if(o_flag & O_WRONLY) {
   }

   ofile_lst[desc].offset=0;

   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_close(desc_t desc){

   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_isset_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_isset_read(desc_t desc){
   return -1;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_isset_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_isset_write(desc_t desc){
   return -1;
}
/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_read
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_read(desc_t desc, char* buf,int size){
   desc_t desc_link;
   //
   if(ofile_lst[desc].oflag & O_RDONLY) {
     desc_link= ofile_lst[desc].desc_nxt[0];
   }
   //
   if(desc_link<0) 
     return -1;
   //
   return ofile_lst[desc_link].pfsop->fdev.fdev_read(desc_link,buf,size);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_write
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_write(desc_t desc, const char* buf,int size){
   desc_t desc_link;
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
     desc_link= ofile_lst[desc].desc_nxt[1];
   }
   //
   if(desc_link<0) 
     return -1;
   //
   return ofile_lst[desc_link].pfsop->fdev.fdev_write(desc_link,buf,size);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_seek
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_seek(desc_t desc,int offset,int origin){
   desc_t desc_link;
   
   if(ofile_lst[desc].oflag & O_RDONLY) {
     desc_link= ofile_lst[desc].desc_nxt[0];
   }
   //
   if(ofile_lst[desc].oflag & O_WRONLY) {
     desc_link= ofile_lst[desc].desc_nxt[1];
   }
   //
   if(desc_link<0) 
     return -1;
   //
   return ofile_lst[desc_link].pfsop->fdev.fdev_seek(desc_link,offset,origin);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_spi_cs_ioctl
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_spi_cs_ioctl(desc_t desc,int request,va_list ap){

   //to do support I_LINK Command
    
   switch(request) {
      case I_LINK:{
         int argc;
         char** argv;
         int i;
         int fd;
         int id=-1;
         //
         fd=va_arg(ap, int);//not used
         //
         argc=va_arg(ap, int);
         argv=va_arg(ap, char**);
         
         for(i=1; i<argc; i++) {
            if(argv[i][0]=='-') {
               unsigned char c;
               unsigned char l=strlen(argv[i]);
               for(c=1; c<l; c++) {
                  switch(argv[i][c]) {
                     //
                     case 'i': {
                        if((i+1) == argc)   //not enough parameter
                           return -1;
                        i++;
                        if(!argv[i])
                           return -1;
                        //
                        id=atoi(argv[i]);
                     }
                     break;
                  }//switch
               }//for
            }//if
         }//for
         
         //
         if(id<0 || id>=MAX_SPI_CHIP)
            return -1;
         //
         ofile_lst[desc].p=(void*)&spi_chip_cs_list[id];
      }
      break;
      //
      case I_UNLINK:{
      }
      break;
      //
      case SPICSENBL:{
         int id;
         spi_chip_cs_info_t* p_spi_chip_cs = (spi_chip_cs_info_t*)ofile_lst[desc].p;
         if(p_spi_chip_cs==(spi_chip_cs_info_t*)0)
           return -1;
         //
         id = p_spi_chip_cs->chip_id;
         //
         switch(id){
            case 0:
               gpio_reset(GPIO_OLED_CS);
            break;
            
            case 1:
            break;
            
            default:
            return-1;
           //
         }
      }
      break;
      //
      case SPICSDISBL:{
         int id;
         spi_chip_cs_info_t* p_spi_chip_cs = (spi_chip_cs_info_t*)ofile_lst[desc].p;
         if(p_spi_chip_cs==(spi_chip_cs_info_t*)0)
           return -1;
         //
         id = p_spi_chip_cs->chip_id;
         //
         switch(id){
            case 0:
               gpio_set(GPIO_OLED_CS);
            break;
            
            case 1:
            break;
            
            default:
            return-1;
           //
         }
         
      }
      break;
      //
      default:
         return -1;
   }

   return 0;
}



/*============================================
| End of Source  : dev_hybrid_tube_spi_cs.c
==============================================*/
