#pragma once

#include "LoadingPredicate.h"

class InputHueLoadingPredicate : public LoadingPredicate {

public:
	void waitForInputReadiness(BitMapInfo& bmi) override;
};

void InputHueLoadingPredicate::waitForInputReadiness(BitMapInfo& bmi) {


}