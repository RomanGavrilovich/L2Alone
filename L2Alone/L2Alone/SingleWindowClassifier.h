#pragma once

#include <vector>

#include "WindowDefinition.h"
#include "VisionUtils.h"
#include "ButtonCapturer.h"
#include "WindowClassifier.h"

class SingleWindowClassifier : public WindowClassifier {

public:

	SingleWindowClassifier(int wWidth, int wHeight, vector<ButtonDefinition> buttonDefinitions, int textMinSize, int textMaxSize);

	~SingleWindowClassifier();

	bool isWindow(BitMapInfo& bmi) override;

private:
	ButtonCapturer* btnCaptor;
	vector<ButtonDefinition> bDefs;
	int textMinSize;
	int textMaxSize;
};

SingleWindowClassifier::SingleWindowClassifier(int wWidth, int wHeight, vector<ButtonDefinition> buttonDefinitions, int textMinSize, int textMaxSize) {

	this->btnCaptor = new ButtonCapturer(wWidth, wHeight);
	this->bDefs = buttonDefinitions;
	this->textMaxSize = textMaxSize;
	this->textMinSize = textMinSize;
}

SingleWindowClassifier::~SingleWindowClassifier() {
	delete btnCaptor;
}

bool SingleWindowClassifier::isWindow(BitMapInfo& bmi) {
	if (bDefs.size() == 0) {
		throw exception("There is no buttons to search");
	}

	auto pFirstBtn = bDefs.begin();
	
	map<int, double> thDistr;
	btnCaptor->captureButton(bmi, *pFirstBtn, thDistr);
		
	for (auto it = ++pFirstBtn; it != bDefs.end(); ++it) {

		map<int, double> chDistr;
		btnCaptor->captureButton(bmi, *it, chDistr);

		if (getDistributionError(thDistr, chDistr) > 5) {
			return false;
		}
	}

	if (textMinSize > 0 && textMaxSize > 0) {
		auto length = getSystemMessageLength(bmi);
		if (!(textMinSize <= length && length <= textMaxSize)) {
			return false;
		}
	}

	return true;
}