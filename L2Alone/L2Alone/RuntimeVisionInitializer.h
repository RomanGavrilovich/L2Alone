#pragma once

#include "VisionUtils.h"
#include "Utils.h"
#include "Logger.h"
#include "VisionInitializer.h"
#include "VisionProvider.h"

class RuntimeVisionInitializer : public VisionInitializer {

public:

	RuntimeVisionInitializer(HueDistributionCapturer* capturer, string debugFolder);

	VisionParams init(VisionProvider& provider, int timeoutMs) override;

private:
	HueDistributionCapturer* capturer;
	string debugFolder;
};

RuntimeVisionInitializer::RuntimeVisionInitializer(HueDistributionCapturer* capturer, string debugFolder) {
	this->capturer = capturer;
	this->debugFolder = debugFolder;
}

VisionParams RuntimeVisionInitializer::init(VisionProvider& vp, int timeoutMs) {

	BitMapInfo bitMapInfo;

	ULONGLONG startTime = GetTickCount64();

	while (GetTickCount64() - startTime < timeoutMs) {
		if (vp.capture(bitMapInfo)) {

			if (!isLoadingWindow(bitMapInfo)) {
				VisionParams vp;
				capturer->capture(bitMapInfo, vp.hRef);

				captureTextReferenceColor(bitMapInfo, vp.textColor);
				
				int systemMessageLength = getSystemMessageLength(bitMapInfo, vp.textColor);
				logger.log("Welcome screen system message length: ", systemMessageLength);

				vp.systemMessageLength = systemMessageLength;

				if (debugFolder.size() > 0) {
					writeBmpToFile((debugFolder + "/Ref.bmp").c_str(), bitMapInfo);
				}
				
				return vp;
			}
		}

		Sleep(CAPTURE_RETRY_DELAY);
	}

	throw exception("Can't create vision parameters");
}