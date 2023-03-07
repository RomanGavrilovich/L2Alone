#pragma once

#include "WindowClassifier.h"
#include "VisionUtils.h"

class LoadingWindowClassifier : public WindowClassifier {
public:

	bool isWindow(BitMapInfo& bitMapInfo) override {
		for (int i = 0; i < bitMapInfo.width; ++i) {
			for (int j = 0; j < bitMapInfo.height; ++j) {
				int r, g, b;
				getPixelRgb(bitMapInfo, i, j, r, g, b);
				if (r != 0 || g != 0 || b != 0) {
					return false;
				}
			}
		}

		return true;
	}
};