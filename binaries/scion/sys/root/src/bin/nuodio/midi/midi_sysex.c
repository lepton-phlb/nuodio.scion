/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2019 <lepton.phlb@gmail.com>.
All Rights Reserved.

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
#include <stdint.h>
#include <stdlib.h>

#ifdef USE_MIDI_SYSEX_STANDALONE
   //#include <assert.h>
   //#include <windows.h>
   #include <stdio.h>
   #include <string.h>
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <io.h>
   //
   #include "dev_win32uart.h"
   //
   #include "crc.h"
#else
   #include "kernel/core/kernelconf.h"
   #include "kernel/core/system.h"
   #include "kernel/core/signal.h"
   #include "kernel/core/libstd.h"
   #include "kernel/core/dirent.h"
   #include "kernel/core/stat.h"
   #include "kernel/core/statvfs.h"
   #include "kernel/core/time.h"
   #include "kernel/core/wait.h"
   #include "kernel/core/fcntl.h"
   #include "kernel/core/ioctl.h"
   #include "kernel/core/select.h"
   #include "kernel/core/stropts.h"
   //
   #include "lib/libc/unistd.h"
   #include "lib/libc/stdio/stdio.h"
   //
   #include "lib/libc/misc/crc.h"
#endif

//
#include "midi_sysex.h"

/* 
nuodio MIDI SYSEX command list:
   get.sysex.id
   set.midi.channel
   get.midi.channel
   get.uuid
   get.model
   get.preset.max
   set.preset
   get.preset
   free.preset
   free.preset.all
   swap.preset
   move.preset
*/

//
typedef int(*pfn_sysex_request_callback_t)(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
//
typedef struct sysex_parser_request_definition_st {
   const char* sysex_request_name;
   pfn_sysex_request_callback_t pfn_sysex_request_callback;
}sysex_parser_request_definition_t;

//
int parser_get_sysex_id_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_set_midi_channel_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_get_midi_channel_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_get_uuid_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_get_model_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_get_preset_max_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_set_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_get_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_free_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_free_preset_all_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_swap_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);
int parser_move_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments);

//
const sysex_parser_request_definition_t sysex_parser_request_definition_list[] = {
   {.sysex_request_name = "get.sysex.id",.pfn_sysex_request_callback = parser_get_sysex_id_callback},
   {.sysex_request_name = "set.midi.channel",.pfn_sysex_request_callback = parser_set_midi_channel_callback},
   {.sysex_request_name = "get.midi.channel",.pfn_sysex_request_callback = parser_get_midi_channel_callback},
   {.sysex_request_name = "get.uuid",.pfn_sysex_request_callback = parser_get_uuid_callback },
   {.sysex_request_name = "get.model",.pfn_sysex_request_callback = parser_get_model_callback },
   {.sysex_request_name = "get.preset.max",.pfn_sysex_request_callback = parser_get_preset_max_callback },
   {.sysex_request_name = "set.preset",.pfn_sysex_request_callback = parser_set_preset_callback },
   {.sysex_request_name = "get.preset",.pfn_sysex_request_callback = parser_get_preset_callback },
   {.sysex_request_name = "free.preset",.pfn_sysex_request_callback = parser_free_preset_callback },
   {.sysex_request_name = "free.preset.all",.pfn_sysex_request_callback = parser_free_preset_all_callback },
   {.sysex_request_name = "swap.preset",.pfn_sysex_request_callback = parser_swap_preset_callback },
   {.sysex_request_name = "move.preset",.pfn_sysex_request_callback = parser_move_preset_callback }
};
//
const uint8_t sysex_parser_request_definition_list_sz = sizeof(sysex_parser_request_definition_list) / sizeof(sysex_parser_request_definition_t);

//
char* sysex_parser_get_data_string_buffer(sysex_parser_context_t* sysex_parser_context);
int sysex_parser_prepare_response(sysex_parser_context_t* sysex_parser_context);

//
int parser_get_sysex_id_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_set_midi_channel_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   //
   if (request_arguments == (char*)0) {
      return -1;
   }
   int midi_channel= atoi(request_arguments);
   if (midi_channel > 0xF) {
      return -1;
   }
   //
   printf("\r\nset midi channel: %d [0x%02x]\r\n", midi_channel + 1, midi_channel);
   //
   return 0;
}

//
int parser_get_midi_channel_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_get_uuid_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_get_model_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_get_preset_max_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_set_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
return -1;
}

//
int parser_get_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_free_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_free_preset_all_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
int parser_swap_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
return -1;
}

//
int parser_move_preset_callback(sysex_parser_context_t* sysex_parser_context, char* request_name, char* request_arguments) {
   return -1;
}

//
char* sysex_parser_get_data_string_buffer(sysex_parser_context_t* sysex_parser_context) {
   return (char*)&sysex_parser_context->p_data_buffer[SYSEX_PROTOCOL_FRAME_STRING_DATA_OFFSET];
}

//
int sysex_parser_prepare_response(sysex_parser_context_t* sysex_parser_context) {
   int cb = 0;
   int crc_cb = 0;

   //
   sysex_parser_context->p_data_buffer[cb++] = 0xF0;
   sysex_parser_context->p_data_buffer[cb++] = 0x00;
   sysex_parser_context->p_data_buffer[cb++] = MIDI_SYSEX_NUODIO_ID_MSB;
   sysex_parser_context->p_data_buffer[cb++] = MIDI_SYSEX_NUODIO_ID_LSB;
   //
   crc_cb = cb;
   //
   sysex_parser_context->p_data_buffer[cb++] = SYSEX_PROTOCOL_DECODER_FRAMETYPE_RESPONSE;
   //
   int str_ascii_data_length = strlen((char*)&sysex_parser_context->p_data_buffer[SYSEX_PROTOCOL_FRAME_STRING_DATA_OFFSET]);
   //
   sysex_parser_context->p_data_buffer[cb++] = 2 + 1 + str_ascii_data_length;//frame length=dst id+src id+request id
   //
   sysex_parser_context->p_data_buffer[cb++] = sysex_parser_context->src_dev_id;//dst id 
   sysex_parser_context->p_data_buffer[cb++] = sysex_parser_context->dst_dev_id;//src id
   //
   sysex_parser_context->p_data_buffer[cb++] = sysex_parser_context->request_counter;
   //
   // data already filled
   //
   cb += str_ascii_data_length;
   //
   crc16_t crc;
   __init_crc16(crc);
   //
   for (int i = crc_cb; i < cb; i++) {
      crc = crc16(crc, sysex_parser_context->p_data_buffer[i]);
   }
   //
   uint8_t frame_crc[3];
   frame_crc[0] = crc / 16384; //msb
   frame_crc[1] = (crc % 16384) / 128;
   frame_crc[2] = (crc % 16384) % 128;//lsb
   //
   sysex_parser_context->p_data_buffer[cb++] = frame_crc[0];//crc msb (not used in discover)
   sysex_parser_context->p_data_buffer[cb++] = frame_crc[1];//crc (not used in discover)
   sysex_parser_context->p_data_buffer[cb++] = frame_crc[2];//crc lsb (not used in discover)
   //
   sysex_parser_context->p_data_buffer[cb++] = 0xF7;//0xF7 EOX
   //
   return cb;
}

//
int sysex_request_parser(sysex_parser_context_t* p_sysex_parser_context, char* str) {
   char* request_name = (char*)0;
   char* request_arg = (char*)0;
   //int length = __get_data_string_length(p_sysex_parser_context);
   int length = strlen(str);

   //
   if (length == 0) {
      //nothing to do 
      return 0;
   }
   //
   if (str[0] != '{' || str[length-1] != '}') {
      return -1;
   }
   //
   for (int i = 0; i < length; i++) {
      if (request_name == (char*)0 && str[i] == '{') {
         str[i] = '\0';
         if (i + 1 >= (length-1)) {
            return -1;
         }
         //
         request_name = &str[i + 1];
      }
      if (request_arg == (char*)0 && str[i] == ':') {
         str[i] = '\0';
         if (i + 1 >= (length-1)) {
            return -1;
         }
         //
         request_arg = &str[i + 1];
      }
      //
      if (str[i] == '}') {
         str[i] = '\0';
      }
   }

   //
   if (request_name == (char*)0) {
      return -1;
   }

   //
   for (int i = 0; i < sysex_parser_request_definition_list_sz; i++) {
      if (!strcmp(sysex_parser_request_definition_list[i].sysex_request_name, request_name)) {
         return sysex_parser_request_definition_list[i].pfn_sysex_request_callback(p_sysex_parser_context, request_name, request_arg);
      }

   }
   //error request name not found
   return -1;
}

//
int sysex_protocol_decoder(sysex_parser_context_t* p_sysex_parser_context, uint8_t midi_byte) {

   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0 && midi_byte != 0xF0) {
      //drop
      sysex_protocol_context_reset(p_sysex_parser_context);
      //error drop packet stay in state wait SYSEX command 0xF0
      return -1;
   }
   //
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0 && midi_byte == 0xF0) {
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_ID00;
      return 0;
   }
   //wait data but receive status command
   if (p_sysex_parser_context->protocol_decoder_state != SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0 && ((midi_byte & 0x80) == 0x80)) {
      //
      if (midi_byte == 0xF0) {
         //warning receive new SYSEX packet. drop all in request buffer
         sysex_protocol_context_reset(p_sysex_parser_context);
         //go to next state
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_ID00;
         return 0;
      }
      //
      if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_EOX && (midi_byte == 0xF7)) {
         //call data parser
         // processing here
         sysex_request_parser(p_sysex_parser_context, (char*)p_sysex_parser_context->p_data_buffer);
         //
         sysex_protocol_context_reset(p_sysex_parser_context);
         //receive EOX end of SYSEX packet, ready to waiting new SYSEX packet
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
         return 0;
      }
      //
      //wait data but receive midi status command
      sysex_protocol_context_reset(p_sysex_parser_context);
      //
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
      //error return to state wait SYSEX command 0xF0
      return 0;
   }
   //data string
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_EOX && ((midi_byte & 0x80) == 0x00)) {
      //no data in discover fram, no crc control (to avoid crc calculation incompatible with tunnel mode)
      if (p_sysex_parser_context->frame_type == SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER) {
         return 0;
      }
      //
      if (p_sysex_parser_context->data_buffer_counter >= (p_sysex_parser_context->data_buffer_sz_max - 1 /* -1 for '\0' */)) {
         //too large drop bytes
         return -1;
      }
      //
      if (p_sysex_parser_context->frame_length > 0) {
         //get string data
         p_sysex_parser_context->crc= crc16(p_sysex_parser_context->crc, midi_byte);
         //
         p_sysex_parser_context->frame_length--;
         //
         p_sysex_parser_context->p_data_buffer[p_sysex_parser_context->data_buffer_counter++] = midi_byte;
         p_sysex_parser_context->p_data_buffer[p_sysex_parser_context->data_buffer_counter] = '\0';
      }
      else {
         //get crc
         if (p_sysex_parser_context->frame_crc_counter > sizeof(p_sysex_parser_context->frame_crc)){
            //
            sysex_protocol_context_reset(p_sysex_parser_context);
            //
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
            //error too large drop bytes return to state wait SYSEX command 0xF0
            return -1;
         }
         //
         p_sysex_parser_context->frame_crc[p_sysex_parser_context->frame_crc_counter++] = midi_byte;
         //
         if (p_sysex_parser_context->frame_crc_counter == sizeof(p_sysex_parser_context->frame_crc)) {
            crc16_t crc = p_sysex_parser_context->frame_crc[0] * 16384 + p_sysex_parser_context->frame_crc[1] * 128 + p_sysex_parser_context->frame_crc[2];
            //
            if (p_sysex_parser_context->crc != crc) {
               //
               sysex_protocol_context_reset(p_sysex_parser_context);
               //
               p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
               //crc error return to state wait SYSEX command 0xF0
               return -1;
            }
         }
         //
         return 0;
      }
      
      //
      return 0;
   }

   //data
   //id 00
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_ID00)  {
      if (midi_byte != 0) {
         //
         sysex_protocol_context_reset(p_sysex_parser_context);
         //
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
         //error return to state wait SYSEX command 0xF0
         return -1;
      }
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_IDMSB;
      return 0;
   }
   //id msb
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_IDMSB) {
      if (midi_byte != MIDI_SYSEX_NUODIO_ID_MSB) {
         //
         sysex_protocol_context_reset(p_sysex_parser_context);
         //
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
         //error return to state wait SYSEX command 0xF0
         return -1;
      }
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_IDLSB;
      return 0;
   }
   //id lsb
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_IDLSB) {
      if (midi_byte != MIDI_SYSEX_NUODIO_ID_LSB) {
         //
         sysex_protocol_context_reset(p_sysex_parser_context);
         //
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
         //error return to state wait SYSEX command 0xF0
         return -1;
      }
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMETYPE;
      return 0;
   }
   //frame type
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMETYPE) {
      //init crc
      __init_crc16(p_sysex_parser_context->crc);
      //
      p_sysex_parser_context->crc= crc16(p_sysex_parser_context->crc, midi_byte);
      //
      p_sysex_parser_context->frame_type = midi_byte;
      //
      switch (p_sysex_parser_context->frame_type) {
         case SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER:
         case SYSEX_PROTOCOL_DECODER_FRAMETYPE_REQUEST:
            //go to next state
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_MSB;
            break;
         case SYSEX_PROTOCOL_DECODER_FRAMETYPE_RESPONSE:
            //
            if (p_sysex_parser_context->mode == MIDI_SYSEX_NUODIO_MODE_DEVICE) {
               //in device mode, wait only request frame type
               sysex_protocol_context_reset(p_sysex_parser_context);
               //
               p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
               //error return to state wait SYSEX command 0xF0
               return -1;
            }
            //go to next state
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_MSB;
            break;
            //
         default:
            //
            sysex_protocol_context_reset(p_sysex_parser_context);
            //
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
            //error unknow opcode return to state wait SYSEX command 0xF0
            return -1;
         }
      //
      return 0;
   }
   //frame length MSB
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_MSB) {
      //
      p_sysex_parser_context->crc= crc16(p_sysex_parser_context->crc, midi_byte);
      //
      p_sysex_parser_context->frame_length = midi_byte*128;
      //
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_LSB;
      return 0;
   }

   //frame length LSB
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_FRAMELENGTH_LSB) {
      //
      p_sysex_parser_context->crc = crc16(p_sysex_parser_context->crc, midi_byte);
      //
      p_sysex_parser_context->frame_length = p_sysex_parser_context->frame_length+midi_byte;
      //
      if ((p_sysex_parser_context->frame_length < 0x02)
         || p_sysex_parser_context->frame_length >(p_sysex_parser_context->data_buffer_sz_max - 1 /* -1 for '\0' */)) {
         //invalid length
         sysex_protocol_context_reset(p_sysex_parser_context);
         //
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
         //error return to state wait SYSEX command 0xF0
         return -1;
      }
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_DSTDEVID;
      return 0;
   }

   //dst dev id
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_DSTDEVID) {
      //
      p_sysex_parser_context->crc = crc16(p_sysex_parser_context->crc, midi_byte);
      //
      p_sysex_parser_context->frame_length--;
      //
      p_sysex_parser_context->dst_dev_id = midi_byte;
      //
      if (p_sysex_parser_context->mode != MIDI_SYSEX_NUODIO_MODE_DEBUG 
          && p_sysex_parser_context->frame_type != SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER
          && p_sysex_parser_context->dst_dev_id != p_sysex_parser_context->dev_id) {
         //dst dev id control, frame not for me
         sysex_protocol_context_reset(p_sysex_parser_context);
         //
         p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
         //error return to state wait SYSEX command 0xF0
         return -1;
      }

      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_SRCDEVID;
      return 0;
   }
   //src dev id
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_SRCDEVID) {
      //
      p_sysex_parser_context->crc= crc16(p_sysex_parser_context->crc, midi_byte);
      //
      p_sysex_parser_context->frame_length--;
      //
      p_sysex_parser_context->src_dev_id = midi_byte;
      //go to next state
      switch (p_sysex_parser_context->frame_type) {
         //
         case SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER:
            //
            p_sysex_parser_context->dev_id = p_sysex_parser_context->src_dev_id + 1;
            //go to next state
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_EOX;
         break;
         //
         case SYSEX_PROTOCOL_DECODER_FRAMETYPE_REQUEST:
         case SYSEX_PROTOCOL_DECODER_FRAMETYPE_RESPONSE:
            //go to next state
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_RQSTCOUNTER;
         break;
            //
         default:
            //
            sysex_protocol_context_reset(p_sysex_parser_context);
            //
            p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_F0;
            //error unknow opcode return to state wait SYSEX command 0xF0
            return -1;
      }
      //
      return 0;
   }

   //request counter
   if (p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_RQSTCOUNTER) {
      //
      p_sysex_parser_context->crc= crc16(p_sysex_parser_context->crc, midi_byte);
      //
      p_sysex_parser_context->frame_length--;
      //
      p_sysex_parser_context->request_counter = midi_byte;
      //go to next state
      p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_WAIT_EOX;
      return 0;
   }

   //
   return 0;
}

//
int sysex_protocol_construct_frame(uint8_t frame_type, uint8_t dst_id, char* str_ascii_data, uint8_t* p_midi_sysex_buffer, int midi_sysex_buffer_sz) {
   static uint8_t request_id = 0;
   int cb = 0;
   int crc_cb = 0;

   p_midi_sysex_buffer[cb++] = 0xF0;
   p_midi_sysex_buffer[cb++] = 0x00;
   p_midi_sysex_buffer[cb++] = MIDI_SYSEX_NUODIO_ID_MSB;
   p_midi_sysex_buffer[cb++] = MIDI_SYSEX_NUODIO_ID_LSB;
   //
   crc_cb = cb;
   //
   p_midi_sysex_buffer[cb++] = frame_type;
   //
   if (frame_type == SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER) {
      p_midi_sysex_buffer[cb++] = 0x00;//frame length msb
      p_midi_sysex_buffer[cb++] = 0x02;//frame length lsb
                                       //
      p_midi_sysex_buffer[cb++] = 0x7F;//dst id broadcast 
      p_midi_sysex_buffer[cb++] = 0x00;//src id
                                       //
      p_midi_sysex_buffer[cb++] = 0x00;//crc msb (not used in discover)
      p_midi_sysex_buffer[cb++] = 0x00;//crc (not used in discover)
      p_midi_sysex_buffer[cb++] = 0x00;//crc lsb (not used in discover)
                                       //
      p_midi_sysex_buffer[cb++] = 0xF7;//0xF7 EOX
      return cb;
   }
   //
   int str_ascii_data_length = strlen(str_ascii_data);
   //frame length =dst id+src id+request id+data str length
   p_midi_sysex_buffer[cb++] = (2 + 1 + str_ascii_data_length) / 128; //frame length msb
   p_midi_sysex_buffer[cb++] = (2 + 1 + str_ascii_data_length) % 128; //frame length lsb
   //
   p_midi_sysex_buffer[cb++] = dst_id;//dst id broadcast 
   p_midi_sysex_buffer[cb++] = 0x00;//src id
   //
   p_midi_sysex_buffer[cb++] = request_id++;
   memcpy(&p_midi_sysex_buffer[cb], str_ascii_data, str_ascii_data_length);
   //
   cb += str_ascii_data_length;
   //
   crc16_t crc;
   __init_crc16(crc);
   //
   for (int i = crc_cb; i < cb; i++) {
      crc = crc16(crc, p_midi_sysex_buffer[i]);
   }
   //
   uint8_t frame_crc[3];
   frame_crc[0] = crc / 16384; //msb
   frame_crc[1] = (crc % 16384) / 128;
   frame_crc[2] = (crc % 16384) % 128;//lsb
                                      //
   p_midi_sysex_buffer[cb++] = frame_crc[0];//crc msb (not used in discover)
   p_midi_sysex_buffer[cb++] = frame_crc[1];//crc (not used in discover)
   p_midi_sysex_buffer[cb++] = frame_crc[2];//crc lsb (not used in discover)
                                            //
   p_midi_sysex_buffer[cb++] = 0xF7;//0xF7 EOX
                                    //
   return cb;
}

//
int sysex_protocol_decoder_tunnel(sysex_parser_context_t* p_sysex_parser_context, uint8_t*  midi_byte_output,uint8_t  midi_byte_input) {
   //default out=in
   *midi_byte_output = midi_byte_input;
   //
   if (p_sysex_parser_context->frame_type == SYSEX_PROTOCOL_DECODER_FRAMETYPE_DISCOVER 
      && p_sysex_parser_context->protocol_decoder_state== SYSEX_PROTOCOL_DECODER_STATE_WAIT_SRCDEVID) {
      //discover frame
      int error;
      error = sysex_protocol_decoder(p_sysex_parser_context, midi_byte_input);
      //
      if (!error && p_sysex_parser_context->protocol_decoder_state == SYSEX_PROTOCOL_DECODER_STATE_WAIT_EOX) {
         *midi_byte_output = p_sysex_parser_context->dev_id;
      }
   }else {
      //
      sysex_protocol_decoder(p_sysex_parser_context, midi_byte_input);
   }

   //
   return 0;
}

//
int sysex_protocol_context_reset(sysex_parser_context_t* p_sysex_parser_context) {
   //
   p_sysex_parser_context->protocol_decoder_state = SYSEX_PROTOCOL_DECODER_STATE_NULL;
   //
   p_sysex_parser_context->frame_type = SYSEX_PROTOCOL_DECODER_FRAMETYPE_INVALID;
   p_sysex_parser_context->frame_length = 0;
   //
   p_sysex_parser_context->src_dev_id = 0x7F;
   p_sysex_parser_context->dst_dev_id = 0x7F;
   //
   memset(p_sysex_parser_context->p_data_buffer, 0, p_sysex_parser_context->data_buffer_sz_max);
   p_sysex_parser_context->data_buffer_counter = 0;
   //
   p_sysex_parser_context->frame_crc_counter = 0;
   p_sysex_parser_context->crc_check_ok = 0;
   //
   return 0;
}

//
int sysex_protocol_context_init(sysex_parser_context_t* p_sysex_parser_context, uint8_t*  data_buffer, int data_buffer_size_max) {
   p_sysex_parser_context->p_data_buffer = data_buffer;
   p_sysex_parser_context->data_buffer_sz_max = data_buffer_size_max;
   //
   p_sysex_parser_context->mode = MIDI_SYSEX_NUODIO_MODE_DEVICE;
   //
   p_sysex_parser_context->dev_id = MIDI_SYSEX_NUODIO_INVALID_DEV_ID;
   //
   return sysex_protocol_context_reset(p_sysex_parser_context);
}

//
int sysex_protocol_context_set_device_id(sysex_parser_context_t* p_sysex_parser_context, uint8_t dev_id) {
   p_sysex_parser_context->dev_id = dev_id;
   //
   return 0;
}

