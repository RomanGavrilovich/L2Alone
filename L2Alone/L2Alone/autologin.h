#pragma once

#include "logger.h"

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

#include "logger.h"
#include "vision.h"

using namespace std;

enum AutologinFLowState {
	NONE,
	INIT,
	WINDOW_DETECTED,
	FILL_CREDENTIALS,
	DO_CONFIRMATION_SEQUENCE,
	DONE
};

volatile AutologinFLowState autoLoginState = NONE;

void postText(HWND hWindow, string& s);
void postCredentials(HWND hWindow, string& login, string& password);
void postControlMessage(HWND hWindow, int vk);
bool postConfirmationSequence(HWND hWindow);

L2Window captureL2Windows(HWND hWindow, std::vector<L2Window> windows);
bool captureL2Window(HWND hWindow, L2Window w);
L2Window captureAuthResultWindows(HWND hWindow);
void handleAccountIsUsing(HWND hWindow);

void doAutologin(HWND hWindow, string& login, string& password)
{
	initializeWindowClassifier(hWindow, "D:\\WinLog\\screens", false);

	postCredentials(hWindow, login, password);
	logger.log("Credentials posted");

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

void handleAccountIsUsing(HWND hWindow) {

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

void postText(HWND hWindow, string& s) {

	for (char c : s) {
		PostMessage(hWindow, WM_CHAR, c, 0);
	}
}

void postControlMessage(HWND hWindow, int vk) {
	PostMessage(hWindow, WM_KEYDOWN, vk, 0);
}

void postCredentials(HWND hWindow, string& login, string& password) {
	postText(hWindow, login);
	postControlMessage(hWindow, VK_TAB);
	postText(hWindow, password);
	postControlMessage(hWindow, VK_RETURN);
}

bool postConfirmationSequence(HWND hWindow) {

	logger.log("Post confirmation sequence");
	autoLoginState = DO_CONFIRMATION_SEQUENCE;

	int unknownCounter = 0;
	// 10 sec timeout
	for (int i = 0; i < 100; ++i) {

		postControlMessage(hWindow, VK_RETURN);
		Sleep(100);

		auto w = CaptureWindow(hWindow, "D:\\WinLog\\screens", logger);
		if (w == UNKNOWN) {
			unknownCounter++;
			if (unknownCounter == 5) {
				logger.log("Unknown window found, complete auto login");
				autoLoginState = DONE;
				break;
			}
		}
	}

	return false;
}

bool captureL2Window(HWND hWindow, L2Window w) {
	std::vector<L2Window> v;
	v.push_back(w);
	return captureL2Windows(hWindow, v) == w;
}

L2Window captureAuthResultWindows(HWND hWindow) {
	std::vector<L2Window> windows;
	windows.push_back(L2Window::AGREEMENT);
	windows.push_back(L2Window::INVALID_CREDENTIALS);
	windows.push_back(L2Window::ACCOUNT_IN_USE);
	return captureL2Windows(hWindow, windows);
}

L2Window captureL2Windows(HWND hWindow, std::vector<L2Window> windows) {

	stringstream ss;
	for (int i = 0; i < windows.size(); ++i) {
		ss << getL2WindowName(windows[i]) << " ";
	}

	logger.log("Start window capturing: ", ss.str());

	for (int i = 0; i < 30; ++i) { // 3 sec
		logger.log("Capture window");
		try {
			L2Window w = CaptureWindow(hWindow, "D:\\WinLog\\screens", logger);
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
			Sleep(50);
		}
		catch (exception e) {
			logger.log("Capturing failed with exception: ", e.what());
		}
	}

	return UNKNOWN;
}