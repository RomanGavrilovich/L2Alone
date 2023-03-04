#ifdef TEST

#include <iostream>
#include <map>

#include "WindowDefinition.h"
#include "WindowsDefinitions.h"
#include "SingleWindowClassifier.h"
#include "VisionTestUtils.h"

using namespace std;

SingleWindowClassifier* createWinClass(WindowDefinition& def) {
	return new SingleWindowClassifier(def.width, def.height, def.bDefs, def.textMinSize, def.textMaxSize);
}

bool testWelcomeWindowClassification() {

	map<L2Window, WindowDefinition> dest;
	initC5WindowsDefinitions(dest);

	auto welcomeDef = dest[L2Window::WELCOME];
	SingleWindowClassifier *classifier = createWinClass(welcomeDef);

	auto bmp = readBmpFile("TestResources/Vision/welcome.bmp");

	return classifier->isWindow(bmp);
}

void testServerClassification() {

	map<L2Window, WindowDefinition> dest;
	initC5WindowsDefinitions(dest);

	auto def = dest[L2Window::SERVERS];
	SingleWindowClassifier* classifier = createWinClass(def);

	auto bmp = readBmpFile("TestResources/Vision/servers.bmp");

	auto result = classifier->isWindow(bmp);

	cout << "Result: " << result;
}

int main(int argc, char* argv[])
{
	testServerClassification();
	//testWelcomeWindowClassification();
}


#endif // TEST