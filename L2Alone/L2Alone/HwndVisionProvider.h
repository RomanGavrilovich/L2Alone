#pragma once

#include "VisionUtils.h"

class HwndVisionProvider : public VisionProvider {

public:
	HwndVisionProvider(HWND hWindow);

	bool capture(BitMapInfo& bmi) override;

	void dispose(BitMapInfo& bmi) override;

private:
	HWND hWindow;
};

HwndVisionProvider::HwndVisionProvider(HWND hWindow) {
	this->hWindow = hWindow;
}

bool HwndVisionProvider::capture(BitMapInfo& bmi) {
	return captureDcBmp(hWindow, bmi);
}

void HwndVisionProvider::dispose(BitMapInfo& bmi) {
	delete[] bmi.data;
}