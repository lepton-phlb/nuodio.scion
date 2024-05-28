
#include <stdint.h>
#include <stdlib.h>


#ifdef USE_NUML_STANDALONE
   #include <stdio.h>
   #include <string.h>
   #include <fcntl.h>
   #include <io.h>
#else
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

   #include "lib/libc/unistd.h"
   #include "lib/libc/termios/termios.h"
   #include "lib/libc/stdio/stdio.h"
#endif

#include "bin/nuodio/compact-series/nu/nu_effect_parameters.h"
#include "numl_parser.h"

#ifdef USE_NUML_STANDALONE
#define __numl_parser_debug_printf(__debug__,...) if(__debug__)printf(__VA_ARGS__)
#else
#define __numl_parser_debug_printf(__debug__,...)
#endif

#define NU_PARSER_BUFFER_SIZE_MAX 128

typedef struct numl_parser_error_message_st{
   numl_parser_error_code_t error_code;
   const char* message;
}numl_parser_error_message_t;

const numl_parser_error_message_t numl_parser_error_message_list[] = {
   { .error_code = NU_PARSER_NO_ERROR,.message = "NU_PARSER_NO_ERROR"},
   { .error_code = NU_PARSER_ERROR_UNEXPECTED_CHARACTER,.message = "NU_PARSER_ERROR_UNEXPECTED_CHARACTER" },
   { .error_code = NU_PARSER_ERROR_INTERNAL_PARSER_ERROR,.message = "NU_PARSER_ERROR_INTERNAL_PARSER_ERROR" },
   { .error_code = NU_PARSER_ERROR_SECTION_NAME_TOO_LONG,.message = "NU_PARSER_ERROR_SECTION_NAME_TOO_LONG" },
   { .error_code = NU_PARSER_ERROR_ATTRIBUTE_NAME_TOO_LONG,.message = "NU_PARSER_ERROR_ATTRIBUTE_NAME_TOO_LONG" },
   { .error_code = NU_PARSER_ERROR_PAIR_VALUE_TOO_LONG,.message = "NU_PARSER_ERROR_PAIR_VALUE_TOO_LONG" },
   { .error_code = NU_PARSER_ERROR_SECTION_NAME_EXPECTED,.message = "NU_PARSER_ERROR_SECTION_NAME_EXPECTED" },
   { .error_code = NU_PARSER_ERROR_EQUAL_SIGN_EXPECTED,.message = "NU_PARSER_ERROR_EQUAL_SIGN_EXPECTED" },
   { .error_code = NU_PARSER_ERROR_ARRAY_BRACKET_EXPECTED,.message = "NU_PARSER_ERROR_ARRAY_BRACKET_EXPECTED" },
   { .error_code = NU_PARSER_ERROR_DQUOTE_EXPECTED,.message = "NU_PARSER_ERROR_DQUOTE_EXPECTED" },
   
   { .error_code = NU_PARSER_ERROR_UNRECOGNIZED_SECTION_NAME,.message = "NU_PARSER_ERROR_UNRECOGNIZED_SECTION_NAME" },
   { .error_code = NU_PARSER_ERROR_UNRECOGNIZED_ATTRIBUTE_NAME,.message = "NU_PARSER_ERROR_UNRECOGNIZED_ATTRIBUTE_NAME" },

   { .error_code = NU_PARSER_ERROR_RANGE_ERROR,.message = "NU_PARSER_ERROR_RANGE_ERROR" },
   { .error_code = NU_PARSER_ERROR_RANGE_DEFINITON_MINMAX_INVALID,.message = "NU_PARSER_ERROR_RANGE_DEFINITON_MINMAX_INVALID" },
   { .error_code = NU_PARSER_ERROR_RANGE_DEFINITON_TOO_LARGE,.message = "NU_PARSER_ERROR_RANGE_DEFINITON_TOO_LARGE" },

   { .error_code = NU_PARSER_ERROR_PAIR_VALUE_EXPECTED,.message = "NU_PARSER_ERROR_PAIR_VALUE_EXPECTED"},
   { .error_code = NU_PARSER_ERROR_INDEX_OUT_RANGE,.message = "NU_PARSER_ERROR_INDEX_OUT_RANGE"},

   { .error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT,.message = "NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT"},
   { .error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_NOT_FOUND,.message = "NU_PARSER_ERROR_PAIR_INDEX_VALUE_NOT_FOUND"},
   { .error_code = NU_PARSER_ERROR_PAIR_UNRECOGNIZED_RANGE_INDEX_NAME,.message = "NU_PARSER_ERROR_PAIR_UNRECOGNIZED_RANGE_INDEX_NAME"},
   { .error_code = NU_PARSER_ERROR_PAIR_VALUE_INTERVAL_INVALID_FORMAT,.message = "NU_PARSER_ERROR_PAIR_VALUE_INTERVAL_INVALID_FORMAT" }
};
const int numl_parser_error_message_list_size = sizeof(numl_parser_error_message_list) / sizeof(numl_parser_error_message_t);


//section definition
#define NU_PARSER_SECTION_NAME_SIZE_MAX 32

//
const char const_section_numl[] = "numl";
//
const char const_section_hardware[] = "hardware";
//
const char const_section_rotary_1[] = "rotary_1";
const char const_section_rotary_2[] = "rotary_2";
const char const_section_rotary_3[] = "rotary_3";
const char const_section_rotary_4[] = "rotary_4";
//
const char const_section_rotary_1_pushed[] = "rotary_1_pushed";
const char const_section_rotary_2_pushed[] = "rotary_2_pushed";
const char const_section_rotary_3_pushed[] = "rotary_3_pushed";
const char const_section_rotary_4_pushed[] = "rotary_4_pushed";
//
const char const_section_authentication[] = "authentication";


//attribute definition
#define NU_PARSER_ATTRIBUTE_NAME_SIZE_MAX 32

const char const_attribute_version[] = "version";

const char const_attribute_range[] = "range";

const char const_attribute_label[] = "label";

const char const_attribute_pot_0[] = "pot_0";
const char const_attribute_pot_1[] = "pot_1";
const char const_attribute_pot_2[] = "pot_2";
const char const_attribute_pot_3[] = "pot_3";
const char const_attribute_pot_4[] = "pot_4";
const char const_attribute_pot_5[] = "pot_5";
const char const_attribute_pot_6[] = "pot_6";
const char const_attribute_pot_7[] = "pot_7";

const char const_attribute_gpio_0[] = "gpio_0";
const char const_attribute_gpio_1[] = "gpio_1";
const char const_attribute_gpio_2[] = "gpio_2";
const char const_attribute_gpio_3[] = "gpio_3";
const char const_attribute_gpio_4[] = "gpio_4";
const char const_attribute_gpio_5[] = "gpio_5";
const char const_attribute_gpio_6[] = "gpio_6";
const char const_attribute_gpio_7[] = "gpio_7";
const char const_attribute_gpio_8[] = "gpio_8";
const char const_attribute_gpio_9[] = "gpio_9";
const char const_attribute_gpio_10[] = "gpio_10";
const char const_attribute_gpio_11[] = "gpio_11";
const char const_attribute_gpio_12[] = "gpio_12";
const char const_attribute_gpio_13[] = "gpio_13";
const char const_attribute_gpio_14[] = "gpio_14";
const char const_attribute_gpio_15[] = "gpio_15";

//
const char const_attribute_display[] = "display";

//
const char const_attribute_dac_waveform[]    = "dac_waveform";
const char const_attribute_dac_frequency[]   = "dac_frequency";
const char const_attribute_dac_level[]       = "dac_level";
//
#define DAC_PARAMETER_TYPE_FREQUENCY ((uint8_t)(1))
#define DAC_PARAMETER_TYPE_LEVEL ((uint8_t)(2))



#define NU_PARSER_PAIR_VALUE_SIZE_MAX 64
//pair index definition
const char const_attribute_pair_index_min[]     = "min";
const char const_attribute_pair_index_max[]     = "max";
const char const_attribute_pair_index_index[]   = "index";
//
const char const_attribute_pair_index_range[] = "range";
//
const char const_attribute_pair_index_mount[] = "mount";
const char const_attribute_pair_index_open[] = "open";
const char const_attribute_pair_index_model[] = "model";
const char const_attribute_pair_index_bus[] = "bus";
const char const_attribute_pair_index_chipselect[] = "chipselect";
const char const_attribute_pair_index_reset[] = "reset";
//
const char const_attribute_pair_index_name[] = "name";
//
const char const_attribute_pair_index_signature_hash[] = "hash";

//pair value definition
const char const_attribute_pair_value_dac_waveform_dc[]        = "dc";
const char const_attribute_pair_value_dac_waveform_sinus[]     = "sinus";
const char const_attribute_pair_value_dac_waveform_square[]    = "square";
const char const_attribute_pair_value_dac_waveform_triangle[]  = "triangle";



//
#define NU_PARSER_WAIT_SECTION_TAG_OPEN  0
#define NU_PARSER_WAIT_SECTION_TAG_CLOSE 1

#define NU_PARSER_SECTION_NULL 0
#define NU_PARSER_SECTION_GET_NAME 1
#define NU_PARSER_SECTION_GET_ATTRIBUTE_NAME  2
#define NU_PARSER_SECTION_WAIT_EQUAL_SIGN 3
#define NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_OPEN 4
#define NU_PARSER_SECTION_WAIT_ATTRIBUTE_PAIR_DQUOTE 5
#define NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE 6
#define NU_PARSER_SECTION_WAIT_LINEFEED 7

#define NU_PARSER_SECTION_WAIT_END_OF_COMMENT 10



typedef int (*pfn_nu_section_callback)(numl_parser_context_t* numl_parser_context,char* section_name,char* attribute_name,char* pairs_index_value);

typedef struct numl_parser_section_callback_list_st{
   const char* section_name;
   const pfn_nu_section_callback section_callback;
}numl_parser_section_callback_list_t;

static int section_numl_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pairs_index_value);
static int section_hardware_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pairs_index_value);
static int section_rotary_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pairs_index_value);
static int section_authentication_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pairs_index_value);


const numl_parser_section_callback_list_t numl_parser_section_callback_list[] = {
   { const_section_numl, section_numl_callback},
   { const_section_hardware, section_hardware_callback},
   { const_section_rotary_1, section_rotary_callback },
   { const_section_rotary_2, section_rotary_callback },
   { const_section_rotary_3, section_rotary_callback },
   { const_section_rotary_4, section_rotary_callback },
   { const_section_rotary_1_pushed, section_rotary_callback },
   { const_section_rotary_2_pushed, section_rotary_callback },
   { const_section_rotary_3_pushed, section_rotary_callback },
   { const_section_rotary_4_pushed, section_rotary_callback },
   { const_section_authentication, section_authentication_callback}
};
//
const int numl_parser_section_callback_list_size = sizeof(numl_parser_section_callback_list) / sizeof(numl_parser_section_callback_list_t);



//
int nu_set_gpio(effect_parameter_t* p_effect_parameter, uint8_t index) {

   uint16_t gpio_msk = 0x0001;
   uint8_t gpio_no=0;

   while (gpio_msk) {
      if (p_effect_parameter->gpio_used & gpio_msk) {
         if ((p_effect_parameter->gpio_value[index] & gpio_msk)) {
            //set to 1 with ioctl(,,gpio_no,)
         }
         else {
            //set to 0 with ioctl()
         }
      }
      //
      gpio_msk = gpio_msk << 1;
      gpio_no++;
   }
   //
   return 0;
}

//
int nu_set_pot(effect_parameter_t* p_effect_parameter, uint8_t index) {

   return 0;
}

//
const char* numl_parser_message_error(numl_parser_context_t* numl_parser_context) {
  
   //
   for (int i = 0; i < numl_parser_error_message_list_size; i++) {
      if (numl_parser_context->error_code != numl_parser_error_message_list[i].error_code)
         continue;
      //
      printf("error at line:%d col:%d : code %d : %s", numl_parser_context->line, numl_parser_context->column, numl_parser_context->error_code, numl_parser_error_message_list[i].message);
      //
      return  numl_parser_error_message_list[i].message;
   }
   //
   return "unrecognized error code";
}

//
int numl_parser_print_error(numl_parser_context_t* numl_parser_context) {
   //
   if (numl_parser_context->error_code == NU_PARSER_NO_ERROR) {
      printf("nu parser no error");
      return 0;
   }

   //
   for (int i = 0; i < numl_parser_error_message_list_size; i++) {
      if (numl_parser_context->error_code != numl_parser_error_message_list[i].error_code)
         continue;
      //
      printf("error at line:%d col:%d : code %d : %s", numl_parser_context->line, numl_parser_context->column, numl_parser_context->error_code, numl_parser_error_message_list[i].message);
      //
      return 0;
   }
   //
   printf("unrecognized error code");
   //
   return -1;
}

//
static int numl_parser_pair_value_to_interval(char** interval_1, char** interval_2, char* pair_value) {
   int len = strlen(pair_value);

   //
   *interval_1 = (char*)pair_value;

   //
   for (int pos = 0; pos < len; pos++) {
      if (pair_value[pos] != '.')
         continue;
      //
      pair_value[pos++] = '\0';
      //
      if (pair_value[pos++] != '.') {
         return -1;
      }
      //
      *interval_2 = pair_value + pos;
      return 0;
   }
   //
   return -1;
}

//
static int numl_parser_pair_to_index_value(char** index, char** value, char* pair_index_value) {
   int len = strlen(pair_index_value);
   
   //
   *index = (char*)pair_index_value;
   *value = (char*)0;

   //
   if (len <= 0) {
      return -1;
   }
   
   //
   for (int pos = 0; pos < len; pos++) {
      if (pair_index_value[pos] != ':')
         continue;
      //
      pair_index_value[pos++] = '\0';
      //
      *value = pair_index_value+pos;
      return 0;
   }
   //
   return -1;
}

//
static int numl_parser_set_attribute_label(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, char* pair_index_value) {
   char *index;
   char *value;
   if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
      //error pair index,value invalid format
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
      return -1;
   }
   //
   if (strlen(value) <= 0) {
      //error value of pair index,value not found
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_NOT_FOUND;
      return -1;
   }
   //
   if (!strcmp(index, const_attribute_pair_index_name)) {
      //nothing to do
   }else {
      //error invalid index value
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_UNRECOGNIZED_RANGE_INDEX_NAME;
      return -1;
   }
   //
   return 0;
}

//
static int numl_parser_set_attribute_range(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, char* pair_index_value) {
   char *index;
   char *value;
   if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
      //error pair index,value invalid format
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
      return -1;
   }
   //
   if (strlen(value) <= 0) {
      //error value of pair index,value not found
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_NOT_FOUND;
      return -1;
   }
   //
   if (!strcmp(index, const_attribute_pair_index_min)) {
      numl_parser_context->effect_parameters_list[effect_parameter_index].index_min = atoi(value);
   }else if(!strcmp(index, const_attribute_pair_index_max)) {
      numl_parser_context->effect_parameters_list[effect_parameter_index].index_max = atoi(value);
   }else if (!strcmp(index, const_attribute_pair_index_index)) {
      numl_parser_context->effect_parameters_list[effect_parameter_index].index = atoi(value);
   }else {
      //error invalid index value
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_UNRECOGNIZED_RANGE_INDEX_NAME;
      return -1;
   }
   //
   return 0;
}

//
static int numl_parser_set_attribute_pot(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index , int8_t pot_no, char* pair_index_value) {
   char *index;
   char *value;
   //
   if (numl_parser_context->effect_parameters_list[effect_parameter_index].index_min >= numl_parser_context->effect_parameters_list[effect_parameter_index].index_max) {
      // range error
      numl_parser_context->error_code = NU_PARSER_ERROR_RANGE_DEFINITON_MINMAX_INVALID;
      return -1;
   }

   if ((numl_parser_context->effect_parameters_list[effect_parameter_index].index_max - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min) > INDEX_RANGE_MAX) {
      //range out of limit error
      numl_parser_context->error_code = NU_PARSER_ERROR_RANGE_DEFINITON_TOO_LARGE;
      return -1;
   }

   if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
      //error pair index,value invalid format
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
      return -1;
   }
   //
   if (strlen(value) <= 0) {
      //error value of pair index,value not found
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_EXPECTED;
      return -1;
   }

   //
   if (!strcmp(index, const_attribute_pair_index_range)) {
      char* str_value_interval_1;
      char* str_value_interval_2;
      uint8_t interval_1;
      uint8_t interval_2;

      //
      if (numl_parser_pair_value_to_interval(&str_value_interval_1, &str_value_interval_2, value) < 0) {
         //error invalid value format
         numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_INTERVAL_INVALID_FORMAT;
         return -1;
      }
      //
      numl_parser_context->effect_parameters_list[effect_parameter_index].potentiometer_used |= (((uint16_t)(0x1)) << pot_no);
      //
      interval_1 = atoi(str_value_interval_1);
      interval_2 = atoi(str_value_interval_2);
      int8_t step;
      uint8_t int_value;
      step = (interval_2 - interval_1) / (numl_parser_context->effect_parameters_list[effect_parameter_index].index_max - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min);
      //
      int_value = interval_1 ;
      //
      for (int i = numl_parser_context->effect_parameters_list[effect_parameter_index].index_min;
         i <= numl_parser_context->effect_parameters_list[effect_parameter_index].index_max;
         i++) {
         potentiometer_t*  potentiometers_list = numl_parser_context->effect_parameters_list[effect_parameter_index].potentiometers_list;
         potentiometers_list[pot_no].value[i - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] = int_value;
         int_value += step;
      }
      //
    
   }else {
      int8_t int_index = atoi(index);

      //
      if (int_index<numl_parser_context->effect_parameters_list[effect_parameter_index].index_min || int_index>numl_parser_context->effect_parameters_list[effect_parameter_index].index_max) {
         numl_parser_context->error_code = NU_PARSER_ERROR_INDEX_OUT_RANGE;
         return -1;
      }

      //
      numl_parser_context->effect_parameters_list[effect_parameter_index].potentiometer_used |= (((uint16_t)(0x1)) << pot_no);
      //
      uint8_t int_value = atoi(value);
      //
      potentiometer_t*  potentiometers_list = numl_parser_context->effect_parameters_list[effect_parameter_index].potentiometers_list;
      potentiometers_list[pot_no].value[int_index - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] = int_value;
   }

   //
   return 0;
}

static int numl_parser_set_attribute_gpio(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, int8_t gpio_no, char* pair_index_value) {

   char *index;
   char *value;
   //
   if (numl_parser_context->effect_parameters_list[effect_parameter_index].index_min >= numl_parser_context->effect_parameters_list[effect_parameter_index].index_max) {
      // range error
      numl_parser_context->error_code = NU_PARSER_ERROR_RANGE_DEFINITON_MINMAX_INVALID;
      return -1;
   }

   if ((numl_parser_context->effect_parameters_list[effect_parameter_index].index_max - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min) > INDEX_RANGE_MAX) {
      //range out of limit error
      numl_parser_context->error_code = NU_PARSER_ERROR_RANGE_DEFINITON_TOO_LARGE;
      return -1;
   }

   if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
      //error pair index,value invalid format
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
      return -1;
   }
   //
   if (strlen(value) <= 0) {
      //error value of pair index,value not found
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_EXPECTED;
      return -1;
   }
   //

   int8_t int_index = atoi(index);

   //
   if (int_index<numl_parser_context->effect_parameters_list[effect_parameter_index].index_min || int_index>numl_parser_context->effect_parameters_list[effect_parameter_index].index_max) {
      //error out of range
      numl_parser_context->error_code = NU_PARSER_ERROR_INDEX_OUT_RANGE;
      return -1;
   }

   //
   numl_parser_context->effect_parameters_list[effect_parameter_index].gpio_used |= (((gpio_vector_t)(0x1)) << gpio_no);
   //
   uint8_t int_value = atoi(value);
   gpio_vector_t gpio_vector_mask= (((gpio_vector_t)(0x1)) << gpio_no);
   //
   if(int_value>0)
      numl_parser_context->effect_parameters_list[effect_parameter_index].gpio_value[int_index - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] |= gpio_vector_mask;
   else
      numl_parser_context->effect_parameters_list[effect_parameter_index].gpio_value[int_index - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] &= ~gpio_vector_mask;
   //
   return 0;
}

//
static int numl_parser_set_attribute_dac_parameters(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index,uint8_t dac_parameter_type, char* pair_index_value) {
   char *index;
   char *value;
   //
   if (numl_parser_context->effect_parameters_list[effect_parameter_index].index_min >= numl_parser_context->effect_parameters_list[effect_parameter_index].index_max) {
      // range error
      numl_parser_context->error_code = NU_PARSER_ERROR_RANGE_DEFINITON_MINMAX_INVALID;
      return -1;
   }

   if ((numl_parser_context->effect_parameters_list[effect_parameter_index].index_max - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min) > INDEX_RANGE_MAX) {
      //range out of limit error
      numl_parser_context->error_code = NU_PARSER_ERROR_RANGE_DEFINITON_TOO_LARGE;
      return -1;
   }

   if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
      //error pair index,value invalid format
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
      return -1;
   }
   //
   if (strlen(value) <= 0) {
      //error value of pair index,value not found
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_EXPECTED;
      return -1;
   }
   //
   if (!strcmp(index, const_attribute_pair_index_range)) {
      char* str_value_interval_1;
      char* str_value_interval_2;
      uint8_t interval_1;
      uint8_t interval_2;

      //
      if (numl_parser_pair_value_to_interval(&str_value_interval_1, &str_value_interval_2, value) < 0) {
         //error invalid value format
         numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_INTERVAL_INVALID_FORMAT;
         return -1;
      }
      //
      numl_parser_context->effect_parameters_list[effect_parameter_index].dac_used = 1;
      //
      interval_1 = atoi(str_value_interval_1);
      interval_2 = atoi(str_value_interval_2);
      int8_t step;
      uint8_t int_value;
      step = (interval_2 - interval_1) / (numl_parser_context->effect_parameters_list[effect_parameter_index].index_max - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min);
      //
      int_value = interval_1;
      //
      for (int i = numl_parser_context->effect_parameters_list[effect_parameter_index].index_min;
         i <= numl_parser_context->effect_parameters_list[effect_parameter_index].index_max;
         i++) { 
          switch (dac_parameter_type) {
            //
            case DAC_PARAMETER_TYPE_FREQUENCY:
               numl_parser_context->effect_parameters_list[effect_parameter_index].dac_frequency_value[i - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] = int_value;
            break;
            //
            case DAC_PARAMETER_TYPE_LEVEL:
               numl_parser_context->effect_parameters_list[effect_parameter_index].dac_level_value[i - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] = int_value;
            break;
         }
         //
         int_value += step;
      }
      //

   }
   else {
      int8_t int_index = atoi(index);
      //
      if (int_index<numl_parser_context->effect_parameters_list[effect_parameter_index].index_min || int_index>numl_parser_context->effect_parameters_list[effect_parameter_index].index_max) {
         numl_parser_context->error_code = NU_PARSER_ERROR_INDEX_OUT_RANGE;
         return -1;
      }
      //
      numl_parser_context->effect_parameters_list[effect_parameter_index].dac_used = 1;
      //
      uint8_t int_value = atoi(value);
      //
      switch (dac_parameter_type) {
         //
         case DAC_PARAMETER_TYPE_FREQUENCY:
            numl_parser_context->effect_parameters_list[effect_parameter_index].dac_frequency_value[int_index - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] = int_value;
            break;
            //
         case DAC_PARAMETER_TYPE_LEVEL:
            numl_parser_context->effect_parameters_list[effect_parameter_index].dac_level_value[int_index - numl_parser_context->effect_parameters_list[effect_parameter_index].index_min] = int_value;
            break;
         }
   }

   //
   return 0;
}

//
static int numl_parser_set_attribute_dac_wavform(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, char* pair_index_value) {
   //to do, manage dc, sinus, square...
   return 0;
}
//
static int numl_parser_set_attribute_dac_frequency(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, char* pair_index_value) {
   return numl_parser_set_attribute_dac_parameters(numl_parser_context, effect_parameter_index, DAC_PARAMETER_TYPE_FREQUENCY, pair_index_value);
}
//
static int numl_parser_set_attribute_dac_level(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, char* pair_index_value) {
   return numl_parser_set_attribute_dac_parameters(numl_parser_context, effect_parameter_index, DAC_PARAMETER_TYPE_LEVEL, pair_index_value);
}

//
static int numl_parser_set_attribute_display(numl_parser_context_t* numl_parser_context, uint8_t effect_parameter_index, char* pair_index_value) {
   return 0;
}
//
static int numl_parser_set_hardware_attribute_pot(numl_parser_context_t* numl_parser_context, char* attribute_name, int8_t pot_no, char* pair_index_value) {
   char *index;
   char *value;
   //
   if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
      //error pair index,value invalid format
      numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
      return -1;
   }
   //
   if (!strcmp(index, const_attribute_pair_index_mount)) {
      nu_effect_parameters_mount_potentiometer(pot_no, value);
   }else if (!strcmp(index, const_attribute_pair_index_open)) {
      nu_effect_parameters_open_potentiometer(pot_no, value);
   }
   //
   if (numl_parser_context->flag_endofarray) {
      //at this day nothing to do
   }
   //
   return 0;
}

//
static int section_numl_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pair_index_value) {
   __numl_parser_debug_printf(numl_parser_context->flag_debug, "enter in callback numl\r\n");

   if (!strcmp(attribute_name, const_attribute_version)) {
      char *index;
      char *value;
      if (numl_parser_pair_to_index_value(&index, &value, pair_index_value) < 0) {
         //error pair index,value invalid format
         numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT;
         return -1;
      }
      //
      __numl_parser_debug_printf(numl_parser_context->flag_debug, "%s , %s\r\n",index,value);
      //
      return 0;
   }

   return 0;
}
//
static int section_hardware_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pair_index_value) {
   __numl_parser_debug_printf(numl_parser_context->flag_debug, "enter in callback hardware\r\n");
   int8_t pot_no = -1;
   //
   //pot
   if (!strcmp(attribute_name, const_attribute_pot_0)) {
      pot_no = 0;
   }
   if (!strcmp(attribute_name, const_attribute_pot_1)) {
      pot_no = 1;
   }
   if (!strcmp(attribute_name, const_attribute_pot_2)) {
      pot_no = 2;
   }
   if (!strcmp(attribute_name, const_attribute_pot_3)) {
      pot_no = 3;
   }
   if (!strcmp(attribute_name, const_attribute_pot_4)) {
      pot_no = 4;
   }
   if (!strcmp(attribute_name, const_attribute_pot_5)) {
      pot_no = 5;
   }
   if (!strcmp(attribute_name, const_attribute_pot_6)) {
      pot_no = 6;
   }
   if (!strcmp(attribute_name, const_attribute_pot_7)) {
      pot_no = 7;
   }
   //
   if (pot_no >= 0) {
      return numl_parser_set_hardware_attribute_pot(numl_parser_context, attribute_name, pot_no, pair_index_value);
   }
   //
  
   return 0;
}

//
static int section_rotary_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pair_index_value) {
   __numl_parser_debug_printf(numl_parser_context->flag_debug,"enter in callback rotary\r\n");
   int effect_parameter_index = 0;
   int8_t pot_no=-1;
   int8_t gpio_no=-1;

   //
   if (!strcmp(section_name, const_section_rotary_1)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_1_INDEX;
   }else if(!strcmp(section_name, const_section_rotary_2)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_2_INDEX;
   }else if (!strcmp(section_name, const_section_rotary_3)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_3_INDEX;
   }else if (!strcmp(section_name, const_section_rotary_4)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_4_INDEX;
   }else if (!strcmp(section_name, const_section_rotary_1_pushed)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_1_PUSHED_INDEX;
   }else if (!strcmp(section_name, const_section_rotary_2_pushed)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_2_PUSHED_INDEX;
   }else if (!strcmp(section_name, const_section_rotary_3_pushed)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_3_PUSHED_INDEX;
   }else if (!strcmp(section_name, const_section_rotary_4_pushed)) {
      effect_parameter_index = EFFECT_PARAMETER_ROTARY_4_PUSHED_INDEX;
   }else {
      //error unrecognized rotary section name
      return -1;
   }

   //label
   if (!strcmp(attribute_name, const_attribute_label)) {
      return numl_parser_set_attribute_label(numl_parser_context, effect_parameter_index, pair_index_value);
   }

   //range
   if (!strcmp(attribute_name, const_attribute_range)) {
      return numl_parser_set_attribute_range(numl_parser_context, effect_parameter_index, pair_index_value);
   }

   //pot
   if (!strcmp(attribute_name, const_attribute_pot_0)) {
      pot_no = 0;
   }
   if (!strcmp(attribute_name, const_attribute_pot_1)) {
      pot_no = 1;
   }
   if (!strcmp(attribute_name, const_attribute_pot_2)) {
      pot_no = 2;
   }
   if (!strcmp(attribute_name, const_attribute_pot_3)) {
      pot_no = 3;
   }
   if (!strcmp(attribute_name, const_attribute_pot_4)) {
      pot_no = 4;
   }
   if (!strcmp(attribute_name, const_attribute_pot_5)) {
      pot_no = 5;
   }
   if (!strcmp(attribute_name, const_attribute_pot_6)) {
      pot_no = 6;
   }
   if (!strcmp(attribute_name, const_attribute_pot_7)) {
      pot_no = 7;
   }
   //
   if (pot_no >= 0) {
      return numl_parser_set_attribute_pot(numl_parser_context, effect_parameter_index, pot_no, pair_index_value);
   }

   //gpio
   if (!strcmp(attribute_name, const_attribute_gpio_0)) {
      gpio_no = 0;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_1)) {
      gpio_no = 1;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_2)) {
      gpio_no = 2;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_3)) {
      gpio_no = 3;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_4)) {
      gpio_no = 4;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_5)) {
      gpio_no = 5;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_6)) {
      gpio_no = 6;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_7)) {
      gpio_no = 7;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_8)) {
      gpio_no = 8;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_9)) {
      gpio_no = 9;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_10)) {
      gpio_no = 10;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_11)) {
      gpio_no = 11;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_12)) {
      gpio_no = 12;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_13)) {
      gpio_no = 13;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_14)) {
      gpio_no = 14;
   }
   if (!strcmp(attribute_name, const_attribute_gpio_15)) {
      gpio_no = 15;
   }
   //
   if (gpio_no >= 0) {
      return numl_parser_set_attribute_gpio(numl_parser_context, effect_parameter_index, gpio_no, pair_index_value);
   }
  
   //display
   if (!strcmp(attribute_name, const_attribute_display)) {
      return 0;
   }
   //dac waveform
   if (!strcmp(attribute_name, const_attribute_dac_waveform)) {
      return numl_parser_set_attribute_dac_wavform(numl_parser_context, effect_parameter_index, pair_index_value);
   }
   //dac frequency
   if (!strcmp(attribute_name, const_attribute_dac_frequency)) {
      return numl_parser_set_attribute_dac_frequency(numl_parser_context, effect_parameter_index, pair_index_value);;
   }
   //dac level
   if (!strcmp(attribute_name, const_attribute_dac_level)) {
      return numl_parser_set_attribute_dac_level(numl_parser_context, effect_parameter_index, pair_index_value);;
   }
   //
   numl_parser_context->error_code = NU_PARSER_ERROR_UNRECOGNIZED_ATTRIBUTE_NAME;
   //
   return -1;
}

//
static int section_authentication_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pairs_index_value) {
   __numl_parser_debug_printf(numl_parser_context->flag_debug,"enter in callback authentication\r\n");
   return 0;
}

//
int numl_parser_callback(numl_parser_context_t* numl_parser_context, char* section_name, char* attribute_name, char* pairs_index_value) {
   //
   for (int i = 0; i < numl_parser_section_callback_list_size; i++) {
      if (strcmp(numl_parser_section_callback_list[i].section_name, section_name)){
         continue;
      }
      //
      return numl_parser_section_callback_list[i].section_callback(numl_parser_context, section_name, attribute_name, pairs_index_value);
   }
   //error unrecognized  section name
   numl_parser_context->error_code = NU_PARSER_ERROR_UNRECOGNIZED_SECTION_NAME;
   return -1;
}

//
int numl_parser_run(numl_parser_context_t* numl_parser_context){
   char buf[NU_PARSER_BUFFER_SIZE_MAX];
   //
   char section_name[NU_PARSER_SECTION_NAME_SIZE_MAX];
   char *p_section_name= section_name;
   *p_section_name = '\0';
   //
   char attribute_name[NU_PARSER_ATTRIBUTE_NAME_SIZE_MAX];
   char *p_attribute_name = attribute_name;
   *p_attribute_name = '\0';
   //
   char pair_value[NU_PARSER_PAIR_VALUE_SIZE_MAX];
   char *p_pair_value = pair_value;
   *pair_value = '\0';
   //
   int cb = 0;
   uint8_t parser_section_status = NU_PARSER_WAIT_SECTION_TAG_OPEN;
   uint8_t parser_section_inside_status = NU_PARSER_SECTION_NULL;
   uint8_t flag_comment_section = 0;
   //
   numl_parser_context->flag_endofarray = 0;
   //
   if (numl_parser_context == (void*)0) {
      return -1;
   }

   //
#ifndef USE_NUML_STANDALONE
   while ((cb = read(numl_parser_context->fd, buf, sizeof(buf))) > 0) {
#else
   while ((cb = _read(numl_parser_context->fd, buf, sizeof(buf))) > 0) {
#endif
      for (int i = 0; i < cb; i++) {
         char c = buf[i];
         
         if (c == '\n') {
            numl_parser_context->line++;
            numl_parser_context->column=1;
         }else {
            numl_parser_context->column++;
         }
         //
         if (c == '\r') {
            //nothing to do
            continue;
         }

         //double quote: pair value get string " "
         if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_PAIR_DQUOTE) {
            if (c == '"') {
               parser_section_inside_status = NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE;
               continue;
            }
            //
            if (strlen(pair_value) >= sizeof(pair_value)-1) {
               //error
               numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_TOO_LONG;
               return -1;
            }
            //
            *p_pair_value++ = c;
            *p_pair_value = '\0';
            continue;
         }

         //
         if (flag_comment_section) {
            if (c == '!' 
               && parser_section_status == NU_PARSER_WAIT_SECTION_TAG_CLOSE
               && parser_section_inside_status == NU_PARSER_SECTION_WAIT_END_OF_COMMENT) {
               parser_section_inside_status = NU_PARSER_SECTION_NULL;
            }else if (c == '}'
               && parser_section_status == NU_PARSER_WAIT_SECTION_TAG_CLOSE
               && parser_section_inside_status == NU_PARSER_SECTION_NULL) {
               parser_section_status = NU_PARSER_WAIT_SECTION_TAG_OPEN;
               flag_comment_section=0;
            }
            //
            continue;
         }

         //
         if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_LINEFEED && ((c != '\n') && (c != ' ') && (c != '\t')) ) {
            //error wait only '\n', ' ', '\t'
            numl_parser_context->error_code = NU_PARSER_ERROR_UNEXPECTED_CHARACTER;
            return -1;
         }
         //
         if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_EQUAL_SIGN && ((c != '=') && (c != ' ') && (c != '\t') ) ) {
            //error wait only '\n', ' ', '\t'
            numl_parser_context->error_code = NU_PARSER_ERROR_UNEXPECTED_CHARACTER;
            return -1;
         }
         if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_OPEN && ((c != '[') && (c != ' ') && (c != '\t'))) {
            //error wait only '\n', ' ', '\t'
            numl_parser_context->error_code = NU_PARSER_ERROR_UNEXPECTED_CHARACTER;
            return -1;
         }

         //
         if (c == '!' && !flag_comment_section) {
            if (parser_section_inside_status == NU_PARSER_SECTION_GET_NAME && !strlen(section_name)) {
               parser_section_inside_status = NU_PARSER_SECTION_WAIT_END_OF_COMMENT;
               flag_comment_section=1;
            }
         } else if (c == '{') {
            if (parser_section_status == NU_PARSER_WAIT_SECTION_TAG_OPEN) {
               parser_section_status = NU_PARSER_WAIT_SECTION_TAG_CLOSE;
               //
               p_section_name = section_name;
               *p_section_name = '\0';
               //
               parser_section_inside_status = NU_PARSER_SECTION_GET_NAME;
               //
               p_attribute_name = attribute_name;
               *p_attribute_name = '\0';

            }
            else {
               //error
               numl_parser_context->error_code = NU_PARSER_ERROR_UNEXPECTED_CHARACTER;
               return -1;
            }

         }else if (c == '}') {
            if (parser_section_status == NU_PARSER_WAIT_SECTION_TAG_CLOSE) {
               parser_section_status = NU_PARSER_WAIT_SECTION_TAG_OPEN;
               parser_section_inside_status = NU_PARSER_SECTION_NULL;
            }
            else {
               //error
               numl_parser_context->error_code = NU_PARSER_ERROR_INTERNAL_PARSER_ERROR;
               return -1;
            }
         }else if ((c >= '0' && c <= '9')
            || (c >= 'a' && c <= 'z')
            || (c >= 'A' &&c <= 'Z')
            || (c == '_') || (c == '.')
            || (c == '+') || (c == '-')
            ) {

            //
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_OPEN) {
               //error wait '[' 
               numl_parser_context->error_code = NU_PARSER_ERROR_ARRAY_BRACKET_EXPECTED;
               return -1;
            }
            //
            if (parser_section_inside_status == NU_PARSER_SECTION_GET_NAME) {
               if (strlen(section_name) >= (sizeof(section_name)-1)) {
                  //error
                  numl_parser_context->error_code = NU_PARSER_ERROR_SECTION_NAME_TOO_LONG;
                  return -1;
               }
               //
               *p_section_name++ = c;
               *p_section_name = '\0';
               //
               continue;
            }
            //
            if (parser_section_inside_status == NU_PARSER_SECTION_GET_ATTRIBUTE_NAME) {
               if (strlen(attribute_name) >= (sizeof(attribute_name)-1)) {
                  //error
                  numl_parser_context->error_code = NU_PARSER_ERROR_ATTRIBUTE_NAME_TOO_LONG;
                  return -1;
               }
               //
               *p_attribute_name++ = c;
               *p_attribute_name = '\0';
               //
               continue;
            }
            //
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE) {
               if (strlen(pair_value) >= (sizeof(pair_value)-1)) {
                  //error
                  numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_TOO_LONG;
                  return -1;
               }
               //
               *p_pair_value++ = c;
               *p_pair_value = '\0';
               //
               continue;
            }
         }else if (c == '\r') {//cariage return
           //nothing to do
         }else if (c == '\n' ) {//line feed (windows/linux)

            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_LINEFEED) {
               parser_section_inside_status = NU_PARSER_SECTION_GET_ATTRIBUTE_NAME;
               continue;
            }

            if (parser_section_inside_status == NU_PARSER_SECTION_GET_NAME) {
               if (!strlen(section_name)) {
                  numl_parser_context->error_code = NU_PARSER_ERROR_SECTION_NAME_EXPECTED;
                  return -1;
               }
               //element name section
               __numl_parser_debug_printf(numl_parser_context->flag_debug,"element name:%s\r\n", section_name);
               //
               parser_section_inside_status = NU_PARSER_SECTION_GET_ATTRIBUTE_NAME;
               //
               p_attribute_name = attribute_name;
               *p_attribute_name = '\0';
            }else if (parser_section_inside_status == NU_PARSER_SECTION_GET_ATTRIBUTE_NAME) {
               if (strlen(attribute_name)) {
                  //error wait '=' 
                  numl_parser_context->error_code = NU_PARSER_ERROR_EQUAL_SIGN_EXPECTED;
                  return -1;
               }
            }else if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_OPEN) {
                  //error wait '[' 
                  numl_parser_context->error_code = NU_PARSER_ERROR_ARRAY_BRACKET_EXPECTED;
                  return -1;
            }
            //
         }else if ((c == ' ') || (c == '\t')) {
            if (parser_section_inside_status == NU_PARSER_SECTION_GET_NAME) {
               if ( strlen(section_name)) {
                  //element name section ready
                  __numl_parser_debug_printf(numl_parser_context->flag_debug,"element name:%s\r\n", section_name);
                  parser_section_inside_status = NU_PARSER_SECTION_WAIT_LINEFEED;
               }
            }else if (parser_section_inside_status == NU_PARSER_SECTION_GET_ATTRIBUTE_NAME) {
               if (strlen(attribute_name)) {
                  //attribute name section ready
                  __numl_parser_debug_printf(numl_parser_context->flag_debug,"attribute name:%s\r\n", attribute_name);
                  parser_section_inside_status = NU_PARSER_SECTION_WAIT_EQUAL_SIGN;
               }
            }
         }else if (c == '=') {
            if (parser_section_inside_status == NU_PARSER_SECTION_GET_ATTRIBUTE_NAME 
               || parser_section_inside_status == NU_PARSER_SECTION_WAIT_EQUAL_SIGN
               ) {
               parser_section_inside_status = NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_OPEN;
            }
         }else if (c == '[') {
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_OPEN) {
               parser_section_inside_status = NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE;
            }else {
               //error invalide sequence 
               numl_parser_context->error_code = NU_PARSER_ERROR_INTERNAL_PARSER_ERROR;
               return -1;
            }
         }else if (c == ']') {
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE) {
               //
               __numl_parser_debug_printf(numl_parser_context->flag_debug,"pair %s \r\n", pair_value);
               //
               numl_parser_context->flag_endofarray = 1;
               // call callback
               if (numl_parser_callback(numl_parser_context, section_name, attribute_name, pair_value) < 0) {
                  //error
                  return -1;
               }
               //
               numl_parser_context->flag_endofarray = 0;
               //
               p_pair_value = pair_value;
               *pair_value = '\0';
               //
               p_attribute_name = attribute_name;
               *p_attribute_name = '\0';
               //
               parser_section_inside_status = NU_PARSER_SECTION_WAIT_LINEFEED;
            }
            else {
               //error invalide sequence 
               numl_parser_context->error_code = NU_PARSER_ERROR_INTERNAL_PARSER_ERROR;
               return -1;
            }
         }else if (c == ',') {
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE) {
               //
               __numl_parser_debug_printf(numl_parser_context->flag_debug,"pair %s ", pair_value);
               // call callback
               if (numl_parser_callback(numl_parser_context, section_name, attribute_name, pair_value) < 0) {
                  //error
                  return -1;
               }
               //
               p_pair_value = pair_value;
               *pair_value = '\0';
            }
         }else if (c == ':') {
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE) {
               if (strlen(pair_value) >= (sizeof(pair_value)-1)) {
                  //error
                  numl_parser_context->error_code = NU_PARSER_ERROR_PAIR_VALUE_TOO_LONG;
                  return -1;
               }
               //
               *p_pair_value++ = c;
               *p_pair_value = '\0';
            }
         }else if (c == '"') {
            if (parser_section_inside_status == NU_PARSER_SECTION_WAIT_ATTRIBUTE_ARRAY_BRACKET_CLOSE) {
               parser_section_inside_status = NU_PARSER_SECTION_WAIT_ATTRIBUTE_PAIR_DQUOTE;
            }else {
               numl_parser_context->error_code = NU_PARSER_ERROR_DQUOTE_EXPECTED;
               return -1;
            }
         }else {
            //nothing to do
         }
              
      }//for
   }//while

   return 0;
}

//
int numl_parser_open(numl_parser_context_t* numl_parser_context, effect_parameter_t* effect_parameters_list,char* file_path) {
#ifndef USE_NUML_STANDALONE
   numl_parser_context->fd = open(file_path, O_RDONLY,0);
#else
   numl_parser_context->fd = _open(file_path, O_RDONLY, 0);
#endif
   //
   if (numl_parser_context->fd < 0) {
      return -1;
   }
   //
   numl_parser_context->flag_debug = 1;
   numl_parser_context->error_code = NU_PARSER_NO_ERROR;
   numl_parser_context->line = 1;
   numl_parser_context->column = 0;
   //
   numl_parser_context->effect_parameters_list = effect_parameters_list;
   //
   return 0;
}

//
int numl_parser_close(numl_parser_context_t* numl_parser_context) {
   //
#ifndef USE_NUML_STANDALONE
   close(numl_parser_context->fd);
#else
   _close(numl_parser_context->fd);
#endif
   //
   return 0;
}
