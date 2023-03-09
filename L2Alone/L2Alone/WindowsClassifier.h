#pragma once

#include <Windows.h>
#include <vector>
#include <map>
#include <chrono>

#include "Logger.h"
#include "WindowDefinition.h"
#include "ButtonHueDistributionCapturer.h"
#include "VisionUtils.h"
#include "VisionInitializer.h"
#include "VisionProvider.h"
#include "Utils.h"

using namespace std;

class WindowsClassifier {

public:
	WindowsClassifier(VisionDefinition& vDef);
	~WindowsClassifier();

	void init(VisionParams& params);

	L2Window waitForWindows(VisionProvider& vp, vector<L2Window>& windows, int timeousMs);
	L2Window waitForWindow(VisionProvider& vp, L2Window window, int timeoutMs);
	L2Window waitForWindow(VisionProvider& vp, int timeoutMs);

private:

	VisionParams vParams;
	VisionDefinition vDef;

	L2Window captureWindow(VisionProvider& vp, vector<L2Window>& windows);

	bool isWindow(BitMapInfo& bmi, L2Window window);
};

WindowsClassifier::WindowsClassifier(VisionDefinition& vDef) {
	this->vDef = vDef;
}

void WindowsClassifier::init(VisionParams &vParams) {
	this->vParams = vParams;
}

WindowsClassifier::~WindowsClassifier() {
}

int k = 0;

bool WindowsClassifier::isWindow(BitMapInfo& bmi, L2Window window) {

	auto wDef = vDef.wDefs[window];

	for (auto& bDef : wDef.bDefs) {
		ButtonHueDistributionCapturer capturer(vDef.wWidth, vDef.wHeight, bDef);

		map<int, double> hDistr;
		capturer.capture(bmi, hDistr);

		if (getDistributionError(vParams.hRef, hDistr) > 5) {
			return false;
		}
	}

	if (wDef.textMaxSize > 0 && wDef.textMinSize > 0) {
		auto length = getSystemMessageLength(bmi);
		if (!(wDef.textMinSize <= length && length <= wDef.textMaxSize)) {
			return false;
		}
	}

	return true;
}

L2Window WindowsClassifier::captureWindow(VisionProvider& vp, vector<L2Window>& windows) {

	BitMapInfo bitMapInfo;

	if (!vp.capture(bitMapInfo)) {
		return L2Window::UNKNOWN;
	}

	L2Window w = L2Window::UNKNOWN;

	for (auto& i : windows) {
		if (isWindow(bitMapInfo, i)) {
			w = i;
			break;
		}
	}

#ifndef L2A_RELEASE
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
#endif // !L2A_RELEASE

	delete[] bitMapInfo.data;

	return w;
}

L2Window WindowsClassifier::waitForWindows(VisionProvider& vp, vector<L2Window>& windows, int timeoutMs) {

	stringstream ss;
	for (int i = 0; i < windows.size(); ++i) {
		ss << getL2WindowName(windows[i]) << "/";
	}
	logger.log("Start window capturing: ", ss.str(), " with ", timeoutMs, " ms timeout");

	int startTick = GetTickCount64();
	while (GetTickCount64() - startTick < timeoutMs) {
		logger.log("Capture window");
		try {
			L2Window w = captureWindow(vp, windows);
			logger.log("Capture window result: ", getL2WindowName(w));
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

L2Window WindowsClassifier::waitForWindow(VisionProvider& vp, L2Window window, int timeoutMs) {
	vector<L2Window> v;
	v.push_back(window);
	return waitForWindows(vp, v, timeoutMs);
}

L2Window WindowsClassifier::waitForWindow(VisionProvider& vp, int timeoutMs) {
	vector<L2Window> v;

	v.push_back(L2Window::AGREEMENT);
	v.push_back(L2Window::INCORRECT_PASSWORD);
	v.push_back(L2Window::ACCOUNT_IN_USE);
	v.push_back(L2Window::SERVERS);
	v.push_back(L2Window::CHARACTERS);

	return waitForWindows(vp, v, timeoutMs);
}