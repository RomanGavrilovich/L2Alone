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
		if (vkCode == VK_SPACE && (GetKeyState(VK_CONTROL) & 0x8000)) {
			switchPvpMode();
		}
		else if (vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_TAB || vkCode == VK_ESCAPE) {
			lostFocus();
		}

		return true;
	}

	bool onKeyUp(KBDLLHOOKSTRUCT* kbdll) override {
		
		int vkCode = kbdll->vkCode;
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

	void enterPvpMode() {
		if (!pvpModeEnabled) {
			pvpModeEnabled = true;
		}
	}

	void exitPvpMode() {
		if (pvpModeEnabled) {
			pvpModeEnabled = false;
			keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
		}
	}

	void lostFocus() {
		if (!hasFocus) {
			return;
		}

		hasFocus = false;
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
	}

	void gotFocus() {
		if (hasFocus) {
			return;
		}

		hasFocus = true;
		if (pvpModeEnabled) {
			keybd_event(VK_CONTROL, 0, 0, 0);
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