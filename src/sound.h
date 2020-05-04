#pragma once

typedef struct VoiceNum {
	SceByte clear;
	SceByte exClear;
	SceByte win;
	SceByte draw;
	SceByte lose;
	SceByte pass;
	SceByte revolverPrepair;
	SceByte revolverFire;
	SceByte revolverSafe;
	SceByte heartbeat;
	SceByte next;
} VoiceNum;

SceVoid soundInitialize(SceVoid);
SceInt32 soundMain(SceSize argc, ScePVoid argv);