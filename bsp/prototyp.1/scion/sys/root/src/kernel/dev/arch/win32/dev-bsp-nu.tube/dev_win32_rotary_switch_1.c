/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2011. All Rights Reserved.

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
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/system.h"
#include "kernel/core/stat.h"
#include "kernel/core/fcntl.h"

#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/fs/vfs/vfstypes.h"

#include "kernel/core/ioctl.h"
#include "kernel/core/kernel_ring_buffer.h"

#include "kernel/dev/arch/win32/dev_win32_rotary_switch/dev_win32_rotary_switch_x.h"

/*===========================================
Global Declaration
=============================================*/

static const char dev_win32_rotary_switch_1_name[] = "rotry1\0win32_rotary_switch_1\0";

static int dev_win32_rotary_switch_1_load(void);
static int dev_win32_rotary_switch_1_open(desc_t desc, int o_flag);

dev_map_t dev_win32_rotary_switch_1_map = {
   dev_win32_rotary_switch_1_name,
   S_IFCHR,
   dev_win32_rotary_switch_1_load,
   dev_win32_rotary_switch_1_open,
   dev_win32_rotary_switch_x_close,
   dev_win32_rotary_switch_x_isset_read,
   dev_win32_rotary_switch_x_isset_write,
   dev_win32_rotary_switch_x_read,
   dev_win32_rotary_switch_x_write,
   __fdev_not_implemented,
   dev_win32_rotary_switch_x_ioctl //ioctl
};

#define ROTARY_SWITCH_INTERRUPT_NO 31
static rotary_switch_info_t  rotary_switch_info_1={
   .rotary_switch_windows_form_name="RotarySwitchControl1",
   .desc_r=INVALID_DESC
};

/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name:dev_rotary_switch_interrupt
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
__hw_interrupt(31, dev_rotary_switch_interrupt_1) {
   dev_rotary_switch_x_interrupt(&rotary_switch_info_1);
}

/*-------------------------------------------
| Name:dev_win32_rotary_switch_1_load
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_win32_rotary_switch_1_load(void) {
   rotary_switch_info_1.interrupt_no = 31;
   return dev_win32_rotary_switch_x_load(&rotary_switch_info_1);
}

/*-------------------------------------------
| Name:dev_win32_rotary_switch_1_open
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
static int dev_win32_rotary_switch_1_open(desc_t desc, int o_flag) {
   return dev_win32_rotary_switch_x_open(desc,o_flag, &rotary_switch_info_1);
}


/*============================================
| End of Source  : dev_win32_rotary_switch_1.c
==============================================*/
