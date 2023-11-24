#pragma once

#include "WindowDefinition.h"
#include "AutologinStrategy.h"
#include "WindowsDefinitions.h"

class C3AutologinStrategy : public AutologinStrategy {

public:
	C3AutologinStrategy(L2AloneConfig& config);
protected:
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
	void onAccountInUse(HWND hWindow) override;
	void beforeCharSlotSelection(HWND hWindow, int targetX, int targetY) override;
};

C3AutologinStrategy::C3AutologinStrategy(L2AloneConfig& config) : AutologinStrategy(WindowsDefinitions::createC4VisionDefinition(), config) {

}

void C3AutologinStrategy::beforeCharSlotSelection(HWND hWindow, int targetX, int targetY) {
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			sendLowLevelMouseEvent(hWindow, targetX - 5 + i, targetY - 5 + j, MOUSEEVENTF_MOVE);
		}
	}
}

void C3AutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	def.startX = 680;
	def.startY = 682;
	def.startAnchor = RefAnchor::CenterBottom;
	def.dropDownX = 198;
	def.dropDownY = 48;
	def.dropdownItemHeight = 17;
	def.actionTimeout = 10;
}

void C3AutologinStrategy::onAccountInUse(HWND hWindow) {
	Sleep(config.inputInitialDelay);
	postControlMessage(hWindow, VK_TAB);
	Sleep(config.accountInUseDelay);
}