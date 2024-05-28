#ifndef __nuodio_parser_h__ 
#define __nuodio_parser_h__ 

//#pragma once
typedef enum numl_parser_error_code_st {
   NU_PARSER_NO_ERROR = 0,
   NU_PARSER_ERROR_UNEXPECTED_CHARACTER,
   NU_PARSER_ERROR_INTERNAL_PARSER_ERROR,
   NU_PARSER_ERROR_SECTION_NAME_TOO_LONG,
   NU_PARSER_ERROR_ATTRIBUTE_NAME_TOO_LONG,
   NU_PARSER_ERROR_PAIR_VALUE_TOO_LONG,
   NU_PARSER_ERROR_SECTION_NAME_EXPECTED,
   NU_PARSER_ERROR_EQUAL_SIGN_EXPECTED,
   NU_PARSER_ERROR_ARRAY_BRACKET_EXPECTED,
   NU_PARSER_ERROR_DQUOTE_EXPECTED,

   NU_PARSER_ERROR_UNRECOGNIZED_SECTION_NAME,
   NU_PARSER_ERROR_UNRECOGNIZED_ATTRIBUTE_NAME,

   NU_PARSER_ERROR_RANGE_ERROR,
   NU_PARSER_ERROR_RANGE_DEFINITON_MINMAX_INVALID,
   NU_PARSER_ERROR_RANGE_DEFINITON_TOO_LARGE,

   NU_PARSER_ERROR_PAIR_VALUE_EXPECTED,
   NU_PARSER_ERROR_INDEX_OUT_RANGE,

   NU_PARSER_ERROR_PAIR_INDEX_VALUE_INVALID_FORMAT,
   NU_PARSER_ERROR_PAIR_INDEX_VALUE_NOT_FOUND,
   NU_PARSER_ERROR_PAIR_UNRECOGNIZED_RANGE_INDEX_NAME,
   NU_PARSER_ERROR_PAIR_VALUE_INTERVAL_INVALID_FORMAT
}numl_parser_error_code_t;

//
typedef struct numl_parser_context_st {
   int fd;
   int line;
   int column;
   //
   uint8_t flag_endofarray;
   //
   uint8_t flag_debug;
   //
   numl_parser_error_code_t error_code;
   
   //
   effect_parameter_t* effect_parameters_list;
}numl_parser_context_t;



//
const char* numl_parser_message_error(numl_parser_context_t* numl_parser_context);
int numl_parser_print_error(numl_parser_context_t* numl_parser_context);

int numl_parser_open(numl_parser_context_t* numl_parser_context, effect_parameter_t* effect_parameters_list, char* file_path);
int numl_parser_run(numl_parser_context_t* numl_parser_context);
int numl_parser_close(numl_parser_context_t* numl_parser_context);

#endif
