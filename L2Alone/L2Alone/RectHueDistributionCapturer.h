#pragma once

#include <map>

#include "VisionUtils.h"
#include "WindowDefinition.h"

using namespace std;

class RectHueDistributionCapturer : public HueDistributionCapturer
{
public:

	RectHueDistributionCapturer(int rtWidth, int rtHeight, RectDefinition& def);

	virtual void capture(BitMapInfo& bmi, map<int, double>& dest) override;

private:

	RectDefinition def;
	int rtWidth;
	int rtHeight;
};

RectHueDistributionCapturer::RectHueDistributionCapturer(int rtWidth, int rtHeight, RectDefinition &def) {
	this->def = def;
	this->rtWidth = rtWidth;
	this->rtHeight = rtHeight;
}

void RectHueDistributionCapturer::capture(BitMapInfo& bmi, map<int, double>& dest) {

	Point targetLb = toLbPoint(rtWidth, rtHeight, bmi, def);
	int lbX = targetLb.x;
	int lbY = targetLb.y - def.height;

	initHDistribution(bmi, lbX, lbY, def.width, def.height, dest);
}