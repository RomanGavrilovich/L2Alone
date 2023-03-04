#pragma once

#include <map>

#include "VisionUtils.h"
#include "WindowDefinition.h"

using namespace std;

class ButtonClassifier {

public:


	ButtonClassifier(int rtWidth, int rtHeight) {
		this->rtWidth = rtWidth;
		this->rtHeight = rtHeight;
	}

	bool isButton(BitMapInfo& bitMapInfo, ButtonDefinition bDef) {
		return hasBorders(bitMapInfo, bDef);
	}

	bool hasBorders(BitMapInfo& bitMapInfo, ButtonDefinition bDef) {

		Point targetLb = toLbPoint(bitMapInfo, bDef);
		int lbX = targetLb.x;
		int lbY = targetLb.y - bDef.height;

		// Bottom border
		map<int, double> botBorderH;
		initHDistribution(bitMapInfo, lbX, lbY, bDef.width, 1, botBorderH);
		// Below bottom border
		map<int, double> underBotBorderH;
		initHDistribution(bitMapInfo, lbX, lbY - 1, bDef.width, 1, underBotBorderH);
		if (getDistributionError(botBorderH, underBotBorderH) < 5) {
			return false;
		}

		// Top border
		map<int, double> topBorderH;
		initHDistribution(bitMapInfo, lbX, lbY + bDef.height - 1, bDef.width, 1, topBorderH);
		// Above top border
		map<int, double> upperTopBorderH;
		initHDistribution(bitMapInfo, lbX, lbY + bDef.height, bDef.width, 1, upperTopBorderH);
		if (getDistributionError(topBorderH, upperTopBorderH) < 5) {
			return false;
		}

		// Left border
		map<int, double> leftBorderH;
		initHDistribution(bitMapInfo, lbX, lbY, 1, bDef.height, leftBorderH);
		// Before left border
		map<int, double> beforeLeftBorderH;
		initHDistribution(bitMapInfo, lbX - 1, lbY, 1, bDef.height, beforeLeftBorderH);
		if (getDistributionError(leftBorderH, beforeLeftBorderH) < 5) {
			return false;
		}

		// Right border
		map<int, double> rightBorderH;
		initHDistribution(bitMapInfo, lbX + bDef.width - 1, lbY, 1, bDef.height, rightBorderH);
		// After right border
		map<int, double> afterRightBorderH;
		initHDistribution(bitMapInfo, lbX + bDef.width, lbY, 1, bDef.height, afterRightBorderH);
		if (getDistributionError(rightBorderH, afterRightBorderH) < 5) {
			return false;
		}

		return true;
	}

private:
	int rtWidth;
	int rtHeight;

	Point toLbViaCenterOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

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

	Point toLbViaCenterBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

		int rtCenterX = rtWidth / 2;
		int rtCenterY = rtHeight / 2;
		int rtOffsetX = rtX - rtCenterX;

		int lbCenterX = lbWidth / 2;

		Point p;
		p.x = lbCenterX + rtOffsetX;
		p.y = rtHeight - rtY;

		return p;
	}

	Point toLbViaRightBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

		Point p;
		p.x = lbWidth - (rtWidth - rtX);
		p.y = rtHeight - rtY;
		return p;
	}

	Point toLbPoint(BitMapInfo& bitMapInfo, ButtonDefinition& bDef) {

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

	void initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest) {

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

	void collapse(map<int, double>& source, map<int, double>& dest) {

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

	double getDistributionError(map<int, double>& first, map<int, double>& second) {

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
};
