#pragma once

#include <map>
#include <vector>

#include "WindowDefinition.h"

using namespace std;

static class WindowsDefinitions {

public:

	static void initC5WindowsDefinitions(VisionDefinition &dest) {

		int refScreenWidth = 1360;
		int refScreenHeight = 768;
		
		dest.wWidth = refScreenWidth;
		dest.wHeight = refScreenHeight;

		// Log-in screen
		vector<ButtonDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(ButtonDefinition{ 583, 402, 94, 21 });
		welcomeBtnDefs.push_back(ButtonDefinition{ 683, 402, 94, 21 });

		dest.wDefs[L2Window::WELCOME] = WindowDefinition{ welcomeBtnDefs, 0, 0 };

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Agreement screen
		vector<ButtonDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(ButtonDefinition{ 603, 568, 74, 21 });
		agreementBtnDefs.push_back(ButtonDefinition{ 683, 568, 74, 21 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<ButtonDefinition> serverBtnDefs;
		serverBtnDefs.push_back(ButtonDefinition{ 563, 410, 74, 21 });
		serverBtnDefs.push_back(ButtonDefinition{ 643, 410, 74, 21 });
		serverBtnDefs.push_back(ButtonDefinition{ 724, 410, 74, 21 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<ButtonDefinition> charsBtnDefs;
		charsBtnDefs.push_back(ButtonDefinition{ 623, 668, 114, 29, RefAnchor::CenterBottom });
		charsBtnDefs.push_back(ButtonDefinition{ 1219, 589, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(ButtonDefinition{ 1219, 613, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(ButtonDefinition{ 1219, 663, 94, 21, RefAnchor::BottomRight });

		auto charsDef = WindowDefinition{ charsBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CHARACTERS] = charsDef;
	}

	static void initC2WindowsDefinitions(VisionDefinition& dest) {

		//int refScreenWidth = 1360;
		//int refScreenHeight = 768;

		//// Log-in screen
		//vector<ButtonDefinition> welcomeBtnDefs;
		//welcomeBtnDefs.push_back(ButtonDefinition{ 606, 407, 64, 19 });
		//welcomeBtnDefs.push_back(ButtonDefinition{ 691, 407, 64, 19 });

		//auto welcomeDef = WindowDefinition{ refScreenWidth, refScreenHeight, welcomeBtnDefs, 0, 0 };
		//dest[L2Window::WELCOME] = welcomeDef;

		//auto accountInUseDef = WindowDefinition{ refScreenWidth, refScreenHeight, welcomeBtnDefs, 200, 300 };
		//dest[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		//auto incorrectPasswordDef = WindowDefinition{ refScreenWidth, refScreenHeight, welcomeBtnDefs, 400, 500 };
		//dest[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		//// Agreement screen
		//vector<ButtonDefinition> agreementBtnDefs;
		//agreementBtnDefs.push_back(ButtonDefinition{ 580, 545, 67, 19 });
		//agreementBtnDefs.push_back(ButtonDefinition{ 713, 545, 67, 19 });

		//auto agreementDef = WindowDefinition{ refScreenWidth, refScreenHeight, agreementBtnDefs, 0, 0 };
		//dest[L2Window::AGREEMENT] = agreementDef;

		//// Servers screen
		//vector<ButtonDefinition> serverBtnDefs;
		//serverBtnDefs.push_back(ButtonDefinition{ 555, 686, 67, 19 });
		//serverBtnDefs.push_back(ButtonDefinition{ 647, 686, 67, 19 });
		//serverBtnDefs.push_back(ButtonDefinition{ 741, 686, 67, 19 });

		//auto serversDef = WindowDefinition{ refScreenWidth, refScreenHeight, serverBtnDefs, 0, 0 };
		//dest[L2Window::SERVERS] = serversDef;

		//// Chars screen
		//vector<ButtonDefinition> charsBtnDefs;
		//charsBtnDefs.push_back(ButtonDefinition{ 1229, 544, 99, 19, RefAnchor::BottomRight });
		//charsBtnDefs.push_back(ButtonDefinition{ 1229, 568, 99, 19, RefAnchor::BottomRight });
		//charsBtnDefs.push_back(ButtonDefinition{ 1229, 592, 99, 19, RefAnchor::BottomRight });
		//charsBtnDefs.push_back(ButtonDefinition{ 1229, 616, 99, 19, RefAnchor::BottomRight });

		//auto charsDef = WindowDefinition{ refScreenWidth, refScreenHeight, charsBtnDefs, 0, 0 };
		//dest[L2Window::CHARACTERS] = charsDef;
	}
};