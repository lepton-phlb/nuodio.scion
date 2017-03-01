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
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_board.h"

#include "kernel/fs/vfs/vfstypes.h"

#include "stm32f4xx_hal.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_sdio.h"

/*============================================
| Global Declaration 
==============================================*/


/*============================================
| Implementation 
==============================================*/
const char dev_hybrid_tube_sdio_name[]="sdio0\0";

static int dev_hybrid_tube_sdio_load(void);
static int dev_hybrid_tube_sdio_open(desc_t desc, int o_flag);
static int dev_hybrid_tube_sdio_close(desc_t desc);

extern int dev_stm32f4xx_sdio_load(board_stm32f4xx_sdio_info_t * sdio_info);
extern int dev_stm32f4xx_sdio_open(desc_t desc, int o_flag, board_stm32f4xx_sdio_info_t * sdio_info);
extern int dev_stm32f4xx_sdio_close(desc_t desc);
extern int dev_stm32f4xx_sdio_isset_read(desc_t desc);
extern int dev_stm32f4xx_sdio_isset_write(desc_t desc);
extern int dev_stm32f4xx_sdio_read(desc_t desc, char* buf,int size);
extern int dev_stm32f4xx_sdio_write(desc_t desc, const char* buf,int size);
extern int dev_stm32f4xx_sdio_seek(desc_t desc,int offset,int origin);
extern int dev_stm32f4xx_sdio_ioctl(desc_t desc,int request,va_list ap);

dev_map_t dev_hybrid_tube_sdio_map={
   dev_hybrid_tube_sdio_name,
   S_IFBLK,
   dev_hybrid_tube_sdio_load,
   dev_hybrid_tube_sdio_open,
   dev_hybrid_tube_sdio_close,
   dev_stm32f4xx_sdio_isset_read,
   dev_stm32f4xx_sdio_isset_write,
   dev_stm32f4xx_sdio_read,
   dev_stm32f4xx_sdio_write,
   dev_stm32f4xx_sdio_seek,
   dev_stm32f4xx_sdio_ioctl
};


static board_stm32f4xx_sdio_info_t  board_sdio_info={0}; 

/*===========================================
Implementation
=============================================*/



/*-------------------------------------------
| Name:dev_hybrid_tube_sdio_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_sdio_load(void){
   
   GPIO_InitTypeDef GPIO_InitStruct;
   
   //
   board_sdio_info.desc_r=INVALID_DESC;
   board_sdio_info.desc_w=INVALID_DESC;
   
    
   //
   dev_stm32f4xx_sdio_load(&board_sdio_info);

   /**SDIO GPIO Configuration    
   PC8     ------> SDIO_D0
   PC9     ------> SDIO_D1
   PC10     ------> SDIO_D2
   PC11     ------> SDIO_D3
   PC12     ------> SDIO_CK
   PD2     ------> SDIO_CMD 
   */
   GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_2;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
   
   //PD0 -> SDON
   /*Configure GPIO pin : PD0 */
   GPIO_InitStruct.Pin = GPIO_PIN_0;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
   
   // to do:  fix hardware bug (voltage not og when sdon=1)
   // workaround force to 0
   /*Configure GPIO pin Output Level */
   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_RESET);
    
   //
   return 0;
}

/*-------------------------------------------
| Name:dev_hybrid_tube_sdio_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_sdio_open(desc_t desc, int o_flag){
   return dev_stm32f4xx_sdio_open(desc,o_flag,&board_sdio_info);
}

/*-------------------------------------------
| Name:dev_hybrid_tube_sdio_close
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int dev_hybrid_tube_sdio_close(desc_t desc){
   return  dev_stm32f4xx_sdio_close(desc);
}



/*============================================
| End of Source  : dev_stm32f4xx_sdio.c
==============================================*/
