#ifdef TEST

#include <iostream>
#include <map>

#include "WindowsDefinitions.h"
#include "SingleWindowClassifier.h"
#include "VisionUtils.h"
#include "VisionTestUtils.h"

using namespace std;

void testWelcomeWindowClassification() {

	map<L2Window, WindowDefinition> dest;
	WindowDefinitions::initC5WindowsDefinitions(dest);

	auto welcomeDef = dest[L2Window::WELCOME];
	SingleWindowClassifier *classifier = createSingleWindowClassifier(welcomeDef);

	auto bmp = readBmpFile("welcome.bmp");

	auto result = classifier->isWindow(bmp);

	cout << "Result" << result;
}

int main(int argc, char* argv[])
{
	testWelcomeWindowClassification();
}


#endif // TEST