#pragma once

#include <Windows.h>

#include "AutologinStrategy.h"
#include "WindowDefinition.h"
#include "WindowsDefinitions.h"

class C2AutologinStrategy : public AutologinStrategy {

public:

protected:
	void initVisionDefinition(VisionDefinition& def) override;
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
	void onAccountInUse(HWND hWindow) override;
};

void C2AutologinStrategy::initVisionDefinition(VisionDefinition& def) {
	WindowsDefinitions::initC2WindowsDefinitions(def);
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
	def.actionTimeout = 50;
}

void C2AutologinStrategy::onAccountInUse(HWND hWindow) {
	Sleep(100);
	postControlMessage(hWindow, VK_TAB);
	Sleep(100);
}