#pragma once

#include <Windows.h>
#include <string>
#include <future>

#include "logger.h"

using namespace std;

struct L2WindowData {
	DWORD dwProcessId;
	DWORD dwThreadId;
	HWND hWindow;
};

DWORD targetL2ProcessId;
string targetL2WindowName;
promise<L2WindowData> l2WindowFound;

void CALLBACK WinEventProc(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (dwEvent == EVENT_OBJECT_CREATE)
	{
		HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwEventThread);
		if (hThread != 0) {
			if (GetProcessIdOfThread(hThread) == targetL2ProcessId) {
				char szWindowText[256];
				int nLength = GetWindowTextA(hWnd, szWindowText, sizeof(szWindowText));

				if (string(szWindowText).find(targetL2WindowName) != string::npos) {
					logger.log("L2 window found: ", hWnd, " (", (DWORD)hWnd, ")");
					l2WindowFound.set_value(L2WindowData{ targetL2ProcessId, dwEventThread, hWnd });
				}
			}

			CloseHandle(hThread);
		}
		else {
			logger.log("Received invalid thread handle");
		}
	}
}

volatile bool tInitL2WindowComplete;

DWORD WINAPI initL2WindowDataEventLoop(LPVOID lpParameter) {
	
	HWINEVENTHOOK hEventHook = SetWinEventHook(EVENT_MIN, EVENT_MAX, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
	if (hEventHook == NULL) {
		return 1;
	}

	MSG msg;
	BOOL bRet;
	logger.log("Start event loop to init l2 window data");
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT) {
			break;
		}
	}

	UnhookWinEvent(hEventHook);
	logger.log("Complete event loop for l2 window data initialization");
	return 0;
}

L2WindowData InitL2WindowData(DWORD processId, string& windowName) {

	logger.log("Init L2 window search for process: ", processId, " and window ", windowName);
	targetL2ProcessId = processId;
	targetL2WindowName = windowName;

	HANDLE hInitThread = CreateThread(NULL, 0, initL2WindowDataEventLoop, NULL, 0, NULL);
	if (!hInitThread) {
		throw exception("Can't create L2 window searching thread");
	}

	future<L2WindowData> foundWindowFuture = l2WindowFound.get_future();
	std::future_status status = foundWindowFuture.wait_for(std::chrono::seconds(10));

	PostThreadMessage(GetThreadId(hInitThread), WM_QUIT, 0, 0);

	WaitForSingleObject(hInitThread, INFINITE);

	if (status != std::future_status::ready)
	{
		throw exception("Can't find L2 window in 10 seconds");
	}

	return foundWindowFuture.get();
}