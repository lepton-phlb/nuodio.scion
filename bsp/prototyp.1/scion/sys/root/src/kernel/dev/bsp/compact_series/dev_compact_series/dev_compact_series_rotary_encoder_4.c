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
#include "kernel/core/ioctl.h"
#include "kernel/core/ioctl_spi.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/stat.h"
#include "kernel/core/cpu.h"
#include "kernel/core/dirent.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx_hal_gpio_ex.h"

#include "kernel/dev/bsp/compact_series/dev_compact_series/dev_rotary_encoder_x.h"


/*============================================
| Global Declaration
==============================================*/

extern rotary_encoder_info_t compact_series_rotary_encoder_info_4;

const char dev_compact_series_rotary_encoder_4_name[]="rotry4\0compact_series_rotary_encoder_4\0";

static int dev_compact_series_rotary_encoder_4_load(void);
static int dev_compact_series_rotary_encoder_4_open(desc_t desc, int o_flag);

dev_map_t dev_compact_series_rotary_encoder_4_map={
   dev_compact_series_rotary_encoder_4_name,
   S_IFCHR,
   dev_compact_series_rotary_encoder_4_load,
   dev_compact_series_rotary_encoder_4_open,
   dev_rotary_encoder_x_close,
   dev_rotary_encoder_x_isset_read,
   dev_rotary_encoder_x_isset_write,
   dev_rotary_encoder_x_read,
   dev_rotary_encoder_x_write,
   dev_rotary_encoder_x_seek,
   dev_rotary_encoder_x_ioctl //ioctl
};

/*============================================
| Implementation
==============================================*/


/*-------------------------------------------
| Name:dev_compact_series_rotary_encoder_4_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_rotary_encoder_4_load(void){
  dev_rotary_encoder_x_load(&compact_series_rotary_encoder_info_4);
  return 0;
}

/*-------------------------------------------
| Name:dev_compact_series_rotary_encoder_4_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_compact_series_rotary_encoder_4_open(desc_t desc, int o_flag){
   dev_rotary_encoder_x_open(desc,o_flag, &compact_series_rotary_encoder_info_4);
   return 0;
}


