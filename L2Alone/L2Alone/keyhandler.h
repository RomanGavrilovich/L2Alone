#pragma once

int keyboardL2ProcessId;
HWND keyboardL2Window;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        
        // Check if a key has been pressed down
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            // Do something when a key is pressed down
            // For example, print the virtual key code to the console
            //printf("Virtual Key Code: %d\n", pKeyboard->vkCode);
        }
    }

    // Pass the message to the next hook procedure
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void CALLBACK ForegroundCheckHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND)
    {
        HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwEventThread);
        if (hThread != 0) {
            int currentPid = GetProcessIdOfThread(hThread);
            if (currentPid == keyboardL2ProcessId) {
                cout << "L2 process foreground" << endl;
            }
            else {
                cout << "L2 process background" << endl;
            }
            CloseHandle(hThread);
        }
        else {
            logger.log("Received invalid thread handle");
        }
    }
}

void StartKeyboardCaptuing(int l2ProcessId, HWND hL2Window) {
	
    keyboardL2ProcessId = l2ProcessId;
	keyboardL2Window = hL2Window;

    // Install the WinEventHook
    HWINEVENTHOOK hHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, ForegroundCheckHook, 0, 0, WINEVENT_OUTOFCONTEXT);
    SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!hHook) {
        throw exception("Can't register foreground checking hook");
    }

    // Enter a message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Uninstall the WinEventHook
    UnhookWinEvent(hHook);
}