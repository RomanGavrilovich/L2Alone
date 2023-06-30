#pragma once

#include <map>
#include <vector>

#include "WindowDefinition.h"

using namespace std;

class WindowsDefinitions {

public:

	static VisionDefinition createC5VisionDefinition() {
		VisionDefinition vDef;
		initC5WindowsDefinitions(vDef);
		return vDef;
	}

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

		// Connecting screen
		vector<ButtonDefinition> connectingBtnDefs;
		connectingBtnDefs.push_back(ButtonDefinition{ 634, 434, 92, 20 });

		auto connectingDef = WindowDefinition{ connectingBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CONNECTING] = connectingDef;

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

	static VisionDefinition createC4VisionDefinition() {
		VisionDefinition vDef;
		initC4WindowsDefinitions(vDef);
		return vDef;
	}

	static void initC4WindowsDefinitions(VisionDefinition& dest) {

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		dest.wWidth = refScreenWidth;
		dest.wHeight = refScreenHeight;

		// Log-in screen
		vector<ButtonDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(ButtonDefinition{ 583, 377, 94, 21 });
		welcomeBtnDefs.push_back(ButtonDefinition{ 683, 377, 94, 21 });

		dest.wDefs[L2Window::WELCOME] = WindowDefinition{ welcomeBtnDefs, 0, 0 };

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Agreement screen
		vector<ButtonDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(ButtonDefinition{ 603, 541, 74, 21 });
		agreementBtnDefs.push_back(ButtonDefinition{ 683, 541, 74, 21 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<ButtonDefinition> serverBtnDefs;
		serverBtnDefs.push_back(ButtonDefinition{ 563, 384, 74, 21 });
		serverBtnDefs.push_back(ButtonDefinition{ 643, 384, 74, 21 });
		serverBtnDefs.push_back(ButtonDefinition{ 724, 384, 74, 21 });

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

	static VisionDefinition createC2VisionDefinition() {
		VisionDefinition vDef;
		initC2WindowsDefinitions(vDef);
		return vDef;
	}

	static void initC2WindowsDefinitions(VisionDefinition& dest) {

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		dest.wWidth = refScreenWidth;
		dest.wHeight = refScreenHeight;

		// Log-in screen
		vector<ButtonDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(ButtonDefinition{ 606, 407, 64, 19 });
		welcomeBtnDefs.push_back(ButtonDefinition{ 691, 407, 64, 19 });

		auto welcomeDef = WindowDefinition{ welcomeBtnDefs, 0, 0 };
		dest.wDefs[L2Window::WELCOME] = welcomeDef;

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Agreement screen
		vector<ButtonDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(ButtonDefinition{ 580, 545, 67, 19 });
		agreementBtnDefs.push_back(ButtonDefinition{ 713, 545, 67, 19 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<ButtonDefinition> serverBtnDefs;
		serverBtnDefs.push_back(ButtonDefinition{ 555, 686, 67, 19 });
		serverBtnDefs.push_back(ButtonDefinition{ 647, 686, 67, 19 });
		serverBtnDefs.push_back(ButtonDefinition{ 741, 686, 67, 19 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<ButtonDefinition> charsBtnDefs;
		charsBtnDefs.push_back(ButtonDefinition{ 1229, 544, 99, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(ButtonDefinition{ 1229, 568, 99, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(ButtonDefinition{ 1229, 592, 99, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(ButtonDefinition{ 1229, 616, 99, 19, RefAnchor::BottomRight });

		auto charsDef = WindowDefinition{ charsBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CHARACTERS] = charsDef;
	}
};