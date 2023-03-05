#pragma once

#include <sstream>
#include <future>
#include <condition_variable>
#include <shellapi.h>
#include <WinUser.h>
#include <dbt.h>
#include <psapi.h>

#include "AutologinStrategy.h"
#include "WindowsClassifier.h"
#include "VisionUtils.h"
#include "WindowDefinition.h"
#include "WindowsDefinitions.h"
#include "Logger.h"
#include "L2EventService.h"

using namespace std;

class C2AutologinStrategy : public AutologinStrategy {

public:

	C2AutologinStrategy();

	~C2AutologinStrategy();

	void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) override;

private:

	WindowsClassifier* wClassifier;

	void handleAccountIsUsing(HWND hWindow, L2CharSlot slot);

	L2Window captureAuthResultWindows(HWND hWindow);

	void doConfirmationFlow(HWND hWindow, L2CharSlot slot);
};

C2AutologinStrategy::C2AutologinStrategy() {

	map<L2Window, WindowDefinition> winDefs;
	WindowsDefinitions::initC2WindowsDefinitions(winDefs);

	wClassifier = new WindowsClassifier(winDefs);
}

C2AutologinStrategy::~C2AutologinStrategy() {
	delete wClassifier;
}

void C2AutologinStrategy::doConfirmationFlow(HWND hWindow, L2CharSlot slot) {

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

	logger.log("Select character");
	if (slot == L2CharSlot::ACTIVE) {
		slot = L2CharSlot::SLOT_1;
	}
	SelectCharacterDefinition def;
	def.dropdownItemHeight = 12;
	def.dropDownX = 185;
	def.dropDownY = 43;
	def.startX = 1280;
	def.startY = 600;
	def.actionTimeout = 50;
	selectCharacter(hWindow, slot, def);

	logger.log("Auto login flow completed");
}

void C2AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) {

	if (wClassifier->waitForWindow(hWindow, L2Window::WELCOME, 10000) != L2Window::WELCOME) {
		throw exception("Can't detect welcome window");
	}

	postCredentials(hWindow, login, password);

	logger.log("Credentials posted");
	Sleep(500); // give some time to process credentials post

	L2Window w = captureAuthResultWindows(hWindow);
	logger.log("Captured auth result: ", getL2WindowName(w));
	if (w == AGREEMENT) {
		logger.log("Agreement window detected");

		doConfirmationFlow(hWindow, slot);
	}
	else if (w == ACCOUNT_IN_USE) {
		logger.log("Account already in use. Try again");
		handleAccountIsUsing(hWindow, slot);
	}
	else if (w == INCORRECT_PASSWORD) {
		logger.log("Invalid credentials entered. Exit");
	}
	else {
		throw exception("Can't understand window state. Expected confirmation screen, account in using or incorrect password");
	}
}

void C2AutologinStrategy::handleAccountIsUsing(HWND hWindow, L2CharSlot slot) {
	
	logger.log("Account is using");
	
	postControlMessage(hWindow, VK_TAB);
	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);

	L2Window w = wClassifier->waitForWindow(hWindow, AGREEMENT, 5000);
	if (w != AGREEMENT) {
		throw exception("Can't find agreement screen");
	}
	doConfirmationFlow(hWindow, slot);
}

L2Window C2AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INCORRECT_PASSWORD);
	windows.push_back(L2Window::ACCOUNT_IN_USE);

	return wClassifier->waitForWindows(hWindow, windows, 5000);
}