#pragma once

#include <string>
#include <Windows.h>
#include "WindowDefinition.h"

using namespace std;

class AutologinStrategy {

public:
	virtual void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) = 0;
};
