/* Copyright (c) 2018 nuodio. All Rights Reserved.
*
* The information contained herein is property of nuodio.
*
*
*/


/*============================================
| Compiler Directive
==============================================*/
#ifndef __NU_PRESET_H__
#define __NU_PRESET_H__


/*============================================
| Includes
==============================================*/
//
#define NU_MIDI_PRESET_MIN   0
#define NU_MIDI_PRESET_MAX  32

/*============================================
| Declaration
==============================================*/
//
#define NU_PRESET_NAME_LENGTH_MAX 16
//
#define NU_EFFECT_PARAMETERS_MAX 8
//
#define NU_PRESET_VECTOR_SIZE ((NU_MIDI_PRESET_MAX/8)+1)
//
#define NU_PRESET_UUID_BASE64_LENGTH_MAX 22

//
#if (__tauon_compiler__!=__compiler_gnuc__)
   #pragma pack(push, 1)
#endif

//
#define NU_PRESET_STATUS_FREE ((uint8_t)(0x00))
#define NU_PRESET_STATUS_USED ((uint8_t)(0x01))

//
typedef struct  nu_preset_st{
   uint8_t status;
   //
   char name[NU_PRESET_NAME_LENGTH_MAX+1];
   char uuid_base64[NU_PRESET_UUID_BASE64_LENGTH_MAX];
   char root_uuid_base64[NU_PRESET_UUID_BASE64_LENGTH_MAX];
   //
   int8_t effect_parameters_index[NU_EFFECT_PARAMETERS_MAX];
   //
   uint32_t used_preset_counter;
}nu_preset_t;

#if (__tauon_compiler__!=__compiler_gnuc__)
   #pragma pack (pop)
#endif

//
typedef struct nu_preset_storage_st{
   int fd;
   uint8_t  vector[NU_PRESET_VECTOR_SIZE];
   nu_preset_t preset_list[NU_MIDI_PRESET_MAX];
}nu_preset_storage_t;

//
int nu_preset_prepare(nu_preset_storage_t* p_nu_preset_storage);
int nu_preset_make(nu_preset_storage_t* p_nu_preset_storage);
int nu_preset_load(nu_preset_storage_t* p_nu_preset_storage);
int nu_preset_sync(nu_preset_storage_t* p_nu_preset_storage);
int nu_preset_lookup_free(nu_preset_storage_t* p_nu_preset_storage);
int nu_preset_is_used(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no);
int nu_preset_set_used(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no);
int nu_preset_set_free(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no);
char* nu_preset_get_name(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no);
int nu_preset_get(nu_preset_storage_t* p_nu_preset_storage,nu_preset_t* p_preset,uint8_t preset_no);
int nu_preset_put(nu_preset_storage_t* p_nu_preset_storage,uint8_t preset_no,nu_preset_t* p_preset);
//
int nu_preset_init(nu_preset_storage_t* p_nu_preset_storage,uint8_t flag_force_make);


#endif