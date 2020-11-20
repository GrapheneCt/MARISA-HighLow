#include <stdbool.h>
#include <vitaSAS.h>
#include <psp2/kernel/clib.h> 
#include <psp2/kernel/iofilemgr.h> 
#include <psp2/types.h> 
#include <psp2/audioout.h>  
#include <psp2/kernel/threadmgr.h> 

#include "sound.h"
#include "misc.h"

extern SceUInt8 g_heartbeat_delay;
extern SceUInt8 g_loading;
extern SceUInt8 g_audio_sample;
extern SceUInt8 g_global_state;

SceVoid soundInitialize(SceVoid)
{
	vitaSAS_init(0);

	VitaSASSystemParam initParam;
	initParam.outputPort = SCE_AUDIO_OUT_PORT_TYPE_MAIN;
	initParam.samplingRate = 48000;
	initParam.numGrain = 256;
	initParam.thPriority = 64;
	initParam.thStackSize = 128 * 1024;
	initParam.thCpu = 0;
	initParam.isSubSystem = SCE_FALSE;
	initParam.subSystemNum = VITASAS_NO_SUBSYSTEM;

	vitaSAS_create_system_with_config("numGrains=256 numVoices=12 numReverbs=0", &initParam);

	SceChar8 path[23];

	vitaSASVoiceParam voiceParam;
	voiceParam.loop = 0;
	voiceParam.loopSize = 0;
	voiceParam.pitch = 4096;
	voiceParam.volLDry = 4096;
	voiceParam.volRDry = 4096;
	voiceParam.volLWet = 0;
	voiceParam.volRWet = 0;
	voiceParam.adsr1 = 0;
	voiceParam.adsr2 = 0;

	for (int i = 0; i < 11; i++) {
		sceClibSnprintf(path, 23, "/gamedata/sound/%d.vag", i);
		DEBUG_PRINT("loading: %s\n", path);
		vitaSAS_set_voice_VAG(i, vitaSAS_load_audio_VAG(path, 1), &voiceParam);
		g_loading++;
	}
}

SceInt32 soundMain(SceSize argc, ScePVoid argv)
{
	SceInt8 subvoice_id = -1;
	SceUInt32 heartbeat_counter = 0;

	while (1) {

		if (heartbeat_counter == 0 && g_global_state != 4 && g_global_state != 7 && g_global_state != 8)
			vitaSAS_set_key_on(3);

		if (subvoice_id == -1 && g_audio_sample < 11) {
			vitaSAS_set_key_on(g_audio_sample);
			subvoice_id = g_audio_sample;
			g_audio_sample = 11;
		}

		if (subvoice_id > -1)
			if (vitaSAS_get_end_state(subvoice_id)) {
				vitaSAS_set_key_off(subvoice_id);
				subvoice_id = -1;
			}

		if (heartbeat_counter < g_heartbeat_delay && g_global_state != 4 && g_global_state != 7 && g_global_state != 8)
			heartbeat_counter++;
		else {
			heartbeat_counter = 0;
			vitaSAS_set_key_off(3);
		}

		sceKernelDelayThread(30000);
	}

	return 0;
}

