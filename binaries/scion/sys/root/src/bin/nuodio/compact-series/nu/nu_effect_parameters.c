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

#include "kernel/core/dirent.h"
#include "kernel/core/libstd.h"
#include "kernel/core/stat.h"
#include "kernel/core/statvfs.h"

#include "kernel/core/time.h"
#include "kernel/core/ioctl.h"
#include "kernel/core/stropts.h"
#include "kernel/core/fcntl.h"

#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"

#include  "nu_effect_parameters.h"
#include "kernel/dev/bsp/compact_series/dev_compact_series/ioctl_compact_series_gpio.h"

/*============================================
| Global Declaration
==============================================*/
//
static potentiometer_t g_potentiometers_list[POTENTIOMETER_LIST_MAX];
//
static const int g_gpio_list[] = {
   GPIOCMD_FXIO_0,
   GPIOCMD_FXIO_1,
   GPIOCMD_FXIO_2,
   GPIOCMD_FXIO_3,
   GPIOCMD_FXIO_4,
   GPIOCMD_FXIO_5,
   GPIOCMD_FXIO_6,
   GPIOCMD_FXIO_7
};
//
static effect_parameter_t g_effect_parameters_list[NU_EFFECT_PARAMETERS_MAX];
/*============================================
| Implementation
==============================================*/

int nu_effect_parameters_mount_potentiometer(int pot_no, char* args) {
   char* argv[16];
   char argc=1;
   char* p_args=args;

   //
   while (*args != '\0') {
      if (*args == ' ') {
         *args = '\0';
         //
         argv[argc++] = p_args;
         while (*++args == ' ');
         p_args = args;
      }
      //
      ++args;
   }
   //
   argv[argc++] = p_args;
#ifndef USE_NUML_STANDALONE
   int fd[2];
   //stream mount operation
   struct stat _stat;

   //
   if (!argv[1])
      return -1;
   if (!argv[2])
      return -1;
   if (!argv[3])
      return -1;

   //
   stat(argv[1], &_stat);
   if (!S_ISBLK(_stat.st_mode) && !S_ISCHR(_stat.st_mode)) {
      return -1;
   }
   //
   stat(argv[2], &_stat);
   if (!S_ISBLK(_stat.st_mode) && !S_ISCHR(_stat.st_mode)) {
      return -1;
   }
   //
   if ((fd[0] = open(argv[1], O_RDWR, 0))<0) {
      return -1;
   }
   //
   if ((fd[1] = open(argv[2], O_RDWR, 0))<0) {
      return -1;
   }
   //
   if (ioctl(fd[0], I_LINK, fd[1], argc, argv)<0) {
      return -1;
   }
   //
   if (fattach(fd[0], argv[3])<0) {
      return -1;
   }
   //
   close(fd[0]);
   //
   close(fd[1]);
   //
#else
#endif
   return 0;
}

//
int nu_effect_parameters_open_potentiometer(int pot_no, char* path) {
#ifndef USE_NUML_STANDALONE
   g_potentiometers_list[pot_no].fd = open(path, O_WRONLY, 0);
#else
   g_potentiometers_list[pot_no].fd = 0;
#endif
   return g_potentiometers_list[pot_no].fd;
}


//
int nu_effect_parameters_set_value(int parameter_no,int index) {
   //which potentiometer is used
   for(int pot_no=0;pot_no<POTENTIOMETER_LIST_MAX;pot_no++){
      if ( !(g_effect_parameters_list[parameter_no].potentiometer_used & (((uint16_t)(0x1)) << pot_no)) ) {
         continue;
      }
      //
      uint8_t resistor_value = g_potentiometers_list[pot_no].value[index - g_effect_parameters_list[parameter_no].index_min];
      //
#ifndef USE_NUML_STANDALONE
      write(g_potentiometers_list[pot_no].fd, &resistor_value, sizeof(resistor_value));
#endif
   }

   //gpio
   for (int gpio_no = 0; gpio_no < (sizeof(gpio_vector_t) * 8); gpio_no++) {
      gpio_vector_t gpio_mask = ((gpio_vector_t)(0x1)) << gpio_no;
      if (!(g_effect_parameters_list[parameter_no].gpio_used & gpio_mask)) {
         continue;
      }
      gpio_vector_t gpio_value = g_effect_parameters_list[parameter_no].gpio_value[index - g_effect_parameters_list[parameter_no].index_min];
      if (gpio_value & gpio_mask) {
         //set 1 to gpio
#ifndef USE_NUML_STANDALONE
         ioctl(g_effect_parameters_list[parameter_no].fd_gpio, g_gpio_list[gpio_no],1);
#endif
      }else {
         //set 0 to gpio
#ifndef USE_NUML_STANDALONE
         ioctl(g_effect_parameters_list[parameter_no].fd_gpio, g_gpio_list[gpio_no], 0);
#endif
      }
   }

   //dac
   if (g_effect_parameters_list[parameter_no].dac_used) {
      //to do set wave form
      //to do set frequency
      //set level
      uint8_t level_value = (uint8_t) g_effect_parameters_list[parameter_no].dac_level_value[index - g_effect_parameters_list[parameter_no].index_min];
#ifndef USE_NUML_STANDALONE
      write(g_effect_parameters_list[parameter_no].fd_dac, &level_value, 1);
#endif
   }
   //
   return 0;
}

//
int nu_effect_parameters_init(void){
   int fd_gpio = 0;
   int fd_dac = 0;
   //
#ifndef USE_NUML_STANDALONE
   //gpio
   if ((fd_gpio = open("/dev/gpio", O_RDWR, 0)) < 0) {
      return -1;
   }
   //
   int on_off=0;
   ioctl(fd_gpio,GPIOCMD_TRUEBYPASS,on_off);
   
   //dac
   if ((fd_dac = open("/dev/dac1", O_WRONLY, 0)) < 0) {
      return -1;
   }
#endif

   //
   for(int i=0; i<NU_EFFECT_PARAMETERS_MAX;i++){
      g_effect_parameters_list[i].potentiometers_list=g_potentiometers_list;
      g_effect_parameters_list[i].fd_gpio = fd_gpio;
      g_effect_parameters_list[i].fd_dac = fd_dac;
   }
   //
   return 0;
}

//
effect_parameter_t* nu_effect_parameters_get_list(void) {
   return g_effect_parameters_list;
}

//
potentiometer_t* nu_effect_parameters_get_potentiometers_list(void) {
   return g_potentiometers_list;
}
/*============================================
| End of Source  : nu_effect_parameters.c
==============================================*/