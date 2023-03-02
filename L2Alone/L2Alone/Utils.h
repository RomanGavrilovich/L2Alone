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