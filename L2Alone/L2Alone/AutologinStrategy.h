#pragma once

#include <string>
#include <Windows.h>
#include "WindowDefinition.h"
#include "L2EventService.h"

using namespace std;

class AutologinStrategy {

public:
	virtual void doAutologin(HWND hWindow, string& login, string& password, L2CharSlot slot) = 0;

protected:

	void selectCharacter(HWND hWindow, L2CharSlot slot, int refX, int refY) {

		RECT r;
		GetClientRect(hWindow, &r);

		int dropdownItemHeight = 12;
		int charDropdownOffset = (slot - 1) * 12;

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		int wWidth = r.right - r.left;
		int wHeight = r.bottom - r.top;

		int offsetX = refScreenWidth - refX;
		int offsetY = refScreenHeight - refY;

		int targetX, targetY;
		convertToGlobalClientRect(hWindow, wWidth - offsetX, wHeight - offsetY, targetX, targetY);

		logger.log("Target x: ", targetX, " target y: ", targetY);

		int dropdownClickX, dropdownClickY;
		convertToGlobalClientRect(hWindow, 198, 48, dropdownClickX, dropdownClickY);

		int charSlotClickX, charSlotClickY;
		convertToGlobalClientRect(hWindow, 124, 58 + charDropdownOffset, charSlotClickX, charSlotClickY);

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

		int actionTimeout = 2000;
		eventService.lockForEvents(v);
		try {
			SetForegroundWindow(hWindow);

			Sleep(actionTimeout);

			POINT p;
			GetCursorPos(&p);

			doClick(hWindow, dropdownClickX, dropdownClickY);
			Sleep(actionTimeout);

			doClick(hWindow, dropdownClickX, dropdownClickY);

			//doClick(hWindow, charSlotClickX, charSlotClickY);
			///Sleep(actionTimeout);

			//doClick(hWindow, targetX, targetY);

			//Sleep(actionTimeout);
			//SetCursorPos(p.x, p.y);
		}
		catch (exception e) {
			logger.error(e.what());
			eventService.releaseLockForEvents();
			throw e;
		}

		eventService.releaseLockForEvents();
	}
};
