#pragma once

#include <Windows.h>
#include <vector>

using namespace std;

struct WindowLayoutConfig {
	int monitor;
	bool fullScreen;
	int x;
	int y;
	int width;
	int height;
};

struct LayoutConfig {
	vector<WindowLayoutConfig> windowConfigs;
};