#pragma once

#include <Windows.h>
#include <fstream>

#include "Logger.h"

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

class HueDistributionCapturer {
public:
	virtual void capture(BitMapInfo& bmi, map<int, double>& dest) = 0;
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

	int h, s, v;
	getPixelHsv(info, x, y, h, s, v);

	if (h == expectedH) {
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
		logger.log("Fail to open file ", filename);
		return;
	}

	file.write(reinterpret_cast<const char*>(&bmfh), sizeof(bmfh));
	file.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
	file.write(reinterpret_cast<const char*>(bitMapInfo.data), dwBmpSize);
}

string getL2WindowName(L2Window window) {
	if (window == UNKNOWN) return "UNKNOWN";
	if (window == WELCOME) return "WELCOME";
	if (window == AGREEMENT) return "AGREEMENT";
	if (window == ACCOUNT_IN_USE) return "ACCOUNT_IN_USE";
	if (window == INCORRECT_PASSWORD) return "INCORRECT_PASSWORD";
	if (window == SERVERS) return "SERVERS";
	if (window == CHARACTERS) return "CHARACTERS";
	return "UNDEFINED";
}

bool hasTextVertical(BitMapInfo& bitMapInfo, int x) {

	int offset = 20;
	int height = 10;

	for (int i = offset; i < offset + height; ++i) {
		if (isTextPixel(bitMapInfo, x, i)) {
			return true;
		}
	}

	return false;
}

int getSystemMessageLength(BitMapInfo& bitMapInfo) {

	int centerX = bitMapInfo.width / 2;
	int maxNoTextPixelCount = 10;

	int noTextPixelCount = 0;
	int startPixelX = -1;
	for (int i = centerX; i >= 0; --i) {

		if (hasTextVertical(bitMapInfo, i)) {
			startPixelX = i;
			noTextPixelCount = 0;
		}
		else {
			noTextPixelCount++;
		}

		if (noTextPixelCount == maxNoTextPixelCount) {
			noTextPixelCount = 0;
			break;
		}
	}
	if (startPixelX == -1) {
		return 0;
	}

	return (centerX - startPixelX) * 2;
}

Point toLbViaCenterOffset(int rtWidth, int rtHeight, int lbWidth, int lbHeight, int rtX, int rtY) {

	int rtCenterX = rtWidth / 2;
	int rtCenterY = rtHeight / 2;
	int rtOffsetX = rtX - rtCenterX;
	int rtOffsetY = rtY - rtCenterY;

	int lbCenterX = lbWidth / 2;
	int lbCenterY = lbHeight / 2;

	Point p;
	p.x = lbCenterX + rtOffsetX;
	p.y = lbCenterY - rtOffsetY;

	return p;
}

Point convertRtPoint(int srcWidth, int srcHeight, int destWidth, int destHeight, int x, int y, RefAnchor anchor) {
	Point p;
	if (anchor == RefAnchor::CenterBottom) {
		int shw = srcWidth / 2;
		int sxOffset = x - shw;
		int syOffset = srcHeight - y;

		int dhw = destWidth / 2;
		p.x = dhw + sxOffset;
		p.y = destHeight - syOffset;
	}
	else if (anchor == RefAnchor::BottomRight) {
		int sxOffset = srcWidth - x;
		int syOffset = srcHeight - y;

		p.x = destWidth - sxOffset;
		p.y = destHeight - syOffset;
	}
	else {
		throw exception("Unsupported reference anchor");
	}

	return p;
}


Point toLbViaCenterBottomOffset(int rtWidth, int rtHeight, int lbWidth, int lbHeight, int rtX, int rtY) {

	int rtCenterX = rtWidth / 2;
	int rtCenterY = rtHeight / 2;
	int rtOffsetX = rtX - rtCenterX;

	int lbCenterX = lbWidth / 2;

	Point p;
	p.x = lbCenterX + rtOffsetX;
	p.y = rtHeight - rtY;

	return p;
}

Point toLbViaRightBottomOffset(int rtWidth, int rtHeight, int lbWidth, int lbHeight, int rtX, int rtY) {

	Point p;
	p.x = lbWidth - (rtWidth - rtX);
	p.y = rtHeight - rtY;
	return p;
}

struct DescDoubleCompare {
	bool operator()(const double& a, const double& b) const {
		return a > b;
	}
};

void removeNoise(map<int, double>& src, map<int, double>& dest) {

	map<double, int, DescDoubleCompare> sortedByValue;
	for (auto& kv : src) {
		sortedByValue[kv.second] = kv.first;
	}

	int countToPeek = 1;
	for (auto& kv : sortedByValue) {
		dest[kv.second] = kv.first;
		if (--countToPeek == 0) {
			break;
		}
	}
}

double getDistributionError(map<int, double>& a, map<int, double>& b) {

	map<int, double> first;
	removeNoise(a, first);

	map<int, double> second;
	removeNoise(b, second);

	auto fp = first.begin();
	auto sp = second.begin();

	double sum = 0;

	while (fp != first.end() || sp != second.end()) {

		if (fp == first.end()) {
			sum += sp->second;
			++sp;
		}
		else if (sp == second.end()) {
			sum += fp->second;
			++fp;
		}
		else if (abs(fp->first - sp->first) < 4) {
			sum += abs(fp->second - sp->second);
			++fp;
			++sp;
		}
		else if (fp->first > sp->first) {
			++sp;
		}
		else {
			sum += fp->second;
			++fp;
		}
	}

	return sum;
}

bool captureDcBmp(HWND hWnd, BitMapInfo& bitMapInfo) {

	bool success = false;

	HDC hWindowDC = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hWindowDC);
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	PrintWindow(hWnd, hMemDC, PW_CLIENTONLY);

	try {
		bitMapInfo = createBitMapInfo(hBitmap);
		success = true;
	}
	catch (exception e) {
		logger.error(e.what());
	}

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hWindowDC);
	DeleteObject(hBitmap);

	return success;
}

void collapse(map<int, double>& source, map<int, double>& dest) {

	int currentH = source.begin()->first;
	double seqVal = 0;

	double dVal = 0;
	int dH = currentH;

	for (auto& pair : source) {

		if (pair.first - currentH <= 3) {
			seqVal += pair.second;

			if (pair.second > dVal) {
				dH = pair.first;
				dVal = pair.second;
			}
		}
		else {
			dest[dH] = seqVal;
			seqVal = pair.second;
			dH = pair.first;
			dVal = pair.second;
		}
		currentH = pair.first;
	}
	dest[dH] = seqVal;
}

void initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest) {

	map<int, double> tmp;

	int total = width * height;
	for (int iy = y; iy < y + height; iy++) {
		for (int ix = x; ix < x + width; ix++) {
			int h, s, v;
			getPixelHsv(bitMapInfo, ix, iy, h, s, v);

			tmp[h] += 100.0 / total;
		}
	}

	collapse(tmp, dest);
}

inline bool isBlack(int r, int g, int b) {
	return r == 0 && g == 0 && b == 0;
}

inline bool isWhite(int r, int g, int b) {
	return r == 255 && g == 255 && b == 255;
}

bool isLoadingWindow(BitMapInfo& bmi) {

	int threshold = bmi.width * bmi.height / 2;
	int count = 0;

	for (int i = 0; i < bmi.width; ++i) {
		for (int j = 0; j < bmi.height; ++j) {
			int r, g, b;
			getPixelRgb(bmi, i, j, r, g, b);
			if (isBlack(r, g, b) || isWhite(r, g, b)) {
				count++;
			}

			if (count > threshold) {
				return true;
			}
		}
	}

	return false;
}

Point toLbPoint(int rtWidth, int rtHeight, BitMapInfo& bitMapInfo, ButtonDefinition& bDef) {

	if (bDef.anchor == RefAnchor::Center) {
		return toLbViaCenterOffset(rtWidth, rtHeight, bitMapInfo.width, bitMapInfo.height, bDef.rtX, bDef.rtY);
	}

	if (bDef.anchor == RefAnchor::CenterBottom) {
		return toLbViaCenterBottomOffset(rtWidth, rtHeight, bitMapInfo.width, bitMapInfo.height, bDef.rtX, bDef.rtY);
	}

	if (bDef.anchor == RefAnchor::BottomRight) {
		return toLbViaRightBottomOffset(rtWidth, rtHeight, bitMapInfo.width, bitMapInfo.height, bDef.rtX, bDef.rtY);
	}

	throw exception("Unrecognised ref anchor");
}