#pragma once

#include <Windows.h>
#include <string>
#include <future>
#include "L2EventService.h"

#include "logger.h"

using namespace std;

struct L2WindowData {
	DWORD dwProcessId;
	DWORD dwThreadId;
	HWND hWindow;
};

L2WindowData InitL2WindowData(DWORD processId, string& windowName) {

	auto futurePtr = eventService.waitForL2Window(processId);
	auto l2WindowFuture = futurePtr->get_future();
	std::future_status status = l2WindowFuture.wait_for(std::chrono::seconds(10));

	if (status != std::future_status::ready)
	{
		throw exception("Can't find L2 window in 10 seconds");
	}

	auto e = l2WindowFuture.get();

	return L2WindowData{e.processId, e.threadId, e.hWindow};
}