#pragma once

#include "VisionUtils.h"
#include "Logger.h"

class RuntimeVisionInitializer : public VisionInitializer {

public:

	RuntimeVisionInitializer(HueDistributionCapturer* capturer);

	VisionParams init(HWND hWindow, int timeoutMs) override;

private:
	bool initialized = false;
	bool loadingScreenDetected = false;

	HueDistributionCapturer* capturer;
};

RuntimeVisionInitializer::RuntimeVisionInitializer(HueDistributionCapturer* capturer) {
	this->capturer = capturer;
}

VisionParams RuntimeVisionInitializer::init(HWND hWindow, int timeoutMs) {

	if (initialized) {
		throw exception("RuntimeVisionInitializer already initialized");
	}

	BitMapInfo bitMapInfo;

	long startTime = GetTickCount64();

	while (GetTickCount64() - startTime < timeoutMs) {
		if (captureDcBmp(hWindow, bitMapInfo)) {
			if (!loadingScreenDetected) {
				if (isLoadingWindow(bitMapInfo)) {
					loadingScreenDetected = true;
					writeBmpToFile("RefCapture/loading.bmp", bitMapInfo);
				}
			}
			else if (!initialized) {
				if (!isLoadingWindow(bitMapInfo)) {
					writeBmpToFile("RefCapture/capture.bmp", bitMapInfo);

					// This is a problem

					VisionParams vp;
					capturer->capture(bitMapInfo, vp.hRef);

					return vp;
					initialized = true;
				}
			}
		}

		Sleep(100);
	}

	throw exception("Can't create vision parameters");
}