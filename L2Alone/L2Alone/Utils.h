#pragma once

#include <string>
#include <Windows.h>

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
	if (CreateDirectoryA(absPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		return;
	}

	throw std::exception(("Can't create directory" + absPath).c_str());
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
}


void convertToGlobalClientRect(HWND hWindow, int x, int y, int& globalX, int& globalY) {

	// Define the client coordinates to be converted
	POINT clientPoint = { x, y };

	// Get the screen coordinates of the entire window
	RECT windowRect;
	GetWindowRect(hWindow, &windowRect);

	// Get the screen coordinates of the top-left corner of the client area
	POINT topLeft;
	topLeft.x = windowRect.left;
	topLeft.y = windowRect.top;
	ClientToScreen(hWindow, &topLeft);

	// Convert the client coordinates to screen coordinates and adjust for the header
	POINT screenPoint;
	screenPoint.x = clientPoint.x + (topLeft.x - windowRect.left);
	screenPoint.y = clientPoint.y + (topLeft.y - windowRect.top);

	// The converted screen coordinates are now stored in the screenPoint structure
	globalX = screenPoint.x;
	globalY = screenPoint.y;
}

void send_low_level_mouse_move(HWND hWindow, DWORD x, DWORD y, int e) {

	int globalX;
	int globalY;

	convertToGlobalClientRect(hWindow, x, y, globalX, globalY);

	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dx = (globalX + 5) * (65535 / GetSystemMetrics(SM_CXSCREEN));
	input.mi.dy = (globalY + 5) * (65535 / GetSystemMetrics(SM_CYSCREEN));
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | e;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(INPUT));
}

void postClick(HWND hWindow, int x, int y) {

	SetForegroundWindow(hWindow);
	for (int i = 0; i < 5; ++i) {
		Sleep(50);
		send_low_level_mouse_move(hWindow, x, y, MOUSEEVENTF_MOVE);
		Sleep(50);
		send_low_level_mouse_move(hWindow, x, y, MOUSEEVENTF_LEFTDOWN);
		Sleep(50);
		send_low_level_mouse_move(hWindow, x, y, MOUSEEVENTF_LEFTUP);
	}

	//send_low_level_mouse_move(x, y, MOUSEEVENTF_LEFTUP);
}
