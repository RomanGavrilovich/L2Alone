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

#ifdef TEST
#include "TestUtils.h"
#endif // TEST

using namespace std;

// TODO: Move to parameters
bool TRACE_CAPTURES = false;


class WindowsClassifier {

public:
	WindowsClassifier(VisionDefinition& vDef, string saveFailurePath);
	~WindowsClassifier();

	void init(VisionParams& params);

	L2Window waitForWindows(VisionProvider& vp, vector<L2Window>& windows, int timeousMs);
	L2Window waitForWindow(VisionProvider& vp, L2Window window, int timeoutMs);
	L2Window waitForWindow(VisionProvider& vp, int timeoutMs);

private:

	string saveFailurePath;

	VisionParams vParams;
	VisionDefinition vDef;

	bool isWindow(BitMapInfo& bmi, L2Window window);
};

WindowsClassifier::WindowsClassifier(VisionDefinition& vDef, string saveFailurePath) {
	this->vDef = vDef;
	this->saveFailurePath = saveFailurePath;
}

void WindowsClassifier::init(VisionParams& vParams) {
	this->vParams = vParams;
	vDef.wDefs[L2Window::WELCOME].textMaxSize = vParams.systemMessageLength + 5;
	vDef.wDefs[L2Window::WELCOME].textMinSize = vParams.systemMessageLength - 5;
}

WindowsClassifier::~WindowsClassifier() {
}

bool WindowsClassifier::isWindow(BitMapInfo& bmi, L2Window window) {

	auto wDef = vDef.wDefs[window];

	bool res = true;
	for (auto& bDef : wDef.bDefs) {
		ButtonHueDistributionCapturer capturer(vDef.wWidth, vDef.wHeight, bDef);

		map<int, double> hDistr;
		capturer.capture(bmi, hDistr);

		if (getDistributionError(vParams.hRef, hDistr) > 10) {
			res = false;
			break;
		}
	}

	if (!res) {
		res = true;
		for (auto& bDef : wDef.bDefs) {
			if (!hasBorders(vDef.wWidth, vDef.wHeight, bmi, bDef)) {
				res = false;
				break;
			}
		}
	}

	if (res) {
		if (wDef.textMaxSize > 0 && wDef.textMinSize > 0) {
			auto length = getSystemMessageLength(bmi, vParams.textColor);
			if (!(wDef.textMinSize <= length && length <= wDef.textMaxSize)) {
				res = false;
			}
		}
	}

#ifdef TEST
	for (auto& bDef : wDef.bDefs) {
		debugDraw(bmi, vDef.wWidth, vDef.wHeight, bDef);
	}

	if (wDef.textMaxSize > 0 && wDef.textMinSize > 0) {
		auto length = getSystemMessageLength(bmi, vParams.textColor);
		int centerX = bmi.width / 2;
		centerX -= length / 2;

		drawRect(centerX, 0, length, 50, bmi);
	}
#endif // TEST

	return res;
}

int k = 0;
L2Window WindowsClassifier::waitForWindows(VisionProvider& vp, vector<L2Window>& windows, int timeoutMs) {

	int sleepTime = 100;
	int maxSleepTime = 1000;

	L2Window w = L2Window::UNKNOWN;

	stringstream ss;
	for (int i = 0; i < windows.size(); ++i) {
		ss << getL2WindowName(windows[i]) << ".";
	}
	logger.log("Start window capturing: ", ss.str(), " with ", timeoutMs, " ms timeout");

	bool failureSaved = false;

	ULONGLONG startTick = GetTickCount64();
	ULONGLONG endTick = startTick + timeoutMs;

	do {
		logger.log("Capture window");
		try {

			BitMapInfo bitMapInfo;

			if (vp.capture(bitMapInfo)) {
				for (auto& i : windows) {
					if (isWindow(bitMapInfo, i)) {
						w = i;
						break;
					}
				}

				if (w != UNKNOWN) {
					logger.log("Capture window result: ", getL2WindowName(w));

					if (TRACE_CAPTURES) {
						stringstream ssOut;
						ssOut << saveFailurePath << "/" << k << "_S_" << ss.str() << "_" << getL2WindowName(w) << ".bmp";

						logger.log("Capture window found ", k);
						writeBmpToFile(ssOut.str().c_str(), bitMapInfo);
					}

					break;
				}
				else {

					if (TRACE_CAPTURES) {
						stringstream ssOut;
						ssOut << saveFailurePath << "/" << k << "_F_" << ss.str() << ".bmp";

						logger.log("Capture window failure ", k);
						writeBmpToFile(ssOut.str().c_str(), bitMapInfo);
						k++;
					}
					else {
						if (saveFailurePath.size() > 0
							&& !failureSaved
							&& GetTickCount64() > endTick - 2 * sleepTime) {

							logger.log("Capture window failure");

							failureSaved = true;

							string debugFile = saveFailurePath + "/" + ss.str() + ".bmp";
							writeBmpToFile(debugFile.c_str(), bitMapInfo);
						}
					}
				}

				vp.dispose(bitMapInfo);

				Sleep(sleepTime);
				sleepTime = sleepTime * 2;
				if (sleepTime > maxSleepTime) {
					sleepTime = maxSleepTime;
				}
			}
		}
		catch (exception e) {
			logger.error("Capturing failed with exception: ", e.what());
			throw e;
		}
	} while (GetTickCount64() < endTick);

	return w;
}

L2Window WindowsClassifier::waitForWindow(VisionProvider& vp, L2Window window, int timeoutMs) {
	vector<L2Window> v;
	v.push_back(window);
	return waitForWindows(vp, v, timeoutMs);
}

L2Window WindowsClassifier::waitForWindow(VisionProvider& vp, int timeoutMs) {
	vector<L2Window> v;

	v.push_back(L2Window::WELCOME);
	v.push_back(L2Window::AGREEMENT);
	v.push_back(L2Window::INCORRECT_PASSWORD);
	v.push_back(L2Window::ACCOUNT_IN_USE);
	v.push_back(L2Window::SERVERS);
	v.push_back(L2Window::CHARACTERS);

	return waitForWindows(vp, v, timeoutMs);
}