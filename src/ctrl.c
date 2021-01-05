#include <stdbool.h>
#include <stdlib.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/systemgesture.h> 

#include "ctrl.h"
#include "misc.h"

#define SWIPE_DETECT_POS 25
#define SWIPE_DETECT_NEG -25
#define SWIPE_PROTECT 10

#define ONPRESS(flag) ((ctrl.buttons & (flag)) && !(ctrl_old.buttons & (flag)))
//#define ONDEPRESS(flag) ((ctrl_old.buttons & (flag)) && !(ctrl.buttons & (flag)))
#define ONSWIPE_RIGHT (swipe_event.property.drag.deltaVector.x > SWIPE_DETECT_POS && abs(swipe_event.property.drag.deltaVector.y) < SWIPE_PROTECT)
#define ONSWIPE_LEFT (swipe_event.property.drag.deltaVector.x < SWIPE_DETECT_NEG && abs(swipe_event.property.drag.deltaVector.y) < SWIPE_PROTECT)
#define ONSWIPE_DOWN (swipe_event.property.drag.deltaVector.y > SWIPE_DETECT_POS && abs(swipe_event.property.drag.deltaVector.x) < SWIPE_PROTECT)
#define ONSWIPE_UP (swipe_event.property.drag.deltaVector.y < SWIPE_DETECT_NEG && abs(swipe_event.property.drag.deltaVector.x) < SWIPE_PROTECT)

extern SceBool g_skip_flag;

extern SceUInt8 g_choice;
extern SceUInt8 g_global_state;

static SceSystemGestureTouchRecognizer s_tap_recognizer;
static SceSystemGestureTouchRecognizer s_swipe_recognizer_front;
static SceSystemGestureTouchRecognizer s_swipe_recognizer_back;

SceVoid ctrlInit(SceVoid)
{
	SceInt32 ret;

	/* touch panels init */

	SceTouchPanelInfo back_panel_info, front_panel_info;
	sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &front_panel_info);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &back_panel_info);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

	/* systemgesture init */
	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_SYSTEM_GESTURE);
	DEBUG_PRINT("SceSystemGesture module: 0x%08x\n", ret);
	sceSystemGestureInitializePrimitiveTouchRecognizer(NULL);

	sceSystemGestureCreateTouchRecognizer(&s_tap_recognizer, SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, NULL, NULL);
	sceSystemGestureCreateTouchRecognizer(&s_swipe_recognizer_front, SCE_SYSTEM_GESTURE_TYPE_DRAG, SCE_TOUCH_PORT_FRONT, NULL, NULL);
	sceSystemGestureCreateTouchRecognizer(&s_swipe_recognizer_back, SCE_SYSTEM_GESTURE_TYPE_DRAG, SCE_TOUCH_PORT_BACK, NULL, NULL);
}

SceVoid ctrlMain(SceVoid)
{
	SceCtrlData ctrl, ctrl_old;
	SceTouchData tdf, tdb;

	SceSystemGestureTouchEvent swipe_event;
	SceSystemGestureTouchEvent tap_event;
	SceUInt32 event_num_buffer = 0;

	while (1) {

		sceTouchPeek(SCE_TOUCH_PORT_FRONT, &tdf, 1);
		sceTouchPeek(SCE_TOUCH_PORT_BACK, &tdb, 1);
		sceSystemGestureUpdatePrimitiveTouchRecognizer(&tdf, &tdb);
		sceSystemGestureUpdateTouchRecognizer(&s_swipe_recognizer_front);
		sceSystemGestureUpdateTouchRecognizer(&s_swipe_recognizer_back);

		switch (g_global_state) {
		case GS_ALL_GREEN:
			/* touch */

			sceSystemGestureGetTouchEvents(&s_swipe_recognizer_front, &swipe_event, 1, &event_num_buffer);

			if (event_num_buffer > 0) {
				if (ONSWIPE_LEFT)
					g_choice = 1;
				else if (ONSWIPE_RIGHT)
					g_choice = 4;
				else if (ONSWIPE_UP)
					g_choice = 2;
				else if (ONSWIPE_DOWN)
					g_choice = 3;
			}

			sceSystemGestureGetTouchEvents(&s_swipe_recognizer_back, &swipe_event, 1, &event_num_buffer);

			if (event_num_buffer > 0) {
				if (ONSWIPE_LEFT)
					g_choice = 1;
				else if (ONSWIPE_RIGHT)
					g_choice = 4;
				else if (ONSWIPE_UP)
					g_choice = 2;
				else if (ONSWIPE_DOWN)
					g_choice = 3;
			}

			/* buttons */

			sceCtrlPeekBufferPositive(0, &ctrl, 1);
			if (ONPRESS(SCE_CTRL_LEFT))
				g_choice = 1;
			else if (ONPRESS(SCE_CTRL_RIGHT))
				g_choice = 4;
			else if (ONPRESS(SCE_CTRL_UP))
				g_choice = 2;
			else if (ONPRESS(SCE_CTRL_DOWN))
				g_choice = 3;
			break;

		case GS_RESULT_WIN:
		case GS_RESULT_EX_WIN:
			/* touch */

			sceSystemGestureUpdateTouchRecognizer(&s_tap_recognizer);
			sceSystemGestureGetTouchEvents(&s_tap_recognizer, &tap_event, 1, &event_num_buffer);

			if (event_num_buffer > 0 && g_skip_flag)
				g_skip_flag = false;
			else if (event_num_buffer > 0)
				g_skip_flag = true;

			/* buttons */

			sceCtrlPeekBufferPositive(0, &ctrl, 1);
			if (ONPRESS(SCE_CTRL_CROSS) || ONPRESS(SCE_CTRL_CIRCLE) && g_skip_flag)
				g_skip_flag = false;
			else if (ONPRESS(SCE_CTRL_CROSS) || ONPRESS(SCE_CTRL_CIRCLE))
				g_skip_flag = true;
			break;

		case GS_DEAD:
			/* touch */

			sceSystemGestureUpdateTouchRecognizer(&s_tap_recognizer);
			sceSystemGestureGetTouchEvents(&s_tap_recognizer, &tap_event, 1, &event_num_buffer);

			if (event_num_buffer > 0)
				g_skip_flag = true;

			/* buttons */

			sceCtrlPeekBufferPositive(0, &ctrl, 1);
			if (ONPRESS(SCE_CTRL_CROSS) || ONPRESS(SCE_CTRL_CIRCLE))
				g_skip_flag = true;
			break;
		}

		ctrl_old = ctrl;

		sceKernelDelayThread(10000);
	}
}

SceInt32 ctrlThread(SceSize argc, ScePVoid argv)
{
	SceCtrlData ctrl, ctrl_old;
	SceTouchData tdf, tdb;

	SceSystemGestureTouchEvent tap_event;
	SceUInt32 event_num_buffer = 0;

	while (1) {

		/* touch */

		sceTouchPeek(SCE_TOUCH_PORT_FRONT, &tdf, 1);
		sceTouchPeek(SCE_TOUCH_PORT_BACK, &tdb, 1);

		sceSystemGestureUpdatePrimitiveTouchRecognizer(&tdf, &tdb);
		sceSystemGestureUpdateTouchRecognizer(&s_tap_recognizer);
		sceSystemGestureGetTouchEvents(&s_tap_recognizer, &tap_event, 1, &event_num_buffer);

		if (event_num_buffer > 0)
			g_skip_flag = true;

		/* buttons */

		sceCtrlPeekBufferPositive(0, &ctrl, 1);
		if (ONPRESS(SCE_CTRL_CROSS) || ONPRESS(SCE_CTRL_CIRCLE))
			g_skip_flag = true;

		ctrl_old = ctrl;

		sceKernelDelayThread(10000);

		if (g_global_state == GS_ALL_GREEN)
			break;
	}

	ctrlMain();

	return 0;
}