#pragma once

#include <Windows.h>

#include "AutologinStrategy.h"
#include "WindowDefinition.h"
#include "WindowsDefinitions.h"

class C2AutologinStrategy : public AutologinStrategy {

public:
	C2AutologinStrategy();

protected:
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
	void onAccountInUse(HWND hWindow) override;

	bool fastFlowSupported(L2CharSlot slot);
	bool stopFastLogin(L2Window w);
};

C2AutologinStrategy::C2AutologinStrategy() : AutologinStrategy(WindowsDefinitions::createC2VisionDefinition()) {

}

bool C2AutologinStrategy::fastFlowSupported(L2CharSlot slot) {
	return true;
}

bool C2AutologinStrategy::stopFastLogin(L2Window w) {
	return w == CHARACTERS;
}

void C2AutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	if (def.slot == L2CharSlot::ACTIVE) {
		def.slot = L2CharSlot::SLOT_1;
	}
	def.dropdownItemHeight = 12;
	def.dropDownX = 185;
	def.dropDownY = 43;
	def.startX = 1280;
	def.startY = 600;
	def.actionTimeout = 25;
}

void C2AutologinStrategy::onAccountInUse(HWND hWindow) {
	Sleep(100);
	postControlMessage(hWindow, VK_TAB);
	Sleep(100);
}