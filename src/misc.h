#pragma once

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