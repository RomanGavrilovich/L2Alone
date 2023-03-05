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

using namespace std;

class C2AutologinStrategy : public AutologinStrategy {

public:

	C2AutologinStrategy();

	~C2AutologinStrategy();

	void doAutologin(HWND hWindow, string& login, string& password) override;

	void selectCharacter(HWND hWindow);

private:

	WindowsClassifier* wClassifier;

	void handleAccountIsUsing(HWND hWindow);

	L2Window captureAuthResultWindows(HWND hWindow);
};

C2AutologinStrategy::C2AutologinStrategy() {

	map<L2Window, WindowDefinition> winDefs;
	WindowsDefinitions::initC2WindowsDefinitions(winDefs);

	wClassifier = new WindowsClassifier(winDefs);
}

C2AutologinStrategy::~C2AutologinStrategy() {
	delete wClassifier;
}

void C2AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password) {

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
		selectCharacter(hWindow);

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

void C2AutologinStrategy::handleAccountIsUsing(HWND hWindow) {
	logger.log("Account is using");
}

L2Window C2AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INCORRECT_PASSWORD);
	windows.push_back(L2Window::ACCOUNT_IN_USE);
	
	return wClassifier->waitForWindows(hWindow, windows, 5000);
}

void C2AutologinStrategy::selectCharacter(HWND hWindow) {

	RECT r;
	GetClientRect(hWindow, &r);

	int refScreenWidth = 1360;
	int refScreenHeight = 768;

	int refX = 1278;
	int refY = 600;

	int wWidth = r.right - r.left;
	int wHeight = r.bottom - r.top;

	int offsetX = refScreenWidth - refX;
	int offsetY = refScreenHeight - refY;

	int targetX = wWidth - offsetX;
	int targetY = wHeight - offsetY;

	SetForegroundWindow(hWindow);
	Sleep(100);

	for (int i = 0; i < 5; ++i) {
		postClick(hWindow, 124, 44);
		Sleep(100);

		postClick(hWindow, 124, 58);
		Sleep(100);

		postClick(hWindow, targetX, targetY);
	
		auto w = wClassifier->waitForWindow(hWindow, 500);
		if (w == UNKNOWN) {
			return;
		}
	}
}