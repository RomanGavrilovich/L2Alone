#pragma once

#include "VisionUtils.h"
#include "Utils.h"
#include "Logger.h"
#include "VisionInitializer.h"
#include "VisionProvider.h"

class RuntimeVisionInitializer : public VisionInitializer {

public:

	RuntimeVisionInitializer(HueDistributionCapturer* capturer);

	VisionParams init(VisionProvider& provider, int timeoutMs) override;

private:
	bool initialized = false;
	bool loadingScreenDetected = false;

	HueDistributionCapturer* capturer;
};

RuntimeVisionInitializer::RuntimeVisionInitializer(HueDistributionCapturer* capturer) {
	this->capturer = capturer;
}

VisionParams RuntimeVisionInitializer::init(VisionProvider& vp, int timeoutMs) {

	if (initialized) {
		throw exception("RuntimeVisionInitializer already initialized");
	}

	BitMapInfo bitMapInfo;

	long startTime = GetTickCount64();

	int k = 0;

	prepareDirectory("RefCapture");
	while (GetTickCount64() - startTime < timeoutMs) {
		if (vp.capture(bitMapInfo)) {
			if (!loadingScreenDetected) {
				if (isLoadingWindow(bitMapInfo)) {
					logger.log("Loading screen detected");

					loadingScreenDetected = true;
				}

				stringstream ss;
				ss << "RefCapture/loading_" << k++ << ".bmp";
				writeBmpToFile(ss.str().c_str(), bitMapInfo);
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