#pragma once

#include <future>
#include <map>
#include <vector>

#include "L2KeyboardEventHandler.h"
#include "L2FocusEventHandler.h"
#include "L2Events.h"
#include "logger.h"

using namespace std;

UINT L2WM_WIN_HOOK = RegisterWindowMessageA("L2Alone_WinHook");
UINT L2WM_KEY_LL_HOOK = RegisterWindowMessageA("L2Alone_KeyLowLevelHook");
UINT L2WM_MOUSE_LL_HOOK = RegisterWindowMessageA("L2Alone_MouseLowLevelHook");

class L2EventService {
public:

	void start();
	void stop();
	shared_ptr<promise<L2WindowCreatedEvent>> waitForL2Window(DWORD processId);

	void publishEventObjCreate(HWND hWnd, DWORD dwEventThread);
	void publishForegroundWindowChanged(HWND hWnd, DWORD dwEventThread);
	bool publishKeyboard(KBDLLHOOKSTRUCT* kbdll, bool keyDown);
	bool publishMouse(WPARAM wParam, MSLLHOOKSTRUCT* msll);

	void setKeyboardHandler(HWND hWindow, shared_ptr<L2KeyboardEventHandler> handler);
	void removeKeyboardHandler(HWND hWindow, shared_ptr<L2KeyboardEventHandler> handler);

	void setFocusHandler(HWND hWindow, shared_ptr<L2FocusEventHandler> handler);
	void removeFocusHandler(HWND hWindow, shared_ptr<L2FocusEventHandler> handler);

	bool waitMutex();
	void releaseMutex();

	void lockForEvents(vector<L2EventLockData> &enabledEvents);
	void releaseLockForEvents();

private:
	map<DWORD, shared_ptr<promise<L2WindowCreatedEvent>>> waitL2WindowPromises;
	map<HWND, vector<shared_ptr<L2KeyboardEventHandler>>> windowKeyHandlers;
	map<HWND, vector<shared_ptr<L2FocusEventHandler>>> windowFocusHandlers;

	DWORD tEventLoopNativeId;
	thread* tEventLoop;

	HWND activeHwnd;

	HANDLE hEventLockMutex;
	vector<L2EventLockData> eventLockData;
};

int l2EventServiceEventLoop();
void CALLBACK l2EventServiceEventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
LRESULT CALLBACK l2EventServiceLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK l2EventServiceLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
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
	
	bool propagate = true;
	
	if (nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
			propagate = eventService.publishKeyboard(pKeyboard, true);
		}
		else if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP) {
			propagate = eventService.publishKeyboard(pKeyboard, false);
		}
	}

	if (propagate) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	else {
		return 1;
	}
}

LRESULT CALLBACK l2EventServiceLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {

	bool propagate = true;

	if (nCode >= 0) {
		propagate = eventService.publishMouse(wParam, reinterpret_cast<MSLLHOOKSTRUCT*>(lParam));
	}

	if (propagate) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	else {
		return 1;
	}
}

void L2EventService::start() {

	logger.log("Start event service");

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
	HHOOK hMouseLowLevelHook = NULL;

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
					logger.log("Set Keyboard Low level hook");
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
		else if (msg.message = L2WM_MOUSE_LL_HOOK) {
			if (msg.wParam) {
				if (hMouseLowLevelHook == NULL) {
					logger.log("Set Mouse low level hook");
					hMouseLowLevelHook = SetWindowsHookEx(WH_MOUSE_LL, l2EventServiceLowLevelMouseProc, nullptr, 0);
				}
				else {
					logger.log("Mouse low level hook already set");
				}
			}
			else {
				if (hMouseLowLevelHook) {
					logger.log("Unhook mouse low level hook");
					UnhookWindowsHookEx(hMouseLowLevelHook);
				}
				else {
					logger.warn("Low level mouse hook is not set yet");
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

			DWORD createdWindowPid = GetProcessIdOfThread(hThread);
			for (auto& pair : waitL2WindowPromises) {
				if (pair.first == createdWindowPid) {

					RECT rcClient;
					GetClientRect(hWnd, &rcClient);
					int wWidth = rcClient.right - rcClient.left;
					if (wWidth < 650) {
						break;
					}

					int wHeight = rcClient.bottom - rcClient.top;
					if (wHeight < 400) {
						break;
					}

					char szWindowText[256];
					int nLength = GetWindowTextA(hWnd, szWindowText, sizeof(szWindowText));
					if (nLength == 0) {
						break;
					}

					logger.log("Window found");
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

	for (auto& pair : windowFocusHandlers) {
		if (pair.first == activeHwnd) {
			for (auto& handler : pair.second) {
				handler->onFocusReceived();
			}
		}
		else {
			for (auto& handler : pair.second) {
				handler->onFocusLost();
			}
		}
	}
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

auto& handlers = windowKeyHandlers[hWindow];
auto pErase = handlers.erase(remove_if(
	handlers.begin(),
	handlers.end(),
	[handler](shared_ptr<L2KeyboardEventHandler> ptr) {return ptr.get() == handler.get();})
);

if (handlers.size() == 0) {
	windowKeyHandlers.erase(hWindow);
}
}

void L2EventService::setFocusHandler(HWND hWindow, shared_ptr<L2FocusEventHandler> handler) {
	windowFocusHandlers[hWindow].push_back(handler);
	if (hWindow != activeHwnd) {
		handler->onFocusLost();
	}
	else {
		handler->onFocusReceived();
	}
}

void L2EventService::removeFocusHandler(HWND hWindow, shared_ptr<L2FocusEventHandler> handler) {

	if (windowFocusHandlers.count(hWindow) == 0) {
		logger.warn("There is no focus handlers for window ", hWindow);
	}

	auto& handlers = windowFocusHandlers[hWindow];
	auto pErase = handlers.erase(remove_if(
		handlers.begin(),
		handlers.end(),
		[handler](shared_ptr<L2FocusEventHandler> ptr) {return ptr.get() == handler.get();})
	);

	if (handlers.size() == 0) {
		windowFocusHandlers.erase(hWindow);
	}
}

bool L2EventService::publishKeyboard(KBDLLHOOKSTRUCT* kbdll, bool keyDown) {

	// We now lock only for mouse events, so can skip it
	if (eventLockData.size() > 0) {
		return false;
	}

	bool propagate = true;
	if (windowKeyHandlers.count(activeHwnd) > 0) {
		auto& handlers = windowKeyHandlers[activeHwnd];
		for (auto& handler : handlers) {
			if (keyDown) {
				if (!handler->onKeyDown(kbdll)) {
					propagate = false;
				}
			}
			else {
				if (!handler->onKeyUp(kbdll)) {
					propagate = false;
				}
			}
		}
	}
	return propagate;
}

bool L2EventService::publishMouse(WPARAM wParam, MSLLHOOKSTRUCT* msll) {

	if (eventLockData.size() == 0) {
		return true;
	}

	logger.log("Event lock data size: ", eventLockData.size());
	if (eventLockData.size() > 0) {
		logger.log("Check ", (HWND)wParam, " x: ", msll->pt.x, " y: ", msll->pt.y);
		for (auto& d : eventLockData) {
			if (wParam == d.mouseEventType && abs(msll->pt.x - d.x) < 2 && abs(msll->pt.y - d.y) < 2) {
				return true;
			}
		}
	}

	return false;
}

bool L2EventService::waitMutex()
{
	for (int i = 0; i < 3; ++i) {
		hEventLockMutex = OpenMutexA(SYNCHRONIZE, FALSE, "L2Alone_L2EventServiceMutex");
		if (hEventLockMutex == NULL) {
			logger.warn("Failed to open mutex, try to create mutex for L2EventService");

			hEventLockMutex = CreateMutexA(NULL, FALSE, "L2Alone_L2EventServiceMutex");
			if (hEventLockMutex == NULL) {
				logger.error("Failed to create mutex. Try again");
				continue;
			}
		}

		break;
	}

	if (hEventLockMutex == NULL) {
		logger.error("Mutex for L2EventService neither created nor opened");
		return false;
	}

	logger.log("Wait for L2EventService mutex");
	if (WaitForSingleObject(hEventLockMutex, INFINITE) != WAIT_OBJECT_0) {
		logger.error("Failed to acquire mutex");
		CloseHandle(hEventLockMutex);
		hEventLockMutex = NULL;

		return false;
	}

	logger.log("Mutex acquired");
	return true;
}

void L2EventService::releaseMutex()
{
	if (hEventLockMutex != NULL) {
		ReleaseMutex(hEventLockMutex);
		CloseHandle(hEventLockMutex);
	}
}

void L2EventService::lockForEvents(vector<L2EventLockData>& enabledEvents) {

	if (eventLockData.size() > 0) {
		throw exception("Lock for events already activated");
	}

	if (!waitMutex()) {
		logger.warn("Can't acquire mutex. Skip lock of events");
	}

	PostThreadMessage(tEventLoopNativeId, L2WM_MOUSE_LL_HOOK, 1, 0);
	eventLockData.insert(eventLockData.end(), enabledEvents.begin(), enabledEvents.end());
}

void L2EventService::releaseLockForEvents() {
	this->eventLockData.clear();
	PostThreadMessage(tEventLoopNativeId, L2WM_MOUSE_LL_HOOK, 0, 0);
	releaseMutex();
}