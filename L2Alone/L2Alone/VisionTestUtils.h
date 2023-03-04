#ifdef TEST

#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>

#include "VisionUtils.h"

using namespace std;

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
 
#endif // TEST
