#pragma once

#include <string>
#include <Windows.h>
#include "WindowDefinition.h"
#include "L2EventService.h"

using namespace std;

struct SelectCharacterDefinition {
	int dropDownX;
	int dropDownY;
	int dropdownItemHeight;
	int startX;
	int startY;
	int actionTimeout;
};

class AutologinStrategy {

public:
	virtual void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) = 0;

protected:

	void selectCharacter(HWND hWindow, L2CharSlot slot, SelectCharacterDefinition &def) {

		RECT r;
		GetClientRect(hWindow, &r);

		int dropdownItemHeight = def.dropdownItemHeight;
		int charDropdownOffset = slot * dropdownItemHeight;

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		int wWidth = r.right - r.left;
		int wHeight = r.bottom - r.top;

		int offsetX = refScreenWidth - def.startX;
		int offsetY = refScreenHeight - def.startY;

		int targetX, targetY;
		convertToGlobalClientRect(hWindow, wWidth - offsetX, wHeight - offsetY, targetX, targetY);

		int dropdownClickX, dropdownClickY;
		convertToGlobalClientRect(hWindow, def.dropDownX, def.dropDownY, dropdownClickX, dropdownClickY);

		int charSlotClickX, charSlotClickY;
		convertToGlobalClientRect(hWindow, def.dropDownX, def.dropDownY + charDropdownOffset, charSlotClickX, charSlotClickY);

		vector<L2EventLockData> v;
		v.push_back(L2EventLockData{ WM_MOUSEMOVE, dropdownClickX, dropdownClickY });
		v.push_back(L2EventLockData{ WM_LBUTTONDOWN, dropdownClickX, dropdownClickY });
		v.push_back(L2EventLockData{ WM_LBUTTONUP, dropdownClickX, dropdownClickY });

		v.push_back(L2EventLockData{ WM_MOUSEMOVE, charSlotClickX, charSlotClickY });
		v.push_back(L2EventLockData{ WM_LBUTTONDOWN, charSlotClickX, charSlotClickY });
		v.push_back(L2EventLockData{ WM_LBUTTONUP, charSlotClickX, charSlotClickY });

		v.push_back(L2EventLockData{ WM_MOUSEMOVE, targetX, targetY });
		v.push_back(L2EventLockData{ WM_LBUTTONDOWN, targetX, targetY });
		v.push_back(L2EventLockData{ WM_LBUTTONUP, targetX, targetY });

		POINT p;
		GetCursorPos(&p);

		eventService.lockForEvents(v);
		try {
			SetForegroundWindow(hWindow);
			SetFocus(hWindow);

			doClick(hWindow, dropdownClickX, dropdownClickY, def.actionTimeout);
			doClick(hWindow, dropdownClickX, dropdownClickY, def.actionTimeout);
			Sleep(def.actionTimeout);

			doClick(hWindow, charSlotClickX, charSlotClickY, def.actionTimeout);
			Sleep(def.actionTimeout);

			doClick(hWindow, targetX, targetY, def.actionTimeout);
		}
		catch (exception e) {
			logger.error(e.what());
			eventService.releaseLockForEvents();
			throw e;
		}

		eventService.releaseLockForEvents();
		Sleep(def.actionTimeout);
		SetCursorPos(p.x, p.y);
	}
};
