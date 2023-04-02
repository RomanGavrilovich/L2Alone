#pragma once

#include <string>
#include <fstream>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <Psapi.h>

#include "Logger.h"
#include "LayoutCache.h"

using namespace std;

struct GlobalFileLayoutCacheEntity {
	int processId;
	RECT r;
};

class GlobalFileLayoutCache : public LayoutCache {
public:

	bool load(LayoutCacheEntity& e) override;

	void save(LayoutCacheEntity& e) override;

private:
	const string CACHE_NAME = "L2Alone.cache";

	void readEntities(HANDLE fileHandle, vector<GlobalFileLayoutCacheEntity>& dest);
	void writeEntities(vector<GlobalFileLayoutCacheEntity>& src, HANDLE fileHandle);
	
	GlobalFileLayoutCacheEntity parse(string& line);
	string toString(GlobalFileLayoutCacheEntity& e);

	bool processCache(vector<GlobalFileLayoutCacheEntity>& src, vector<GlobalFileLayoutCacheEntity>& dest, GlobalFileLayoutCacheEntity& available);
};

bool GlobalFileLayoutCache::load(LayoutCacheEntity& destEntity) {

	bool loaded = false;

	HANDLE fileHandle = CreateFileA(CACHE_NAME.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		logger.error("Can't open cache file");
		return loaded;
	}

	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	try {
		if (!LockFileEx(fileHandle, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
			logger.error("Can't receive cache file lock");
			CloseHandle(fileHandle);
			return loaded;
		}

		vector<GlobalFileLayoutCacheEntity> entities;
		readEntities(fileHandle, entities);

		vector<GlobalFileLayoutCacheEntity> processed;
		GlobalFileLayoutCacheEntity available;
		if (processCache(entities, processed, available)) {
			destEntity.r = available.r;
			loaded = true;
			writeEntities(processed, fileHandle);
		}
	}
	catch (exception e) {
		logger.error("Unexpected error during cache reading: ", e.what());
	}

	if (!UnlockFileEx(fileHandle, 0, MAXDWORD, MAXDWORD, &overlapped)) {
		logger.error("Can't release lock on cache file");
	}
	CloseHandle(fileHandle);
	
	return loaded;
}

void GlobalFileLayoutCache::save(LayoutCacheEntity& e) {

	HANDLE fileHandle = CreateFileA(CACHE_NAME.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		logger.error("Can't open cache file for save");
		return;
	}

	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	try {
		if (!LockFileEx(fileHandle, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
			logger.error("Can't receive cache file lock for save");
			CloseHandle(fileHandle);
			return;
		}

		vector<GlobalFileLayoutCacheEntity> entities;
		readEntities(fileHandle, entities);

		int currentProcessId = GetCurrentProcessId();
		
		bool replaced = false;
		for (auto& entity : entities) {
			if (entity.processId == currentProcessId) {
				replaced = true;
				entity.r = e.r;
			}
		}

		if (!replaced) {
			GlobalFileLayoutCacheEntity globalEntity;
			globalEntity.processId = currentProcessId;
			globalEntity.r = e.r;
			entities.push_back(globalEntity);
		}

		writeEntities(entities, fileHandle);
	}
	catch (exception e) {
		logger.error("Unexpected error during cache reading: ", e.what());
	}

	if (!UnlockFileEx(fileHandle, 0, MAXDWORD, MAXDWORD, &overlapped)) {
		logger.error("Can't release lock on cache file");
	}
	CloseHandle(fileHandle);
}

GlobalFileLayoutCacheEntity GlobalFileLayoutCache::parse(string& line) {

	logger.log("Parse global file layout cache line: ", line);
	
	istringstream iss(line);
	vector<string> tokens;

	string token = "";
	while (getline(iss, token, ' ')) {
		tokens.push_back(token);
	}

	GlobalFileLayoutCacheEntity entity;
	entity.processId = stoi(tokens[0]);
	entity.r.left = stoi(tokens[1]);
	entity.r.top = stoi(tokens[2]);
	entity.r.right = stoi(tokens[3]);
	entity.r.bottom = stoi(tokens[4]);

	return entity;
}

bool isL2Alone(int processId) {
	bool flag = false;

	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
	if (processHandle) {
		char moduleName[MAX_PATH];
		DWORD moduleNameSize = sizeof(moduleName);
		if (GetModuleBaseNameA(processHandle, NULL, moduleName, moduleNameSize)) {
			std::string moduleNameStr(moduleName);
			if (moduleNameStr == "L2Alone.exe") {
				logger.log("Found alive l2 alone process with id ", processId);
				flag = true;
			}
		}
		else {
			logger.error("Can't get base module name for ", processId);
		}

		CloseHandle(processHandle);
	}
	else {
		logger.log("Can't open process with id ", processId);
	}

	return flag;
}

bool GlobalFileLayoutCache::processCache(vector<GlobalFileLayoutCacheEntity>& src, vector<GlobalFileLayoutCacheEntity>& dest, GlobalFileLayoutCacheEntity& available) {
	
	bool availableFound = false;
	for (auto& e : src) {
		if (availableFound || isL2Alone(e.processId)) {
			dest.push_back(e);
		}
		else {
			e.processId = GetCurrentProcessId();
			available = e;
			availableFound = true;

			dest.push_back(e);
		}
	}

	return availableFound;
}

void GlobalFileLayoutCache::writeEntities(vector<GlobalFileLayoutCacheEntity>& src, HANDLE fileHandle) {

	ostringstream oss;
	for (auto& e : src) {
		oss << e.processId << " " << e.r.left << " " << e.r.top << " " << e.r.right << " " << e.r.bottom << endl;
	}

	auto str = oss.str();
	
	int bytesCount = sizeof(char) * str.length();

	logger.log("Flush cache value: ", str);

	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;

	DWORD bytesWritten; 
	if (!WriteFile(fileHandle, str.c_str(), bytesCount, &bytesWritten, &overlapped)) {
		logger.error("Can't write entities to cache file");
	}

	LARGE_INTEGER fileSize;
	fileSize.QuadPart = bytesWritten;

	if (!SetFilePointerEx(fileHandle, fileSize, NULL, FILE_BEGIN)) {
		return;
	}

	if (!SetEndOfFile(fileHandle)) {
		return;
	}
}

void GlobalFileLayoutCache::readEntities(HANDLE fileHandle, vector<GlobalFileLayoutCacheEntity>& dest) {

	char buffer[4096];
	DWORD bytesRead = 0;
	if (!ReadFile(fileHandle, buffer, sizeof(buffer), &bytesRead, NULL)) {
		logger.error("Can't read cache file");
		CloseHandle(fileHandle);
		return;
	}

	buffer[bytesRead] = 0;

	if (bytesRead > 0) {
		istringstream iss(buffer);

		string line;
		while (getline(iss, line)) {
			if (line.size() > 0) {
				dest.push_back(parse(line));
			}
		}
	}
}