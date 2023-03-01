#pragma once

#include <future>
#include <map>

#include "L2KeyboardEventHandler.h"
#include "L2Events.h"
#include "logger.h"

using namespace std;

UINT L2WM_WIN_HOOK = RegisterWindowMessageA("L2Alone_WinHook");

class L2EventService {
public:
	
	L2EventService();
	void start(string l2WindowName);
	void stop();
	shared_ptr<promise<L2WindowCreatedEvent>> waitForL2Window(DWORD processId);
	void publishEventObjCreate(HWND hWnd, DWORD dwEventThread);

private:
	string l2WindowName;
	map<DWORD, shared_ptr<promise<L2WindowCreatedEvent>>> waitL2WindowPromises;
	
	DWORD tEventLoopNativeId;
	thread* tEventLoop;
};

int eventLoop();
void CALLBACK EventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

// Impl

L2EventService eventService;

void CALLBACK EventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (dwEvent == EVENT_OBJECT_CREATE)
	{
		eventService.publishEventObjCreate(hWnd, dwEventThread);
	}
}

void L2EventService::start(string l2WindowName) {

	logger.log("Start event service");

	this->l2WindowName = l2WindowName;

	tEventLoop = new thread(eventLoop);
	tEventLoopNativeId = GetThreadId(reinterpret_cast<HANDLE>(tEventLoop->native_handle()));

	logger.log("Complete event loop for l2 window data initialization");
}

void L2EventService::stop() {

	logger.log("Stop event service");

	if (this->tEventLoop != nullptr) {
		PostThreadMessage(tEventLoopNativeId, WM_QUIT, 0, 0);
		this->tEventLoop->join();
		delete this->tEventLoop;
	}
}

int eventLoop() {
	
	HWINEVENTHOOK hEventObjectCreate = NULL;

	MSG msg;
	BOOL bRet;
	logger.log("Start event loop");
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (msg.message == L2WM_WIN_HOOK) {
			if (msg.wParam) {
				logger.log("Set EVENT_OBJECT_CREATE hook");
				hEventObjectCreate = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL, EventObjCreateHandler, 0, 0, WINEVENT_OUTOFCONTEXT);
			}
			else {
				if (hEventObjectCreate) {
					logger.log("Unhook EVENT_OBJECT_CREATE");
					UnhookWinEvent(hEventObjectCreate);
				}
				else {
					logger.warn("EVENT_OBJECT_CREATE was not set");
				}
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!bRet) {
		logger.log("Exit event loop");
	}
	else {
		logger.log("Event loop failed: ", bRet);
	}

	return 0;
}

shared_ptr<promise<L2WindowCreatedEvent>> L2EventService::waitForL2Window(DWORD processId) {

	PostThreadMessage(tEventLoopNativeId, L2WM_WIN_HOOK, 1, 0);

	auto promiseRef = new promise<L2WindowCreatedEvent>();

	shared_ptr<promise<L2WindowCreatedEvent>> p(promiseRef);
	waitL2WindowPromises[processId] = p;

	return p;
}

void L2EventService::publishEventObjCreate(HWND hWnd, DWORD dwEventThread) {

	try {
		HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwEventThread);
		if (hThread != 0) {

			bool windowFound = false;

			char szWindowText[256];
			int nLength = GetWindowTextA(hWnd, szWindowText, sizeof(szWindowText));
			if (string(szWindowText).find(l2WindowName) == string::npos) {
				return;
			}

			DWORD createdWindowPid = GetProcessIdOfThread(hThread);
			for (auto& pair : waitL2WindowPromises) {
				if (pair.first == createdWindowPid) {
					pair.second->set_value(L2WindowCreatedEvent{ hWnd, createdWindowPid, dwEventThread });
					windowFound = true;
					break;
				}
			}

			CloseHandle(hThread);

			if (windowFound) {
				logger.log("L2 window found: ", hWnd);
				waitL2WindowPromises.erase(createdWindowPid);

				PostThreadMessage(tEventLoopNativeId, L2WM_WIN_HOOK, 0, 0);
			}
			else {
				logger.error("L2 window ", hWnd, " has been created without event handler");
			}
		}
		else {
			logger.error("Received invalid thread handle");
		}
	}
	catch (exception e) {
		logger.error("Error during publish event obj create: ", e.what());
	}
}

L2EventService::L2EventService() {

}

