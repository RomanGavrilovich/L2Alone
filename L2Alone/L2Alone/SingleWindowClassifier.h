#pragma once

#include <vector>

#include "WindowDefinition.h"
#include "VisionUtils.h"
#include "ButtonClassifier.h"
#include "WindowClassifier.h"

class SingleWindowClassifier : public WindowClassifier {

public:

	SingleWindowClassifier(int wWidth, int wHeight, vector<ButtonDefinition> buttonDefinitions, int textMinSize, int textMaxSize);

	~SingleWindowClassifier();

	bool isWindow(BitMapInfo& bmi) override;

private:
	ButtonClassifier* btnClassifier;
	vector<ButtonDefinition> bDefs;
	int textMinSize;
	int textMaxSize;
};

SingleWindowClassifier::SingleWindowClassifier(int wWidth, int wHeight, vector<ButtonDefinition> buttonDefinitions, int textMinSize, int textMaxSize) {

	this->btnClassifier = new ButtonClassifier(wWidth, wHeight);
	this->bDefs = buttonDefinitions;
	this->textMaxSize = textMaxSize;
	this->textMinSize = textMinSize;
}

SingleWindowClassifier::~SingleWindowClassifier() {
	delete btnClassifier;
}

bool SingleWindowClassifier::isWindow(BitMapInfo& bmi) {
	if (bDefs.size() == 0) {
		throw exception("There is no buttons to search");
	}

	for (auto it = bDefs.begin(); it != bDefs.end(); ++it) {

		if (!btnClassifier->isButton(bmi, *it)) {
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