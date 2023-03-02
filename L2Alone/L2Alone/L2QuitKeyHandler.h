#pragma once

#include "L2KeyboardEventHandler.h"
#include <Windows.h>

class L2QuitKeyHandler : public L2KeyboardEventHandler {
public:

	L2QuitKeyHandler(DWORD l2ProcessId) {
		this->l2ProcessId = l2ProcessId;
	}

	bool onKeyDown(KBDLLHOOKSTRUCT* kbdll) override {

		if (kbdll->vkCode == VK_ESCAPE) {
			DWORD currentTick = GetTickCount64();
			if (currentTick - lastEscapeTick < 1000) {
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, l2ProcessId);
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}

			lastEscapeTick = currentTick;
		}

		return true;
	}

private:
	DWORD lastEscapeTick = INT_MAX;
	DWORD l2ProcessId;
};