// L2Alone.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#ifndef TEST

#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <tlhelp32.h>
#include <winuser.h>
#include <exception>
#include <string>

#include "config_utils.h"
#include "Utils.h"

#include "Logger.h"
#include "L2EventService.h"
#include "L2QuitKeyHandler.h"
#include "L2PvpModeHandler.h"
#include "AutologinStrategy.h"
#include "C5AutologinStrategy.h"
#include "C2AutoLoginStrategy.h"
#include "LayoutManager.h"
#include "LayoutManagerUpdateHandler.h"
#include "GlobalFileLayoutCache.h"
#include "InMemoryLayoutCache.h"

using namespace std;

struct FindL2WindowParams {
	string windowName;
	DWORD dwProcess;
};

#define APP_NAME "L2Alone"
#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"

string getLogFilePath(string absFilePath);
void autoLoginL2(string login, string password, L2CharSlot slot, L2AloneConfig& config, AutologinStrategy* s);

void showMessage(string message);
bool isRunnedFromExe(string process);
L2WindowCreatedEvent waitL2WindowCreated(int processId, int timeoutMs);

int main(int argc, char* argv[])
{
	try {

#ifdef L2A_RELEASE
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, SW_HIDE);
#endif // L2A_RELEASE

		L2CharSlot slot = L2CharSlot::ACTIVE;

		if (argc < 2) {
			showMessage("Login is not provided");
			return 1;
		}

		if (argc < 3) {
			showMessage("Password is not provided");
			return 1;
		}

		if (argc > 3) {
			try {
				int slotValue = stoi(argv[3]);
				if (1 <= slotValue && slotValue <= 7) {
					slot = (L2CharSlot)slotValue;
				}
				else {
					throw exception("Incorrect char slot value");
				}
			}
			catch (exception e) {
				stringstream ss;
				ss << "Incorrect char slot index: " << argv[3] << ". Expected index in range [1,7]";
				showMessage(ss.str());
				return 1;
			}
		}

		stringstream ss;

		L2AloneConfig config = loadL2AloneConfig();
		if (config.debugEnabled) {
			ss << "Debug/" << GetCurrentProcessId();
			prepareDirectory("Debug");
			prepareDirectory(ss.str());

			config.debugBmpPath = ss.str() + "/Captures";
			if (config.debugSaveRefScreen || config.debugSaveWcFailures) {
				prepareDirectory(config.debugBmpPath);
			}
		}

		if (config.debugLogEnabled) {
			logger.open((ss.str() + "/L2Alone.log").c_str());
		}

		eventService.start();

		string account = argv[1];
		string password = argv[2];

		AutologinStrategy *autologinStrategy;
		if (config.version == C2) {
			autologinStrategy = new C2AutologinStrategy(config);
		}
		else if (config.version == C5) {
			autologinStrategy = new C5AutologinStrategy(config);
		}
		else {
			throw exception("Unsupported L2 version");
		}

		autoLoginL2(account, password, slot, config, autologinStrategy);

		return 0;
	}
	catch (exception e) {
		cout << "ERROR: " << e.what() << endl;
		MessageBoxA(NULL, e.what(), APP_NAME, MB_OK);

		eventService.stop();

		return 1;
	}
}

void showMessage(string message) {
	MessageBoxA(NULL, message.c_str(), APP_NAME, MB_OK);
}

void autoLoginL2(string login, string password, L2CharSlot slot, L2AloneConfig& config, AutologinStrategy *autologinStrategy) {

	shared_ptr<LayoutCache> fileCache(new GlobalFileLayoutCache());
	shared_ptr<LayoutCache> layoutCache(new InMemoryLayoutCache(fileCache));
	shared_ptr<LayoutManager> layoutManager(new LayoutManager(layoutCache));

	auto lastFailureTime = GetTickCount64();
	int minDelayBetweenRecovery = 60000; // 1 min

	while (true) {
		HANDLE hProcess;
		DWORD dwProcessId;
		startProcess(config.pathToL2, hProcess, dwProcessId);

		try {
			logger.log("L2 process started with id: ", dwProcessId);

			L2WindowCreatedEvent d = waitL2WindowCreated(dwProcessId, config.visionInitTimeout);
			if (config.centerWindow) {
				centerWindow(d.hWindow);
			}

			auto hotKeyHandler = shared_ptr<L2QuitKeyHandler>(new L2QuitKeyHandler(d.processId, config.quitEnabled, &config.accountHotKeys));
			eventService.setKeyboardHandler(d.processId, hotKeyHandler);

			auto pvpHandler = shared_ptr<L2PvpModeHandler>(new L2PvpModeHandler());
			eventService.setKeyboardHandler(d.processId, pvpHandler);
			eventService.setFocusHandler(d.processId, pvpHandler);

			autologinStrategy->doAutologin(d.hWindow, login, password, slot);

			auto layoutManagerUpdater = shared_ptr<L2WindowRectChangeHandler>(new LayoutManagerUpdateHandler(layoutManager));

			if (config.layoutManagerEnabled) {
				RECT layout;
				if (layoutManager->getWindowLayout(layout)) {
					WINDOWPLACEMENT wp;
					wp.length = sizeof(WINDOWPLACEMENT);
					GetWindowPlacement(d.hWindow, &wp);

					wp.rcNormalPosition = layout;
					SetWindowPlacement(d.hWindow, &wp);
				}
			}

			if (config.layoutManagerEnabled) {
				eventService.setWindowRectChangeHandler(d.hWindow, layoutManagerUpdater);
			}

			WaitForSingleObject(hProcess, INFINITE);

			if (config.layoutManagerEnabled) {
				eventService.removeWindowRectChange(d.hWindow, layoutManagerUpdater);
			}

			eventService.removeKeyboardHandler(d.processId, hotKeyHandler);
			eventService.removeKeyboardHandler(d.processId, pvpHandler);
			eventService.removeFocusHandler(d.processId, pvpHandler);

			DWORD exitCode;
			if (!GetExitCodeProcess(hProcess, &exitCode)) {
				throw exception("Can't run new window on hot key, because can't get previous process exit code");
			}

			if (config.crashRecoveryEnabled) {
				if (exitCode == 1) {
					if (GetTickCount64() - lastFailureTime > minDelayBetweenRecovery) {
						logger.log("Recovery on crash");
						lastFailureTime = GetTickCount64();
						continue;
					}
					else {
						throw exception("L2 client has issue leading to permanent crashes");
					}
				}
			}

			if (isExitHotKeyCode(exitCode)) {
				logger.log("Swap L2 window");

				int nextCharIndex = getNextHotKeyIndex(exitCode);
				auto nextConfig = config.accountHotKeys[nextCharIndex];
				login = nextConfig.login;
				password = nextConfig.password;
				slot = nextConfig.slot;
				CloseHandle(hProcess);
				continue;
			}

			logger.log("Close L2 Alone on L2.exe completion");
		}
		catch (exception e) {
			logger.log("Auto login failure: ", e.what());
			showMessage(e.what());
			TerminateProcess(hProcess, 0);
		}

		CloseHandle(hProcess);
		return;
	}
}

L2WindowCreatedEvent waitL2WindowCreated(int processId, int timeoutMs) {

	auto futurePtr = eventService.waitForL2Window(processId);
	auto l2WindowFuture = futurePtr->get_future();
	std::future_status status = l2WindowFuture.wait_for(std::chrono::milliseconds(timeoutMs));

	if (status != std::future_status::ready)
	{
		stringstream ss;
		ss << "Can't find L2 window in " << timeoutMs << " milliseconds";
		throw exception(ss.str().c_str());
	}

	return l2WindowFuture.get();
}

string getLogFilePath(string absFilePath) {
	stringstream ss;
	ss << absFilePath << "\\" << APP_NAME << "_" << GetCurrentProcessId() << ".log";
	return ss.str();
}

bool isRunnedFromExe(string process) {

	// Get the handle to the console window
	HWND consoleWindow = GetConsoleWindow();

	// Get the process ID of the console window
	DWORD consoleProcessId;
	GetWindowThreadProcessId(consoleWindow, &consoleProcessId);

	// Get the handle to the process that created the console window
	HANDLE consoleProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, consoleProcessId);

	// Get the name of the executable that created the console window
	CHAR consoleProcessName[MAX_PATH];
	GetModuleFileNameExA(consoleProcessHandle, NULL, consoleProcessName, MAX_PATH);

	// Close the process handle
	CloseHandle(consoleProcessHandle);

	string consoleProcessNameStr(consoleProcessName);
	logger.log("Console process name: ", consoleProcessNameStr);
	logger.log("Process name: ", process);

	return consoleProcessNameStr == process;
}
#endif // !TEST
