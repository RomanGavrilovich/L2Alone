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

void testWelcomeWindowClassification() {

	map<L2Window, WindowDefinition> dest;
	initC5WindowsDefinitions(dest);

	auto welcomeDef = dest[L2Window::WELCOME];
	SingleWindowClassifier *classifier = createWinClass(welcomeDef);

	auto bmp = readBmpFile("TestResources/Vision/welcome.bmp");

	auto result = classifier->isWindow(bmp);

	cout << "Result" << result;
}

int main(int argc, char* argv[])
{
	testWelcomeWindowClassification();
}


#endif // TEST