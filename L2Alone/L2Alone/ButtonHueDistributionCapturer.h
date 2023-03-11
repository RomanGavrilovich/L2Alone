#pragma once

#include <map>

#include "VisionUtils.h"
#include "WindowDefinition.h"

using namespace std;

class ButtonHueDistributionCapturer : public HueDistributionCapturer
{
public:

	ButtonHueDistributionCapturer(int rtWidth, int rtHeight, ButtonDefinition& def);

	virtual void capture(BitMapInfo& bmi, map<int, double>& dest) override;

private:

	ButtonDefinition def;
	int rtWidth;
	int rtHeight;
};

ButtonHueDistributionCapturer::ButtonHueDistributionCapturer(int rtWidth, int rtHeight, ButtonDefinition &def) {
	this->def = def;
	this->rtWidth = rtWidth;
	this->rtHeight = rtHeight;
}

void ButtonHueDistributionCapturer::capture(BitMapInfo& bmi, map<int, double>& dest) {

	Point targetLb = toLbPoint(rtWidth, rtHeight, bmi, def);
	int lbX = targetLb.x;
	int lbY = targetLb.y - def.height;

	initHDistribution(bmi, lbX, lbY, def.width, def.height, dest);
}