#pragma once

#include "L2KeyboardEventHandler.h"
#include "L2FocusEventHandler.h"
#include "logger.h"

#include <Windows.h>

class L2PvpModeHandler : public L2KeyboardEventHandler, public L2FocusEventHandler {

public:

	L2PvpModeHandler() {
	}

	bool onKeyDown(KBDLLHOOKSTRUCT* kbdll) override {

		int vkCode = kbdll->vkCode;
		int extraInfo = kbdll->dwExtraInfo;

		if (extraInfo != syntheticPayload && (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL)) {
			ctrlDown = true;
		}

		if (vkCode == VK_SPACE && ctrlDown) {
			switchPvpMode();
		}
		else if (vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_TAB || vkCode == VK_ESCAPE) {
			lostFocus();
		}

		return true;
	}

	bool onKeyUp(KBDLLHOOKSTRUCT* kbdll) override {
		
		int vkCode = kbdll->vkCode;
		int extraInfo = kbdll->dwExtraInfo;
		if (extraInfo != syntheticPayload && (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL)) {
			ctrlDown = false;
		}

		if (pvpModeEnabled && hasFocus && (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL)) {
			return false;
		}
		else if (vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_TAB || vkCode == VK_ESCAPE) {
			gotFocus();
		}

		return true;
	}

	void onFocusLost() override {
		logger.log("PVP Focus Lost");
		lostFocus();
	}

	void onFocusReceived() override {
		logger.log("PVP Focus Received");
		gotFocus();
	}

private:
	bool hasFocus = false;
	bool pvpModeEnabled = false;
	bool ctrlDown = false;

	const int syntheticPayload = 4300043;

	void enterPvpMode() {
		if (!pvpModeEnabled) {
			pvpModeEnabled = true;
		}
	}

	void exitPvpMode() {
		if (pvpModeEnabled) {
			pvpModeEnabled = false;
			keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, syntheticPayload);
		}
	}

	void lostFocus() {
		if (!hasFocus) {
			return;
		}

		hasFocus = false;
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, syntheticPayload);
	}

	void gotFocus() {
		if (hasFocus) {
			return;
		}

		hasFocus = true;
		if (pvpModeEnabled) {
			keybd_event(VK_CONTROL, 0, 0, syntheticPayload);
		}
	}

	void switchPvpMode() {
		if (pvpModeEnabled) {
			exitPvpMode();
		}
		else {
			enterPvpMode();
		}
	}
};