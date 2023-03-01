#pragma once

#include <Windows.h>

struct L2WindowCreatedEvent {
	HWND hWindow;
	DWORD processId;
	DWORD threadId;
};