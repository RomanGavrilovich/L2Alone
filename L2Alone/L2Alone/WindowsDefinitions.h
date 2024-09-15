#pragma once

#include <map>
#include <vector>

#include "WindowDefinition.h"

using namespace std;

class WindowsDefinitions {

public:

	static VisionDefinition createEssenseVisionDefinition() {
		VisionDefinition vDef;
		initEssenseWindowsDefinitions(vDef);
		return vDef;
	}

	static void initEssenseWindowsDefinitions(VisionDefinition& dest) {

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		dest.wWidth = refScreenWidth;
		dest.wHeight = refScreenHeight;

		// Log-in screen
		vector<RectDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(RectDefinition{ 585, 467, 93, 18 });
		welcomeBtnDefs.push_back(RectDefinition{ 685, 467, 93, 18 });

		dest.inputFieldsDef.push_back(RectDefinition{ 636,413,128,17 });
		dest.inputFieldsDef.push_back(RectDefinition{ 636,435,128,17 });

		dest.wDefs[L2Window::WELCOME] = WindowDefinition{ welcomeBtnDefs, 0, 0 };

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Agreement screen
		vector<RectDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(RectDefinition{ 605, 610, 71, 19 });
		agreementBtnDefs.push_back(RectDefinition{ 685, 610, 71, 19 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<RectDefinition> serverBtnDefs;
		serverBtnDefs.push_back(RectDefinition{ 565, 548, 71, 19 });
		serverBtnDefs.push_back(RectDefinition{ 645, 548, 71, 19 });
		serverBtnDefs.push_back(RectDefinition{ 725, 548, 71, 19 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<RectDefinition> charsBtnDefs;
		charsBtnDefs.push_back(RectDefinition{ 624, 669, 113, 28, RefAnchor::CenterBottom });
		charsBtnDefs.push_back(RectDefinition{ 1221, 590, 93, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1221, 615, 93, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1221, 665, 93, 19, RefAnchor::BottomRight });

		auto charsDef = WindowDefinition{ charsBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CHARACTERS] = charsDef;
	}

	static VisionDefinition createC5VisionDefinition() {
		VisionDefinition vDef;
		initC5WindowsDefinitions(vDef);
		return vDef;
	}

	static void initC5WindowsDefinitions(VisionDefinition& dest) {

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		dest.wWidth = refScreenWidth;
		dest.wHeight = refScreenHeight;

		// Log-in screen
		vector<RectDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(RectDefinition{ 583, 402, 94, 21 });
		welcomeBtnDefs.push_back(RectDefinition{ 683, 402, 94, 21 });

		dest.inputFieldsDef.push_back(RectDefinition{ 640,351 ,123,12 });
		dest.inputFieldsDef.push_back(RectDefinition{ 640,373,123,12 });

		dest.wDefs[L2Window::WELCOME] = WindowDefinition{ welcomeBtnDefs, 0, 0 };

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Connecting screen
		/*vector<RectDefinition> connectingBtnDefs;
		connectingBtnDefs.push_back(RectDefinition{ 634, 434, 92, 20 });

		auto connectingDef = WindowDefinition{ connectingBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CONNECTING] = connectingDef;*/

		// Agreement screen
		vector<RectDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(RectDefinition{ 603, 568, 74, 21 });
		agreementBtnDefs.push_back(RectDefinition{ 683, 568, 74, 21 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<RectDefinition> serverBtnDefs;
		serverBtnDefs.push_back(RectDefinition{ 563, 410, 74, 21 });
		serverBtnDefs.push_back(RectDefinition{ 643, 410, 74, 21 });
		serverBtnDefs.push_back(RectDefinition{ 724, 410, 74, 21 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<RectDefinition> charsBtnDefs;
		charsBtnDefs.push_back(RectDefinition{ 623, 668, 114, 29, RefAnchor::CenterBottom });
		charsBtnDefs.push_back(RectDefinition{ 1219, 589, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1219, 613, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1219, 663, 94, 21, RefAnchor::BottomRight });

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
		vector<RectDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(RectDefinition{ 583, 377, 94, 21 });
		welcomeBtnDefs.push_back(RectDefinition{ 683, 377, 94, 21 });

		dest.wDefs[L2Window::WELCOME] = WindowDefinition{ welcomeBtnDefs, 0, 0 };

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Agreement screen
		vector<RectDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(RectDefinition{ 603, 541, 74, 21 });
		agreementBtnDefs.push_back(RectDefinition{ 683, 541, 74, 21 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<RectDefinition> serverBtnDefs;
		serverBtnDefs.push_back(RectDefinition{ 563, 384, 74, 21 });
		serverBtnDefs.push_back(RectDefinition{ 643, 384, 74, 21 });
		serverBtnDefs.push_back(RectDefinition{ 724, 384, 74, 21 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<RectDefinition> charsBtnDefs;
		charsBtnDefs.push_back(RectDefinition{ 623, 668, 114, 29, RefAnchor::CenterBottom });
		charsBtnDefs.push_back(RectDefinition{ 1219, 589, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1219, 613, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1219, 663, 94, 21, RefAnchor::BottomRight });

		auto charsDef = WindowDefinition{ charsBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CHARACTERS] = charsDef;
	}

	static VisionDefinition createC3VisionDefinition() {
		VisionDefinition vDef;

		// C3 layout is the same as Ñ4
		initC4WindowsDefinitions(vDef);
		return vDef;
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
		vector<RectDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(RectDefinition{ 606, 407, 64, 19 });
		welcomeBtnDefs.push_back(RectDefinition{ 691, 407, 64, 19 });

		auto welcomeDef = WindowDefinition{ welcomeBtnDefs, 0, 0 };
		dest.wDefs[L2Window::WELCOME] = welcomeDef;

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Agreement screen
		vector<RectDefinition> agreementBtnDefs;
		agreementBtnDefs.push_back(RectDefinition{ 580, 545, 67, 19 });
		agreementBtnDefs.push_back(RectDefinition{ 713, 545, 67, 19 });

		auto agreementDef = WindowDefinition{ agreementBtnDefs, 0, 0 };
		dest.wDefs[L2Window::AGREEMENT] = agreementDef;

		// Servers screen
		vector<RectDefinition> serverBtnDefs;
		serverBtnDefs.push_back(RectDefinition{ 555, 686, 67, 19 });
		serverBtnDefs.push_back(RectDefinition{ 647, 686, 67, 19 });
		serverBtnDefs.push_back(RectDefinition{ 741, 686, 67, 19 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<RectDefinition> charsBtnDefs;
		charsBtnDefs.push_back(RectDefinition{ 1229, 544, 99, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1229, 568, 99, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1229, 592, 99, 19, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1229, 616, 99, 19, RefAnchor::BottomRight });

		auto charsDef = WindowDefinition{ charsBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CHARACTERS] = charsDef;
	}

	static void initGeWindowsDefinitions(VisionDefinition& dest) {

		int refScreenWidth = 1360;
		int refScreenHeight = 768;

		dest.wWidth = refScreenWidth;
		dest.wHeight = refScreenHeight;

		// Log-in screen
		vector<RectDefinition> welcomeBtnDefs;
		welcomeBtnDefs.push_back(RectDefinition{ 583, 433, 94, 21 });
		welcomeBtnDefs.push_back(RectDefinition{ 683, 433, 94, 21 });

		// Right buttons
	/*	welcomeBtnDefs.push_back(RectDefinition{ 1248, 569, 94, 21, RefAnchor::BottomRight});
		welcomeBtnDefs.push_back(RectDefinition{ 1248, 595, 94, 21, RefAnchor::BottomRight });
		welcomeBtnDefs.push_back(RectDefinition{ 1248, 621, 94, 21, RefAnchor::BottomRight });
		welcomeBtnDefs.push_back(RectDefinition{ 1248, 647, 94, 21, RefAnchor::BottomRight });*/

		dest.wDefs[L2Window::WELCOME] = WindowDefinition{ welcomeBtnDefs, 0, 0 };

		auto accountInUseDef = WindowDefinition{ welcomeBtnDefs, 200, 300 };
		dest.wDefs[L2Window::ACCOUNT_IN_USE] = accountInUseDef;

		auto incorrectPasswordDef = WindowDefinition{ welcomeBtnDefs, 400, 500 };
		dest.wDefs[L2Window::INCORRECT_PASSWORD] = incorrectPasswordDef;

		// Loading modal
		vector<RectDefinition> loadingModal;
		loadingModal.push_back(RectDefinition{ 635, 434, 92, 21 });

		auto loadingWindow = WindowDefinition{ loadingModal, 0, 0 };
		dest.wDefs[L2Window::CONNECTING] = loadingWindow;

		// Servers screen
		vector<RectDefinition> serverBtnDefs;
		serverBtnDefs.push_back(RectDefinition{ 558, 393, 76, 21 });
		serverBtnDefs.push_back(RectDefinition{ 642, 393, 76, 21 });
		serverBtnDefs.push_back(RectDefinition{ 726, 393, 76, 21 });

		auto serversDef = WindowDefinition{ serverBtnDefs, 0, 0 };
		dest.wDefs[L2Window::SERVERS] = serversDef;

		// Chars screen
		vector<RectDefinition> charsBtnDefs;
		charsBtnDefs.push_back(RectDefinition{ 623, 668, 114, 29, RefAnchor::CenterBottom });
		charsBtnDefs.push_back(RectDefinition{ 1219, 589, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1219, 613, 94, 21, RefAnchor::BottomRight });
		charsBtnDefs.push_back(RectDefinition{ 1219, 663, 94, 21, RefAnchor::BottomRight });

		auto charsDef = WindowDefinition{ charsBtnDefs, 0, 0 };
		dest.wDefs[L2Window::CHARACTERS] = charsDef;
	}

	static VisionDefinition createGeVisionDefinition() {
		VisionDefinition vDef;
		initGeWindowsDefinitions(vDef);
		return vDef;
	}
};