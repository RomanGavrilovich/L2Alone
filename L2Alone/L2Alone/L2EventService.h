#pragma once

#include <future>
#include <map>
#include <vector>

#include "L2KeyboardEventHandler.h"
#include "L2Events.h"
#include "logger.h"

using namespace std;

UINT L2WM_WIN_HOOK = RegisterWindowMessageA("L2Alone_WinHook");
UINT L2WM_KEY_LL_HOOK = RegisterWindowMessageA("L2Alone_KeyLowLevelHook");

class L2EventService {
public:
	
	L2EventService();
	void start(string l2WindowName);
	void stop();
	shared_ptr<promise<L2WindowCreatedEvent>> waitForL2Window(DWORD processId);
	
	void publishEventObjCreate(HWND hWnd, DWORD dwEventThread);
	void publishForegroundWindowChanged(HWND hWnd, DWORD dwEventThread);
	void publishKeyboard(KBDLLHOOKSTRUCT* kbdll, bool keyDown);

	void setKeyboardHandler(HWND hWindow, shared_ptr<L2KeyboardEventHandler> handler);
	void removeKeyboardHandler(HWND hWindow, shared_ptr<L2KeyboardEventHandler> handler);

private:
	string l2WindowName;
	map<DWORD, shared_ptr<promise<L2WindowCreatedEvent>>> waitL2WindowPromises;
	map<HWND, vector<shared_ptr<L2KeyboardEventHandler>>> windowKeyHandlers;
	
	DWORD tEventLoopNativeId;
	thread* tEventLoop;

	HWND activeHwnd;
};

int l2EventServiceEventLoop();
void CALLBACK l2EventServiceEventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
LRESULT CALLBACK l2EventServiceLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void CALLBACK L2EventServiceForegroundCheckHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

// Impl

L2EventService eventService;

void CALLBACK l2EventServiceEventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (dwEvent == EVENT_OBJECT_CREATE)
	{
		eventService.publishEventObjCreate(hWnd, dwEventThread);
	}
}

void CALLBACK L2EventServiceForegroundCheckHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (event == EVENT_SYSTEM_FOREGROUND)
	{
		eventService.publishForegroundWindowChanged(hwnd, dwEventThread);
	}
}

LRESULT CALLBACK l2EventServiceLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
			eventService.publishKeyboard(pKeyboard, true);
		}
		else if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP) {
			eventService.publishKeyboard(pKeyboard, false);
		}
		else {
			logger.log("Received unexpected WM param", wParam);
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void L2EventService::start(string l2WindowName) {

	logger.log("Start event service");

	this->l2WindowName = l2WindowName;

	tEventLoop = new thread(l2EventServiceEventLoop);
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

int l2EventServiceEventLoop() {
	
	HWINEVENTHOOK hForegroundHook = SetWinEventHook(
		EVENT_SYSTEM_FOREGROUND, 
		EVENT_SYSTEM_FOREGROUND, 
		NULL, 
		L2EventServiceForegroundCheckHook, 
		0, 
		0, 
		WINEVENT_OUTOFCONTEXT);
	HWINEVENTHOOK hEventObjectCreate = NULL;
	HHOOK hKeyLowLevelHook = NULL;

	MSG msg;
	BOOL bRet;
	logger.log("Start event loop");
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (msg.message == L2WM_WIN_HOOK) {
			if (msg.wParam) {
				logger.log("Set EVENT_OBJECT_CREATE hook");
				hEventObjectCreate = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL, l2EventServiceEventObjCreateHandler, 0, 0, WINEVENT_OUTOFCONTEXT);
			}
			else {
				if (hEventObjectCreate) {
					logger.log("Unhook EVENT_OBJECT_CREATE");
					UnhookWinEvent(hEventObjectCreate);
					hEventObjectCreate = NULL;
				}
				else {
					logger.warn("EVENT_OBJECT_CREATE was not set");
				}
			}
		}
		else if (msg.message == L2WM_KEY_LL_HOOK) {
			if (msg.wParam) {
				if (hKeyLowLevelHook == NULL) {
					logger.log("Set Keyboard Low level Hook");
					hKeyLowLevelHook = SetWindowsHookEx(WH_KEYBOARD_LL, l2EventServiceLowLevelKeyboardProc, nullptr, 0);
				}
				else {
					logger.log("Keyboard low level hook already set");
				}
			}
			else {
				if (hKeyLowLevelHook) {
					logger.log("Unhook low level keyboard hook");
					UnhookWindowsHookEx(hKeyLowLevelHook);
					hKeyLowLevelHook = NULL;
				}
				else {
					logger.warn("Low level keybard hook is not set yet");
				}
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (hEventObjectCreate) {
		logger.warn("EVENT_OBJECT_CREATE hook was not unhooked properly");
		UnhookWinEvent(hEventObjectCreate);
		hEventObjectCreate = NULL;
	}

	if (hKeyLowLevelHook) {
		logger.warn("Low level keybard hook was not unhooked properly");
		UnhookWindowsHookEx(hKeyLowLevelHook);
		hKeyLowLevelHook = NULL;
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

void L2EventService::publishForegroundWindowChanged(HWND hWindow, DWORD dwEventThread) {
	activeHwnd = hWindow;
}

void L2EventService::setKeyboardHandler(HWND hWindow, shared_ptr<L2KeyboardEventHandler> handler) {
	
	if (windowKeyHandlers.size() == 0) {
		PostThreadMessage(tEventLoopNativeId, L2WM_KEY_LL_HOOK, 1, 0);
	}

	windowKeyHandlers[hWindow].push_back(handler);
}

void L2EventService::removeKeyboardHandler(HWND hWindow, shared_ptr<L2KeyboardEventHandler> handler) {

	if (windowKeyHandlers.count(hWindow) == 0) {
		logger.warn("There is no key handlers for window ", hWindow);
	}

	auto &handlers = windowKeyHandlers[hWindow];
	auto pErase = handlers.erase(remove_if(
		handlers.begin(),
		handlers.end(),
		[handler](shared_ptr<L2KeyboardEventHandler> ptr) {return ptr.get() == handler.get();})
	);

	if (pErase != handlers.end()) {
		logger.log("Handler has been removed");
	}
	else {
		logger.warn("Handler was not found");
	}
}

void L2EventService::publishKeyboard(KBDLLHOOKSTRUCT* kbdll, bool keyDown) {

	logger.log("Received input with active window ", activeHwnd);
	if (windowKeyHandlers.count(activeHwnd) > 0) {
		auto &handlers = windowKeyHandlers[activeHwnd];
		for (auto& handler : handlers) {
			if (keyDown) {
				handler->onKeyDown(kbdll);
			}
			else {
				handler->onKeyUp(kbdll);
			}
		}
	}
	else {
		logger.log("No window key handlers found for ", activeHwnd);
	}
}

L2EventService::L2EventService() {
}

