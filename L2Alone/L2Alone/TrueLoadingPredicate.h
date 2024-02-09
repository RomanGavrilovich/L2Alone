#pragma once

#include "LoadingPredicate.h"

class TrueLoadingPredicate : public LoadingPredicate {

public:
	void waitForInputReadiness(BitMapInfo& bmi) override;
};

void TrueLoadingPredicate::waitForInputReadiness(BitMapInfo& bmi) {
	return;
}