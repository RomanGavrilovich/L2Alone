#pragma once

#include <map>

#include "VisionUtils.h"
#include "WindowDefinition.h"

using namespace std;

class ButtonCapturer {

public:

	ButtonCapturer(int rtWidth, int rtHeight, ButtonDefinition refDef);

	void captureReferenceButton(BitMapInfo& bitMapInfo);

	bool isButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef);

private:
	int rtWidth;
	int rtHeight;
	ButtonDefinition refDef;

	map<int, double> hTargetDistr;

	Point toLbViaCenterOffset(int lbWidth, int lbHeight, int rtX, int rtY);

	Point toLbViaCenterBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY);

	Point toLbViaRightBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY);

	Point toLbPoint(BitMapInfo& bitMapInfo, ButtonDefinition& bDef);

	void initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest);

	void collapse(map<int, double>& source, map<int, double>& dest);

	void drawDebugBorders(BitMapInfo& bitMapInfo, ButtonDefinition bDef);

	void captureButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef, map<int, double>& hDistr);
};

ButtonCapturer::ButtonCapturer(int rtWidth, int rtHeight, ButtonDefinition refDef) {
	this->rtWidth = rtWidth;
	this->rtHeight = rtHeight;
	this->refDef = refDef;
}

bool ButtonCapturer::isButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef) {
	map<int, double> bhDistr;
	captureButton(bitMapInfo, bDef, bhDistr);

	return getDistributionError(bhDistr, hTargetDistr) < 5;
}

void ButtonCapturer::captureReferenceButton(BitMapInfo& bitMapInfo) {
	captureButton(bitMapInfo, refDef, hTargetDistr);
}

void ButtonCapturer::captureButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef, map<int, double> &hDistr) {

	Point targetLb = toLbPoint(bitMapInfo, bDef);
	int lbX = targetLb.x;
	int lbY = targetLb.y - bDef.height;

	initHDistribution(bitMapInfo, lbX, lbY, bDef.width, bDef.height, hDistr);

#ifdef TEST
	drawDebugBorders(bitMapInfo, bDef);
#endif // TEST
}

Point ButtonCapturer::toLbViaCenterOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

	int rtCenterX = rtWidth / 2;
	int rtCenterY = rtHeight / 2;
	int rtOffsetX = rtX - rtCenterX;
	int rtOffsetY = rtY - rtCenterY;

	int lbCenterX = lbWidth / 2;
	int lbCenterY = lbHeight / 2;

	Point p;
	p.x = lbCenterX + rtOffsetX;
	p.y = lbCenterY - rtOffsetY;

	return p;
}

Point ButtonCapturer::toLbViaCenterBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

	int rtCenterX = rtWidth / 2;
	int rtCenterY = rtHeight / 2;
	int rtOffsetX = rtX - rtCenterX;

	int lbCenterX = lbWidth / 2;

	Point p;
	p.x = lbCenterX + rtOffsetX;
	p.y = rtHeight - rtY;

	return p;
}

Point ButtonCapturer::toLbViaRightBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

	Point p;
	p.x = lbWidth - (rtWidth - rtX);
	p.y = rtHeight - rtY;
	return p;
}

Point ButtonCapturer::toLbPoint(BitMapInfo& bitMapInfo, ButtonDefinition& bDef) {

	if (bDef.anchor == RefAnchor::Center) {
		return toLbViaCenterOffset(bitMapInfo.width, bitMapInfo.height, bDef.rtX, bDef.rtY);
	}

	if (bDef.anchor == RefAnchor::CenterBottom) {
		return toLbViaCenterBottomOffset(bitMapInfo.width, bitMapInfo.height, bDef.rtX, bDef.rtY);
	}

	if (bDef.anchor == RefAnchor::BottomRight) {
		return toLbViaRightBottomOffset(bitMapInfo.width, bitMapInfo.height, bDef.rtX, bDef.rtY);
	}

	throw exception("Unrecognised ref anchor");
}

void ButtonCapturer::initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest) {

	map<int, double> tmp;

	int total = width * height;
	for (int iy = y; iy < y + height; iy++) {
		for (int ix = x; ix < x + width; ix++) {
			int h, s, v;
			getPixelHsv(bitMapInfo, ix, iy, h, s, v);

			tmp[h] += 100.0 / total;
		}
	}

	collapse(tmp, dest);
}

void ButtonCapturer::collapse(map<int, double>& source, map<int, double>& dest) {

	int currentH = source.begin()->first;
	double seqVal = 0;

	double dVal = 0;
	int dH = currentH;

	for (auto& pair : source) {

		if (pair.first - currentH <= 3) {
			seqVal += pair.second;

			if (pair.second > dVal) {
				dH = pair.first;
				dVal = pair.second;
			}
		}
		else {
			dest[dH] = seqVal;
			seqVal = pair.second;
			dH = pair.first;
			dVal = pair.second;
		}
		currentH = pair.first;
	}
	dest[dH] = seqVal;
}

void ButtonCapturer::drawDebugBorders(BitMapInfo& bitMapInfo, ButtonDefinition bDef) {
	Point targetLb = toLbPoint(bitMapInfo, bDef);
	int lbX = targetLb.x;
	int lbY = targetLb.y - bDef.height;

	drawRect(lbX, lbY, bDef.width, bDef.height, bitMapInfo);
}