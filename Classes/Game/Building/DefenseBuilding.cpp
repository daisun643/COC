#include "DefenseBuilding.h"

DefenseBuilding::DefenseBuilding()
    : _damage(10), _range(200.0f), _attackSpeed(1.0f),
      _defenseType(DefenseType::CANNON) {
  _buildingType = BuildingType::DEFENSE;
  _maxLevel = 10;
}

DefenseBuilding::~DefenseBuilding() {}

DefenseBuilding *DefenseBuilding::create(DefenseType defenseType, int level) {
  DefenseBuilding *building = new (std::nothrow) DefenseBuilding();
  if (building && building->init(defenseType, level)) {
    building->autorelease();
    return building;
  }
  CC_SAFE_DELETE(building);
  return nullptr;
}

bool DefenseBuilding::init(DefenseType defenseType, int level) {
  _defenseType = defenseType;

  // 根据防御类型设置名称
  switch (defenseType) {
  case DefenseType::CANNON:
    _buildingName = "Cannon";
    break;
  case DefenseType::ARCHER_TOWER:
    _buildingName = "Archer Tower";
    break;
  case DefenseType::WALL:
    _buildingName = "Wall";
    break;
  case DefenseType::MORTAR:
    _buildingName = "Mortar";
    break;
  }

  if (!Building::init("", BuildingType::DEFENSE, level)) {
    return false;
  }

  initDefenseProperties();

  return true;
}

void DefenseBuilding::initDefenseProperties() {
  // 根据防御类型和等级初始化属性
  switch (_defenseType) {
  case DefenseType::CANNON:
    _damage = 20 * _level;
    _range = 150.0f;
    _attackSpeed = 1.0f;
    break;
  case DefenseType::ARCHER_TOWER:
    _damage = 15 * _level;
    _range = 200.0f;
    _attackSpeed = 0.8f;
    break;
  case DefenseType::WALL:
    _damage = 0; // 城墙不攻击
    _range = 0;
    _attackSpeed = 0;
    break;
  case DefenseType::MORTAR:
    _damage = 50 * _level;
    _range = 300.0f;
    _attackSpeed = 2.0f;
    break;
  }
}

void DefenseBuilding::attack(Node *target) {
  if (!target || _defenseType == DefenseType::WALL) {
    return; // 城墙不攻击
  }

  // 计算距离
  Vec2 targetPos = target->getPosition();
  Vec2 myPos = this->getPosition();
  float distance = myPos.distance(targetPos);

  if (distance <= _range) {
    // 可以在这里添加攻击动画和伤害计算
    // 例如：创建子弹、播放音效等
    CCLOG("%s attacks target at distance %.2f", _buildingName.c_str(),
          distance);
  }
}

std::string DefenseBuilding::getBuildingInfo() const {
  char buffer[256];
  if (_defenseType == DefenseType::WALL) {
    snprintf(buffer, sizeof(buffer),
             "%s\nLevel: %d/%d\nDefense: %d HP\nUpgrade Cost: %d",
             _buildingName.c_str(), _level, _maxLevel, _level * 100,
             getUpgradeCost());
  } else {
    snprintf(buffer, sizeof(buffer),
             "%s\nLevel: %d/%d\nDamage: %d\nRange: %.0f\nSpeed: %.1f\nUpgrade "
             "Cost: %d",
             _buildingName.c_str(), _level, _maxLevel, _damage, _range,
             _attackSpeed, getUpgradeCost());
  }
  return std::string(buffer);
}
