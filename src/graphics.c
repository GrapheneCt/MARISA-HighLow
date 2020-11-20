#include <stdbool.h>
#include <vita2d_sys.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/clib.h> 
#include <psp2/kernel/threadmgr.h> 

#include "graphics.h"
#include "sound.h"
#include "psarc.h"
#include "misc.h"

#define MARISA_X 630
#define MARISA_Y 44
#define CARD_L_X 40
#define CARD_R_X 270
#define CARD_Y 290
#define CONCLUSION_TX_X 460
#define CONCLUSION_TX_Y 100
#define RULE "\n\nYour task is to predict whether the number of\nthe next card to be flipped is higher or lower\nthan that of the card currently displayed.\
\n\n* If prediction was correct, your money will be\ndoubled.\n* If prediction was incorrect, you will have to\nplay Russian roulette.\n* If numbers are the \
same, nothing will happen.\n\nWin condition: obtain 100 million yen before\nyou run out of cards."

extern SceBool g_skip_flag;
extern SceBool g_skip_flag_lock;
extern SceUInt8 g_loading;

/* Common textures */

static vita2d_texture* s_tex_dolce;
static vita2d_texture* s_tex_warning;
static vita2d_texture* s_tex_rule;
static vita2d_texture* s_tex_spotlight;
static vita2d_texture* s_tex_deck;
static vita2d_texture* s_tex_card_backside;
static vita2d_texture* s_tex_clear;
static vita2d_texture* s_tex_ex_clear;
static vita2d_texture* s_tex_revolver;
static vita2d_texture* s_tex_speech_balloon;
static vita2d_texture* s_tex_message_window;

/* Marisa textures */

static vita2d_texture* s_tex_body[5];
static vita2d_texture* s_tex_eye[16];
static vita2d_texture* s_tex_head[5];
static vita2d_texture* s_tex_mouth[16];

/* Card animation */

static SceUInt16 s_x_anim = CARD_L_X;

/* Fonts */

static vita2d_pvf* s_pvf_card_font;
static vita2d_pvf* s_pvf_latin_font;

static SceUInt16 s_common_alpha = 0;
static SceUInt16 s_flash_alpha = 0;
static SceBool s_state_end = false;

SceVoid graphicsDrawStart(SceVoid)
{
	vita2d_start_drawing();
	vita2d_clear_screen();
}

SceVoid graphicsDrawFinish(SceVoid)
{
	vita2d_end_drawing();
	vita2d_end_shfb();
}

SceInt8 graphicsFader(SceInt8 status)
{
	int ret = 1;

	switch (status) {
	case 1:
		if (s_common_alpha != 255)
			s_common_alpha += 5;
		else
			ret = 2;
		break;
	case 0:
		if (s_common_alpha != 0)
			s_common_alpha -= 5;
		else
			ret = 0;
		break;
	}

	return ret;
}

SceVoid graphicsInit(SceVoid)
{
	vita2d_init();
	vita2d_set_vblank_wait(0);
	vita2d_set_clear_color(RGBA8(39, 39, 39, 255));
	graphicsDrawStart();
	graphicsDrawFinish();
}

SceVoid graphicsPrepairIntro(SceVoid)
{
	s_tex_dolce = vita2d_load_GXT_file("/gamedata/tex/intro.gxt", 0, 1);
	s_tex_warning = vita2d_load_additional_GXT(s_tex_dolce, 1);
	s_tex_rule = vita2d_load_additional_GXT(s_tex_dolce, 2);

	s_pvf_latin_font = vita2d_load_custom_pvf("app0:font/07LogoTypeGothic-CondenseLatin.ttf", 16.0f, 16.0f);
}

SceInt8 graphicsDrawIntro(SceInt8 status)
{
	graphicsDrawStart();
	vita2d_draw_texture_tint(s_tex_dolce, 200, 20, RGBA8(255, 255, 255, s_common_alpha));
	vita2d_pvf_draw_text(s_pvf_latin_font, 86, 450, RGBA8(255, 255, 255, s_common_alpha), 1, "This game uses an autosave feature. Think before you act.");
	graphicsDrawFinish();

	return graphicsFader(status);
}

SceVoid graphicsFinishIntro(SceVoid)
{
	s_common_alpha = 0;
}

SceInt8 graphicsDrawWarning(SceInt8 status)
{
	graphicsDrawStart();
	vita2d_draw_texture_tint(s_tex_warning, 338, 104, RGBA8(255, 255, 255, s_common_alpha));
	graphicsDrawFinish();

	return graphicsFader(status);
}

SceVoid graphicsFinishWarning(SceVoid)
{
	s_common_alpha = 0;
}

SceInt8 graphicsDrawRule(SceInt8 status)
{
	if (status)
		s_common_alpha = 255;

	graphicsDrawStart();
	vita2d_draw_texture_tint(s_tex_rule, 30, 20, RGBA8(255, 255, 255, s_common_alpha));
	vita2d_pvf_draw_text(s_pvf_latin_font, 480, 40, RGBA8(255, 255, 255, s_common_alpha), 1, "Rules of the Game");
	vita2d_pvf_draw_text(s_pvf_latin_font, 287, 40, RGBA8(255, 255, 255, s_common_alpha), 1, RULE);
	vita2d_draw_rectangle(0, 504, g_loading * 24, 40, RGBA8(255, 255, 255, s_common_alpha));
	graphicsDrawFinish();

	if (status)
		return 0;
	else
		return graphicsFader(status);
}

SceVoid graphicsFinishRule(SceVoid)
{
	vita2d_wait_rendering_done();
	vita2d_free_additional_GXT(s_tex_rule);
	vita2d_free_additional_GXT(s_tex_warning);
	vita2d_free_texture(s_tex_dolce);
	s_common_alpha = 0;
}

SceInt32 graphicsPrepairMain(SceSize argc, ScePVoid argv)
{
	g_skip_flag_lock = true;

	/* Load common textures */

	s_tex_spotlight = vita2d_load_GXT_file("/gamedata/tex/common.gxt", 0, 1);
	g_loading++;
	s_tex_deck = vita2d_load_additional_GXT(s_tex_spotlight, 1);
	g_loading++;
	s_tex_card_backside = vita2d_load_additional_GXT(s_tex_spotlight, 2);
	g_loading++;
	s_tex_revolver = vita2d_load_additional_GXT(s_tex_spotlight, 3);
	g_loading++;
	s_tex_speech_balloon = vita2d_load_additional_GXT(s_tex_spotlight, 4);
	g_loading++;
	s_tex_message_window = vita2d_load_additional_GXT(s_tex_spotlight, 5);
	g_loading++;
	s_tex_clear = vita2d_load_additional_GXT(s_tex_spotlight, 6);
	g_loading++;
	s_tex_ex_clear = vita2d_load_additional_GXT(s_tex_spotlight, 7);
	g_loading++;

	/* Load Marisa textures */

	s_tex_body[0] = vita2d_load_GXT_file("/gamedata/tex/marisa/body.gxt", 0, 1);
	g_loading++;

	for (SceInt8 i = 1; i < 5; i++) {
		s_tex_body[i] = vita2d_load_additional_GXT(s_tex_body[0], i);
		g_loading++;
	}

	s_tex_head[0] = vita2d_load_GXT_file("/gamedata/tex/marisa/head.gxt", 0, 1);
	g_loading++;

	for (SceInt8 i = 1; i < 5; i++) {
		s_tex_head[i] = vita2d_load_additional_GXT(s_tex_head[0], i);
		g_loading++;
	}

	s_tex_eye[0] = vita2d_load_GXT_file("/gamedata/tex/marisa/eye.gxt", 0, 1);
	g_loading++;

	for (SceInt8 i = 1; i < 16; i++) {
		s_tex_eye[i] = vita2d_load_additional_GXT(s_tex_eye[0], i);
		g_loading++;
	}

	s_tex_mouth[0] = vita2d_load_GXT_file("/gamedata/tex/marisa/mouth.gxt", 0, 1);
	g_loading++;

	for (SceInt8 i = 1; i < 16; i++) {
		s_tex_mouth[i] = vita2d_load_additional_GXT(s_tex_mouth[0], i);
		g_loading++;
	}

	s_pvf_card_font = vita2d_load_custom_pvf("app0:font/07LogoTypeGothic-CondenseLatin.ttf", 86.0f, 87.0f);
	g_loading++;
	soundInitialize();

	g_skip_flag_lock = false;

	return sceKernelExitDeleteThread(0);
}

SceVoid graphicsDrawMain(SceInt8 color_unk, SceInt8 color_curr, SceByte number_curr, SceUInt8 cards, SceUInt32 money)
{
	vita2d_draw_texture(s_tex_spotlight, 520, 0);
	vita2d_draw_rectangle(CARD_R_X, CARD_Y, 133, 200, RGBA8(255, 255, 255, 255));
	if (cards != 0) {
		vita2d_draw_rectangle(CARD_L_X, CARD_Y, 133, 200, RGBA8(255, 255, 255, 255));
		switch (color_unk) {
		case 0:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(218, 43, 56, 255));
			break;
		case 1:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(12, 52, 217, 255));
			break;
		case 2:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(49, 122, 33, 255));
			break;
		case 3:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(127, 26, 132, 255));
			break;
		}
		vita2d_draw_texture(s_tex_card_backside, CARD_L_X, CARD_Y);
	}
	switch (color_curr) {
	case 0:
		vita2d_draw_rectangle(CARD_R_X + 5, CARD_Y + 5, 123, 190, RGBA8(218, 43, 56, 255));
		break;
	case 1:
		vita2d_draw_rectangle(CARD_R_X + 5, CARD_Y + 5, 123, 190, RGBA8(12, 52, 217, 255));
		break;
	case 2:
		vita2d_draw_rectangle(CARD_R_X + 5, CARD_Y + 5, 123, 190, RGBA8(49, 122, 33, 255));
		break;
	case 3:
		vita2d_draw_rectangle(CARD_R_X + 5, CARD_Y + 5, 123, 190, RGBA8(127, 26, 132, 255));
		break;
	}
	vita2d_pvf_draw_textf(s_pvf_card_font, CARD_R_X + 20, CARD_Y + 155, RGBA8(255, 255, 255, 255), 1, "%u", number_curr);
	vita2d_pvf_draw_textf(s_pvf_latin_font, 20, 530, RGBA8(255, 255, 255, 255), 1.2f, "Money: %u yen", money);
	vita2d_pvf_draw_textf(s_pvf_latin_font, CONCLUSION_TX_X - 30, CONCLUSION_TX_Y - 50, RGBA8(255, 255, 255, 255), 1.2f, "Cards: %u", cards);
}

SceUInt8 graphicsDrawCardAnimation(SceUInt8 color, SceByte number, SceUInt8 state, SceUInt8 action, SceUInt8 result)
{
	SceInt8 ret = 0;

	switch (state) {
	case 0:
		s_common_alpha = 255;
		vita2d_draw_rectangle(CARD_L_X, CARD_Y, 133, 200, RGBA8(255, 255, 255, 255));
		vita2d_draw_texture_tint(s_tex_speech_balloon, MARISA_X + 75, MARISA_Y + 220, RGBA8(255, 255, 255, s_common_alpha));
		switch (action) {
		case 0:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Pass!");
			break;
		case 1:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "High!");
			break;
		case 2:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Low!");
			break;
		case 3:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Same!");
			break;
		}
		switch (result) {
		case 0:
			vita2d_pvf_draw_text(s_pvf_latin_font, CONCLUSION_TX_X, CONCLUSION_TX_Y, RGBA8(197, 49, 44, s_common_alpha), 1.2f, "WIN !");
			break;
		case 1:
			vita2d_pvf_draw_text(s_pvf_latin_font, CONCLUSION_TX_X, CONCLUSION_TX_Y, RGBA8(43, 91, 36, s_common_alpha), 1.2f, "DRAW");
			break;
		case 2:
			vita2d_pvf_draw_text(s_pvf_latin_font, CONCLUSION_TX_X, CONCLUSION_TX_Y, RGBA8(13, 47, 190, s_common_alpha), 1.2f, "LOSE...");
			break;
		}
		switch (color) {
		case 0:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(218, 43, 56, 255));
			break;
		case 1:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(12, 52, 217, 255));
			break;
		case 2:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(49, 122, 33, 255));
			break;
		case 3:
			vita2d_draw_rectangle(CARD_L_X + 5, CARD_Y + 5, 123, 190, RGBA8(127, 26, 132, 255));
			break;
		}
		vita2d_pvf_draw_textf(s_pvf_card_font, CARD_L_X + 20, CARD_Y + 155, RGBA8(255, 255, 255, 255), 1, "%u", number);
		break;
	case 1:
		vita2d_draw_rectangle(s_x_anim, CARD_Y, 133, 200, RGBA8(255, 255, 255, 255));
		vita2d_draw_texture_tint(s_tex_speech_balloon, MARISA_X + 75, MARISA_Y + 220, RGBA8(255, 255, 255, s_common_alpha));
		switch (action) {
		case 0:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Pass!");
			if (s_common_alpha != 0)
				s_common_alpha -= 15;
			break;
		case 1:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "High!");
			break;
		case 2:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Low!");
			break;
		case 3:
			vita2d_pvf_draw_text(s_pvf_latin_font, MARISA_X + 110, MARISA_Y + 300, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Same!");
			break;
		}
		switch (result) {
		case 0:
			vita2d_pvf_draw_text(s_pvf_latin_font, CONCLUSION_TX_X, CONCLUSION_TX_Y, RGBA8(197, 49, 44, s_common_alpha), 1.2f, "WIN !");
			if (s_common_alpha != 0)
				s_common_alpha -= 15;
			break;
		case 1:
			vita2d_pvf_draw_text(s_pvf_latin_font, CONCLUSION_TX_X, CONCLUSION_TX_Y, RGBA8(43, 91, 36, s_common_alpha), 1.2f, "DRAW");
			if (s_common_alpha != 0)
				s_common_alpha -= 15;
			break;
		case 2:
			vita2d_pvf_draw_text(s_pvf_latin_font, CONCLUSION_TX_X, CONCLUSION_TX_Y, RGBA8(13, 47, 190, s_common_alpha), 1.2f, "LOSE...");
			if (s_common_alpha != 0)
				s_common_alpha -= 15;
			break;
		}
		switch (color) {
		case 0:
			vita2d_draw_rectangle(s_x_anim + 5, CARD_Y + 5, 123, 190, RGBA8(218, 43, 56, 255));
			break;
		case 1:
			vita2d_draw_rectangle(s_x_anim + 5, CARD_Y + 5, 123, 190, RGBA8(12, 52, 217, 255));
			break;
		case 2:
			vita2d_draw_rectangle(s_x_anim + 5, CARD_Y + 5, 123, 190, RGBA8(49, 122, 33, 255));
			break;
		case 3:
			vita2d_draw_rectangle(s_x_anim + 5, CARD_Y + 5, 123, 190, RGBA8(127, 26, 132, 255));
			break;
		}
		vita2d_pvf_draw_textf(s_pvf_card_font, s_x_anim + 20, CARD_Y + 155, RGBA8(255, 255, 255, 255), 1, "%u", number);
		s_x_anim += 5;
		if (s_x_anim == CARD_R_X) {
			s_common_alpha = 0;
			s_x_anim = CARD_L_X;
			ret = 1;
		}
		break;
	}

	return ret;
}

SceVoid graphicsDrawMarisa(SceByte* marisa)
{
	vita2d_draw_texture(s_tex_body[marisa[0]], MARISA_X, MARISA_Y + 263);
	vita2d_draw_texture(s_tex_head[marisa[1]], MARISA_X, MARISA_Y);
	if (marisa[1] != 4) {
		vita2d_draw_texture(s_tex_eye[marisa[3]], MARISA_X + 94, MARISA_Y + 157);
		vita2d_draw_texture(s_tex_mouth[marisa[2]], MARISA_X + 108, MARISA_Y + 191);
	}
	if (marisa[4])
		vita2d_draw_texture(s_tex_revolver, MARISA_X - 185, MARISA_Y + 95);
}

SceVoid graphicsDrawDeck(SceByte card[4][9])
{
	SceInt32 x_pos = 40;
	SceInt32 y_pos = 10;

	vita2d_draw_texture(s_tex_deck, 40, 10);

	for (SceInt8 i = 0; i < 4; i++) {
		switch (i) {
		case 0:
			y_pos = 10;
			break;
		case 1:
			y_pos = 71;
			break;
		case 2:
			y_pos = 132;
			break;
		case 3:
			y_pos = 193;
			break;
		}
		for (SceInt8 j = 0; j < 9; j++) {
			if (!card[i][j])
				vita2d_draw_rectangle(x_pos, y_pos, 41, 61, RGBA8(39, 39, 39, 255));
			x_pos += 41;
		}
		x_pos = 40;
	}
}

SceUInt8 graphicsDrawFlashUp(SceVoid)
{
	SceUInt8 ret = 0;

	vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(255, 255, 255, s_flash_alpha));
	if (s_flash_alpha != 255)
		s_flash_alpha += 51;
	else
		ret = 1;

	return ret;
}

SceUInt8 graphicsDrawFlashDown(SceVoid)
{
	SceUInt8 ret = 0;

	vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(255, 255, 255, s_flash_alpha));
	if (s_flash_alpha != 0)
		s_flash_alpha -= 51;
	else
		ret = 1;

	return ret;
}

SceVoid graphicsDrawPreDeadMessage(SceVoid)
{
	vita2d_draw_texture_tint(s_tex_message_window, 180, 212, RGBA8(255, 255, 255, s_common_alpha));
	vita2d_pvf_draw_text(s_pvf_latin_font, 250, 285, RGBA8(255, 0, 0, s_common_alpha), 1.2f, "Win condition is not fulfilled!");
	if (s_common_alpha != 255)
		s_common_alpha += 5;
}

SceVoid graphicsDrawDeadMessage(SceVoid)
{
	vita2d_draw_texture_tint(s_tex_message_window, 180, 212, RGBA8(255, 255, 255, s_common_alpha));
	vita2d_pvf_draw_text(s_pvf_latin_font, 301, 285, RGBA8(255, 0, 0, s_common_alpha), 1.2f, "Marisa was shot dead...");
	if (s_common_alpha != 255)
		s_common_alpha += 5;
}

SceUInt8 graphicsDrawClear(SceUInt32 money)
{
	SceUInt8 ret = 0;

	if (s_common_alpha != 255) {
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(39, 39, 39, s_common_alpha));
		s_common_alpha += 5;
	}
	else {
		vita2d_draw_texture(s_tex_clear, 0, 0);
		if (!g_skip_flag) {
			vita2d_pvf_draw_text(s_pvf_latin_font, 20, 35, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Congratulations!!");
			vita2d_pvf_draw_textf(s_pvf_latin_font, 20, 35, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "\nMoney won: %u yen", money);
			vita2d_pvf_draw_textf(s_pvf_latin_font, 20, 35, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "\n\nTouch the screen to hide this message", money);
		}
		if (!s_state_end) {
			ret = 1;
			s_state_end = true;
		}
	}

	return ret;
}

SceUInt8 graphicsDrawExClear(SceUInt32 money)
{
	SceUInt8 ret = 0;

	if (s_common_alpha != 255) {
		vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(39, 39, 39, s_common_alpha));
		s_common_alpha += 5;
	}
	else {
		vita2d_draw_texture(s_tex_ex_clear, 0, 0);
		if (!g_skip_flag) {
			vita2d_pvf_draw_text(s_pvf_latin_font, 20, 35, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "Congratulations!!");
			vita2d_pvf_draw_textf(s_pvf_latin_font, 20, 35, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "\nMoney won: %u yen", money);
			vita2d_pvf_draw_textf(s_pvf_latin_font, 20, 35, RGBA8(0, 0, 0, s_common_alpha), 1.2f, "\n\nTouch the screen to hide this message", money);
		}
		if (!s_state_end) {
			ret = 1;
			s_state_end = true;
		}
	}

	return ret;
}