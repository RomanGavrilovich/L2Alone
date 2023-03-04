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

	void doAutologin(HWND hWindow, string& login, string& password) override;

private:

	WindowsClassifier* wClassifier;

	void handleAccountIsUsing(HWND hWindow);

	bool postConfirmationSequence(HWND hWindow);

	L2Window captureAuthResultWindows(HWND hWindow);

	static void initWindowsDefinitions(map<L2Window, WindowDefinition>& dest);
};

C5AutologinStrategy::C5AutologinStrategy() {

	map<L2Window, WindowDefinition> winDefs;
	initWindowsDefinitions(winDefs);

	wClassifier = new WindowsClassifier(winDefs);
}

C5AutologinStrategy::~C5AutologinStrategy() {
	delete wClassifier;
}

void C5AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password) {

	map<L2Window, WindowDefinition> winDefs;
	initWindowsDefinitions(winDefs);

	unique_ptr<WindowsClassifier> wClassifier(new WindowsClassifier(winDefs));

	if (wClassifier->waitForWindow(hWindow, L2Window::WELCOME, 10000) != L2Window::WELCOME) {
		throw exception("Can't detect welcome window");
	}

	postCredentials(hWindow, login, password);
	logger.log("Credentials posted");
	Sleep(100); // give some time to process credentials post

	L2Window w = captureAuthResultWindows(hWindow);
	logger.log("Captured auth result: ", getL2WindowName(w));
	if (w == AGREEMENT) {
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

		int targetX = 1259;
		int targetY = 675;

		for (int i = 0; i < 100; ++i) {
			Sleep(100);
			logger.log("CLICK");
			postClick(hWindow, targetX, targetY);
			return;
		}

		return;

		for (int i = 0; i < 10; ++i) {
			Sleep(100);
			postControlMessage(hWindow, VK_RETURN);
		}
		logger.log("Auto login flow completed");
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

	for (int i = 0; i < 10; ++i) {
		postControlMessage(hWindow, VK_RETURN);
		L2Window w = captureAuthResultWindows(hWindow);

		if (w == AGREEMENT) {
			if (!postConfirmationSequence(hWindow)) {
				logger.log("Post agreement sequence timed out after handling accoint is using");
			}
			return;
		}
		Sleep(200);
	}
}

bool C5AutologinStrategy::postConfirmationSequence(HWND hWindow) {

	logger.log("Post confirmation sequence");

	int unknownCounter = 0;
	// 10 sec timeout
	for (int i = 0; i < 100; ++i) {

		postControlMessage(hWindow, VK_RETURN);
		Sleep(100);

		auto w = wClassifier->waitForWindow(hWindow, 100);
		if (w == UNKNOWN) {
			logger.log("Unknown window found, complete auto login");
			return true;
		}
	}

	return false;
}

L2Window C5AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INCORRECT_PASSWORD);
	windows.push_back(L2Window::ACCOUNT_IN_USE);
	return wClassifier->waitForWindows(hWindow, windows, 5000);
}

void C5AutologinStrategy::initWindowsDefinitions(map<L2Window, WindowDefinition>& dest) {

	int refScreenWidth = 1360;
	int refScreenHeight = 768;

	// Log-in screen
	vector<ButtonDefinition> welcomeBtnDefs;
	welcomeBtnDefs.push_back(ButtonDefinition{ 583, 402, 94, 21 });
	welcomeBtnDefs.push_back(ButtonDefinition{ 683, 402, 94, 21 });

	auto welcomeDef = WindowDefinition{ refScreenWidth, refScreenHeight, welcomeBtnDefs, 0, 0 };
	dest[L2Window::WELCOME] = welcomeDef;

	auto accountInUseDef = WindowDefinition{ refScreenWidth, refScreenHeight, welcomeBtnDefs, 200, 300 };
	dest[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

	auto incorrectPasswordDef = WindowDefinition{ refScreenWidth, refScreenHeight, welcomeBtnDefs, 400, 500 };
	dest[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

	// Agreement screen
	vector<ButtonDefinition> agreementBtnDefs;
	agreementBtnDefs.push_back(ButtonDefinition{ 603, 568, 74, 21 });
	agreementBtnDefs.push_back(ButtonDefinition{ 683, 568, 74, 21 });

	auto agreementDef = WindowDefinition{ refScreenWidth, refScreenHeight, agreementBtnDefs, 0, 0 };
	dest[L2Window::AGREEMENT] = agreementDef;


	// Servers screen
	vector<ButtonDefinition> serverBtnDefs;
	serverBtnDefs.push_back(ButtonDefinition{ 563, 410, 74, 21 });
	serverBtnDefs.push_back(ButtonDefinition{ 643, 410, 74, 21 });
	serverBtnDefs.push_back(ButtonDefinition{ 724, 410, 74, 21 });

	auto serversDef = WindowDefinition{ refScreenWidth, refScreenHeight, serverBtnDefs, 0, 0 };
	dest[L2Window::SERVERS] = serversDef;

	// Chars screen
	vector<ButtonDefinition> charsBtnDefs;
	charsBtnDefs.push_back(ButtonDefinition{ 623, 668, 114, 29, RefAnchor::CenterBottom });
	charsBtnDefs.push_back(ButtonDefinition{ 1219, 589, 94, 21, RefAnchor::BottomRight });
	charsBtnDefs.push_back(ButtonDefinition{ 1219, 613, 94, 21, RefAnchor::BottomRight });
	charsBtnDefs.push_back(ButtonDefinition{ 1219, 663, 94, 21, RefAnchor::BottomRight });

	auto charsDef = WindowDefinition{ refScreenWidth, refScreenHeight, charsBtnDefs, 0, 0 };
	dest[L2Window::CHARACTERS] = charsDef;
}