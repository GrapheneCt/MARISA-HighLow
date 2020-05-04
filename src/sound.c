#include <stdbool.h>
#include <vitaSAS.h>
#include <psp2/kernel/clib.h> 
#include <psp2/io/fcntl.h> 
#include <psp2/types.h> 
#include <psp2/sas.h> 
#include <psp2/audioout.h>  
#include <psp2/kernel/threadmgr.h> 

#include "sound.h"
#include "misc.h"

extern SceUInt8 g_heartbeat_delay;
extern SceUInt8 g_loading;
extern SceUInt8 g_audio_sample;
extern ScePVoid g_mspace;
extern SceUInt8 g_global_state;

SceVoid soundInitialize(SceVoid)
{
	vitaSAS_pass_mspace(g_mspace);
	vitaSAS_init(SCE_AUDIO_OUT_PORT_TYPE_MAIN, 48000, 256, 64, 128 * 1024, 0, 0);
	SceChar8 path[23];

	for (int i = 0; i < 11; i++) {
		sceClibSnprintf(path, 23, "/gamedata/sound/%d.vag", i);
		DEBUG_PRINT("loading: %s\n", path);
		vitaSAS_set_voice_VAG(i, vitaSAS_load_audio_VAG(path, 1), SCE_SAS_LOOP_DISABLE, 4096, 4096, 4096, 0, 0, 0, 0);
		g_loading++;
	}

	//vitaSAS_set_voice_VAG(0, audioBlock[3], SCE_SAS_LOOP_DISABLE, 4096, 4096, 4096, 0, 0, 0, 0);
}

SceInt32 soundMain(SceSize argc, ScePVoid argv)
{
	SceInt8 subvoice_id = -1;
	SceUInt32 heartbeat_counter = 0;

	while (1) {


		if (heartbeat_counter == 0 && g_global_state != 4 && g_global_state != 7 && g_global_state != 8)
			sceSasSetKeyOn(3);

		if (subvoice_id == -1) {
			switch (g_audio_sample) {
			case 0:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 0;
				break;
			case 1:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 1;
				break;
			case 2:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 2;
				break;
			case 3:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 3;
				break;
			case 4:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 4;
				break;
			case 5:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 5;
				break;
			case 6:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 6;
				break;
			case 7:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 7;
				break;
			case 8:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 8;
				break;
			case 9:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 9;
				break;
			case 10:
				sceSasSetKeyOn(g_audio_sample);
				g_audio_sample = 11;
				subvoice_id = 10;
				break;
			}
		}

		if (subvoice_id > -1)
			if (sceSasGetEndState(subvoice_id)) {
				sceSasSetKeyOff(subvoice_id);
				subvoice_id = -1;
			}

		if (heartbeat_counter < g_heartbeat_delay && g_global_state != 4 && g_global_state != 7 && g_global_state != 8)
			heartbeat_counter++;
		else {
			heartbeat_counter = 0;
			sceSasSetKeyOff(3);
		}

		sceKernelDelayThread(30000);
	}

	return 0;
}

