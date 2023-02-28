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

#include "logger.h"
#include "autologin.h"
#include "config_utils.h"
#include "files_utils.h"

using namespace std;

struct FindL2WindowParams {
	string windowName;
	DWORD dwProcess;
};

#define APP_NAME "L2Alone"
#define L2_ALONE_CONFIG_FILE_NAME "L2Alone.config"
#define L2_ALONE_LOGS_DIR "logs"

string getAbsoluteFilePath(string relative);
string getLogFilePath(string absFilePath);
void autoLoginL2(string login, string password, L2AloneConfig& config);

void showMessage(string message);

volatile int dwL2WindowThread;
volatile int dwL2Window;
volatile int dwL2Process;

int main(int argc, char* argv[])
{
	try {

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
			prepareDirectory(getAbsoluteFilePath(L2_ALONE_LOGS_DIR));

			stringstream ss;
			ss << L2_ALONE_LOGS_DIR << "l2_" << GetCurrentProcessId() << ".log";
			logger.open(getAbsoluteFilePath(ss.str().c_str()));
		}

		autoLoginL2(argv[1], argv[2], config);

		return 0;
	}
	catch (exception e) {
		cout << "ERROR: " << e.what() << endl;
		MessageBoxA(NULL, e.what(), APP_NAME, MB_OK);
		return 1;
	}
}

void showMessage(string message) {
	MessageBoxA(NULL, message.c_str(), APP_NAME, MB_OK);
}

bool contains(string s1, string s2) {
	return s1.find(s2) != std::string::npos;
}

string getWindowName(HWND HWnd) {

	DWORD processId;
	GetWindowThreadProcessId(HWnd, &processId);

	int textLength = GetWindowTextLength(HWnd);
	std::string text(textLength + 1, '\0');
	GetWindowTextA(HWnd, &text[0], textLength + 1);

	return text;
}

BOOL CALLBACK FindL2MainWindow(HWND hwnd, LPARAM lParam)
{
	auto params = (FindL2WindowParams*)lParam;

	DWORD windowProcessId;
	DWORD windowThreadId = GetWindowThreadProcessId(hwnd, &windowProcessId);

	if (windowProcessId == params->dwProcess)
	{
		string windowText = getWindowName(hwnd);

		bool eq = contains(windowText, params->windowName) && !contains(windowText, "(");

		if (eq) {
			dwL2Window = (int)hwnd;
			dwL2WindowThread = (int)windowThreadId;
			dwL2Process = windowProcessId;

			return FALSE;
		}
	}

	return TRUE;
}


void InitL2WindowData(int processId, string &windowName) {

	FindL2WindowParams params;
	params.dwProcess = processId;
	params.windowName = windowName;

	logger.log("Wait for main window");
	for (int i = 0; i < 20; ++i) { // 10 sec timeout

		logger.log("Run search for main window");
		EnumWindows(FindL2MainWindow, (LPARAM)&params);

		if (dwL2Window != NULL) {
			logger.log("Waiting for main window completed");
			break;
		}

		Sleep(500);
	}

	if (dwL2Window == NULL) {
		throw exception("Didn't find L2 main window");
	}
}

thread* tMonitor;


void autoLoginL2(string login, string password, L2AloneConfig& config) {

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

		InitL2WindowData(pi.dwProcessId, config.l2WindowName);
		
		logger.log("L2 window: ", (HWND)dwL2Window, "(", dwL2Window, ")", "L2 thread : ", dwL2WindowThread);

		stringstream ssPathToCoreLogs;
		ssPathToCoreLogs << L2_ALONE_LOGS_DIR << "\\" << "l2_" << pi.dwProcessId << ".log";
		string pathToCoreLogs = ssPathToCoreLogs.str();

		doAutologin((HWND)dwL2Window, login, password);

		DWORD result = WaitForSingleObject(pi.hProcess, INFINITE);
	}
	catch (exception e) {
		logger.log("Auto login failure: ", e.what());
		TerminateProcess(pi.hProcess, 0);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
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
