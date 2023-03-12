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
		BitMapInfo bmi;
		provider.capture(bmi);

		map<int, double> distr;
		capturer->capture(bmi, distr);

		return VisionParams{ distr, getSystemMessageLength(bmi)};
	}

private:
	HueDistributionCapturer* capturer;
};