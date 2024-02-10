#pragma once

#include "LoadingAwaiter.h"
#include "Logger.h"

#include <Windows.h>

#include <vector>

class InputHueLoadingAwaiter : public LoadingAwaiter {

public:
	InputHueLoadingAwaiter(int rtWidth, int rtHeight, vector<RectDefinition> inputRects);
	
	void await(HWND hWindow, VisionProvider& bmi) override;

private:
	int rtWidth;
	int rtHeight;
	vector<RectDefinition> inputRects;

	void initDistribution(BitMapInfo& bmi, RectDefinition& rect, map<int, double>& dest);
	void tryFocusInput(HWND hWindow);
};

InputHueLoadingAwaiter::InputHueLoadingAwaiter(int rtWidth, int rtHeight, vector<RectDefinition> inputRects) {
	this->inputRects = inputRects;
	this->rtWidth = rtWidth;
	this->rtHeight = rtHeight;
}

void InputHueLoadingAwaiter::tryFocusInput(HWND hWindow) {

	postControlMessage(hWindow, VK_RETURN);
}

void InputHueLoadingAwaiter::await(HWND hWindow, VisionProvider& vp) {

	int retryDelay = 50;
	for (int i = 0; i < 3; ++i) {
		BitMapInfo bmi;
		if (vp.capture(bmi)) {
			RectDefinition def1 = inputRects[0];
			map<int, double> h1;
			initDistribution(bmi, def1, h1);

			RectDefinition def2 = inputRects[1];
			map<int, double> h2;
			initDistribution(bmi, def2, h2);

			double distrError = getDistributionError(h1, h2);
			logger.log("Distribution error between input fields are ", distrError);
			if (distrError < 5) {

				logger.log("Try to set focus for input field");
				tryFocusInput(hWindow);

				vp.dispose(bmi);
			}
			else {
				logger.log("Loading awaiting completed");
				vp.dispose(bmi);
				return;
			}
		}
		else {
			logger.log("Can't capture screen. Retry");
		}

		Sleep(retryDelay);
	}
}

void InputHueLoadingAwaiter::initDistribution(BitMapInfo& bmi, RectDefinition& rect, map<int, double>& dest) {

	Point targetLb1 = toLbPoint(rtWidth, rtHeight, bmi, rect);
	int lbX1 = targetLb1.x;
	int lbY1 = targetLb1.y - rect.height;

	initHDistribution(bmi, lbX1, lbY1, rect.width, rect.height, dest);
}