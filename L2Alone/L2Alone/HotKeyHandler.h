#pragma once

#include "L2KeyboardEventHandler.h"

class L2HotKeyHandler : public L2KeyboardEventHandler {
public:

	L2HotKeyHandler(DWORD l2ProcessId) {
		this->l2ProcessId = l2ProcessId;
	}

	void onKeyDown(KBDLLHOOKSTRUCT* kbdll) override {

		if (kbdll->vkCode == VK_ESCAPE) {
			DWORD currentTick = GetTickCount64();
			if (currentTick - lastEscapeTick < 1000) {
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, l2ProcessId);
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}

			lastEscapeTick = currentTick;
		}
	}

	void onKeyUp(KBDLLHOOKSTRUCT* kbdll) override {
	}

private:
	DWORD lastEscapeTick = INT_MAX;
	DWORD l2ProcessId;
};