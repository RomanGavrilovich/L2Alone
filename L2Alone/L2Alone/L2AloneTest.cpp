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

int main(int argc, char* argv[])
{
	testC2();

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


#endif // TEST