/*===========================================
Compiler Directive
=============================================*/
#ifndef _USER_KERNEL_MKCONF_H_
#define _USER_KERNEL_MKCONF_H_

//nuodio compact series

//force definition of realtime kernel
//#define __KERNEL_UCORE_FREERTOS
#define __KERNEL_UCORE_EMBOS

//see kernel/core/kernelconf.h
#ifdef __KERNEL_UCORE_FREERTOS
   #include "kernel/core/ucore/freeRTOS_8-0-0/source/arch/cortex-m4/stm32f4/FreeRTOSConfig.h"
#endif 


//see in kernel\dev\arch\cortexm\stm32f4xx\cubemx_hal_driver\inc\legacy\stm32f4xx_hal_conf.h
//force definition ST cpu target type
#ifndef STM32F429xx
   #define STM32F429xx
#endif
//force definition of external clock (8MHz) on discovery board (see in stm32f4xx.h)
#define HSE_VALUE    ((uint32_t)8000000)

//force definition of cpu device
#define __tauon_cpu_device__ __tauon_cpu_device_cortexM4_stm32f4__

//use pipe
#define __tauon_kernel_profile__ __tauon_kernel_profile_classic__
#define __KERNEL_PIPE_SIZE 1024

//rootfs 
#define __KERNEL_RTFS_NODETBL_SIZE 60
#define __KERNEL_RTFS_NODE_BLOCK_NB_MAX 64
#define __KERNEL_RTFS_BLOCK_SIZE 16
#define __KERNEL_RTFS_MAX_FILENAME 8

//force EFFS for stm32f407 on olimex-stm32-p407 board
#define __file_system_profile__  __file_system_profile_classic__
#define __KERNEL_VFS_SUPPORT_EFFS   0
#define __KERNEL_VFS_SUPPORT_FATFS  1

//kernel printk on /dev/console
//#define __KERNEL_PRINTK
//kernel trace_printk on /dev/trace
//#define __KERNEL_TRACE_PRINTK
//kernel console for initd and printk dev output on /dev/console stream
//#define __KERNEL_DEV_TTY "/dev/ttys3"


//specific target include for pinout definition
#include "kernel/dev/bsp/compact_series/dev_compact_series/dev_compact_series_board_io.h"

#define __USER_MONGOOSE_PTHREAD_STACK_SIZE   (8*1024)
#define __USER_MONGOOSE_CGI_ENVIRONMENT_SIZE (2*1024)
#define __USER_MONGOOSE_MAX_REQUEST_SIZE     (2*1024)

/*===========================================
Includes
=============================================*/


/*===========================================
Declaration
=============================================*/

#endif
