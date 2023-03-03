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
#include "logger.h"
#include "vision.h"

using namespace std;

class C5AutologinStrategy : public AutologinStrategy {

public:
	virtual void doAutologin(HWND hWindow, string& login, string& password) override;

private:
	void handleAccountIsUsing(HWND hWindow);

	bool postConfirmationSequence(HWND hWindow);

	bool captureL2Window(HWND hWindow, L2Window w);

	L2Window captureAuthResultWindows(HWND hWindow);

	L2Window captureL2Windows(HWND hWindow, std::vector<L2Window> windows);
};

void C5AutologinStrategy::doAutologin(HWND hWindow, string& login, string& password) {

	initializeWindowClassifier(hWindow, false);

	postCredentials(hWindow, login, password);
	logger.log("Credentials posted");
	Sleep(100); // give some time to process credentials post

	L2Window w = captureAuthResultWindows(hWindow);
	logger.log("Captured auth result: ", getL2WindowName(w));
	if (w == AGREEMENT) {
		logger.log("Agreement window detected");

		if (!postConfirmationSequence(hWindow)) {
			logger.log("Post agreement sequence timed out");
		}
	}
	else if (w == ACCOUNT_IN_USE) {
		logger.log("Account already in use. Try again");
		handleAccountIsUsing(hWindow);
	}
	else if (w == INVALID_CREDENTIALS) {
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

		string s = "";
		auto w = CaptureWindow(hWindow, s);
		if (w == UNKNOWN) {
			unknownCounter++;
			if (unknownCounter == 5) {
				logger.log("Unknown window found, complete auto login");
				break;
			}
		}
	}

	return false;
}

bool C5AutologinStrategy::captureL2Window(HWND hWindow, L2Window w) {
	std::vector<L2Window> v;
	v.push_back(w);
	return captureL2Windows(hWindow, v) == w;
}

L2Window C5AutologinStrategy::captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INVALID_CREDENTIALS);
	windows.push_back(L2Window::ACCOUNT_IN_USE);
	return captureL2Windows(hWindow, windows);
}

L2Window C5AutologinStrategy::captureL2Windows(HWND hWindow, std::vector<L2Window> windows) {

	stringstream ss;
	for (int i = 0; i < windows.size(); ++i) {
		ss << getL2WindowName(windows[i]) << "_";
	}
	logger.log("Start window capturing: ", ss.str());

	string s = ss.str().c_str();
	for (int i = 0; i < 30; ++i) { // 3 sec
		logger.log("Capture window");
		try {
			L2Window w = CaptureWindow(hWindow, s);
			logger.log("Capture window result: ", w);
			if (find(windows.begin(), windows.end(), w) != windows.end()) {
				return w;
			}
			else {
				if (w != UNKNOWN) {
					logger.log("Find unexpected window: ", getL2WindowName(w));
				}
			}

			logger.log("Didn't capture window");
			Sleep(100);
		}
		catch (exception e) {
			logger.log("Capturing failed with exception: ", e.what());
		}
	}

	return UNKNOWN;
}