#pragma once

#include "WindowDefinition.h"
#include "AutologinStrategy.h"
#include "WindowsDefinitions.h"

class GeAutologinStrategy : public AutologinStrategy {

public:
	GeAutologinStrategy(L2AloneConfig& config);
protected:
	void initSelectCharDefinition(SelectCharacterDefinition& def) override;
};

GeAutologinStrategy::GeAutologinStrategy(L2AloneConfig& config) 
	: AutologinStrategy(WindowsDefinitions::createGeVisionDefinition(), config) {

}

void GeAutologinStrategy::initSelectCharDefinition(SelectCharacterDefinition& def) {
	def.startX = 680;
	def.startY = 682;
	def.startAnchor = RefAnchor::CenterBottom;
	def.dropDownX = 198;
	def.dropDownY = 48;
	def.dropdownItemHeight = 17;
	def.actionTimeout = 10;
}