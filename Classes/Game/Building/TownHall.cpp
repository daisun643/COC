#include "TownHall.h"

TownHall::TownHall() {
  _buildingType = BuildingType::TOWN_HALL;
  _buildingName = "Town Hall";
  _maxLevel = 10;
}

TownHall::~TownHall() {}

TownHall *TownHall::create(int level) {
  TownHall *townHall = new (std::nothrow) TownHall();
  if (townHall && townHall->init(level)) {
    townHall->autorelease();
    return townHall;
  }
  CC_SAFE_DELETE(townHall);
  return nullptr;
}

bool TownHall::init(int level) {
  // 使用TownHall图片
  if (!Building::init("images/buildings/TownHall.png", BuildingType::TOWN_HALL, level)) {
    return false;
  }

  // 缩放会在GameScene中根据2*deltaX设置
  // 这里不再设置固定缩放

  return true;
}

TownHall *TownHall::create(int level, int gridSize, float anchorRatioX, float anchorRatioY,
                           float deltaX, float grassWidth) {
  TownHall *townHall = new (std::nothrow) TownHall();
  if (townHall && townHall->init(level, gridSize, anchorRatioX, anchorRatioY, deltaX, grassWidth)) {
    townHall->autorelease();
    return townHall;
  }
  CC_SAFE_DELETE(townHall);
  return nullptr;
}

bool TownHall::init(int level, int gridSize, float anchorRatioX, float anchorRatioY,
                    float deltaX, float grassWidth) {
  // 使用新的Building::init方法，传入gridSize和anchorRatio
  if (!Building::init("images/buildings/TownHall.png", BuildingType::TOWN_HALL, level,
                      gridSize, anchorRatioX, anchorRatioY, deltaX, grassWidth)) {
    return false;
  }
  
  return true;
}

int TownHall::getMaxBuildingLevel() const {
  // 大本营等级决定其他建筑的最大等级
  // 例如：大本营1级 -> 其他建筑最大1级
  //      大本营5级 -> 其他建筑最大5级
  return _level;
}

std::string TownHall::getBuildingInfo() const {
  char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "%s\nLevel: %d/%d\nMax Building Level: %d\nUpgrade Cost: %d",
           _buildingName.c_str(), _level, _maxLevel, getMaxBuildingLevel(),
           getUpgradeCost());
  return std::string(buffer);
}
