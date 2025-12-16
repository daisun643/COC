#include "TownHall.h"

#include "Manager/Config/ConfigManager.h"

TownHall::TownHall() {
  _buildingType = BuildingType::TOWN_HALL;
  _buildingName = "TownHall";
  _maxLevel = 10;
}

TownHall::~TownHall() {}

TownHall* TownHall::create(int level) {
  TownHall* townHall = new (std::nothrow) TownHall();
  auto constantConfig = ConfigManager::getInstance()->getConstantConfig();
  auto townHallConfig =
      ConfigManager::getInstance()->getBuildingConfig("TownHall");

  if (townHall &&
      townHall->init(level, townHallConfig.image, townHallConfig.gridCount,
                     townHallConfig.anchorRatioX, townHallConfig.anchorRatioY,
                     townHallConfig.imageScale)) {
    townHall->autorelease();
    return townHall;
  }
  CC_SAFE_DELETE(townHall);
  return nullptr;
}

bool TownHall::init(int level, std::string imagePath, int gridCount,
                    float anchorRatioX, float anchorRatioY, float imageScale) {
  if (!Building::init(imagePath, BuildingType::TOWN_HALL, level, gridCount,
                      anchorRatioX, anchorRatioY, imageScale)) {
    return false;
  }

  // 设置最大生命值（当前生命值将在 BuildingManager 中设置，默认为 MaxHP）
  auto config = ConfigManager::getInstance()->getBuildingConfig("TownHall");
  this->_maxHP = config.maxHP;

  return true;
}

void TownHall::upgrade() {
  // 调用基类升级（更新等级、纹理、血量）
  Building::upgrade();

  // 如果大本营升级有特殊逻辑（比如触发成就、解锁新功能），可以在这里添加
  // 目前主要依赖配置文件的数值变化
  CCLOG("TownHall upgraded to level %d.", _level);
}