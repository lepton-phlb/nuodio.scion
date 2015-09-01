/*
The contents of this file are subject to the Mozilla Public License Version 1.1 
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis, 
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the 
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2011 <lepton.phlb@gmail.com>.
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
#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/spi.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_spi_x.h"

/*===========================================
Global Declaration
=============================================*/
const char dev_hybrid_tube_spi_4_name[]="spi4\0";

int dev_hybrid_tube_spi_4_load(void);
int dev_hybrid_tube_spi_4_open(desc_t desc, int o_flag);

extern int dev_stm32f4xx_spi_x_load(board_stm32f4xx_spi_info_t * uart_info);
extern int dev_stm32f4xx_spi_x_open(desc_t desc, int o_flag, board_stm32f4xx_spi_info_t * uart_info);
extern int dev_stm32f4xx_spi_x_close(desc_t desc);
extern int dev_stm32f4xx_spi_x_read(desc_t desc, char* buf,int cb);
extern int dev_stm32f4xx_spi_x_write(desc_t desc, const char* buf,int cb);
extern int dev_stm32f4xx_spi_x_ioctl(desc_t desc,int request,va_list ap);
extern int dev_stm32f4xx_spi_x_isset_read(desc_t desc);
extern int dev_stm32f4xx_spi_x_isset_write(desc_t desc);
extern int dev_stm32f4xx_spi_x_seek(desc_t desc,int offset,int origin);

dev_map_t dev_hybrid_tube_spi_4_map={
   dev_hybrid_tube_spi_4_name,
   S_IFCHR,
   dev_hybrid_tube_spi_4_load,
   dev_hybrid_tube_spi_4_open,
   dev_stm32f4xx_spi_x_close,
   dev_stm32f4xx_spi_x_isset_read,
   dev_stm32f4xx_spi_x_isset_write,
   dev_stm32f4xx_spi_x_read,
   dev_stm32f4xx_spi_x_write,
   dev_stm32f4xx_spi_x_seek,
   dev_stm32f4xx_spi_x_ioctl
};

// spi 1 configuration. see in board device driver of current bsp
extern board_stm32f4xx_spi_info_t hybrid_tube_spi_4;

/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name:dev_hybrid_tube_spi_4_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_hybrid_tube_spi_4_load(void) {
   
   hybrid_tube_spi_4.spi_descriptor.board_spi_info=&hybrid_tube_spi_4;
   hybrid_tube_spi_4.desc_r=-1;
   hybrid_tube_spi_4.desc_w=-1;
   //
   return dev_stm32f4xx_spi_x_load(&hybrid_tube_spi_4);
}
   
/*-------------------------------------------
| Name:dev_hybrid_tube_spi_4_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_hybrid_tube_spi_4_open(desc_t desc, int o_flag) {
   return dev_stm32f4xx_spi_x_open(desc, o_flag, &hybrid_tube_spi_4);
}

/*============================================
| End of Source  : dev_hybrid_tube_spi_4.c
==============================================*/
