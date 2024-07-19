#pragma once

#include <future>
#include <map>
#include <vector>

#include "L2KeyboardEventHandler.h"
#include "L2FocusEventHandler.h"
#include "L2Events.h"
#include "logger.h"
#include "L2EventHandlers.h"

using namespace std;

UINT L2WM_WIN_PCHANGE_HOOK = RegisterWindowMessageA("L2Alone_WinHook_PosChange");
UINT L2WM_KEY_LL_HOOK = RegisterWindowMessageA("L2Alone_KeyLowLevelHook");
UINT L2WM_MOUSE_LL_HOOK = RegisterWindowMessageA("L2Alone_MouseLowLevelHook");

class L2EventService {
public:

	void start();
	void stop();
	shared_ptr<promise<L2WindowCreatedEvent>> waitForL2Window(DWORD processId);

	void publishEventObjCreate(HWND hWnd, DWORD dwEventThread);
	void publishForegroundWindowChanged(HWND hWnd, DWORD dwEventThread);
	void publishWinPosChange(HWND hWnd, DWORD dwEventThread);
	bool publishKeyboard(KBDLLHOOKSTRUCT* kbdll, bool keyDown);
	bool publishMouse(WPARAM wParam, MSLLHOOKSTRUCT* msll);

	void setKeyboardHandler(DWORD processId, shared_ptr<L2KeyboardEventHandler> handler);
	void removeKeyboardHandler(DWORD processId, shared_ptr<L2KeyboardEventHandler> handler);

	void setFocusHandler(DWORD processId, shared_ptr<L2FocusEventHandler> handler);
	void removeFocusHandler(DWORD processId, shared_ptr<L2FocusEventHandler> handler);

	void setWindowRectChangeHandler(HWND hWindow, shared_ptr<L2WindowRectChangeHandler> handler);
	void removeWindowRectChange(HWND hWindow, shared_ptr<L2WindowRectChangeHandler> handler);

	void setWindowCreateHandler(DWORD dwProcessId, shared_ptr<L2WindowCreateHandler> handler);
	void removeWindowCreateHandler(DWORD dwProcessId, shared_ptr<L2WindowCreateHandler> handler);

	bool waitMutex();
	void releaseMutex();

	void lockForEvents(vector<L2EventLockData>& enabledEvents);
	void releaseLockForEvents();
	void l2EventServiceEventLoop();

private:
	map<DWORD, shared_ptr<promise<L2WindowCreatedEvent>>> waitL2WindowPromises;
	map<DWORD, vector<shared_ptr<L2KeyboardEventHandler>>> windowKeyHandlers;
	map<DWORD, vector<shared_ptr<L2FocusEventHandler>>> windowFocusHandlers;
	map<HWND, vector<shared_ptr<L2WindowRectChangeHandler>>> windowRectChangeHandlers;
	map<DWORD, vector<shared_ptr<L2WindowCreateHandler>>> windowCreateHandlers;

	DWORD tEventLoopNativeId;
	thread* tEventLoop;

	DWORD dwActiveProcess = 0;

	HANDLE hEventLockMutex;
	vector<L2EventLockData> eventLockData;

	promise<bool> startCompleted;

	bool notifyWindowCreateHandlers(HWND hWnd, DWORD dwProcessId);
};

void CALLBACK l2EventServiceEventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
LRESULT CALLBACK l2EventServiceLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK l2EventServiceLowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
void CALLBACK L2EventServiceForegroundCheckHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

// Impl

L2EventService eventService;

void CALLBACK l2EventServiceEventObjCreateHandler(HWINEVENTHOOK hEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (dwEvent == EVENT_OBJECT_CREATE) {
		eventService.publishEventObjCreate(hWnd, dwEventThread);
	}
}

void CALLBACK L2EventServiceForegroundCheckHook(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	eventService.publishForegroundWindowChanged(hwnd, dwEventThread);
}

void CALLBACK l2EventServiceWinPosChange(HWINEVENTHOOK hHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
	eventService.publishWinPosChange(hwnd, dwEventThread);
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

	tEventLoop = new thread(&L2EventService::l2EventServiceEventLoop, this);
	tEventLoopNativeId = GetThreadId(reinterpret_cast<HANDLE>(tEventLoop->native_handle()));

	startCompleted.get_future().wait();

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

void L2EventService::l2EventServiceEventLoop() {

	HWINEVENTHOOK hForegroundHook = SetWinEventHook(
		EVENT_OBJECT_FOCUS,
		EVENT_OBJECT_FOCUS,
		NULL,
		L2EventServiceForegroundCheckHook,
		0,
		0,
		WINEVENT_OUTOFCONTEXT);

	HWINEVENTHOOK hEventObjectCreate = SetWinEventHook(
		EVENT_OBJECT_CREATE,
		EVENT_OBJECT_CREATE,
		NULL,
		l2EventServiceEventObjCreateHandler,
		0,
		0,
		WINEVENT_OUTOFCONTEXT
	);

	startCompleted.set_value(true);

	HWINEVENTHOOK hEventPosChange = NULL;
	HHOOK hKeyLowLevelHook = NULL;
	HHOOK hMouseLowLevelHook = NULL;

	MSG msg;
	BOOL bRet;
	logger.log("Start event loop");
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (msg.message == L2WM_WIN_PCHANGE_HOOK) {
			if (msg.wParam) {
				logger.log("Set EVENT_OBJECT_LOCATIONCHANGE hook");
				hEventPosChange = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, NULL, l2EventServiceWinPosChange, 0, 0, WINEVENT_OUTOFCONTEXT);
			}
			else {
				if (hEventPosChange) {
					logger.log("Unhook EVENT_OBJECT_LOCATIONCHANGE");
					UnhookWinEvent(hEventPosChange);
					hEventPosChange = NULL;
				}
				else {
					logger.warn("EVENT_OBJECT_LOCATIONCHANGE was not set");
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
}

shared_ptr<promise<L2WindowCreatedEvent>> L2EventService::waitForL2Window(DWORD processId) {

	auto promiseRef = new promise<L2WindowCreatedEvent>();

	shared_ptr<promise<L2WindowCreatedEvent>> p(promiseRef);
	waitL2WindowPromises[processId] = p;

	return p;
}

bool L2EventService::notifyWindowCreateHandlers(HWND hWnd, DWORD dwProcessId) {

	if (windowCreateHandlers.count(dwProcessId) > 0) {
		auto& handlers = windowCreateHandlers[dwProcessId];
		for (auto& handler : handlers) {
			handler->onWindowCreate(hWnd);
		}

		return true;
	}

	return false;
}

void L2EventService::publishEventObjCreate(HWND hWnd, DWORD dwEventThread) {

	try {
		HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwEventThread);
		if (hThread != 0) {

			bool windowFound = false;

			DWORD createdWindowPid = GetProcessIdOfThread(hThread);

			if (notifyWindowCreateHandlers(hWnd, createdWindowPid)) {
				return;
			}

			for (auto& pair : waitL2WindowPromises) {
				if (pair.first == createdWindowPid) {

					RECT rcClient;
					GetClientRect(hWnd, &rcClient);
					int wWidth = rcClient.right - rcClient.left;
					if (wWidth < 650) {
						postControlMessage(hWnd, VK_RETURN);
						break;
					}

					int wHeight = rcClient.bottom - rcClient.top;
					if (wHeight < 400) {
						postControlMessage(hWnd, VK_RETURN);
						break;
					}

					char szWindowText[256];
					int nLength = GetWindowTextA(hWnd, szWindowText, sizeof(szWindowText));
					if (nLength == 0) {
						postControlMessage(hWnd, VK_RETURN);
						break;
					}

					logger.log("L2 Main window found: ", hWnd);
					pair.second->set_value(L2WindowCreatedEvent{ hWnd, createdWindowPid, dwEventThread });
					windowFound = true;
					break;
				}
			}

			CloseHandle(hThread);

			if (windowFound) {
				waitL2WindowPromises.erase(createdWindowPid);
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

void L2EventService::publishWinPosChange(HWND hWindow, DWORD dwEventThread) {

	if (windowRectChangeHandlers.count(hWindow) > 0) {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWindow, &wp);

		auto& handlers = windowRectChangeHandlers[hWindow];
		for (auto& handler : handlers) {
			handler->onWindowChange(wp.rcNormalPosition);
		}
	}
}

void L2EventService::publishForegroundWindowChanged(HWND hWindow, DWORD dwEventThread) {

	DWORD dwFocusProcess;
	HWND foregroundWindow = GetForegroundWindow();
	GetWindowThreadProcessId(foregroundWindow, &dwFocusProcess);

	logger.log("Foreground window changed: ", foregroundWindow, ". Of process ", dwFocusProcess);

	if (dwFocusProcess != dwActiveProcess) {
		dwActiveProcess = dwFocusProcess;

		logger.log("dwActiveProcess updated ", dwActiveProcess);

		for (auto& pair : windowFocusHandlers) {
			if (pair.first == dwActiveProcess) {
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
}

void L2EventService::setWindowCreateHandler(DWORD dwProcessId, shared_ptr<L2WindowCreateHandler> handler) {
	windowCreateHandlers[dwProcessId].push_back(handler);
}

void L2EventService::removeWindowCreateHandler(DWORD dwProcessId, shared_ptr<L2WindowCreateHandler> handler) {
	
	if (windowCreateHandlers.count(dwProcessId) == 0) {
		logger.warn("There is no window create handlers for process ", dwProcessId);
		return;
	}

	auto& handlers = windowCreateHandlers[dwProcessId];
	auto pErase = handlers.erase(remove_if(
		handlers.begin(),
		handlers.end(),
		[handler](shared_ptr<L2WindowCreateHandler> ptr) {return ptr.get() == handler.get();})
	);
	if (handlers.size() == 0) {
		windowCreateHandlers.erase(dwProcessId);
	}
}

void L2EventService::setWindowRectChangeHandler(HWND hWindow, shared_ptr<L2WindowRectChangeHandler> handler) {

	if (windowRectChangeHandlers.size() == 0) {
		PostThreadMessage(tEventLoopNativeId, L2WM_WIN_PCHANGE_HOOK, 1, 0);
	}

	windowRectChangeHandlers[hWindow].push_back(handler);
}

void L2EventService::removeWindowRectChange(HWND hWindow, shared_ptr<L2WindowRectChangeHandler> handler) {

	if (windowRectChangeHandlers.count(hWindow) == 0) {
		logger.warn("There is no window pos change handlers for window ", hWindow);
		return;
	}

	auto& handlers = windowRectChangeHandlers[hWindow];
	auto pErase = handlers.erase(remove_if(
		handlers.begin(),
		handlers.end(),
		[handler](shared_ptr<L2WindowRectChangeHandler> ptr) {return ptr.get() == handler.get();})
	);

	if (handlers.size() == 0) {
		windowRectChangeHandlers.erase(hWindow);
	}

	if (windowRectChangeHandlers.size() == 0) {
		PostThreadMessage(tEventLoopNativeId, L2WM_WIN_PCHANGE_HOOK, 0, 0);
	}
}

void L2EventService::setKeyboardHandler(DWORD processId, shared_ptr<L2KeyboardEventHandler> handler) {

	if (windowKeyHandlers.size() == 0) {
		PostThreadMessage(tEventLoopNativeId, L2WM_KEY_LL_HOOK, 1, 0);
	}

	windowKeyHandlers[processId].push_back(handler);
}

void L2EventService::removeKeyboardHandler(DWORD processId, shared_ptr<L2KeyboardEventHandler> handler) {

	if (windowKeyHandlers.count(processId) == 0) {
		logger.warn("There is no key handlers for window process: ", processId);
	}

	auto& handlers = windowKeyHandlers[processId];
	auto pErase = handlers.erase(remove_if(
		handlers.begin(),
		handlers.end(),
		[handler](shared_ptr<L2KeyboardEventHandler> ptr) {return ptr.get() == handler.get();})
	);

	if (handlers.size() == 0) {
		windowKeyHandlers.erase(processId);
	}
}

void L2EventService::setFocusHandler(DWORD processId, shared_ptr<L2FocusEventHandler> handler) {
	windowFocusHandlers[processId].push_back(handler);

	if (processId != dwActiveProcess) {
		handler->onFocusLost();
	}
	else {
		handler->onFocusReceived();
	}
}

void L2EventService::removeFocusHandler(DWORD processId, shared_ptr<L2FocusEventHandler> handler) {

	if (windowFocusHandlers.count(processId) == 0) {
		logger.warn("There is no focus handlers for process ", processId);
	}

	auto& handlers = windowFocusHandlers[processId];
	auto pErase = handlers.erase(remove_if(
		handlers.begin(),
		handlers.end(),
		[handler](shared_ptr<L2FocusEventHandler> ptr) {return ptr.get() == handler.get();})
	);

	if (handlers.size() == 0) {
		windowFocusHandlers.erase(processId);
	}
}

bool L2EventService::publishKeyboard(KBDLLHOOKSTRUCT* kbdll, bool keyDown) {

	// We now lock only for mouse events, so can skip it
	if (eventLockData.size() > 0) {
		return false;
	}

	// TODO: Remove
	if (isKeyPressed(VK_ESCAPE)) {
		logger.log("Press escape with active process: ", dwActiveProcess);
		HWND foregroundWindow = GetForegroundWindow();
		DWORD foregroundProcessId;
		GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId);

		dwActiveProcess = foregroundProcessId;
	}

	bool propagate = true;
	if (windowKeyHandlers.count(dwActiveProcess) > 0) {
		auto& handlers = windowKeyHandlers[dwActiveProcess];
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