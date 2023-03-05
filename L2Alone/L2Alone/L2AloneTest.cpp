#ifdef TEST

#include <iostream>
#include <map>
#include <sstream>

#include "WindowDefinition.h"
#include "WindowsDefinitions.h"
#include "SingleWindowClassifier.h"
#include "VisionTestUtils.h"
#include "WindowsDefinitions.h"
#include "Utils.h"

using namespace std;

SingleWindowClassifier* createWinClass(WindowDefinition& def) {
	return new SingleWindowClassifier(def.width, def.height, def.bDefs, def.textMinSize, def.textMaxSize);
}

bool testWelcomeWindowClassification() {

	map<L2Window, WindowDefinition> dest;
	WindowsDefinitions::initC5WindowsDefinitions(dest);

	auto welcomeDef = dest[L2Window::WELCOME];
	SingleWindowClassifier* classifier = createWinClass(welcomeDef);

	auto bmp = readBmpFile("TestResources/Vision/welcome.bmp");

	return classifier->isWindow(bmp);
}

void testServerClassification() {

	map<L2Window, WindowDefinition> dest;
	WindowsDefinitions::initC5WindowsDefinitions(dest);

	auto def = dest[L2Window::SERVERS];
	SingleWindowClassifier* classifier = createWinClass(def);

	auto bmp = readBmpFile("TestResources/Vision/servers.bmp");

	auto result = classifier->isWindow(bmp);

	cout << "Result: " << result;
}

// C2 tests
void testC2();
void testC2WelcomeWindow();
void testC2AgreementScreen();
void testC2InUseSreen();


// C5 tests
void testC5();

int main(int argc, char* argv[])
{
	testC5();

	//testServerClassification();
	//testWelcomeWindowClassification();
}

void testC2() {

	testC2InUseSreen();

	//testC2WelcomeWindow();
	//testC2AgreementScreen();
}

void testC2WelcomeWindow() {

	map<L2Window, WindowDefinition> dest;
	WindowsDefinitions::initC2WindowsDefinitions(dest);

	auto def = dest[L2Window::WELCOME];
	SingleWindowClassifier* classifier = createWinClass(def);

	auto bmp = readBmpFile("TestResources/Vision/C2/welcome.bmp");

	auto result = classifier->isWindow(bmp);

	if (!result) {
		prepareDirectory("TestFailure");
		writeBmpToFile("TestFailure/testC2WelcomeWindow.bmp", bmp);
	}

	if (!result) {
		throw exception("Test failed");
	}
}

void testC2AgreementScreen() {
	map<L2Window, WindowDefinition> dest;
	WindowsDefinitions::initC2WindowsDefinitions(dest);

	auto def = dest[L2Window::AGREEMENT];
	SingleWindowClassifier* classifier = createWinClass(def);

	auto bmp = readBmpFile("TestResources/Vision/C2/agreement.bmp");

	auto result = classifier->isWindow(bmp);

	if (!result) {
		prepareDirectory("TestFailure");
		writeBmpToFile("TestFailure/testC2AgreementWindow.bmp", bmp);
	}

	if (!result) {
		throw exception("Test failed");
	}
}

void testC2InUseSreen() {

	map<L2Window, WindowDefinition> dest;
	WindowsDefinitions::initC2WindowsDefinitions(dest);

	auto def = dest[L2Window::ACCOUNT_IN_USE];
	SingleWindowClassifier* classifier = createWinClass(def);

	auto bmp = readBmpFile("TestResources/Vision/C2/use.bmp");

	auto result = classifier->isWindow(bmp);

	if (!result) {
		prepareDirectory("TestFailure");
		writeBmpToFile("TestFailure/testC2InUseWindow.bmp", bmp);
	}

	if (!result) {
		throw exception("Test failed");
	}
}

bool testClassifier(L2Window window, string testFile, string testName, string caseName) {

	map<L2Window, WindowDefinition> dest;
	WindowsDefinitions::initC5WindowsDefinitions(dest);

	auto def = dest[window];
	SingleWindowClassifier* classifier = createWinClass(def);

	auto bmp = readBmpFile(testFile.c_str());

	auto result = classifier->isWindow(bmp);

	prepareDirectory("TestCaptures");
	prepareDirectory("TestCaptures/" + testName);

	stringstream ss;
	ss << "TestCaptures/" << testName << "/" << caseName << ".bmp";
	writeBmpToFile(ss.str().c_str(), bmp);

	delete classifier;
	return result;
}

void assertFalse(bool value) {
	if (value) {
		throw exception("Assert failed");
	}
}

void assertTrue(bool value) {
	if (!value) {
		throw exception("Assert failure");
	}
}

// C5
void testC5WindowClassifier(L2Window window) {

	stringstream ssName;
	ssName << "testC5_" << getL2WindowName(window);
	string testName = ssName.str().c_str();

	bool isWelcome = testClassifier(window, "TestResources/Vision/C5/welcome.bmp", testName, "welcome");
	if (window == L2Window::WELCOME) {
		assertTrue(isWelcome);
	}
	else {
		assertFalse(isWelcome);
	}

	bool isPassword = testClassifier(window, "TestResources/Vision/C5/password.bmp", testName, "password");
	if (window == L2Window::INCORRECT_PASSWORD) {
		assertTrue(isPassword);
	}
	else {
		assertFalse(isPassword);
	}

	bool isUse = testClassifier(window, "TestResources/Vision/C5/use.bmp", testName, "use");
	if (window == L2Window::ACCOUNT_IN_USE) {
		assertTrue(isUse);
	}
	else {
		assertFalse(isUse);
	}

	bool isAgreement = testClassifier(window, "TestResources/Vision/C5/agreement.bmp", testName, "agreement");
	if (window == L2Window::AGREEMENT) {
		assertTrue(isAgreement);
	}
	else {
		assertFalse(isAgreement);
	}

	bool isServers = testClassifier(window, "TestResources/Vision/C5/servers.bmp", testName, "servers");
	if (window == L2Window::SERVERS) {
		assertTrue(isServers);
	}
	else {
		assertFalse(isServers);
	}
}

void testC5() {

	//testC5WindowClassifier(L2Window::WELCOME);
	testC5WindowClassifier(L2Window::INCORRECT_PASSWORD);
	testC5WindowClassifier(L2Window::ACCOUNT_IN_USE);
	testC5WindowClassifier(L2Window::AGREEMENT);
	testC5WindowClassifier(L2Window::SERVERS);
	testC5WindowClassifier(L2Window::CHARACTERS);
}

#endif // TEST