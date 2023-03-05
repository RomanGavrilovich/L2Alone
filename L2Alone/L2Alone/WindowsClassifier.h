#pragma once

#include <Windows.h>
#include <vector>
#include <map>
#include <chrono>

#include "Logger.h"
#include "SingleWindowClassifier.h"
#include "WindowDefinition.h"
#include "ButtonClassifier.h"
#include "VisionUtils.h"
#include "Utils.h"

using namespace std;

class WindowsClassifier {

public:
	WindowsClassifier(map<L2Window, WindowDefinition> &wDefs);
	~WindowsClassifier();

	L2Window waitForWindows(HWND hWnd, vector<L2Window>& windows, int timeousMs);
	L2Window waitForWindow(HWND hWnd, L2Window window, int timeoutMs);
	L2Window waitForWindow(HWND hWnd, int timeoutMs);

	inline SingleWindowClassifier* createSingleWindowClassifier(WindowDefinition& def);
	
private:

	map<L2Window, SingleWindowClassifier*> classifiers;

	L2Window captureWindow(HWND hWnd, vector<L2Window>& windows);
};

WindowsClassifier::WindowsClassifier(map<L2Window, WindowDefinition> &wDefs) {
	for (auto& kv : wDefs) {
		classifiers[kv.first] = createSingleWindowClassifier(kv.second);
	}
}

WindowsClassifier::~WindowsClassifier() {
	for (auto& kv : classifiers) {
		delete kv.second;
	}
}

int k = 0;

L2Window WindowsClassifier::captureWindow(HWND hWnd, vector<L2Window>& windows) {

	BitMapInfo bitMapInfo;

	HDC hWindowDC = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hWindowDC);
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	BitBlt(hMemDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWindowDC, 0, 0, SRCCOPY);

	try {
		bitMapInfo = createBitMapInfo(hBitmap);
	}
	catch (exception e) {
		logger.error(e.what());
		return L2Window::UNKNOWN;
	}

	L2Window w = L2Window::UNKNOWN;

	for (auto& i : windows) {
		if (classifiers[i]->isWindow(bitMapInfo)) {
			w = i;
			break;
		}
	}

	// TODO: Remove
	if (w == UNKNOWN) {
		prepareDirectory("CapturesFailure");
		stringstream ss;
		ss << "CapturesFailure/" << k++;
		for (auto& w : windows) {
			ss << getL2WindowName(w) << ".";
		}
		ss << "bmp";

		writeBmpToFile(ss.str().c_str(), bitMapInfo);
	}

	delete[] bitMapInfo.data;

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hWindowDC);
	DeleteObject(hBitmap);

	return w;
}

L2Window WindowsClassifier::waitForWindows(HWND hWnd, vector<L2Window>& windows, int timeoutMs) {

	stringstream ss;
	for (int i = 0; i < windows.size(); ++i) {
		ss << getL2WindowName(windows[i]) << "/";
	}
	logger.log("Start window capturing: ", ss.str(), " with ", timeoutMs, " ms timeout");

	int startTick = GetTickCount64();
	while(GetTickCount64() - startTick < timeoutMs) {
		logger.log("Capture window");
		try {
			L2Window w = captureWindow(hWnd, windows);
			logger.log("Capture window result: ", w);
			if (find(windows.begin(), windows.end(), w) != windows.end()) {
				return w;
			}
			else {
				if (w != UNKNOWN) {
					logger.warn("Find unexpected window: ", getL2WindowName(w));
				}
			}

			logger.log("Didn't capture window");
			Sleep(100);
		}
		catch (exception e) {
			logger.error("Capturing failed with exception: ", e.what());
			throw e;
		}
	}

	return UNKNOWN;

}

L2Window WindowsClassifier::waitForWindow(HWND hWnd, L2Window window, int timeoutMs) {
	vector<L2Window> v;
	v.push_back(window);
	return waitForWindows(hWnd, v, timeoutMs);
}

L2Window WindowsClassifier::waitForWindow(HWND hWnd, int timeoutMs) {
	vector<L2Window> v;

	v.push_back(L2Window::WELCOME);
	v.push_back(L2Window::AGREEMENT);
	v.push_back(L2Window::INCORRECT_PASSWORD);
	v.push_back(L2Window::ACCOUNT_IN_USE);
	v.push_back(L2Window::SERVERS);
	v.push_back(L2Window::CHARACTERS);

	return waitForWindows(hWnd, v, timeoutMs);
}


inline SingleWindowClassifier* WindowsClassifier::createSingleWindowClassifier(WindowDefinition& def) {
	return new SingleWindowClassifier(def.width, def.height, def.bDefs, def.textMinSize, def.textMaxSize);
}