#pragma once

enum L2Window {
	UNKNOWN,
	WELCOME,
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

struct ButtonDefinition {
	int rtX;
	int rtY;
	int width;
	int height;
	RefAnchor anchor = RefAnchor::Center;
};

struct WindowDefinition {
	int width = 0;
	int height = 0;
	vector<ButtonDefinition> bDefs;
	int textMinSize = 0;
	int textMaxSize = 0;
};