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
#include "config_utils.h"
#include "HwndVisionProvider.h"

using namespace std;

struct SelectCharacterDefinition {
	int dropDownX;
	int dropDownY;
	int dropdownItemHeight;
	int startX;
	int startY;
	RefAnchor startAnchor;
	int actionTimeout;
	L2CharSlot slot;
};

class AutologinStrategy {

public:

	AutologinStrategy(VisionDefinition vDef, L2AloneConfig& config);

	void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot);

protected:

	virtual void initSelectCharDefinition(SelectCharacterDefinition& def) = 0;
	virtual void onAccountInUse(HWND hWindow);

	void selectCharacter(HWND hWindow, SelectCharacterDefinition& def);
	L2Window captureAuthResultWindows(HWND hWindow);

protected:
	virtual bool fastFlowSupported(L2CharSlot slot);
	virtual bool stopFastLogin(L2Window w);

private:
	L2AloneConfig config;
	WindowsClassifier* wClassifier;
	HueDistributionCapturer* capturer;
	VisionInitializer* vInitializer;
	InMemoryVisionCache* inMemoryVisionCache;
	RuntimeVisionInitializer* runtimeVisionInitializer;

	void doConfirmationFlow(HWND hWindow, SelectCharacterDefinition &selectCharDef);
	void handleAccountIsUsing(HWND hWindow, SelectCharacterDefinition &selectCharDef);
	
	L2Window doFastAutoLogin(HWND hWindow);
};

AutologinStrategy::AutologinStrategy(VisionDefinition vDef, L2AloneConfig& config) {
	this->config = config;

	auto bDef = vDef.wDefs[L2Window::WELCOME].bDefs[0];
	capturer = new ButtonHueDistributionCapturer(vDef.wWidth, vDef.wHeight, bDef);
	inMemoryVisionCache = new InMemoryVisionCache();
	runtimeVisionInitializer = new RuntimeVisionInitializer(capturer);
	vInitializer = new CachedVisionInitializer(inMemoryVisionCache, runtimeVisionInitializer);
	wClassifier = new WindowsClassifier(vDef);
}

bool AutologinStrategy::fastFlowSupported(L2CharSlot slot) {
	return slot == ACTIVE;
}

bool AutologinStrategy::stopFastLogin(L2Window w) {
	return false;
}

L2Window AutologinStrategy::doFastAutoLogin(HWND hWindow) {
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 3; ++j) {
			Sleep(100);
			postControlMessage(hWindow, VK_RETURN);
		}

		HwndVisionProvider provider(hWindow);
		auto w = wClassifier->waitForWindow(provider, 100);
		if (w == L2Window::ACCOUNT_IN_USE || w == L2Window::INCORRECT_PASSWORD || w == L2Window::UNKNOWN || stopFastLogin(w)) {
			return w;
		}
	}
}

void AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) {

	// Workaround to avoid capturing defect
	// Since we can receive following flow: black window -> loading image with partially transparent background -> black window -> welcome screen
	int preloadingTime = 100;
	if (config.preloadingTime > 0) {
		preloadingTime = config.preloadingTime;
	}
	//Sleep(preloadingTime);

	HwndVisionProvider provider(hWindow);
	auto vp = vInitializer->init(provider, config.visionInitTimeout);
	wClassifier->init(vp);

	SelectCharacterDefinition charDef;
	charDef.slot = slot;
	initSelectCharDefinition(charDef);
	if (config.mouseInputSpeed > 0) {
		charDef.actionTimeout = config.mouseInputSpeed;
	}

	if (fastFlowSupported(slot)) {
		postCredentials(hWindow, login, password);
		
		L2Window w = doFastAutoLogin(hWindow);
		if (w == ACCOUNT_IN_USE) {
			logger.log("Account in use");
			handleAccountIsUsing(hWindow, charDef);
		}
		else if (w == INCORRECT_PASSWORD) {
			throw exception("Invalid credentials entered");
		}
		else if (w == CHARACTERS) {
			selectCharacter(hWindow, charDef);
		}
		else if (w == UNKNOWN) {
			logger.log("Unknown window found. Complete autologin flow");
		}
	}
	else {
		HwndVisionProvider provider(hWindow);
		if (wClassifier->waitForWindow(provider, L2Window::WELCOME, 3000) != L2Window::WELCOME) {
			throw exception("Can't detect welcome window");
		}

		postCredentials(hWindow, login, password);
		Sleep(100); // give some time to process credentials post

		L2Window w = captureAuthResultWindows(hWindow);
		logger.log("Captured auth result: ", getL2WindowName(w));
		if (w == AGREEMENT) {
			doConfirmationFlow(hWindow, charDef);
		}
		else if (w == ACCOUNT_IN_USE) {
			logger.log("Account already in use. Try again");
			handleAccountIsUsing(hWindow, charDef);
		}
		else if (w == INCORRECT_PASSWORD) {
			throw exception("Invalid credentials entered. Exit");
		}
	}
}

void AutologinStrategy::onAccountInUse(HWND hWindow) {
	// Do nothing
}

void AutologinStrategy::handleAccountIsUsing(HWND hWindow, SelectCharacterDefinition& charDef) {

	logger.log("Account is using");
	onAccountInUse(hWindow);

	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);

	HwndVisionProvider provider(hWindow);
	L2Window w = wClassifier->waitForWindow(provider, AGREEMENT, 5000);
	if (w != AGREEMENT) {
		throw exception("Can't find agreement screen");
	}
	doConfirmationFlow(hWindow, charDef);
}

void AutologinStrategy::doConfirmationFlow(HWND hWindow, SelectCharacterDefinition& selectCharDef) {
	logger.log("Agreement window detected");

	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);
	HwndVisionProvider provider(hWindow);
	if (wClassifier->waitForWindow(provider, L2Window::SERVERS, 3000) != L2Window::SERVERS) {
		throw exception("Can't detect servers window");
	}

	logger.log("Post agreement");
	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);
	if (wClassifier->waitForWindow(provider, L2Window::CHARACTERS, 3000) != L2Window::CHARACTERS) {
		throw exception("Can't detect characters window");
	}

	if (selectCharDef.slot == ACTIVE) {
		for (int i = 0; i < 10; ++i) {
			Sleep(100);
			postControlMessage(hWindow, VK_RETURN);
		}
	}
	else {
		selectCharacter(hWindow, selectCharDef);
	}

	logger.log("Auto login flow completed");
}


void AutologinStrategy::selectCharacter(HWND hWindow, SelectCharacterDefinition& def) {

	RECT r;
	GetClientRect(hWindow, &r);

	int dropdownItemHeight = def.dropdownItemHeight;
	int charDropdownOffset = def.slot * dropdownItemHeight;

	int refScreenWidth = 1360;
	int refScreenHeight = 768;

	int wWidth = r.right - r.left;
	int wHeight = r.bottom - r.top;

	auto p = convertRtPoint(refScreenWidth, refScreenHeight, wWidth, wHeight, def.startX, def.startY, def.startAnchor);

	int targetX, targetY;
	convertToGlobalClientRect(hWindow, p.x, p.y, targetX, targetY);

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

	POINT cursorPointer;
	GetCursorPos(&cursorPointer);

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
	SetCursorPos(cursorPointer.x, cursorPointer.y);
}

L2Window AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INCORRECT_PASSWORD);
	windows.push_back(L2Window::ACCOUNT_IN_USE);

	HwndVisionProvider provider(hWindow);
	return wClassifier->waitForWindows(provider, windows, 5000);
}
