#pragma once

#include <Windows.h>

#include "L2KeyboardEventHandler.h"
#include "config_utils.h"

class L2QuitKeyHandler : public L2KeyboardEventHandler {
public:

	L2QuitKeyHandler(DWORD l2ProcessId, vector<L2AccountHotKey> *accountHotKeys) {
		this->l2ProcessId = l2ProcessId;
		this->accountHotKeys = accountHotKeys;
	}

	bool onKeyDown(KBDLLHOOKSTRUCT* kbdll) override {

		int vkCode = kbdll->vkCode;
		if ((VK_F1 <= vkCode && vkCode <= VK_F12) && isKeyPressed(VK_ESCAPE)) {
			int index = 0;
			int fKey = (vkCode - VK_F1) + 1;

			for (auto& ahk : *accountHotKeys) {
				if (ahk.fKey == fKey) {
					int exitCode = getExitHotKeyCode(index);
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, l2ProcessId);
					TerminateProcess(hProcess, (UINT)exitCode);
					CloseHandle(hProcess);
					return false;
				}

				index++;
			}

			logger.log("Hot key not found for F", fKey);
		}

		if (kbdll->vkCode == VK_ESCAPE) {
			ULONGLONG currentTick = GetTickCount64();
			if (currentTick - lastEscapeTick < 100) {
				escapeCount++;
			}
			else {
				escapeCount = 0;
			}

			if (escapeCount == 1) {
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, l2ProcessId);
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}
		}
		else {
			escapeCount = 0;
		}

		return true;
	}

	bool onKeyUp(KBDLLHOOKSTRUCT* kbdll) override {
		
		if (kbdll->vkCode == VK_ESCAPE) {
			lastEscapeTick = GetTickCount64();
		}

		return true;
	}

private:
	ULONGLONG lastEscapeTick = LONG_MAX;
	int escapeCount = 0;
	DWORD l2ProcessId;
	vector<L2AccountHotKey> *accountHotKeys;
};