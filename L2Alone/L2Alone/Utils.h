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

void send_low_level_mouse_move(DWORD x, DWORD y) {
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dx = x * (65535 / GetSystemMetrics(SM_CXSCREEN));
	input.mi.dy = y * (65535 / GetSystemMetrics(SM_CYSCREEN));
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(INPUT));
}

void postClick(HWND hWindow, int x, int y) {

	LPARAM lparam = MAKELPARAM(x, y);

	send_low_level_mouse_move(x, y);

	//PostMessage(hWindow, WM_MOUSEMOVE, MK_LBUTTON, lparam);

	//Sleep(100);
	//PostMessage(hWindow, WM_LBUTTONUP, 0, lparam);
}
