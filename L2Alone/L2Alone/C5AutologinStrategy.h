#pragma once

#include "WindowDefinition.h"
#include "AutologinStrategy.h"
#include "WindowsDefinitions.h"

class C5AutologinStrategy : public AutologinStrategy {

public:
	C5AutologinStrategy(L2AloneConfig &config);
protected:
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
};

C5AutologinStrategy::C5AutologinStrategy(L2AloneConfig& config) : AutologinStrategy(WindowsDefinitions::createC5VisionDefinition(), config) {

}

void C5AutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	def.startX = 680;
	def.startY = 682;
	def.startAnchor = RefAnchor::CenterBottom;
	def.dropDownX = 198;
	def.dropDownY = 48;
	def.dropdownItemHeight = 17;
	def.actionTimeout = 10;
}