#include <stdbool.h>
#include <psp2/kernel/threadmgr.h> 
#include <psp2/kernel/modulemgr.h> 
#include <psp2/kernel/sysmem.h> 
#include <psp2/kernel/rng.h> 
#include <psp2/kernel/clib.h> 
#include <psp2/sas.h> 
#include <psp2/apputil.h> 

#include "ctrl.h"
#include "graphics.h"
#include "sound.h"
#include "psarc.h"
#include "misc.h"

/*CPU Affinity mask:
0x00000 - all user cores (auto)
0x80000 - system-reserved core
0xF0000 - ???
*/

#define CLIB_HEAP_SIZE 7 * 1024 * 1024
#define MONEY_CLEAR 100000000
#define MONEY_EX_CLEAR 2000000000
#define NORMAL_HEARTBEAT 32
#define FAST_HEARTBEAT 20

/* global vars */

SceBool g_skip_flag = false;
SceBool g_skip_flag_lock = false;

ScePVoid g_mspace;
SceUInt32 g_heartbeat_delay = NORMAL_HEARTBEAT;
SceUInt8 g_loading = 0;
SceUInt8 g_choice = 0;
SceUInt8 g_audio_sample = 11;
SceUInt8 g_global_state = 0;

/*g_global_state:
0 - intro/busy,
1 - all green
2 - swapping cards
3 - lose status
4 - dead status
5 - checking win conditions
6 - lose (not enough money)
7 - win
8 - ex_win
9 - transition
*/

SceInt32 main(SceVoid) 
{
	SceUInt8 count;
	SceInt32 ret;

	/* game init */

	ret = psarcInit();
	DEBUG_PRINT("FIOS2 init: 0x%08x\n", ret);
	psarcOpenArchive("app0:gamedata.psarc", "/gamedata");
	graphicsInit();
	ctrlInit();

	SceUID id_ctrl = sceKernelCreateThread("MHL_ctrlThread", ctrlThread, 66, 4096, 0, 0x80000, NULL);
	ret = sceKernelStartThread(id_ctrl, 0, NULL);
	DEBUG_PRINT("MHL_ctrlThread start: 0x%08x\n", ret);

	/* intro */

	graphicsPrepairIntro();
	count = 0;

	while (!g_skip_flag) {
		ret = graphicsDrawIntro(1);
		if (count > 150)
			break;
		if (ret == 2)
			count++;
	}

	count = 0;
	g_skip_flag = false;

	while (!g_skip_flag) {
		ret = graphicsDrawIntro(0);
		if (ret == 0)
			break;
	}

	g_skip_flag = false;
	graphicsFinishIntro();

	while (!g_skip_flag) {
		ret = graphicsDrawWarning(1);
		if (count > 100)
			break;
		if (ret == 2)
			count++;
	}

	count = 0;
	g_skip_flag = false;

	while (!g_skip_flag) {
		ret = graphicsDrawWarning(0);
		if (ret == 0)
			break;
	}

	g_skip_flag = false;
	graphicsFinishWarning();

	/* rule screen */

	SceUID id_loader = sceKernelCreateThread("MHL_loaderThread", graphicsPrepairMain, 80, 4096, 0, 0, NULL);
	ret = sceKernelStartThread(id_loader, 0, NULL);
	DEBUG_PRINT("MHL_loaderThread start: 0x%08x\n", ret);

	while (!g_skip_flag || g_skip_flag_lock) {
		graphicsDrawRule(1);
	}

	g_skip_flag = false;

	while (!g_skip_flag) {
		ret = graphicsDrawRule(0);
		if (ret == 0)
			break;
	}

	g_skip_flag = false;
	graphicsFinishRule();
	psarcFinish();

	/* prepare main screen */

	SceByte marisa[5];
	SceByte card[4][9];
	sceClibMemset(&marisa, 0, 5);
	sceClibMemset(&card, 1, 36);

	sceAppUtilLoadSafeMemory(&count, sizeof(SceUInt8), 0);

	SceUInt32 money;
	SceUInt8 chance_modifier, expression_modifier, remaining_cards;
	SceUInt8 card_curr[2];
	SceUInt8 card_new[2];
	SceUInt8 card_old[2];

	if (!count) {
		DEBUG_PRINT("SafeMem: New\n");
		miscGetRandomNumberForDeck(card, card_old);
		card[card_old[0]][card_old[1]] = 0;
		miscGetRandomNumberForDeck(card, card_new);
		chance_modifier = 5;
		expression_modifier = 0;
		remaining_cards = 35;
		money = 1;
		miscSave(marisa, card, card_old, card_new, &expression_modifier, &chance_modifier, &remaining_cards, &money);
	}
	else {
		DEBUG_PRINT("SafeMem: Load\n");
		miscLoad(marisa, card, card_old, card_new, &expression_modifier, &chance_modifier, &remaining_cards, &money);
		DEBUG_PRINT("SafeMem: Old card: Row: %d, Num : %d\n", card_old[0], card_old[1]);
		DEBUG_PRINT("SafeMem: New card: Row: %d, Num : %d\n", card_new[0], card_new[1]);
		DEBUG_PRINT("SafeMem: Expression modifier: %d\n", expression_modifier);
		DEBUG_PRINT("SafeMem: Chance modifier: %d\n", chance_modifier);
		DEBUG_PRINT("SafeMem: Remaining cards: %d\n", remaining_cards);
		DEBUG_PRINT("SafeMem: Money: %d\n", money);
	}

	g_global_state = 1;
	SceUID id_sound = sceKernelCreateThread("MHL_soundThread", soundMain, 64, 4096, 0, 0x80000, NULL);
	ret = sceKernelStartThread(id_sound, 0, NULL);
	DEBUG_PRINT("MHL_soundThread start: 0x%08x\n", ret);

	/* main vars */

	SceBool dead = false;
	SceBool lost = false;
	SceUInt8 anim_state, anim_action, anim_result, heartbeat_delta, flash_state;
	SceUInt32 anim_cycle;

	/* init main vars */

	anim_state = 2;
	anim_result = 3;
	anim_action = 4;
	anim_cycle = 0;
	flash_state = 0;
	count = 1;
	heartbeat_delta = 2;

	/* main loop */

	DEBUG_PRINT("Expression modifier: %d, Chance modifier: %d\n", expression_modifier, chance_modifier);
	while (1) {

		/* main events switch */

		if (g_global_state == 1)
			switch (g_choice) {
			case 1: //pass
				remaining_cards--;
				g_audio_sample = 6;
				g_choice = 0;
				count = 0;
				g_global_state = 0;
				anim_state = 0;
				anim_action = 0;
				break;
			case 2: //high
				remaining_cards--;
				g_choice = 0;
				count = 0;
				g_global_state = 0;
				anim_state = 0;
				anim_action = 1;
				if (card_new[1] > card_old[1]) {
					money *= 2;
					g_audio_sample = 10;
					anim_result = 0;
				}
				else if (card_new[1] == card_old[1]) {
					g_audio_sample = 1;
					anim_result = 1;
				}
				else {
					g_audio_sample = 4;
					anim_result = 2;
					lost = true;
				}
				break;
			case 3: //low
				remaining_cards--;
				g_choice = 0;
				count = 0;
				g_global_state = 0;
				anim_state = 0;
				anim_action = 2;
				if (card_new[1] < card_old[1]) {
					money *= 2;
					g_audio_sample = 10;
					anim_result = 0;
				}
				else if (card_new[1] == card_old[1]) {
					g_audio_sample = 1;
					anim_result = 1;
				}
				else {
					g_audio_sample = 4;
					anim_result = 2;
					lost = true;
				}
				break;
			case 4: //same
				remaining_cards--;
				g_choice = 0;
				count = 0;
				g_global_state = 0;
				anim_state = 0;
				anim_action = 3;
				if (card_new[1] == card_old[1]) {
					money *= 10;
					g_audio_sample = 10;
					anim_result = 0;
				}
				else {
					g_audio_sample = 4;
					anim_result = 2;
					lost = true;
				}
				break;
			}

		/* subevents switch */

		switch (g_global_state) {
		case 2: //animation finish events
			card_old[0] = card_curr[0];
			card_old[1] = card_curr[1];
			if (remaining_cards == 0) {
				g_global_state = 5;
			}
			else if (lost) {
				g_global_state = 3;
				marisa[4] = 1;
				g_heartbeat_delay = FAST_HEARTBEAT;
				g_audio_sample = 9;
				lost = false;
			}
			else {
				marisa[2] = miscGetRandomNumber(expression_modifier, expression_modifier + 4, marisa[2]);
				marisa[3] = miscGetRandomNumber(expression_modifier, expression_modifier + 4, marisa[3]);
				DEBUG_PRINT("New mouth: %d, New eyes: %d\n", marisa[3], marisa[2]);
				miscSave(marisa, card, card_old, card_new, &expression_modifier, &chance_modifier, &remaining_cards, &money);
				g_global_state = 1;
			}
			break;
		case 3: //lose events
			if (anim_cycle < 150)
				anim_cycle++;
			else {
				if (miscGetRandomNumber(1, chance_modifier, 10) == 1 || dead) {
					g_audio_sample = 7;
					marisa[1] = 4;
					marisa[2] = 4;
					marisa[4] = 0;
					anim_cycle = 0;
					miscInitSafeMemory();
					flash_state = 1;
				}
				else {
					g_audio_sample = 8; 
					marisa[4] = 0;
					marisa[0]++;
					marisa[1]++;
					expression_modifier += 4;
					chance_modifier--;
					anim_cycle = 0;
					g_heartbeat_delay = NORMAL_HEARTBEAT - heartbeat_delta;
					heartbeat_delta += 2;
					marisa[2] = miscGetRandomNumber(expression_modifier, expression_modifier + 4, marisa[2]);
					marisa[3] = miscGetRandomNumber(expression_modifier, expression_modifier + 4, marisa[3]);
					DEBUG_PRINT("New mouth: %d, New eyes: %d\n", marisa[3], marisa[2]);
					miscSave(marisa, card, card_old, card_new, &expression_modifier, &chance_modifier, &remaining_cards, &money);
					DEBUG_PRINT("Expression modifier: %d, Chance modifier: %d\n", expression_modifier, chance_modifier);
					g_global_state = 1;
				}
			}
			break;
		case 5: //win condition check
			if (money < MONEY_CLEAR) {
				g_global_state = 3;
				marisa[4] = 1;
				g_audio_sample = 9;
				miscInitSafeMemory();
				dead = true;
			}
			else if (money >= MONEY_CLEAR && money < MONEY_EX_CLEAR) {
				g_global_state = 7;
			}
			else {
				g_global_state = 8;
			}
			break;
		}

		/* get new card after main event */

		if (!count) {
			if (remaining_cards == 0) {
				card_curr[0] = card_new[0];
				card_curr[1] = card_new[1];
				card[card_curr[0]][card_curr[1]] = 0;
				count = 1;
			}
			else {
				card_curr[0] = card_new[0];
				card_curr[1] = card_new[1];
				card[card_curr[0]][card_curr[1]] = 0;
				miscGetRandomNumberForDeck(card, card_new);
				DEBUG_PRINT("Got new card: row: %d, Num: %d\n", card_new[0], card_new[1]);
				count = 1;
			}
		}

		/* animation delay */

		if (anim_state == 0) {
			if (anim_cycle < 30)
				anim_cycle++;
			else {
				anim_state = 1;
				anim_cycle = 0;
			}
		}

		/* drawing */

		graphicsDrawStart();
		graphicsDrawMain(card_new[0], card_old[0], card_old[1] + 1, remaining_cards, money);
		graphicsDrawMarisa(marisa);
		graphicsDrawDeck(card);
		if (graphicsDrawCardAnimation(card_curr[0], card_curr[1] + 1, anim_state, anim_action, anim_result)) {
			g_global_state = 2;
			anim_state = 2;
			anim_action = 4;
			anim_result = 3;
		}
		switch (flash_state) {
		case 1:
			if (graphicsDrawFlashUp())
				flash_state = 2;
			break;
		case 2:
			if (graphicsDrawFlashDown()) {
				flash_state = 0;
				g_global_state = 4;
			}
			break;
		}
		switch (g_global_state) {
		case 3:
			if (dead)
				graphicsDrawPreDeadMessage();
			break;
		case 4:
			graphicsDrawDeadMessage();
			break;
		case 7:
			if (graphicsDrawClear(money)) {
				miscInitSafeMemory();
				g_audio_sample = 0;
			}
			break;
		case 8:
			if (graphicsDrawExClear(money)) {
				miscInitSafeMemory();
				g_audio_sample = 2;
			}
			break;
		}
		graphicsDrawFinish();
	}

	return 0;
}

void _start(unsigned int args, void *argp)
{
	SceInt32 ret;

	/* load libc module (required for SceSystemGesture) */

	ret = sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);
	DEBUG_PRINT("Libc module: 0x%08x\n", ret);

	/* create clib mspace */

	ScePVoid clibm_base;
	SceUID clib_heap = sceKernelAllocMemBlock("MHL_ClibHeap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, CLIB_HEAP_SIZE, NULL);
	sceKernelGetMemBlockBase(clib_heap, &clibm_base);
	g_mspace = sceClibMspaceCreate(clibm_base, CLIB_HEAP_SIZE);

	/* init apputil */

	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	sceClibMemset(&init_param, 0, sizeof(SceAppUtilInitParam));
	sceClibMemset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);

	main();
}

