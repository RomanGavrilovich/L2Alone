#ifdef TEST

#include <iostream>
#include <map>
#include <sstream>
#include <Windows.h>

#include "WindowDefinition.h"
#include "WindowsDefinitions.h"
#include "TestUtils.h"
#include "WindowsDefinitions.h"
#include "Utils.h"
#include "WindowsClassifier.h"
#include "VisionUtils.h"
#include "BmpVisionInitializer.h"
#include "BmpVisionProvider.h"
#include "config_utils.h"

using namespace std;

struct TestL2VersionFailure {
	string path;
	L2Window expected;
	L2Window actual;
};

struct TestL2VersionReport {
	int testCount = 0;
	vector<TestL2VersionFailure> failures;
};

L2Window testL2WindowClassifier(string& pathToTestBmp, WindowsClassifier& classifier) {

	auto bmi = readBmpFile(pathToTestBmp.c_str());

	BmpVisionProvider provider(bmi);

	auto r = classifier.waitForWindow(provider, 100);

	delete[] bmi.data;

	return r;
}

void testL2VersionSuite(L2Version version, string& pathToSuit, TestL2VersionReport& report) {

	vector<L2Window> windowsToTest;
	windowsToTest.push_back(L2Window::AGREEMENT);
	windowsToTest.push_back(L2Window::ACCOUNT_IN_USE);
	windowsToTest.push_back(L2Window::INCORRECT_PASSWORD);
	windowsToTest.push_back(L2Window::SERVERS);
	windowsToTest.push_back(L2Window::CHARACTERS);

	string referenceImage = pathToSuit + "/WELCOME.bmp";

	VisionDefinition vDef;
	if (version == L2Version::C5) {
		WindowsDefinitions::initC5WindowsDefinitions(vDef);
	}
	else {
		throw exception("Unexpected l2 version");
	}

	auto bDef = vDef.wDefs[L2Window::WELCOME].bDefs[0];
	ButtonHueDistributionCapturer capturer(vDef.wWidth, vDef.wHeight, bDef);

	// For every window we iterate and check 
	auto refBmp = readBmpFile(referenceImage.c_str());

	BmpVisionInitializer initializer(capturer);
	BmpVisionProvider provider(refBmp);

	VisionParams vParams = initializer.init(provider, 0);

	WindowsClassifier classifier(vDef);
	classifier.init(vParams);

	for (L2Window expected : windowsToTest) {

		string testPath = pathToSuit + "/" + getL2WindowName(expected) + ".bmp";
		auto actual = testL2WindowClassifier(testPath, classifier);

		if (expected != actual) {
			report.failures.push_back(TestL2VersionFailure{ testPath, expected, actual });
		}

		report.testCount++;
	}
}

void testL2Version(L2Version version, TestL2VersionReport& report) {

	vector<string> testCasePaths;
	getTestVisionDirectories(version, testCasePaths);

	for (auto testCasePath : testCasePaths) {
		testL2VersionSuite(version, testCasePath, report);
	}
}


int main(int argc, char* argv[])
{
	L2Version version = L2Version::C5;
	TestL2VersionReport report;
	testL2Version(version, report);

	cout << "========================================" << endl;

	cout << "L2 Version " << getL2VersionName(version)
		<< ". Total tests count: " << report.testCount 
		<< ". Failed tests count: " << report.failures.size() << endl;
}

#endif // TEST