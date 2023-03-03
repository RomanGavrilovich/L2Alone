// L2Alone.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>
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

using namespace std;

struct FindL2WindowParams {
	string windowName;
	DWORD dwProcess;
};

#define APP_NAME "L2Alone"
#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"
#define L2_ALONE_LOGS_DIR "logs"

string getLogFilePath(string absFilePath);
int autoLoginL2(string login, string password, L2AloneConfig& config);

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

		if (argc < 2) {
			showMessage("Login is not provided");
			return 0;
		}

		if (argc < 3) {
			showMessage("Password is not provided");
			return 0;
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

		int nextAutoLoginIndex = autoLoginL2(account, password, config);
		while (nextAutoLoginIndex >= 0) {
			L2AccountHotKey nextConfig = config.accountHotKeys[nextAutoLoginIndex];
			nextAutoLoginIndex = autoLoginL2(nextConfig.login, nextConfig.password, config);
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

int autoLoginL2(string login, string password, L2AloneConfig& config) {

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	logger.log("Create L2 process from file: ", config.pathToL2);

	auto r = CreateProcessA(
		config.pathToL2.c_str(),   // the path
		NULL,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
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

		unique_ptr<AutologinStrategy> pAutoLoginStrategy;
		if (config.version == C5) {
			pAutoLoginStrategy = unique_ptr<AutologinStrategy>(new C5AutologinStrategy());
		}

		pAutoLoginStrategy->doAutologin((HWND)d.hWindow, login, password);

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
