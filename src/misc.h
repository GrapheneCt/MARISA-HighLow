#pragma once

typedef enum GlobalGameState {
	GS_INTRO_BUSY,
	GS_ALL_GREEN,
	GS_SWAPPING_CARDS,
	GS_LOSE,
	GS_DEAD,
	GS_CHECKING_WIN,
	GS_RESULT_LOSE,
	GS_RESULT_WIN,
	GS_RESULT_EX_WIN,
	GS_TRANSITION
} GlobalGameState;

//#define DEBUG_ON

#ifdef DEBUG_ON
#  define DEBUG_PRINT(...) sceClibPrintf(__VA_ARGS__)
#else
#  define DEBUG_PRINT(...)
#endif

SceUInt8 miscGetRandomNumber(SceUInt8 min, SceUInt8 max, SceByte check_val);
SceVoid miscGetRandomNumberForDeck(SceByte card[4][9], SceUInt8* result);
SceVoid miscInitSafeMemory(SceVoid);

SceVoid miscLoad(SceByte* marisa, SceByte card[4][9], SceUInt8* card_old, SceUInt8* card_new,
	SceUInt8* expression_modifier, SceUInt8* chance_modifier, SceUInt8* remaining_cards, SceUInt32* money);

SceVoid miscSave(SceByte* marisa, SceByte card[4][9], SceUInt8* card_old, SceUInt8* card_new,
	SceUInt8* expression_modifier, SceUInt8* chance_modifier, SceUInt8* remaining_cards, SceUInt32* money);