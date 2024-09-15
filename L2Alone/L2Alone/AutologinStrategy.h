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
#include "RectHueDistributionCapturer.h"
#include "CachedVisionInitializer.h"
#include "InMemoryVisionCache.h"
#include "RuntimeVisionInitializer.h"
#include "Logger.h"
#include "L2Events.h"
#include "L2EventService.h"
#include "ConfigUtils.h"
#include "HwndVisionProvider.h"
#include "LoadingAwaiter.h"
#include "InputHueLoadingAwaiter.h"
#include "NoOpLoadingAwaiter.h"

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
	~AutologinStrategy();

	void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot);

protected:

	virtual void initSelectCharDefinition(SelectCharacterDefinition& def) = 0;
	virtual void onAccountInUse(HWND hWindow);

	void selectCharacter(HWND hWindow, SelectCharacterDefinition& def);
	L2Window captureAuthResultWindows(HWND hWindow);

	virtual void beforeCharSlotSelection(HWND hWindow, int targetX, int targetY);

protected:
	virtual bool fastFlowSupported(L2CharSlot slot);
	virtual bool stopFastLogin(L2Window w);

	L2AloneConfig config;

private:
	WindowsClassifier* wClassifier;
	HueDistributionCapturer* capturer;
	VisionInitializer* vInitializer;
	InMemoryVisionCache* inMemoryVisionCache;
	RuntimeVisionInitializer* runtimeVisionInitializer;
	LoadingAwaiter* loadingAwaiter;

	void doConfirmationFlow(HWND hWindow, SelectCharacterDefinition& selectCharDef);
	void handleAccountIsUsing(HWND hWindow, SelectCharacterDefinition& selectCharDef);

	L2Window transitWindow(HWND hWindow, L2Window to);
	L2Window transitWindow(HWND hWindow, vector<L2Window>& to);

	L2Window doFastAutoLogin(HWND hWindow, bool fromAccountInUse);
};

AutologinStrategy::AutologinStrategy(VisionDefinition vDef, L2AloneConfig& config) {
	this->config = config;

	auto bDef = vDef.wDefs[L2Window::WELCOME].bDefs[0];
	capturer = new RectHueDistributionCapturer(vDef.wWidth, vDef.wHeight, bDef);

	inMemoryVisionCache = new InMemoryVisionCache();
	runtimeVisionInitializer = new RuntimeVisionInitializer(capturer, config.debugBmpPath);
	vInitializer = new CachedVisionInitializer(inMemoryVisionCache, runtimeVisionInitializer);
	wClassifier = new WindowsClassifier(vDef, config.debugBmpPath);

	if (vDef.inputFieldsDef.empty()) {
		loadingAwaiter = new NoOpLoadingAwaiter();
	}
	else {
		loadingAwaiter = new InputHueLoadingAwaiter(vDef.wWidth, vDef.wHeight, vDef.inputFieldsDef);
	}
}

AutologinStrategy::~AutologinStrategy() {
	delete wClassifier;
	delete capturer;
	delete vInitializer;
	delete inMemoryVisionCache;
	delete runtimeVisionInitializer;
	delete loadingAwaiter;
}

bool AutologinStrategy::fastFlowSupported(L2CharSlot slot) {
	return slot == ACTIVE;
}

bool AutologinStrategy::stopFastLogin(L2Window w) {
	return false;
}

void AutologinStrategy::beforeCharSlotSelection(HWND hWindow, int targetX, int targetY) {
}

L2Window AutologinStrategy::doFastAutoLogin(HWND hWindow, bool fromAccountInUse) {
	
	for (int i = 0; i < 50; ++i) {
		Sleep(config.inputInitialDelay);
		logger.log("Post VK_RETURN");

		postControlMessage(hWindow, VK_RETURN);

		HwndVisionProvider provider(hWindow);
		auto w = wClassifier->waitForWindow(provider, 100);

		if (w == ACCOUNT_IN_USE && fromAccountInUse) {
			continue;
		}

		if (w == L2Window::ACCOUNT_IN_USE || w == L2Window::INCORRECT_PASSWORD || w == L2Window::UNKNOWN || stopFastLogin(w)) {
			return w;
		}
	}

	throw exception("Auto login flow failure");
}

void AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) {

	HwndVisionProvider provider(hWindow);
	auto vp = vInitializer->init(provider, config.visionInitTimeout);
	wClassifier->init(vp);

	SelectCharacterDefinition charDef;
	charDef.slot = slot;
	initSelectCharDefinition(charDef);
	if (config.mouseClickDelay >= 0) {
		charDef.actionTimeout = config.mouseClickDelay;
	}

	vector<L2EventLockData> lockData;
	lockData.push_back(L2EventLockData{0, 0, 0});
	try {
		eventService.lockForEvents(lockData);
		loadingAwaiter->await(hWindow, provider);

		postCredentials(hWindow, login, password);
	}
	catch (exception e) {
		eventService.releaseLockForEvents();
		throw e;
	}
	eventService.releaseLockForEvents();

	if (config.fastFlowEnabled && fastFlowSupported(slot)) {

		bool fromAccountInUse = false;
		for (int i = 0; i < 2; ++i) {
			L2Window w = doFastAutoLogin(hWindow, fromAccountInUse);
			if (w == ACCOUNT_IN_USE) {
				logger.log("Account in use");
				onAccountInUse(hWindow);
				fromAccountInUse = true;
			}
			else if (w == INCORRECT_PASSWORD) {
				throw exception("Invalid credentials entered");
			}
			else if (w == CHARACTERS) {
				selectCharacter(hWindow, charDef);
				break;
			}
			else if (w == UNKNOWN) {
				logger.log("Unknown window found. Complete autologin flow");
				break;
			}
		}
	}
	else {
		config.windowTransitionRetryCount = 1;
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

	if (transitWindow(hWindow, AGREEMENT) != AGREEMENT) {
		throw exception("Can't find agreement screen");
	}

	doConfirmationFlow(hWindow, charDef);
}

L2Window AutologinStrategy::transitWindow(
	HWND hWindow,
	vector<L2Window>& to) {

	HwndVisionProvider provider(hWindow);

	int currentDelay = config.inputInitialDelay;

	vector<L2Window> awaitedWindows;
	for (auto& w : to) {
		awaitedWindows.push_back(w);
	}

	auto startTime = GetTickCount64();
	auto endTime = startTime + config.inputFallbackDelay;

	for (int i = 0; i < config.windowTransitionRetryCount; ++i) {
		Sleep(currentDelay);
		postControlMessage(hWindow, VK_RETURN);
		Sleep(currentDelay);

		auto currentWindow = wClassifier->waitForWindows(provider, awaitedWindows, 1000);
		if (currentWindow == UNKNOWN) {
			currentDelay = config.inputFallbackDelay;
		}
		else {
			return currentWindow;
		}
	}

	return UNKNOWN;
}

L2Window AutologinStrategy::transitWindow(HWND hWindow, L2Window to) {
	vector<L2Window> v;
	v.push_back(to);
	return transitWindow(hWindow, v);
}

void AutologinStrategy::doConfirmationFlow(HWND hWindow, SelectCharacterDefinition& selectCharDef) {
	logger.log("Agreement window detected");

	int transitionDelay = 0;
	int transitionTimeout = 3000;

	HwndVisionProvider provider(hWindow);

	logger.log("Wait for servers window");
	if (transitWindow(hWindow, L2Window::SERVERS) != L2Window::SERVERS) {
		throw exception("Can't detect servers window");
	}

	logger.log("Wait for characters window");
	if (transitWindow(hWindow, L2Window::CHARACTERS) != L2Window::CHARACTERS) {
		throw exception("Can't detect characters window");
	}

	if (selectCharDef.slot == ACTIVE) {
		for (int i = 0; i < 10; ++i) {
			Sleep(config.inputInitialDelay);
			postControlMessage(hWindow, VK_RETURN);
		}
	}
	else {
		selectCharacter(hWindow, selectCharDef);
	}

	logger.log("Auto login flow completed");
}


void AutologinStrategy::selectCharacter(HWND hWindow, SelectCharacterDefinition& def) {

	float scaleFactor = getScaleFactor(hWindow);
	logger.log("Scale factor: ", scaleFactor);

	RECT r;
	GetClientRect(hWindow, &r);

	int dropdownItemHeight = def.dropdownItemHeight;
	int charDropdownOffset = def.slot * dropdownItemHeight;

	int refScreenWidth = 1360;
	int refScreenHeight = 768;

	int wWidth = r.right - r.left;
	int wHeight = r.bottom - r.top;

	int dropDownX = def.dropDownX / scaleFactor;
	int dropDownY = def.dropDownY / scaleFactor;

	int dropdownClickX, dropdownClickY;
	convertToGlobalClientRect(hWindow, dropDownX, dropDownY, dropdownClickX, dropdownClickY);
	logger.log("Drop down click x: ", dropdownClickX, ", Drop down click y: ", dropdownClickY);

	int charSlotClickX, charSlotClickY;
	convertToGlobalClientRect(hWindow, dropDownX, dropDownY + charDropdownOffset, charSlotClickX, charSlotClickY);
	logger.log("Char slot click x: ", charSlotClickX, ", Char slot click y: ", charSlotClickY);

	auto p = convertRtPoint(refScreenWidth, refScreenHeight, wWidth, wHeight, def.startX, def.startY, def.startAnchor, scaleFactor);

	int targetX, targetY;
	convertToGlobalClientRect(hWindow, p.x, p.y, targetX, targetY);
	logger.log("Target click x: ", targetX, ", Target click y: ", targetY);

	vector<L2EventLockData> v;

	int lockClickX = dropdownClickX * scaleFactor;
	int lockClickY = dropdownClickY * scaleFactor;
	v.push_back(L2EventLockData{ WM_MOUSEMOVE, lockClickX, lockClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONDOWN, lockClickX, lockClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONUP, lockClickX, lockClickY });

	int lockSlotClickX = charSlotClickX * scaleFactor;
	int lockSlotClickY = charSlotClickY * scaleFactor;
	v.push_back(L2EventLockData{ WM_MOUSEMOVE, lockSlotClickX, lockSlotClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONDOWN, lockSlotClickX, lockSlotClickY });
	v.push_back(L2EventLockData{ WM_LBUTTONUP, lockSlotClickX, lockSlotClickY });

	int lockTargetX = targetX * scaleFactor;
	int lockTargetY = targetY * scaleFactor;
	v.push_back(L2EventLockData{ WM_MOUSEMOVE, lockTargetX, lockTargetY });
	v.push_back(L2EventLockData{ WM_LBUTTONDOWN, lockTargetX, lockTargetY });
	v.push_back(L2EventLockData{ WM_LBUTTONUP, lockTargetX, lockTargetY });

	POINT cursorPointer;
	GetCursorPos(&cursorPointer);

	eventService.lockForEvents(v);
	try {
		SetForegroundWindow(hWindow);
		SetFocus(hWindow);

		doClick(hWindow, dropdownClickX, dropdownClickY, config.mouseEventsDelay);
		doClick(hWindow, dropdownClickX, dropdownClickY, config.mouseEventsDelay);
		Sleep(def.actionTimeout);

		beforeCharSlotSelection(hWindow, charSlotClickX, charSlotClickY);
		doClick(hWindow, charSlotClickX, charSlotClickY, config.mouseEventsDelay);
		Sleep(def.actionTimeout);

		doClick(hWindow, targetX, targetY, config.mouseEventsDelay);
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
	return transitWindow(hWindow, windows);
}
