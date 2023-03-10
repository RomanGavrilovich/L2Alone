#pragma once

#include "VisionUtils.h"
#include "Utils.h"
#include "Logger.h"
#include "VisionInitializer.h"
#include "VisionProvider.h"

class RuntimeVisionInitializer : public VisionInitializer {

public:

	RuntimeVisionInitializer(HueDistributionCapturer* capturer, bool saveRefScreen);

	VisionParams init(VisionProvider& provider, int timeoutMs) override;

private:
	HueDistributionCapturer* capturer;
	bool saveRefScreen;
};

RuntimeVisionInitializer::RuntimeVisionInitializer(HueDistributionCapturer* capturer, bool saveRefScreen) {
	this->capturer = capturer;
	this->saveRefScreen = saveRefScreen;
}

VisionParams RuntimeVisionInitializer::init(VisionProvider& vp, int timeoutMs) {

	BitMapInfo bitMapInfo;

	long startTime = GetTickCount64();

	prepareDirectory("RefCapture");
	while (GetTickCount64() - startTime < timeoutMs) {
		if (vp.capture(bitMapInfo)) {

			if (!isLoadingWindow(bitMapInfo)) {
				VisionParams vp;
				capturer->capture(bitMapInfo, vp.hRef);

				if (saveRefScreen) {
					prepareDirectory("Debug");
					writeBmpToFile("Debug/Ref.bmp", bitMapInfo);
				}

				return vp;
			}
		}

		Sleep(100);
	}

	throw exception("Can't create vision parameters");
}