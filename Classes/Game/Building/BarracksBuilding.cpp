#include "BarracksBuilding.h"
#include "Manager/Config/ConfigManager.h"

BarracksBuilding::BarracksBuilding() 
: _queueSize(0) {
  _buildingType = BuildingType::BARRACKS;
}

BarracksBuilding::~BarracksBuilding() {}

BarracksBuilding* BarracksBuilding::create(int level, const std::string& buildingName) {
  BarracksBuilding* p = new (std::nothrow) BarracksBuilding();
  if (p && p->init(level, buildingName)) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool BarracksBuilding::init(int level, const std::string& buildingName) {
  auto config = ConfigManager::getInstance()->getBuildingConfig(buildingName);
  _buildingName = buildingName;

  if (!Building::init(config.image, BuildingType::BARRACKS, level, 
                      config.gridCount, config.anchorRatioX, config.anchorRatioY, config.imageScale)) {
    return false;
  }

  this->_queueSize = config.queueSize;

  return true;
}