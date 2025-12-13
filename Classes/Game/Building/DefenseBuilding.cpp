#include "DefenseBuilding.h"

#include "Manager/Config/ConfigManager.h"

DefenseBuilding::DefenseBuilding()
    : _attackRange(0.0f), _damage(0), _attackSpeed(1.0f) {
  _buildingType = BuildingType::DEFENSE;
}

DefenseBuilding::~DefenseBuilding() {}

DefenseBuilding* DefenseBuilding::create(int level,
                                         const std::string& buildingName) {
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
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(buildingName, level);
  _buildingName = buildingName;

  // 调用基类初始化通用外观
  if (!Building::init(config.image, BuildingType::DEFENSE, level,
                      config.gridCount, config.anchorRatioX,
                      config.anchorRatioY, config.imageScale)) {
    return false;
  }

  // 初始化防御特有属性
  this->_attackRange = config.attackRange;
  this->_damage = config.damage;  // 实际游戏中可能需要乘以 level 系数
  this->_attackSpeed = config.attackSpeed;

  return true;
}

void DefenseBuilding::upgrade() {
  // 调用基类升级（处理等级+1，纹理更新，血量更新）
  Building::upgrade();

  // 获取新等级的配置来更新防御属性
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);

  this->_attackRange = config.attackRange;
  this->_damage = config.damage;
  this->_attackSpeed = config.attackSpeed;

  CCLOG("DefenseBuilding upgraded: Damage -> %d", _damage);
}