#include <psp2/types.h>
#include <psp2/apputil.h> 
#include <psp2/kernel/rng.h> 
#include <psp2/kernel/clib.h> 

#include "misc.h"

static const SceInt8 s_save_flag = 1;

typedef struct ScePhotoExportParam {
	SceUInt32				version;
	const SceChar8			*photoTitle;
	const SceChar8			*gameTitle;
	const SceChar8			*gameComment;
	SceChar8				reserved[32];
} ScePhotoExportParam;

SceInt32 scePhotoExportFromFile(
	const SceChar8				*photodataPath,
	const ScePhotoExportParam	*param,
	void						*workMemory,
	void						*cancelFunc,
	void						*userdata,
	SceChar8					*exportedPath,
	SceInt32					exportedPathLength
);

SceUInt8 miscGetRandomNumber(SceUInt8 min, SceUInt8 max, SceByte check_val)
{
repeat:;

	SceUInt8 result, temp_result;
	sceKernelGetRandomNumber(&temp_result, 1);
	result = temp_result % max;
	if (result == check_val || result < min)
		goto repeat;

	return result;
}

SceVoid miscGetRandomNumberForDeck(SceByte card[4][9], SceUInt8* result)
{
	SceUInt8 temp_result_i, temp_result_j;

repeat:

	sceKernelGetRandomNumber(&temp_result_i, 1);
	sceKernelGetRandomNumber(&temp_result_j, 1);
	result[0] = temp_result_i % 4;
	result[1] = temp_result_j % 9;
	if (!card[result[0]][result[1]])
		goto repeat;
}

SceVoid miscLoad(SceByte* marisa, SceByte card[4][9], SceUInt8* card_old, SceUInt8* card_new, 
	SceUInt8* expression_modifier, SceUInt8* chance_modifier, SceUInt8* remaining_cards, SceUInt32* money)
{
	sceAppUtilLoadSafeMemory(card, 36, 1);
	sceAppUtilLoadSafeMemory(marisa, 5, 37);
	sceAppUtilLoadSafeMemory(card_old, 2, 42);
	sceAppUtilLoadSafeMemory(card_new, 2, 44);
	sceAppUtilLoadSafeMemory(expression_modifier, 1, 46);
	sceAppUtilLoadSafeMemory(chance_modifier, 1, 47);
	sceAppUtilLoadSafeMemory(remaining_cards, 1, 48);
	sceAppUtilLoadSafeMemory(money, 4, 49);
}

SceVoid miscSave(SceByte* marisa, SceByte card[4][9], SceUInt8* card_old, SceUInt8* card_new, 
	SceUInt8* expression_modifier, SceUInt8* chance_modifier, SceUInt8* remaining_cards, SceUInt32* money)
{
	sceAppUtilSaveSafeMemory((ScePVoid)&s_save_flag, 1, 0);
	sceAppUtilSaveSafeMemory(card, 36, 1);
	sceAppUtilSaveSafeMemory(marisa, 5, 37);
	sceAppUtilSaveSafeMemory(card_old, 2, 42);
	sceAppUtilSaveSafeMemory(card_new, 2, 44);
	sceAppUtilSaveSafeMemory(expression_modifier, 1, 46);
	sceAppUtilSaveSafeMemory(chance_modifier, 1, 47);
	sceAppUtilSaveSafeMemory(remaining_cards, 1, 48);
	sceAppUtilSaveSafeMemory(money, 4, 49);
}

SceVoid miscInitSafeMemory(SceVoid)
{
	SceChar8 blank[43];
	sceClibMemset(blank, 0, 43);
	sceAppUtilSaveSafeMemory(blank, 43, 0);
}

