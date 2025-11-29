#include "TownHall.h"
#include "Manager/Config/ConfigManager.h"

TownHall::TownHall() {
  _buildingType = BuildingType::TOWN_HALL;
  _buildingName = "Town Hall";
  _maxLevel = 10;
}

TownHall::~TownHall() {}

TownHall* TownHall::create(int level) {
  TownHall* townHall = new (std::nothrow) TownHall();
  auto constantConfig = ConfigManager::getInstance()->getConstantConfig();
  auto townHallConfig = ConfigManager::getInstance()->getTownHallConfig();

  if (townHall && townHall->init(level, townHallConfig.image, townHallConfig.gridCount, townHallConfig.anchorRatioX, 
      townHallConfig.anchorRatioY, townHallConfig.imageScale)) {
    townHall->autorelease();
    return townHall;
  }
  CC_SAFE_DELETE(townHall);
  return nullptr;
}

bool TownHall::init(int level,std::string imagePath, int gridCount, float anchorRatioX,
                    float anchorRatioY,float imageScale) {
  if (!Building::init(imagePath, BuildingType::TOWN_HALL,
                      level, gridCount, anchorRatioX, anchorRatioY, imageScale)) {
    return false;
  }
  return true;
}