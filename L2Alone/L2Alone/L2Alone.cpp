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

using namespace std;

struct FindL2WindowParams {
	string windowName;
	DWORD dwProcess;
};

#define APP_NAME "L2Alone"
#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"
#define L2_ALONE_LOGS_DIR "logs"

string getLogFilePath(string absFilePath);
int autoLoginL2(string login, string password, L2CharSlot slot, L2AloneConfig& config, AutologinStrategy* s);

void showMessage(string message);
bool isRunnedFromExe(string process);
L2WindowCreatedEvent waitL2WindowCreated(int processId);

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

		L2AloneConfig config = loadL2AloneConfig();
		if (config.logsEnabled) {
			prepareDirectory(L2_ALONE_LOGS_DIR);

			stringstream ss;
			ss << L2_ALONE_LOGS_DIR << "\\l2_" << GetCurrentProcessId() << ".log";
			logger.open(ss.str().c_str());
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

		int nextAutoLoginIndex = autoLoginL2(account, password, slot, config, autologinStrategy);
		while (nextAutoLoginIndex >= 0) {
			L2AccountHotKey nextConfig = config.accountHotKeys[nextAutoLoginIndex];
			nextAutoLoginIndex = autoLoginL2(nextConfig.login, nextConfig.password, nextConfig.slot, config, autologinStrategy);
		}

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

int autoLoginL2(string login, string password, L2CharSlot slot, L2AloneConfig& config, AutologinStrategy *autologinStrategy) {

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	string pathToExe = config.pathToL2;
	string folder = pathToExe.substr(0, pathToExe.length() - 6);

	logger.log("Create L2 process from file: ", config.pathToL2);

	auto r = CreateProcessA(
		pathToExe.c_str(),   // the path
		NULL,           // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		folder.c_str(), // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	if (r == NULL) {
		stringstream ss;
		ss << "Can't start L2 process with path " << config.pathToL2.c_str();
		throw exception(ss.str().c_str());
	}

	try {
		logger.log("L2 process started with id: ", pi.dwProcessId);

		L2WindowCreatedEvent d = waitL2WindowCreated(pi.dwProcessId);

		if (config.captureLogsEnabled) {
			stringstream ssPathToCoreLogs;
			ssPathToCoreLogs << L2_ALONE_LOGS_DIR << "\\" << "l2_" << pi.dwProcessId << ".log";
			string pathToCoreLogs = ssPathToCoreLogs.str();
		}

		auto hotKeyHandler = shared_ptr<L2QuitKeyHandler>(new L2QuitKeyHandler(d.processId, &config.accountHotKeys));
		eventService.setKeyboardHandler(d.hWindow, hotKeyHandler);

		auto pvpHandler = shared_ptr<L2PvpModeHandler>(new L2PvpModeHandler());
		eventService.setKeyboardHandler(d.hWindow, pvpHandler);
		eventService.setFocusHandler(d.hWindow, pvpHandler);

		autologinStrategy->doAutologin((HWND)d.hWindow, login, password, slot);

		WaitForSingleObject(pi.hProcess, INFINITE);

		eventService.removeKeyboardHandler(d.hWindow, hotKeyHandler);
		eventService.removeKeyboardHandler(d.hWindow, pvpHandler);
		eventService.removeFocusHandler(d.hWindow, pvpHandler);

		DWORD exitCode;
		if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
			throw exception("Can't run new window on hot key, because can't get previous process exit code");
		}

		if (isExitHotKeyCode(exitCode)) {
			return getNextHotKeyIndex(exitCode);
		}

		logger.log("Close L2 Alone on L2.exe completion");
	}
	catch (exception e) {
		logger.log("Auto login failure: ", e.what());
		showMessage(e.what());
		TerminateProcess(pi.hProcess, 0);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return -1;
}

L2WindowCreatedEvent waitL2WindowCreated(int processId) {

	auto futurePtr = eventService.waitForL2Window(processId);
	auto l2WindowFuture = futurePtr->get_future();
	std::future_status status = l2WindowFuture.wait_for(std::chrono::seconds(10));

	if (status != std::future_status::ready)
	{
		throw exception("Can't find L2 window in 10 seconds");
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
