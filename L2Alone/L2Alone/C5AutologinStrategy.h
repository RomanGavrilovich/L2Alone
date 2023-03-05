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

#include "logger.h"

using namespace std;

class C5AutologinStrategy : public AutologinStrategy {

public:
	C5AutologinStrategy();
	~C5AutologinStrategy();

	void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) override;

	void doConfirmationFlow(HWND hWindow, L2CharSlot slot);

private:

	WindowsClassifier* wClassifier;

	void handleAccountIsUsing(HWND hWindow);

	L2Window captureAuthResultWindows(HWND hWindow);
};

C5AutologinStrategy::C5AutologinStrategy() {

	map<L2Window, WindowDefinition> winDefs;
	WindowsDefinitions::initC5WindowsDefinitions(winDefs);

	wClassifier = new WindowsClassifier(winDefs);
}

C5AutologinStrategy::~C5AutologinStrategy() {
	delete wClassifier;
}

void C5AutologinStrategy::doConfirmationFlow(HWND hWindow, L2CharSlot slot) {
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
		selectCharacter(hWindow, slot, 680, 682);
	}

	logger.log("Auto login flow completed");
}

void C5AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) {

	if (wClassifier->waitForWindow(hWindow, L2Window::WELCOME, 10000) != L2Window::WELCOME) {
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
		handleAccountIsUsing(hWindow);
	}
	else if (w == INCORRECT_PASSWORD) {
		logger.log("Invalid credentials entered. Exit");
		return;
	}
}

void C5AutologinStrategy::handleAccountIsUsing(HWND hWindow) {

	logger.log("Account is using");

	Sleep(100);
	postControlMessage(hWindow, VK_RETURN);

	L2Window w = wClassifier->waitForWindow(hWindow, AGREEMENT, 5000);
	if (w != AGREEMENT) {
		throw exception("Can't find agreement screen");
	}
	doConfirmationFlow(hWindow, L2CharSlot::ACTIVE);
}

L2Window C5AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INCORRECT_PASSWORD);
	windows.push_back(L2Window::ACCOUNT_IN_USE);
	return wClassifier->waitForWindows(hWindow, windows, 5000);
}