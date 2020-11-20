#pragma once

SceVoid graphicsInit(SceVoid);

SceVoid graphicsDrawStart(SceVoid);
SceVoid graphicsDrawFinish(SceVoid);

SceVoid graphicsPrepairIntro(SceVoid);
SceInt8 graphicsDrawIntro(SceInt8 status);
SceVoid graphicsFinishIntro(SceVoid);
SceInt8 graphicsDrawWarning(SceInt8 status);
SceVoid graphicsFinishWarning(SceVoid);
SceInt8 graphicsDrawRule(SceInt8 status);
SceVoid graphicsFinishRule(SceVoid);
SceInt32 graphicsPrepairMain(SceSize argc, ScePVoid argv);
SceVoid graphicsDrawMain(SceInt8 color_unk, SceInt8 color_curr, SceByte number_curr, SceUInt8 cards, SceUInt32 money);
SceUInt8 graphicsDrawCardAnimation(SceUInt8 color, SceByte number, SceUInt8 state, SceUInt8 action, SceUInt8 result);
SceVoid graphicsDrawMarisa(SceByte* marisa);
SceVoid graphicsDrawDeck(SceByte card[4][9]);
SceVoid graphicsDrawDeadMessage(SceVoid);
SceUInt8 graphicsDrawFlashUp(SceVoid);
SceUInt8 graphicsDrawFlashDown(SceVoid);
SceVoid graphicsDrawPreDeadMessage(SceVoid);
SceUInt8 graphicsDrawClear(SceUInt32 money);
SceUInt8 graphicsDrawExClear(SceUInt32 money);