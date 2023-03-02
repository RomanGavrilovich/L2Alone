#pragma once

#include <string>
#include <Windows.h>

using namespace std;

void prepareDirectory(string absPath) {
	if (CreateDirectoryA(absPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		return;
	}

	throw std::exception(("Can't create directory" + absPath).c_str());
}
