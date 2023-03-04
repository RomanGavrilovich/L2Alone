#pragma once

#include <Windows.h>
#include <fstream>

using namespace std;

struct BitMapInfo {

	BYTE* data;
	int bitCount;
	int width;
	int height;
	int multiplier;
	string path;
};

struct Point {
	int x;
	int y;
};

struct HSV {
	int h;
	int s;
	int v;
};

BitMapInfo createBitMapInfo(HBITMAP hBitmap);

void getPixelRgb(BitMapInfo& bitMapInfo, int pixelX, int pixelY, int& r, int& g, int& hasHorizontalBorder);
int getPixelIndex(BitMapInfo& bitMapInfo, int x, int y);
void rgbToHsv(int R, int G, int B, int& H, int& S, int& V);
void drawPixelRgb(BitMapInfo& bitMapInfo, int x, int y, int r, int g, int hasHorizontalBorder);
void getPixelHsv(BitMapInfo& info, int x, int y, int& h, int& s, int& v);
bool isTextPixel(BitMapInfo& info, int x, int y);

void writeBmpToFile(const char* filename, BitMapInfo& info);
void drawRect(int startX, int startY, int width, int height, BitMapInfo& info);

void getPixelHsv(BitMapInfo& info, int x, int y, int& h, int& s, int& v) {
	int r, g, hasHorizontalBorder;

	getPixelRgb(info, x, y, r, g, hasHorizontalBorder);
	rgbToHsv(r, g, hasHorizontalBorder, h, s, v);
}

bool isTextPixel(BitMapInfo& info, int x, int y) {

	int expectedH = 300;
	int expectedV = 86;

	int h, s, v;
	getPixelHsv(info, x, y, h, s, v);

	if (h == expectedH && abs(expectedV - v) < 5) {
		return true;
	}

	return false;
}

BitMapInfo createBitMapInfo(HBITMAP hBitmap) {

	BITMAP bmp;
	if (!GetObject(hBitmap, sizeof(BITMAP), &bmp))
	{
		throw std::exception("There is no bitmap at provided handle");
	}

	BitMapInfo bitMapInfo;

	BITMAPINFOHEADER bih = {};
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = bmp.bmWidth;
	bih.biHeight = bmp.bmHeight;
	bih.biPlanes = 1;
	bih.biBitCount = bmp.bmBitsPixel;
	bih.biCompression = BI_RGB;

	bitMapInfo.width = bmp.bmWidth;
	bitMapInfo.height = bmp.bmHeight;
	bitMapInfo.bitCount = bmp.bmBitsPixel;
	if (bitMapInfo.bitCount == 24) {
		bitMapInfo.multiplier = 3;
	}
	else if (bitMapInfo.bitCount == 32) {
		bitMapInfo.multiplier = 4;
	}
	else {
		throw exception("Unsupported bit count received");
	}

	DWORD dwBmpSize = ((bitMapInfo.width * bitMapInfo.bitCount + bitMapInfo.bitCount - 1) / bitMapInfo.bitCount) * bitMapInfo.multiplier * bitMapInfo.height;
	bitMapInfo.data = new BYTE[dwBmpSize];

	HDC hDC = GetDC(NULL);
	GetDIBits(hDC, hBitmap, 0, bih.biHeight, bitMapInfo.data, (BITMAPINFO*)&bih, DIB_RGB_COLORS);
	ReleaseDC(NULL, hDC);

	return bitMapInfo;
}

double getMax(double a, double hasHorizontalBorder, double c) {
	if (a > hasHorizontalBorder && a > c) {
		return a;
	}

	if (hasHorizontalBorder > a && hasHorizontalBorder > c) {
		return hasHorizontalBorder;
	}

	return c;
}

double getMin(double a, double hasHorizontalBorder, double c) {
	if (a < hasHorizontalBorder && a < c) {
		return a;
	}

	if (hasHorizontalBorder < a && hasHorizontalBorder < c) {
		return hasHorizontalBorder;
	}

	return c;
}

void rgbToHsv(double R, double G, double B, double& H, double& S, double& V) {
	double eps = 1e-6;

	double Cmax = getMax(R, G, B);
	double Cmin = getMin(R, G, B);

	V = Cmax;
	double delta = Cmax - Cmin;
	S = Cmax == 0 ? 0 : delta / Cmax;
	double Hprime;
	if (delta == 0) {
		Hprime = 0;
	}
	else if (Cmax == R) {
		Hprime = std::fmod((G - B) / delta, 6);
	}
	else if (Cmax == G) {
		Hprime = (B - R) / delta + 2;
	}
	else { // Cmax == B
		Hprime = (R - G) / delta + 4;
	}
	H = Hprime / 6.0;
	if (H < 0) {
		H += 1;
	}
	else if (H >= 1) {
		H -= 1;
	}
}

void rgbToHsv(int R, int G, int B, int& H, int& S, int& V) {
	double Rf = R / 255.0;
	double Gf = G / 255.0;
	double Bf = B / 255.0;
	double Hf, Sf, Vf;
	rgbToHsv(Rf, Gf, Bf, Hf, Sf, Vf);
	H = static_cast<int>(std::round(Hf * 360.0));
	S = static_cast<int>(std::round(Sf * 100.0));
	V = static_cast<int>(std::round(Vf * 100.0));
}

void drawRect(int startX, int startY, int width, int height, BitMapInfo& info) {

	for (int i = 0; i <= width; ++i) {
		drawPixelRgb(info, startX + i, startY, 255, 0, 0);
	}

	for (int i = 0; i <= width; ++i) {
		drawPixelRgb(info, startX + i, startY + height, 255, 0, 0);
	}

	for (int i = 0; i <= height; ++i) {
		drawPixelRgb(info, startX, startY + i, 255, 0, 0);
	}

	for (int i = 0; i <= height; ++i) {
		drawPixelRgb(info, startX + width, startY + i, 255, 0, 0);
	}
}

void getPixelRgb(BitMapInfo& bitMapInfo, int pixelX, int pixelY, int& r, int& g, int& hasHorizontalBorder) {

	int index = getPixelIndex(bitMapInfo, pixelX, pixelY);

	r = bitMapInfo.data[index + 2];
	g = bitMapInfo.data[index + 1];
	hasHorizontalBorder = bitMapInfo.data[index];
}

void drawPixelRgb(BitMapInfo& bitMapInfo, int x, int y, int r, int g, int hasHorizontalBorder) {

	int index = getPixelIndex(bitMapInfo, x, y);
	bitMapInfo.data[index + 2] = r;
	bitMapInfo.data[index + 1] = g;
	bitMapInfo.data[index] = hasHorizontalBorder;
}

int getPixelIndex(BitMapInfo& bitMapInfo, int x, int y) {
	return (y * bitMapInfo.width + x) * bitMapInfo.multiplier;
}

void writeBmpToFile(const char* filename, BitMapInfo& bitMapInfo) {

#ifndef L2A_RELEASE
	BITMAPINFOHEADER bih = {};
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = bitMapInfo.width;
	bih.biHeight = bitMapInfo.height;
	bih.biPlanes = 1;
	bih.biBitCount = bitMapInfo.bitCount;
	bih.biCompression = BI_RGB;

	DWORD dwBmpSize = ((bih.biWidth * bih.biBitCount + bitMapInfo.bitCount - 1) / bitMapInfo.bitCount) * bitMapInfo.multiplier * bih.biHeight;
	BITMAPFILEHEADER bmfh = {};
	bmfh.bfType = 0x4d42;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	std::ofstream file(filename, std::ios::out | std::ios::binary);
	if (!file.is_open())
	{
		return;
	}

	file.write(reinterpret_cast<const char*>(&bmfh), sizeof(bmfh));
	file.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
	file.write(reinterpret_cast<const char*>(bitMapInfo.data), dwBmpSize);
	return;
#endif // !L2A_RELEASE
}

string getL2WindowName(L2Window window) {
	if (window == UNKNOWN) return "UNKNOWN";
	if (window == WELCOME) return "WELCOME";
	if (window == AGREEMENT) return "AGREEMENT";
	if (window == ACCOUNT_IN_USE) return "ACCOUNT_IN_USE";
	if (window == INCORRECT_PASSWORD) return "INVALID_CREDENTIALS";
	if (window == SERVERS) return "SERVERS";
	if (window == CHARACTERS) return "CHARACTERS";
	return "UNDEFINED";
}

int getSystemMessageLength(BitMapInfo& bitMapInfo) {

	int targetH = 300;
	int targetV = 86;
	int frameH = 50;

	int textStartIndex = -1;
	for (int i = 0; i < bitMapInfo.width; ++i) {
		for (int j = 0; j < frameH; ++j) {
			if (isTextPixel(bitMapInfo, i, j)) {
				textStartIndex = i;
				break;
			}
		}

		if (textStartIndex > 0) {
			break;
		}
	}

	int textEndIndex = -1;
	for (int i = bitMapInfo.width - 1; i >= 0; --i) {
		for (int j = 0; j < frameH; ++j) {
			if (isTextPixel(bitMapInfo, i, j)) {
				textEndIndex = i;
				break;
			}
		}

		if (textEndIndex > 0) {
			break;
		}
	}

	int length = textEndIndex - textStartIndex;
	return length;
}
