#pragma once

#include <future>
#include <map>

#include "L2KeyboardEventHandler.h"
#include "L2Events.h"
#include "logger.h"

using namespace std;

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
	
	HANDLE hEventLoop;
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

	logger.log("Complete event loop for l2 window data initialization");
}

void L2EventService::stop() {

	logger.log("Stop event service");

	if (this->tEventLoop != nullptr) {
		DWORD threadId = GetThreadId(reinterpret_cast<HANDLE>(tEventLoop->native_handle()));
		PostThreadMessage(threadId, WM_QUIT, 0, 0);
		this->tEventLoop->join();
		delete this->tEventLoop;
	}
}

int eventLoop() {
	
	HWINEVENTHOOK hEventObjectCreate = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL, EventObjCreateHandler, 0, 0, WINEVENT_OUTOFCONTEXT);

	MSG msg;
	BOOL bRet;
	logger.log("Start event loop");
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (msg.message == WM_QUIT) {
			logger.log("Stop event loop");
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

shared_ptr<promise<L2WindowCreatedEvent>> L2EventService::waitForL2Window(DWORD processId) {

	auto promiseRef = new promise<L2WindowCreatedEvent>();

	shared_ptr<promise<L2WindowCreatedEvent>> p(promiseRef);
	waitL2WindowPromises[processId] = p;

	return p;
}

void L2EventService::publishEventObjCreate(HWND hWnd, DWORD dwEventThread) {

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

		if (windowFound) {
			logger.log("L2 window found: ", hWnd);
			waitL2WindowPromises.erase(createdWindowPid);
		}
		else {
			logger.error("L2 window ", hWnd, " has been created without event handler");
		}

		CloseHandle(hThread);
	}
	else {
		logger.error("Received invalid thread handle");
	}
}

L2EventService::L2EventService() {

}

