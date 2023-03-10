#pragma once

#include <string>
#include <Windows.h>
#include "Logger.h"

using namespace std;

#define EXIT_HOT_KEY_OFFSET 34200

bool startWith(string target, string prefix) {

	return target.find(prefix) == 0;
}

bool isKeyPressed(int vk) {
	return GetAsyncKeyState(vk) & 0x8000;
}

int getExitHotKeyCode(int hotKeyIndex) {
	return EXIT_HOT_KEY_OFFSET + hotKeyIndex;
}

int getNextHotKeyIndex(int exitCode) {
	return exitCode - EXIT_HOT_KEY_OFFSET;
}

bool isExitHotKeyCode(int exitCode) {
	return EXIT_HOT_KEY_OFFSET <= exitCode && exitCode < EXIT_HOT_KEY_OFFSET + 12;
}

void prepareDirectory(string absPath) {
#ifndef L2A_RELEASE
	if (CreateDirectoryA(absPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		return;
	}

	throw std::exception(("Can't create directory" + absPath).c_str());
#endif // !L2A_RELEASE
}

void postText(HWND hWindow, string& s) {

	for (char c : s) {
		PostMessage(hWindow, WM_CHAR, c, 0);
	}
}

void postControlMessage(HWND hWindow, int vk) {
	PostMessage(hWindow, WM_KEYDOWN, vk, 0);
}

void postCredentials(HWND hWindow, string& login, string& password) {
	postText(hWindow, login);
	postControlMessage(hWindow, VK_TAB);
	postText(hWindow, password);
	postControlMessage(hWindow, VK_RETURN);
	logger.log("Credentials posted");
}

void convertToGlobalClientRect(HWND hWindow, int x, int y, int& globalX, int& globalY) {

	POINT clientPoint = { x, y };

	RECT windowRect;
	GetWindowRect(hWindow, &windowRect);

	POINT topLeft;
	topLeft.x = windowRect.left;
	topLeft.y = windowRect.top;
	ClientToScreen(hWindow, &topLeft);

	POINT screenPoint;
	screenPoint.x = clientPoint.x + (topLeft.x - windowRect.left);
	screenPoint.y = clientPoint.y + (topLeft.y - windowRect.top);

	// The converted screen coordinates are now stored in the screenPoint structure
	globalX = screenPoint.x;
	globalY = screenPoint.y;
}

void sendLowLevelMouseEvent(HWND hWindow, DWORD globalX, DWORD globalY, int e) {

	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dx = (long)(globalX * (65535.0 / GetSystemMetrics(SM_CXSCREEN)));
	input.mi.dy = (long)(globalY * (65535.0 / GetSystemMetrics(SM_CYSCREEN)));
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | e;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(INPUT));
}

void doClick(HWND hWindow, int globalX, int globalY, int timeout) {

	sendLowLevelMouseEvent(hWindow, globalX, globalY, MOUSEEVENTF_MOVE);
	Sleep(timeout);

	sendLowLevelMouseEvent(hWindow, globalX, globalY, MOUSEEVENTF_LEFTDOWN);
	Sleep(timeout);

	sendLowLevelMouseEvent(hWindow, globalX, globalY, MOUSEEVENTF_LEFTUP);
	Sleep(timeout);
}
