#include "Wall.h"

#include "Manager/Config/ConfigManager.h"

Wall::Wall() 
: _defense(0.0f) {
  _buildingType = BuildingType::WALL;
}

Wall::~Wall() {}

Wall* Wall::create(int level, const std::string& buildingName) {
  Wall* p = new (std::nothrow) Wall();
  if (p && p->init(level, buildingName)) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool Wall::init(int level, const std::string& buildingName) {
  // 获取配置
  auto config = ConfigManager::getInstance()->getBuildingConfig(buildingName);
  _buildingName = buildingName;

  // 调用基类初始化通用外观
  if (!Building::init(config.image, BuildingType::WALL, level, 
                      config.gridCount, config.anchorRatioX, config.anchorRatioY, config.imageScale)) {
    return false;
  }

  // 初始化城墙特有属性
  this->_defense = config.defense;

  // 设置最大生命值（当前生命值将在 BuildingManager 中设置，默认为 MaxHP）
  this->_maxHP = config.maxHP;

  return true;
}
