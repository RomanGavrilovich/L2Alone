#pragma once

#include "WindowDefinition.h"
#include "AutologinStrategy.h"
#include "WindowsDefinitions.h"

class C5AutologinStrategy : public AutologinStrategy {

protected:
	void initVisionDefinition(VisionDefinition& def) override;
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
};

void C5AutologinStrategy::initVisionDefinition(VisionDefinition& def) {
	WindowsDefinitions::initC5WindowsDefinitions(def);
}

void C5AutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	def.startX = 680;
	def.startY = 682;
	def.dropDownX = 198;
	def.dropDownY = 48;
	def.dropdownItemHeight = 17;
	def.actionTimeout = 10;
}