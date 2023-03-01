#pragma once

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <sstream>

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "logger.h"

using namespace std;

struct BitMapInfo {

	BYTE* data;
	int bitCount;
	int width;
	int height;
	int multiplier;
	string path;
};

enum L2Window {
	UNKNOWN,
	MAIN_WINDOW,
	AGREEMENT,
	ACCOUNT_IN_USE,
	INVALID_CREDENTIALS,
	SERVERS,
	CHARACTERS
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

BitMapInfo ReadBmpFile(const char* filename);

void WriteBmpToFile(const char* filename, BitMapInfo& info);

void getPixelRgb(BitMapInfo bitMapInfo, int pixelX, int pixelY, int& r, int& g, int& b);

int getPixelIndex(BitMapInfo bitMapInfo, int x, int y);
void RgbToHsv(int R, int G, int B, int& H, int& S, int& V);
void drawRect(int startX, int startY, int width, int height, BitMapInfo& info);
void drawPixelRgb(BitMapInfo bitMapInfo, int x, int y, int r, int g, int b);

void getPixelHsv(BitMapInfo info, int x, int y, int& h, int& s, int& v);

void drawRect(int startX, int startY, int width, int height, BitMapInfo& info);

int getSystemMessageLength(BitMapInfo& bitMapInfo);

bool capturingEnabled = false;

enum RefAnchor {
	Center,
	CenterBottom,
	BottomRight
};

struct ButtonRef {
	int rtX;
	int rtY;
	int width;
	int height;
	RefAnchor anchor = RefAnchor::Center;
};


class ButtonClassifier {

public:
	ButtonClassifier(int rtWidth, int rtHeight) {
		this->rtWidth = rtWidth;
		this->rtHeight = rtHeight;
	}

	bool captureRefButton(BitMapInfo& bitMapInfo, ButtonRef bRef) {

		if (hDistribution.size() > 0) {
			hDistribution.clear();
		}

		Point targetLb = toLbPoint(bitMapInfo, bRef);
		int lbX = targetLb.x;
		int lbY = targetLb.y - bRef.height;

		initHDistribution(bitMapInfo, lbX, lbY, bRef.width, bRef.height, hDistribution);

		// Bottom border
		map<int, double> botBorderH;
		initHDistribution(bitMapInfo, lbX, lbY, bRef.width, 1, botBorderH);
		// Below bottom border
		map<int, double> underBotBorderH;
		initHDistribution(bitMapInfo, lbX, lbY - 1, bRef.width, 1, underBotBorderH);
		if (getDistributionError(botBorderH, underBotBorderH) < 5) {
			return false;
		}

		// Top border
		map<int, double> topBorderH;
		initHDistribution(bitMapInfo, lbX, lbY + bRef.height - 1, bRef.width, 1, topBorderH);
		// Above top border
		map<int, double> upperTopBorderH;
		initHDistribution(bitMapInfo, lbX, lbY + bRef.height, bRef.width, 1, upperTopBorderH);
		if (getDistributionError(topBorderH, upperTopBorderH) < 5) {
			return false;
		}

		// Left border
		map<int, double> leftBorderH;
		initHDistribution(bitMapInfo, lbX, lbY, 1, bRef.height, leftBorderH);
		// Before left border
		map<int, double> beforeLeftBorderH;
		initHDistribution(bitMapInfo, lbX - 1, lbY, 1, bRef.height, beforeLeftBorderH);
		if (getDistributionError(leftBorderH, beforeLeftBorderH) < 5) {
			return false;
		}

		// Right border
		map<int, double> rightBorderH;
		initHDistribution(bitMapInfo, lbX + bRef.width - 1, lbY, 1, bRef.height, rightBorderH);
		// After right border
		map<int, double> afterRightBorderH;
		initHDistribution(bitMapInfo, lbX + bRef.width, lbY, 1, bRef.height, afterRightBorderH);
		if (getDistributionError(rightBorderH, afterRightBorderH) < 5) {
			return false;
		}

		return true;
	}

	bool isButton(BitMapInfo& bitMapInfo, ButtonRef bRef) {

		map<int, double> actualDest;
		Point targetLb = toLbPoint(bitMapInfo, bRef);
		int lbX = targetLb.x;
		int lbY = targetLb.y - bRef.height;

		initHDistribution(bitMapInfo, lbX, lbY, bRef.width, bRef.height, actualDest);

		auto dError = getDistributionError(hDistribution, actualDest);

		return dError < 5;
	}

private:
	int rtWidth;
	int rtHeight;
	map<int, double> hDistribution;

	Point toLbViaCenterOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

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

	Point toLbViaCenterBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

		int rtCenterX = rtWidth / 2;
		int rtCenterY = rtHeight / 2;
		int rtOffsetX = rtX - rtCenterX;

		int lbCenterX = lbWidth / 2;

		Point p;
		p.x = lbCenterX + rtOffsetX;
		p.y = rtHeight - rtY;

		return p;
	}

	Point toLbViaRightBottomOffset(int lbWidth, int lbHeight, int rtX, int rtY) {

		Point p;
		p.x = lbWidth - (rtWidth - rtX);
		p.y = rtHeight - rtY;
		return p;
	}

	Point toLbPoint(BitMapInfo& bitMapInfo, ButtonRef& buttonRef) {

		if (buttonRef.anchor == RefAnchor::Center) {
			return toLbViaCenterOffset(bitMapInfo.width, bitMapInfo.height, buttonRef.rtX, buttonRef.rtY);
		}

		if (buttonRef.anchor == RefAnchor::CenterBottom) {
			return toLbViaCenterBottomOffset(bitMapInfo.width, bitMapInfo.height, buttonRef.rtX, buttonRef.rtY);
		}

		if (buttonRef.anchor == RefAnchor::BottomRight) {
			return toLbViaRightBottomOffset(bitMapInfo.width, bitMapInfo.height, buttonRef.rtX, buttonRef.rtY);
		}

		throw exception("Unrecognised ref anchor");
	}

	void initHDistribution(BitMapInfo& bitMapInfo, int x, int y, int width, int height, map<int, double>& dest) {

		map<int, double> tmp;

		double total = width * height;
		for (int iy = y; iy < y + height; iy++) {
			for (int ix = x; ix < x + width; ix++) {
				int h, s, v;
				getPixelHsv(bitMapInfo, ix, iy, h, s, v);

				tmp[h] += 100.0 / total;
			}
		}

		collapse(tmp, dest);
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
	}

	double getDistributionError(map<int, double>& first, map<int, double>& second) {

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
				sum += abs(fp->second - fp->second);
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
};

class WindowClassifier {

public:

	WindowClassifier(L2Window window) {
		this->window = window;
	}

	void addBtn(ButtonRef buttonRef) {
		bRtPoints.push_back(buttonRef);
	}

	void addText(int textMinSize, int textMaxSize) {
		this->textMinSize = textMinSize;
		this->textMaxSize = textMaxSize;
	}

	L2Window getWindow() {
		return this->window;
	}

	bool isWindow(BitMapInfo& bitMapInfo, ButtonClassifier& buttonClassifier) {

		if (bRtPoints.size() == 0) {
			throw exception("There is no buttons to search");
		}

		for (auto it = bRtPoints.begin(); it != bRtPoints.end(); ++it) {

			if (!buttonClassifier.isButton(bitMapInfo, *it)) {
				return false;
			}
		}

		if (textMinSize > 0 && textMaxSize > 0) {
			auto length = getSystemMessageLength(bitMapInfo);
			if (!(textMinSize <= length && length <= textMaxSize)) {
				return false;
			}
		}

		return true;
	}

private:
	L2Window window;
	vector<ButtonRef> bRtPoints;
	int textMinSize = 0;
	int textMaxSize = 0;
};


void getPixelHsv(BitMapInfo info, int x, int y, int& h, int& s, int& v) {
	int r, g, b;

	getPixelRgb(info, x, y, r, g, b);
	RgbToHsv(r, g, b, h, s, v);
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

std::string bitmapOutoutPath(string bitmapPath, string suffix) {

	int i = bitmapPath.find(".bmp");

	stringstream name;
	name << suffix;
	name << "_result.bmp";

	std::string oPath = bitmapPath;
	oPath.replace(i, 4, name.str().c_str());

	return oPath;
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

	if (length > 0 && capturingEnabled) {
		cout << "Text length: " << length << endl;
		drawRect(textStartIndex, 0, length, 60, bitMapInfo);
	}

	return length;
}

bool IsIncorrectPassword(BitMapInfo& bitMapInfo) {

	int length = getSystemMessageLength(bitMapInfo);
	return length >= 450 && length <= 460;
}

bool IsAccountIsUsing(BitMapInfo& bitMapInfo) {

	int length = getSystemMessageLength(bitMapInfo);
	return length > 200 && length < 300;
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

void getPixelRgb(BitMapInfo bitMapInfo, int pixelX, int pixelY, int& r, int& g, int& b) {

	int index = getPixelIndex(bitMapInfo, pixelX, pixelY);

	r = bitMapInfo.data[index + 2];
	g = bitMapInfo.data[index + 1];
	b = bitMapInfo.data[index];
}

void drawPixelRgb(BitMapInfo bitMapInfo, int x, int y, int r, int g, int b) {

	int index = getPixelIndex(bitMapInfo, x, y);
	bitMapInfo.data[index + 2] = r;
	bitMapInfo.data[index + 1] = g;
	bitMapInfo.data[index] = b;
}

int getPixelIndex(BitMapInfo bitMapInfo, int x, int y) {
	return (y * bitMapInfo.width + x) * bitMapInfo.multiplier;
}

BitMapInfo ReadBmpFile(const char* filename)
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

void WriteBmpToFile(const char* filename, BitMapInfo& bitMapInfo) {

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
}

double getMax(double a, double b, double c) {
	if (a > b && a > c) {
		return a;
	}

	if (b > a && b > c) {
		return b;
	}

	return c;
}

double getMin(double a, double b, double c) {
	if (a < b && a < c) {
		return a;
	}

	if (b < a && b < c) {
		return b;
	}

	return c;
}

void RgbToHsv(double R, double G, double B, double& H, double& S, double& V) {
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

void RgbToHsv(int R, int G, int B, int& H, int& S, int& V) {
	double Rf = R / 255.0;
	double Gf = G / 255.0;
	double Bf = B / 255.0;
	double Hf, Sf, Vf;
	RgbToHsv(Rf, Gf, Bf, Hf, Sf, Vf);
	H = static_cast<int>(std::round(Hf * 360.0));
	S = static_cast<int>(std::round(Sf * 100.0));
	V = static_cast<int>(std::round(Vf * 100.0));
}

string getL2WindowName(L2Window window) {
	if (window == UNKNOWN) return "UNKNOWN";
	if (window == AGREEMENT) return "AGREEMENT";
	if (window == ACCOUNT_IN_USE) return "ACCOUNT_IN_USE";
	if (window == INVALID_CREDENTIALS) return "INVALID_CREDENTIALS";
	if (window == SERVERS) return "SERVERS";
	if (window == CHARACTERS) return "CHARACTERS";
	return "UNDEFINED";
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

ButtonClassifier buttonClassifier = ButtonClassifier(1360, 768);
vector<WindowClassifier> windowClassifiers;

void logCapture(BitMapInfo& bitMapInfo, string pathToLogs, string suffix) {

	stringstream ss;
	ss << pathToLogs << "_" << suffix << ".bmp";

	auto path = ss.str();
	logger.log("Write capture ", path);

	WriteBmpToFile(path.c_str(), bitMapInfo);
}

bool isLoaded(BitMapInfo& bitMapInfo) {

	int r, g, b;
	getPixelRgb(bitMapInfo, 0, 0, r, g, b);
	for (int i = 0; i < bitMapInfo.height; ++i) {
		for (int j = 0; j < bitMapInfo.width; ++j) {
			int r2, g2, b2;
			getPixelRgb(bitMapInfo, j, i, r2, g2, b2);
			if (r != r2 && g != g2 && b != b2) {
				return true;
			}
		}
	}

	return false;
}

void initializeWindowClassifier(HWND hWnd, string pathToLogs, bool capturingEnabled)
{
	logger.log("Initialize window classifier");

	bool initialized = false;

	for (int i = 0; i < 50; ++i) {
		HDC hWindowDC = GetDC(hWnd);
		HDC hMemDC = CreateCompatibleDC(hWindowDC);
		RECT rcClient;
		GetClientRect(hWnd, &rcClient);
		HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
		BitBlt(hMemDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWindowDC, 0, 0, SRCCOPY);

		BitMapInfo bitMapInfo = createBitMapInfo(hBitmap);

		if (isLoaded(bitMapInfo)) {
			if (buttonClassifier.captureRefButton(bitMapInfo, ButtonRef{ 583, 402, 94, 21 })) {

				auto accountInUseClassifier = WindowClassifier(L2Window::ACCOUNT_IN_USE);
				accountInUseClassifier.addBtn(ButtonRef{ 583, 402, 94, 21 });
				accountInUseClassifier.addBtn(ButtonRef{ 683, 402, 94, 21 });
				accountInUseClassifier.addText(200, 300);
				windowClassifiers.push_back(accountInUseClassifier);

				auto incorrectPasswordClassifier = WindowClassifier(L2Window::INVALID_CREDENTIALS);
				incorrectPasswordClassifier.addBtn(ButtonRef{ 583, 402, 94, 21 });
				incorrectPasswordClassifier.addBtn(ButtonRef{ 683, 402, 94, 21 });
				incorrectPasswordClassifier.addText(400, 500);
				windowClassifiers.push_back(incorrectPasswordClassifier);

				auto agreementWindowClassifier = WindowClassifier(L2Window::AGREEMENT);
				agreementWindowClassifier.addBtn(ButtonRef{ 603, 568, 74, 21 });
				agreementWindowClassifier.addBtn(ButtonRef{ 683, 568, 74, 21 });
				windowClassifiers.push_back(agreementWindowClassifier);

				auto serverWindowClassifier = WindowClassifier(L2Window::SERVERS);
				serverWindowClassifier.addBtn(ButtonRef{ 563, 410, 74, 21 });
				serverWindowClassifier.addBtn(ButtonRef{ 643, 410, 74, 21 });
				serverWindowClassifier.addBtn(ButtonRef{ 724, 410, 74, 21 });
				windowClassifiers.push_back(serverWindowClassifier);

				auto charsWindowClassifier = WindowClassifier(L2Window::CHARACTERS);
				charsWindowClassifier.addBtn(ButtonRef{ 623, 668, 114, 29, RefAnchor::CenterBottom });
				charsWindowClassifier.addBtn(ButtonRef{ 1219, 589, 94, 21, RefAnchor::BottomRight });
				charsWindowClassifier.addBtn(ButtonRef{ 1219, 613, 94, 21, RefAnchor::BottomRight });
				charsWindowClassifier.addBtn(ButtonRef{ 1219, 663, 94, 21, RefAnchor::BottomRight });
				windowClassifiers.push_back(charsWindowClassifier);

				initialized = true;
			}
		}

		Sleep(100);

		delete[] bitMapInfo.data;

		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
		ReleaseDC(hWnd, hWindowDC);
		DeleteObject(hBitmap);

		if (initialized) {
			break;
		}
	}

	if (!initialized) {
		logger.log("Initialize window classifier failed");
		throw exception("Can't capture window after 5s time out");
	}

	logger.log("Initialize window classifier completed");
}

int k = 0;

L2Window CaptureWindow(HWND hWnd, string captureResultLogPath, Logger& logger)
{
	HDC hWindowDC = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hWindowDC);
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWindowDC, 0, 0, SRCCOPY);

	BitMapInfo bitMapInfo = createBitMapInfo(hBitmap);

	L2Window w = L2Window::UNKNOWN;
	for (auto& wc : windowClassifiers) {
		if (wc.isWindow(bitMapInfo, buttonClassifier)) {
			w = wc.getWindow();
			break;
		}
	}

	stringstream ss;
	ss << captureResultLogPath << k++ << ".bmp";
	auto path = ss.str();

	WriteBmpToFile(path.c_str(), bitMapInfo);

	delete[] bitMapInfo.data;

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hWindowDC);
	DeleteObject(hBitmap);

	return w;
}