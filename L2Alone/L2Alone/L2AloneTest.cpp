#ifdef TEST

#include <iostream>
#include <map>
#include <sstream>
#include <Windows.h>

#include "WindowDefinition.h"
#include "WindowsDefinitions.h"
#include "TestUtils.h"
#include "Utils.h"
#include "WindowsClassifier.h"
#include "VisionUtils.h"
#include "BmpVisionInitializer.h"
#include "BmpVisionProvider.h"
#include "ConfigUtils.h"

using namespace std;

bool CAPTURE_ALL_WINDOWS = true;

struct TestL2VersionFailure {
	string path;
	L2Window expected;
	L2Window actual;
};

struct TestL2VersionReport {
	L2Version version;
	int testCount = 0;
	vector<TestL2VersionFailure> failures;
};

L2Window testL2WindowClassifier(string& pathToTestBmp, L2Window expected, WindowsClassifier& classifier) {

	if (CAPTURE_ALL_WINDOWS) {
		L2Window r = UNKNOWN;

		vector<L2Window> v;

		v.push_back(L2Window::AGREEMENT);
		v.push_back(L2Window::INCORRECT_PASSWORD);
		v.push_back(L2Window::ACCOUNT_IN_USE);
		v.push_back(L2Window::SERVERS);
		v.push_back(L2Window::CHARACTERS);

		for (L2Window w : v) {
			auto bmi = readBmpFile(pathToTestBmp.c_str());

			BmpVisionProvider provider(bmi);

			auto actual = classifier.waitForWindow(provider, w, 0);
			if (actual != UNKNOWN) {
				r = actual;
			}

			writeTestOutput(pathToTestBmp, expected, w, bmi);

			delete[] bmi.data;
		}

		return r;
	}
	else {
		auto bmi = readBmpFile(pathToTestBmp.c_str());

		BmpVisionProvider provider(bmi);

		auto actual = classifier.waitForWindow(provider, 0);

		writeTestOutput(pathToTestBmp, expected, actual, bmi);

		delete[] bmi.data;

		return actual;
	}
}

void debugCollision(L2Version version, L2Window expected, L2Window collision, string pathToSuit) {

	string referenceImage = pathToSuit + "/REF.bmp";

	VisionDefinition vDef;
	if (version == L2Version::C5) {
		WindowsDefinitions::initC5WindowsDefinitions(vDef);
	}
	else if(version == L2Version::C2){
		WindowsDefinitions::initC2WindowsDefinitions(vDef);
	}
	else if (version == L2Version::ESSENSE) {
		WindowsDefinitions::initEssenseWindowsDefinitions(vDef);
	}
	else {
		throw exception("Unexpected l2 version");
	}

	auto bDef = vDef.wDefs[L2Window::WELCOME].bDefs[0];
	RectHueDistributionCapturer capturer(vDef.wWidth, vDef.wHeight, bDef);

	// For every window we iterate and check 
	auto refBmp = readBmpFile(referenceImage.c_str());

	BmpVisionInitializer initializer(capturer);
	BmpVisionProvider initProvider(refBmp);

	VisionParams vParams = initializer.init(initProvider, 0);

	WindowsClassifier classifier(vDef, "");
	classifier.init(vParams);


	// Test
	string testPath = pathToSuit + "/" + getL2WindowName(expected) + ".bmp";

	auto bmi = readBmpFile(testPath.c_str());
	BmpVisionProvider testProvider(bmi);

	auto actual = classifier.waitForWindow(testProvider, collision, 100);

	writeTestOutput(testPath, expected, actual, bmi);

	delete[] bmi.data;

	if (expected != actual) {
		cout << "    >> "
			<< "Expected " << getL2WindowName(expected)
			<< ", but got " << getL2WindowName(actual)
			<< " for " << testPath << endl;
	}

	exit(0);
}

void testL2VersionSuite(L2Version version, string& pathToSuit, TestL2VersionReport& report) {

	vector<L2Window> windowsToTest;
	windowsToTest.push_back(L2Window::WELCOME);
	windowsToTest.push_back(L2Window::AGREEMENT);
	windowsToTest.push_back(L2Window::ACCOUNT_IN_USE);
	windowsToTest.push_back(L2Window::INCORRECT_PASSWORD);
	windowsToTest.push_back(L2Window::SERVERS);
	windowsToTest.push_back(L2Window::CHARACTERS);

	string referenceImage = pathToSuit + "/REF.bmp";

	VisionDefinition vDef;
	if (version == L2Version::C5) {
		WindowsDefinitions::initC5WindowsDefinitions(vDef);
	}
	else if (version == L2Version::C2) {
		WindowsDefinitions::initC2WindowsDefinitions(vDef);
	}
	else if (version == L2Version::ESSENSE) {
		WindowsDefinitions::initEssenseWindowsDefinitions(vDef);
	}
	else {
		throw exception("Unexpected l2 version");
	}

	auto bDef = vDef.wDefs[L2Window::WELCOME].bDefs[0];
	RectHueDistributionCapturer capturer(vDef.wWidth, vDef.wHeight, bDef);

	// For every window we iterate and check 
	auto refBmp = readBmpFile(referenceImage.c_str());

	BmpVisionInitializer initializer(capturer);
	BmpVisionProvider provider(refBmp);

	VisionParams vParams = initializer.init(provider, 0);

	WindowsClassifier classifier(vDef, "");
	classifier.init(vParams);

	// Test classification
	for (L2Window expected : windowsToTest) {

		string testPath = pathToSuit + "/" + getL2WindowName(expected) + ".bmp";
		auto actual = testL2WindowClassifier(testPath, expected, classifier);

		if (expected != actual) {
			report.failures.push_back(TestL2VersionFailure{ testPath, expected, actual });
		}

		report.testCount++;
	}
}

void testL2Version(TestL2VersionReport& report) {

	vector<string> testCasePaths;
	getTestVisionDirectories(report.version, testCasePaths);

	for (auto testCasePath : testCasePaths) {
		testL2VersionSuite(report.version, testCasePath, report);
	}
}

int main(int argc, char* argv[])
{
	CAPTURE_ALL_WINDOWS = false;

	/*debugCollision(L2Version::ESSENSE, L2Window::ACCOUNT_IN_USE, L2Window::ACCOUNT_IN_USE, "TestResources/Vision/Essense/1360x768");
	return 0;*/

	vector<TestL2VersionReport> reports;
	reports.push_back(TestL2VersionReport{ L2Version::C5 });
	reports.push_back(TestL2VersionReport{ L2Version::C2 });
	reports.push_back(TestL2VersionReport{ L2Version::ESSENSE });

	for (auto& report : reports) {
		testL2Version(report);
	}

	for (auto& report : reports) {
		cout << "========================================" << endl;
		cout << "Version: " << getL2VersionName(report.version) << endl;
		if (report.failures.size() > 0) {
			cout << "Test failures:" << endl;
			for (auto f : report.failures) {
				cout << "    >> "
					<< "Expected " << getL2WindowName(f.expected)
					<< ", but got " << getL2WindowName(f.actual)
					<< " for " << f.path << endl;
			}
		}

		cout << "Total tests count: " << report.testCount
			<< ". Failed tests count: " << report.failures.size() << endl;
	}
}

#endif // TEST