#pragma once

#include <vector>
#include <map>

using namespace std;

enum L2CharSlot {
	ACTIVE,
	SLOT_1,
	SLOT_2,
	SLOT_3,
	SLOT_4,
	SLOT_5,
	SLOT_6,
	SLOT_7
};

enum L2Window {
	UNKNOWN,
	WELCOME,
	CONNECTING,
	AGREEMENT,
	ACCOUNT_IN_USE,
	INCORRECT_PASSWORD,
	SERVERS,
	CHARACTERS
};

enum RefAnchor {
	Center,
	CenterBottom,
	BottomRight
};

struct RectDefinition {
	int rtX;
	int rtY;
	int width;
	int height;
	RefAnchor anchor = RefAnchor::Center;
};

struct WindowDefinition {
	vector<RectDefinition> bDefs;
	int textMinSize = 0;
	int textMaxSize = 0;
};

struct VisionDefinition {
	int wWidth;
	int wHeight;
	map<L2Window, WindowDefinition> wDefs;
};