#pragma once
#pragma once

#include "WindowDefinition.h"
#include "AutologinStrategy.h"
#include "WindowsDefinitions.h"

class KamaelAutologinStrategy : public AutologinStrategy {

public:
	KamaelAutologinStrategy(L2AloneConfig& config);
protected:
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
};

KamaelAutologinStrategy::KamaelAutologinStrategy(L2AloneConfig& config)
	: AutologinStrategy(WindowsDefinitions::createKamaelVisionDefinition(), config) {

}

void KamaelAutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	def.startX = 680;
	def.startY = 682;
	def.startAnchor = RefAnchor::CenterBottom;
	def.dropDownX = 198;
	def.dropDownY = 48;
	def.dropdownItemHeight = 17;
	def.actionTimeout = 10;
}