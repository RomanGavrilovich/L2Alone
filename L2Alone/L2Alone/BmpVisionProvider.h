#pragma once

#include "VisionUtils.h"
#include "VisionProvider.h"

class BmpVisionProvider : public VisionProvider {

public:
	BmpVisionProvider(BitMapInfo &bmi) {
		this->bmi = bmi;
	}

	bool capture(BitMapInfo& bmi) override {
		bmi = this->bmi;
		return true;
	}

private:
	BitMapInfo bmi;
};