#pragma once

#include <map>

#include "VisionUtils.h"
#include "WindowDefinition.h"

using namespace std;

class ButtonClassifier {

public:

	ButtonClassifier(int rtWidth, int rtHeight);

	bool isButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef);

	bool hasBorders(BitMapInfo& bitMapInfo, ButtonDefinition bDef);

private:
	int rtWidth;
	int rtHeight;

	Point toLbViaCenterOffset(int lbWidth, int lbHeight, int rtX, int rtY);

	Point toLbViaCenterBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY);

	Point toLbViaRightBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY);

	Point toLbPoint(BitMapInfo& bitMapInfo, ButtonDefinition& bDef);

	void initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest);

	void collapse(map<int, double>& source, map<int, double>& dest);

	double getDistributionError(map<int, double>& first, map<int, double>& second);

	bool hasHorizontalBorder(BitMapInfo& bitMapInfo, int lbX, int lbY, int width, int errorRange);

	bool hasVerticalBorder(BitMapInfo& bitMapInfo, int lbX, int lbY, int height, int errorRange);
};

ButtonClassifier::ButtonClassifier(int rtWidth, int rtHeight) {
	this->rtWidth = rtWidth;
	this->rtHeight = rtHeight;
}

bool ButtonClassifier::isButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef) {
	return hasBorders(bitMapInfo, bDef);
}

bool ButtonClassifier::hasHorizontalBorder(BitMapInfo& bitMapInfo, int lbX, int lbY, int width, int errorRange) {

	int startY = lbY - errorRange / 2;
	for (int i = 0; i < errorRange; ++i) {
		map<int, double> hA;
		initHDistribution(bitMapInfo, lbX, startY + i, width, 1, hA);
		map<int, double> hB;
		initHDistribution(bitMapInfo, lbX, startY + i - 1, width, 1, hB);
		if (getDistributionError(hA, hB) > 5) {
			return true;
		}
	}

	return false;
}

bool ButtonClassifier::hasVerticalBorder(BitMapInfo& bitMapInfo, int lbX, int lbY, int height, int errorRange) {

	int startX = lbX - errorRange / 2;
	for (int i = 0; i < errorRange; ++i) {
		map<int, double> leftBorderH;
		initHDistribution(bitMapInfo, startX + i, lbY, 1, height, leftBorderH);
		map<int, double> beforeLeftBorderH;
		initHDistribution(bitMapInfo, startX + i - 1, lbY, 1, height, beforeLeftBorderH);
		if (getDistributionError(leftBorderH, beforeLeftBorderH) > 5) {
			return true;
		}
	}

	return false;
}

bool ButtonClassifier::hasBorders(BitMapInfo& bitMapInfo, ButtonDefinition bDef) {

	int pixelRange = 5;

	Point targetLb = toLbPoint(bitMapInfo, bDef);
	int lbX = targetLb.x;
	int lbY = targetLb.y - bDef.height;

	// Bottom border
	if (!hasHorizontalBorder(bitMapInfo, lbX, lbY, bDef.width, pixelRange)) {
		return false;
	}

	// Top border
	if (!hasHorizontalBorder(bitMapInfo, lbX, lbY + bDef.height, bDef.width, pixelRange)) {
		return false;
	}

	// Left border
	if (!hasVerticalBorder(bitMapInfo, lbX, lbY, bDef.height, pixelRange)) {
		return false;
	}

	// Right border
	if (!hasVerticalBorder(bitMapInfo, lbX + bDef.width, lbY, bDef.height, pixelRange)) {
		return false;
	}

	return true;
}

Point ButtonClassifier::toLbViaCenterOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

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


Point ButtonClassifier::toLbViaCenterBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

	int rtCenterX = rtWidth / 2;
	int rtCenterY = rtHeight / 2;
	int rtOffsetX = rtX - rtCenterX;

	int lbCenterX = lbWidth / 2;

	Point p;
	p.x = lbCenterX + rtOffsetX;
	p.y = rtHeight - rtY;

	return p;
}

Point ButtonClassifier::toLbViaRightBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

	Point p;
	p.x = lbWidth - (rtWidth - rtX);
	p.y = rtHeight - rtY;
	return p;
}

Point ButtonClassifier::toLbPoint(BitMapInfo& bitMapInfo, ButtonDefinition& bDef) {

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

void ButtonClassifier::initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest) {

	map<int, double> tmp;

	double total = width * height;
	for (int iy = y; iy < y + height; iy++) {
		for (int ix = x; ix < x + width; ix++) {
			int h, s, v;
			getPixelHsv(bitMapInfo, ix, iy, h, s, v);

			tmp[h] += 100.0 / total;
		}
	}

	collapse(tmp, dest);
}

void ButtonClassifier::collapse(map<int, double>& source, map<int, double>& dest) {

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

double ButtonClassifier::getDistributionError(map<int, double>& first, map<int, double>& second) {

	auto fp = first.begin();
	auto sp = second.begin();

	double sum = 0;

	while (fp != first.end() || sp != second.end()) {

		if (fp == first.end()) {
			sum += sp->second;
			++sp;
		}
		else if (sp == second.end()) {
			sum += fp->second;
			++fp;
		}
		else if (abs(fp->first - sp->first) < 4) {
			sum += abs(fp->second - fp->second);
			++fp;
			++sp;
		}
		else if (fp->first > sp->first) {
			++sp;
		}
		else {
			sum += fp->second;
			++fp;
		}
	}

	return sum;
}