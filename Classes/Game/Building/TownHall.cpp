#include "TownHall.h"

#include "Manager/Config/ConfigManager.h"

TownHall::TownHall() {
  _buildingType = BuildingType::TOWN_HALL;
  _buildingName = "TownHall";  // 这里硬编码对应配置文件中的 Key
}

TownHall::~TownHall() {}

TownHall* TownHall::create(int level) {
  TownHall* townHall = new (std::nothrow) TownHall();
  // 简化了 create，直接调用 init(level)
  if (townHall && townHall->init(level)) {
    townHall->autorelease();
    return townHall;
  }
  CC_SAFE_DELETE(townHall);
  return nullptr;
}

bool TownHall::init(int level) {
  // 根据 _buildingName ("TownHall") 和 level 获取配置
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(_buildingName, level);

  // 调用基类初始化
  if (!Building::init(config.image, BuildingType::TOWN_HALL, level,
                      config.gridCount, config.anchorRatioX,
                      config.anchorRatioY, config.imageScale)) {
    return false;
  }

  // 设置属性
  this->_maxLevel = config.maxLevel;
  this->_maxHealth = config.health;
  this->_currentHealth = _maxHealth;

  return true;
}

void TownHall::upgrade() {
  // 调用基类升级（更新等级、纹理、血量）
  Building::upgrade();

  // 如果大本营升级有特殊逻辑（比如触发成就、解锁新功能），可以在这里添加
  // 目前主要依赖配置文件的数值变化
  CCLOG("TownHall upgraded to level %d. MaxHP: %.0f", _level, _maxHealth);
}