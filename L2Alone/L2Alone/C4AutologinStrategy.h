#pragma once

#include "WindowDefinition.h"
#include "AutologinStrategy.h"
#include "WindowsDefinitions.h"

class C4AutologinStrategy : public AutologinStrategy {

public:
	C4AutologinStrategy(L2AloneConfig& config);
protected:
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
	void onAccountInUse(HWND hWindow) override;
};

C4AutologinStrategy::C4AutologinStrategy(L2AloneConfig& config) : AutologinStrategy(WindowsDefinitions::createC4VisionDefinition(), config) {

}

void C4AutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	def.startX = 680;
	def.startY = 682;
	def.startAnchor = RefAnchor::CenterBottom;
	def.dropDownX = 198;
	def.dropDownY = 48;
	def.dropdownItemHeight = 17;
	def.actionTimeout = 10;
}

void C4AutologinStrategy::onAccountInUse(HWND hWindow) {
	Sleep(config.inputInitialDelay);
	postControlMessage(hWindow, VK_TAB);
	Sleep(config.accountInUseDelay);
}