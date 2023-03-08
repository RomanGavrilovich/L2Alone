#pragma once

#include <iostream>
#include <fstream>
#include <ostream>
#include <Windows.h>
#include <map>
#include <set>

#include <string>
#include <codecvt>
#include <locale>
#include <vector>

#include <cstdlib>
#include <cstring>
#include <limits.h>

#include <sstream>
#include <future>
#include <condition_variable>
#include <shellapi.h>
#include <WinUser.h>
#include <dbt.h>
#include <psapi.h>

#include "AutologinStrategy.h"
#include "WindowsClassifier.h"
#include "WindowsDefinitions.h"
#include "WindowDefinition.h"
#include "ButtonHueDistributionCapturer.h"
#include "CachedVisionInitializer.h"
#include "InMemoryVisionCache.h"
#include "RuntimeVisionInitializer.h"
#include "Logger.h"
#include "L2Events.h"
#include "L2EventService.h"

using namespace std;

struct SelectCharacterDefinition {
	int dropDownX;
	int dropDownY;
	int dropdownItemHeight;
	int startX;
	int startY;
	int actionTimeout;
	L2CharSlot slot;
};

class AutologinStrategy {

public:

	void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot);

protected:

	virtual void initVisionDefinition(VisionDefinition& def) = 0;
	virtual void initSelectCharDefinition(SelectCharacterDefinition& def) = 0;
	virtual void onAccountInUse(HWND hWindow);

	void selectCharacter(HWND hWindow, L2CharSlot slot, SelectCharacterDefinition& def);
	L2Window captureAuthResultWindows(HWND hWindow);

private:
	WindowsClassifier* wClassifier;
	HueDistributionCapturer* capturer;
	VisionInitializer* vInitializer;
	InMemoryVisionCache* inMemoryVisionCache;
	RuntimeVisionInitializer* runtimeVisionInitializer;

	void doConfirmationFlow(HWND hWindow, L2CharSlot slot);
	void handleAccountIsUsing(HWND hWindow, L2CharSlot slot);
};

void AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) {

	VisionDefinition vDef;
	initVisionDefinition(vDef);

	auto bDef = vDef.wDefs[L2Window::WELCOME].bDefs[0];
	capturer = new ButtonHueDistributionCapturer(vDef.wWidth, vDef.wHeight, bDef);
	inMemoryVisionCache = new InMemoryVisionCache();
	runtimeVisionInitializer = new RuntimeVisionInitializer(capturer);
	vInitializer = new CachedVisionInitializer(inMemoryVisionCache, runtimeVisionInitializer);
	wClassifier = new WindowsClassifier(vDef);

	auto vParams = vInitializer->init(hWindow, 10000);
	wClassifier->init(vParams);

	if (wClassifier->waitForWindow(hWindow, L2Window::WELCOME, 3000) != L2Window::WELCOME) {
		throw exception("Can't detect welcome window");
	}

	postCredentials(hWindow, login, password);
	logger.log("Credentials posted");
	Sleep(100); // give some time to process credentials post

	L2Window w = captureAuthResultWindows(hWindow);
	logger.log("Captured auth result: ", getL2WindowName(w));
	if (w == AGREEMENT) {
		doConfirmationFlow(hWindow, slot);
	}
	else if (w == ACCOUNT_IN_USE) {
		logger.log("Account already in use. Try again");
		handleAccountIsUsing(hWindow, slot);
	}
	else if (w == INCORRECT_PASSWORD) {
		logger.log("Invalid credentials entered. Exit");
		return;
	}
}

void AutologinStrategy::onAccountInUse(HWND hWindow) {
	// Do nothing
}

void AutologinStrategy::handleAccountIsUsing(HWND hWindow, L2CharSlot slot) {

	logger.log("Account is using");
	onAccountInUse(hWindow);

	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);

	L2Window w = wClassifier->waitForWindow(hWindow, AGREEMENT, 5000);
	if (w != AGREEMENT) {
		throw exception("Can't find agreement screen");
	}
	doConfirmationFlow(hWindow, slot);
}

void AutologinStrategy::doConfirmationFlow(HWND hWindow, L2CharSlot slot) {
	logger.log("Agreement window detected");

	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);
	if (wClassifier->waitForWindow(hWindow, L2Window::SERVERS, 3000) != L2Window::SERVERS) {
		throw exception("Can't detect servers window");
	}

	logger.log("Post agreement");
	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);
	if (wClassifier->waitForWindow(hWindow, L2Window::CHARACTERS, 3000) != L2Window::CHARACTERS) {
		throw exception("Can't detect characters window");
	}

	if (slot == ACTIVE) {
		for (int i = 0; i < 10; ++i) {
			Sleep(100);
			postControlMessage(hWindow, VK_RETURN);
		}
	}
	else {
		SelectCharacterDefinition def;
		def.slot = slot;

		initSelectCharDefinition(def);

		selectCharacter(hWindow, slot, def);
	}

	logger.log("Auto login flow completed");
}


void AutologinStrategy::selectCharacter(HWND hWindow, L2CharSlot slot, SelectCharacterDefinition& def) {

	RECT r;
	GetClientRect(hWindow, &r);

	int dropdownItemHeight = def.dropdownItemHeight;
	int charDropdownOffset = slot * dropdownItemHeight;

	int refScreenWidth = 1360;
	int refScreenHeight = 768;

	int wWidth = r.right - r.left;
	int wHeight = r.bottom - r.top;

	int offsetX = refScreenWidth - def.startX;
	int offsetY = refScreenHeight - def.startY;

	int targetX, targetY;
	convertToGlobalClientRect(hWindow, wWidth - offsetX, wHeight - offsetY, targetX, targetY);

	int dropdownClickX, dropdownClickY;
	convertToGlobalClientRect(hWindow, def.dropDownX, def.dropDownY, dropdownClickX, dropdownClickY);

	int charSlotClickX, charSlotClickY;
	convertToGlobalClientRect(hWindow, def.dropDownX, def.dropDownY + charDropdownOffset, charSlotClickX, charSlotClickY);

	vector<L2EventLockData> v;
	v.push_back(L2EventLockData{ WM_MOUSEMOVE, dropdownClickX, dropdownClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONDOWN, dropdownClickX, dropdownClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONUP, dropdownClickX, dropdownClickY });

	v.push_back(L2EventLockData{ WM_MOUSEMOVE, charSlotClickX, charSlotClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONDOWN, charSlotClickX, charSlotClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONUP, charSlotClickX, charSlotClickY });

	v.push_back(L2EventLockData{ WM_MOUSEMOVE, targetX, targetY });
	v.push_back(L2EventLockData{ WM_LBUTTONDOWN, targetX, targetY });
	v.push_back(L2EventLockData{ WM_LBUTTONUP, targetX, targetY });

	POINT p;
	GetCursorPos(&p);

	eventService.lockForEvents(v);
	try {
		SetForegroundWindow(hWindow);
		SetFocus(hWindow);

		doClick(hWindow, dropdownClickX, dropdownClickY, def.actionTimeout);
		doClick(hWindow, dropdownClickX, dropdownClickY, def.actionTimeout);
		Sleep(def.actionTimeout);

		doClick(hWindow, charSlotClickX, charSlotClickY, def.actionTimeout);
		Sleep(def.actionTimeout);

		doClick(hWindow, targetX, targetY, def.actionTimeout);
	}
	catch (exception e) {
		logger.error(e.what());
		eventService.releaseLockForEvents();
		throw e;
	}

	eventService.releaseLockForEvents();
	Sleep(def.actionTimeout);
	SetCursorPos(p.x, p.y);
}

L2Window AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INCORRECT_PASSWORD);
	windows.push_back(L2Window::ACCOUNT_IN_USE);
	return wClassifier->waitForWindows(hWindow, windows, 5000);
}
