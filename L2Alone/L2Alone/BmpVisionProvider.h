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

	void dispose(BitMapInfo& bmi) override {
		// Do nothing
	}

private:
	BitMapInfo bmi;
};