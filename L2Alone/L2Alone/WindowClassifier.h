#pragma once

class WindowClassifier {

public:
	virtual bool isWindow(BitMapInfo& bmi) = 0;
};