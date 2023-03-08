#pragma once

#include <vector>

#include "WindowDefinition.h"
#include "VisionUtils.h"
#include "ButtonCapturer.h"
#include "WindowClassifier.h"

class SingleWindowClassifier : public WindowClassifier {

public:

	SingleWindowClassifier(ButtonCapturer* capturer, vector<ButtonDefinition> buttonDefinitions, int textMinSize, int textMaxSize);

	~SingleWindowClassifier();

	bool isWindow(BitMapInfo& bmi) override;

private:
	ButtonCapturer* btnCaptor;
	vector<ButtonDefinition> bDefs;
	int textMinSize;
	int textMaxSize;
};

SingleWindowClassifier::SingleWindowClassifier(ButtonCapturer* capturer, vector<ButtonDefinition> buttonDefinitions, int textMinSize, int textMaxSize) {

	this->btnCaptor = capturer;
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

	for (auto& def : bDefs) {
		if (!btnCaptor->isButton(bmi, def)) {
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