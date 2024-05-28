/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2018 <lepton.phlb@gmail.com>.
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
#include "kernel/core/ioctl_lcd.h"
#include "kernel/core/ioctl_fb.h"
//
#include "kernel/dev/dev_mem/dev_mem.h"
//
#include "lib/libc/unistd.h"
#include "lib/libc/stdio/stdio.h"
//
#include "kernel/dev/bsp/compact_series/dev_compact_series/ioctl_rotary_encoder.h"
#include "kernel/dev/bsp/compact_series/dev_compact_series/ioctl_compact_series_gpio.h"

//
#include "bin/nuodio/compact-series/nu/nu_effect_parameters.h"
#include "bin/nuodio/compact-series/nu/preset/nu_preset.h"
#include "bin/nuodio/compact-series/nu/nu.h"
//
#include "lib/libc/misc/crc.h"
//
#include "bin/nuodio/midi/midi_sysex.h"



/*===========================================
Global Declaration
=============================================*/
#define MIDI_STATE_MACHINE_WAIT_STATUS  0x01
#define MIDI_STATE_MACHINE_WAIT_DATA1   0x02
#define MIDI_STATE_MACHINE_WAIT_DATA2   0x03
#define MIDI_STATE_MACHINE_WAIT_ENDSYSEX 0x04



/*===========================================
Implementation
=============================================*/


/*-------------------------------------------
| Name: midid_main
| Description:
| Parameters:
| Return Type:
| Comments:
| See:
---------------------------------------------*/
int midid_main(int argc,char* argv[]){
   int fd_ttys_midi_in=-1;
   int fd_ttys_midi_out=-1;
   uint8_t midi_packet[256];
   uint8_t data_buffer[256];
   sysex_parser_context_t sysex_parser_context;
   shared_memory_midi_t shared_memory_midi;
   int cb;
   int i;
   int fd_mem;
   //
   int midi_state_machine;
   int midi_sysex_data_length=0;
   //
   sysex_protocol_context_init(&sysex_parser_context, data_buffer, sizeof(data_buffer));
   //
   for(i=1;i<argc;i++){
      //
      if (!strcmp(argv[i], "dev-in")) {
         i++;
         if(!argv[i]){
            printf("midid: error: invalid dev-in argument\r\n");
            return -1;
         }
            
         fd_ttys_midi_in= open(argv[i],O_RDONLY,0);
         if(fd_ttys_midi_in<0){
            printf("midid: error: cannot open %s\r\n",argv[i]);
            return -1;
         }
        
      }else if (!strcmp(argv[i], "dev-out")) {
         i++;
         if(!argv[i]){
            printf("midid: error: invalid dev-out argument\r\n");
            return -1;
         }
            
         fd_ttys_midi_out= open(argv[i],O_WRONLY,0);
         if(fd_ttys_midi_out<0){
            printf("midid: error: cannot open %s\r\n",argv[i]);
            return -1;
         }
        
      }
   }
       
   //
   if((fd_mem =  open("/dev/mem",O_RDWR,0))<0){
      fprintf(stderr,"warning: cannot open /dev/mem\r\n");
   }
   
   //
   while((cb=read(fd_ttys_midi_in,midi_packet,sizeof(midi_packet)))>0){
      for(i=0;i<cb;i++){
         fprintf(stderr,"0x%02x:",midi_packet[i]);
         //
         uint8_t  midi_byte_output;
         uint8_t  midi_byte_input;      
         //
         midi_byte_input = midi_packet[i];
         //
         sysex_protocol_decoder_tunnel(&sysex_parser_context, &midi_byte_output, midi_byte_input);
         //
         write(fd_ttys_midi_out,(const void *)&midi_byte_output,1);
         
         //status part
         if(midi_packet[i]&0x80){
            //
            shared_memory_midi.status=midi_packet[i]&0xF0;
            shared_memory_midi.channel=midi_packet[i]&0x0F;
            //
            switch(midi_packet[i]){
               //wait 2 data
               case 0x80:
               case 0x90:
               case 0xA0:
               case 0xB0:
               case 0xE0:
                  shared_memory_midi.length=2;
               break;
                    
               //wait 1 data
               case 0xC0:
               case 0xD0:
                  shared_memory_midi.length=1;
               break;
               
               //system
               case 0xF0: //System Exclusive Begin
                shared_memory_midi.length=0;
               break;
               
               case 0xF1: //MTC Q frame
                shared_memory_midi.length=1;
               break;
               //
               case 0xF2: //Song position
               shared_memory_midi.length=2;
               //(uint16_t)(data1 & 0x7F) << 7) | (data2 & 0x7F);
               break;
               //
               case 0xF3: //Song select
               shared_memory_midi.length=1;
               break;
               //
               case 0xF6: //Tune request
               shared_memory_midi.length=0;
               break;
               
               //endof system exclusive 
               case 0xF7:
               shared_memory_midi.length=midi_sysex_data_length;
               break;
               
               //system realtime 
               case 0xF8: //Timing Clock
               case 0xFA: //Start
               case 0xFB: //Continue
               case 0xFC: //Stop
               case 0xFE: //Active Sensing
               case 0xFF: //Reset
               shared_memory_midi.length=0;
               break;     
            }
            //
            if(shared_memory_midi.length>0){
               midi_state_machine = MIDI_STATE_MACHINE_WAIT_DATA1;
            }
            if(shared_memory_midi.length==0){
               midi_state_machine = MIDI_STATE_MACHINE_WAIT_STATUS;
               //do operation 
            }
            if(shared_memory_midi.length==-1){
               midi_sysex_data_length=0;
               midi_state_machine = MIDI_STATE_MACHINE_WAIT_ENDSYSEX;
            }
            //
            continue;
         }
         
         //data part
         //runnig status?
         if(midi_state_machine==MIDI_STATE_MACHINE_WAIT_STATUS){
            //wait status but received data, it's running status
            //set midi_state_machine in MIDI_STATE_MACHINE_WAIT_DATA1
            //keep previous shared_memory_midi values.
             midi_state_machine = MIDI_STATE_MACHINE_WAIT_DATA1;
         }
         
         //
         switch(midi_state_machine){
            //
            case MIDI_STATE_MACHINE_WAIT_STATUS://not possible at this point. see runing status.
            break;
            //
            case MIDI_STATE_MACHINE_WAIT_DATA1:
               if(shared_memory_midi.length==1){
                  midi_state_machine = MIDI_STATE_MACHINE_WAIT_STATUS;
                  shared_memory_midi.data1 = midi_packet[i];
                  //do operation
                  lseek(fd_mem,SHARED_MEM_ADDRESS_MIDI,SEEK_SET);
                  write(fd_mem,&shared_memory_midi,sizeof(shared_memory_midi));
               }
               //
               if(shared_memory_midi.length==2){
                  shared_memory_midi.data1 = midi_packet[i];
                  midi_state_machine = MIDI_STATE_MACHINE_WAIT_DATA2;
               }
            break;
            //
            case MIDI_STATE_MACHINE_WAIT_DATA2:
               if(shared_memory_midi.length==2){
                  midi_state_machine = MIDI_STATE_MACHINE_WAIT_STATUS;
                  shared_memory_midi.data2 = midi_packet[i];
                  //do operation
                  lseek(fd_mem,SHARED_MEM_ADDRESS_MIDI,SEEK_SET);
                  write(fd_mem,&shared_memory_midi,sizeof(shared_memory_midi));
               }
            break;
            //
            case MIDI_STATE_MACHINE_WAIT_ENDSYSEX:
               //to do: put = midi_packet[i] in sysex data buffer
               midi_sysex_data_length++;
            break;
         }
         
      }
   }
   
   return 0;
}
