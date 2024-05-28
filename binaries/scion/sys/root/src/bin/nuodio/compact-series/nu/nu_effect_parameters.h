/* Copyright (c) 2017 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __NU_EFFECT_PARAMETERS_H__
#define __NU_EFFECT_PARAMETERS_H__


/*============================================
| Includes
==============================================*/


/*============================================
| Declaration
==============================================*/

#define INDEX_RANGE_MAX 48

#define DEFAULT_PARAMETER_INDEX_VALUE_MIN 0
#define DEFAULT_PARAMETER_INDEX_VALUE_MAX 9
#define DEFAULT_PARAMETER_INDEX_VALUE     5

#define EFFECT_PARAMETER_ROTARY_1_INDEX 1
#define EFFECT_PARAMETER_ROTARY_2_INDEX 2
#define EFFECT_PARAMETER_ROTARY_3_INDEX 3
#define EFFECT_PARAMETER_ROTARY_4_INDEX 4
#define EFFECT_PARAMETER_ROTARY_1_PUSHED_INDEX 5
#define EFFECT_PARAMETER_ROTARY_2_PUSHED_INDEX 6
#define EFFECT_PARAMETER_ROTARY_3_PUSHED_INDEX 7
#define EFFECT_PARAMETER_ROTARY_4_PUSHED_INDEX 8

//
#define POTENTIOMETER_LIST_MAX 16
//
typedef struct potentiometer_st {
   int fd;
   uint8_t value[INDEX_RANGE_MAX];
}potentiometer_t;

typedef uint16_t gpio_vector_t;
typedef struct effect_parameter_st{
   int8_t index_max;
   int8_t index_min;
   int8_t index;
   //
   potentiometer_t*  potentiometers_list;
   //
   char* display_string[INDEX_RANGE_MAX];

   //potentiometer pot.0 to pot.16
   uint16_t potentiometer_used;

   //gpio.0 to gpio.15
   int fd_gpio;
   gpio_vector_t gpio_used; //16bits vector 
   gpio_vector_t gpio_value[INDEX_RANGE_MAX]; //16bits vectors

   //dac
   int fd_dac;
   uint8_t  dac_used;
   uint16_t dac_frequency_value[INDEX_RANGE_MAX];
   uint16_t dac_level_value[INDEX_RANGE_MAX];
   uint32_t dac_waveform_value[INDEX_RANGE_MAX];
}effect_parameter_t;

#define NU_EFFECT_PARAMETERS_MAX 8

int nu_effect_parameters_init(void);
effect_parameter_t* nu_effect_parameters_get_list(void);
potentiometer_t* nu_effect_parameters_get_potentiometers_list(void);

int nu_effect_parameters_mount_potentiometer(int pot_no, char* args);
int nu_effect_parameters_open_potentiometer(int pot_no, char* path);
int nu_effect_parameters_set_value(int parameter_no, int index);

#endif