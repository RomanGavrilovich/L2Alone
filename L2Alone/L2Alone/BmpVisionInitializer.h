#pragma once

#include <map>

#include "VisionUtils.h"
#include "VisionInitializer.h"

using namespace std;

class BmpVisionInitializer : VisionInitializer {

public:

	BmpVisionInitializer(HueDistributionCapturer &capturer) {
		this->capturer = &capturer;
	}

	VisionParams init(VisionProvider& provider, int timeoutMs) override {
		
		VisionParams vp;
		
		BitMapInfo bmi;
		provider.capture(bmi);

		capturer->capture(bmi, vp.hRef);

		captureTextReferenceColor(bmi, vp.textColor);
		vp.systemMessageLength = getSystemMessageLength(bmi, vp.textColor);

		return vp;
	}

private:
	HueDistributionCapturer* capturer;
};