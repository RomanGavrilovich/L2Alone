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

string getAbsoluteFilePath(string relative)
{
	char absoluteFilePath[MAX_PATH];

	// Get the absolute file path
	DWORD result = GetFullPathNameA(relative.c_str(), MAX_PATH, absoluteFilePath, NULL);
	if (result == 0)
	{
		std::string message = "Can't find absolute path for ";
		message += relative;

		throw std::exception(message.c_str());
	}
	else {
		return string(absoluteFilePath, result);
	}
}
