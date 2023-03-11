#ifdef TEST

#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "VisionUtils.h"
#include "Utils.h"
#include "config_utils.h"

using namespace std;

#define TEST_RESOURCES_DIRECTORY "TestResources/Vision"
#define TEST_RESULT_OUTPUT "TestResult/Vision"

BitMapInfo readBmpFile(const char* filename)
{
	// Open the .bmp file in binary mode
	std::ifstream file(filename, std::ios::binary);

	if (!file) {
		std::cerr << "Error: Could not open file\n";
		throw std::exception("Could not open file");
	}

	BITMAPFILEHEADER bfh;

	file.read(reinterpret_cast<char*>(&bfh), sizeof(bfh));

	if (bfh.bfType != 0x4d42) {
		cerr << "Error: Not a valid file type";
		throw std::exception("Not a valid file type");
	}

	BitMapInfo bitMapInfo;
	bitMapInfo.path = std::string(filename);

	BITMAPINFOHEADER bih;
	file.read(reinterpret_cast<char*>(&bih), sizeof(bih));
	bitMapInfo.width = bih.biWidth;
	bitMapInfo.height = bih.biHeight;

	if (bih.biBitCount == 24) {
		bitMapInfo.multiplier = 3;
		bitMapInfo.bitCount = 24;
	}
	else if (bih.biBitCount == 32) {
		bitMapInfo.multiplier = 4;
		bitMapInfo.bitCount = 32;
	}
	else {
		throw std::exception("Unsupported bit count");
	}

	// Allocate memory for the image data
	uint8_t* image_data = new uint8_t[bitMapInfo.width * bitMapInfo.height * bitMapInfo.multiplier];

	// Read the image data from the file
	file.read(reinterpret_cast<char*>(image_data), bitMapInfo.width * bitMapInfo.height * bitMapInfo.multiplier);
	bitMapInfo.data = image_data;

	// Close the file
	file.close();

	return bitMapInfo;
}

void createTestFilePaths(L2Version version, L2Window classifiedWindow, vector<string>& dest) {

	stringstream ss;
	ss << TEST_RESOURCES_DIRECTORY << "/" << getL2VersionName(version) << "/" << getL2WindowName(classifiedWindow);

	string pathToFile = ss.str();

	ss << "/*.bmp";
	string filter = ss.str();

	WIN32_FIND_DATAA fileData;
	HANDLE hFind = FindFirstFileA(filter.c_str(), &fileData);

	if (hFind == NULL) {
		throw exception("Can't find test file");
	}

	do {
		// Check if the file is not a directory
		if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			dest.push_back(pathToFile + "/" + fileData.cFileName);
		}
	} while (FindNextFileA(hFind, &fileData));
	FindClose(hFind);
}

void getBmpInDir(string root, vector<string> &dest) {

	string filter = root + "/*.bmp";

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(filter.c_str(), &findData);

	if (hFind == NULL) {
		throw exception("Can't find test files");
	}

	do {
		// Check if the file is not a directory
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			dest.push_back(root + "/" + findData.cFileName);
		}
	} while (FindNextFileA(hFind, &findData));
	FindClose(hFind);
}

void getTestVisionDirectories(L2Version version, vector<string> &dest) {

	stringstream ss;
	ss << TEST_RESOURCES_DIRECTORY << "/" << getL2VersionName(version);

	string pathToFile = ss.str();

	ss << "/*";
	string filter = ss.str();

	WIN32_FIND_DATAA dirData;
	HANDLE hFind = FindFirstFileA(filter.c_str(), &dirData);

	if (hFind == NULL) {
		throw exception("Can't find test file");
	}

	do {
		// Check if the file is not a directory
		if (dirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(dirData.cFileName, ".") != 0 && strcmp(dirData.cFileName, "..") != 0) {
				dest.push_back(pathToFile + "/" + dirData.cFileName);
			}
		}
	} while (FindNextFileA(hFind, &dirData));
	FindClose(hFind);
}

void debugDraw(BitMapInfo& bmi, int rtWidth, int rtHeight, ButtonDefinition def) {

	auto point = toLbPoint(rtWidth, rtHeight, bmi, def);
	point.y -= def.height;

	drawRect(point.x, point.y, def.width, def.height, bmi);
}

void prepareDirectoriesForFile(string file) {
	
	vector<string> v;
	
	stringstream ss(file);
	string directory;

	while (std::getline(ss, directory, '/')) {
		v.push_back(directory);
	}

	stringstream ssFullPath;
	for (int i = 0; i < v.size() - 1; ++i) {
		ssFullPath << v.at(i);
		prepareDirectory(ssFullPath.str());
		if (i < v.size() - 2) {
			ssFullPath << "/";
		}
	}
}

string toTestOutputFile(string testFile) {
	string resourcePath = TEST_RESOURCES_DIRECTORY;
	string testResultDir = testFile.replace(testFile.find(resourcePath), resourcePath.length(), TEST_RESULT_OUTPUT);
	return testResultDir;
}

void writeTestOutput(string testFile, L2Window expected, L2Window actual, BitMapInfo& bmi) {
	string testOutputFile = toTestOutputFile(testFile) + "/" + getL2WindowName(actual) + ".bmp";
	prepareDirectoriesForFile(testOutputFile);
	writeBmpToFile(testOutputFile.c_str(), bmi);
}

#endif // TEST
