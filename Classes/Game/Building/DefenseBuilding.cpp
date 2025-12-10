#include "DefenseBuilding.h"
#include "Manager/Config/ConfigManager.h"

DefenseBuilding::DefenseBuilding() 
: _attackRange(0.0f), _damage(0), _attackSpeed(1.0f) {
  _buildingType = BuildingType::DEFENSE;
}

DefenseBuilding::~DefenseBuilding() {}

DefenseBuilding* DefenseBuilding::create(int level, const std::string& buildingName) {
  DefenseBuilding* p = new (std::nothrow) DefenseBuilding();
  if (p && p->init(level, buildingName)) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool DefenseBuilding::init(int level, const std::string& buildingName) {
  // 获取配置
  auto config = ConfigManager::getInstance()->getBuildingConfig(buildingName);
  _buildingName = buildingName;

  // 调用基类初始化通用外观
  if (!Building::init(config.image, BuildingType::DEFENSE, level, 
                      config.gridCount, config.anchorRatioX, config.anchorRatioY, config.imageScale)) {
    return false;
  }

  // 初始化防御特有属性
  this->_attackRange = config.attackRange;
  this->_damage = config.damage; // 实际游戏中可能需要乘以 level 系数
  this->_attackSpeed = config.attackSpeed;

  // 设置最大生命值（当前生命值将在 BuildingManager 中设置，默认为 MaxHP）
  this->_maxHP = config.maxHP;

  return true;
}