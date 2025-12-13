#include "DefenseBuilding.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "Manager/Config/ConfigManager.h"

DefenseBuilding::DefenseBuilding()
    : _attackRange(0.0f),
      _damage(0),
      _attackSpeed(1.0f),
      _currentTarget(nullptr),
      _attackCooldown(0.0f) {
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
  auto config = ConfigManager::getInstance()->getBuildingConfig(buildingName, level);
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

  // 设置最大生命值（当前生命值将在 BuildingManager 中设置，默认为 MaxHP）
  this->_maxHP = config.maxHP;

  return true;
}

bool DefenseBuilding::attackSoldiers(
    const std::vector<BasicSoldier*>& soldiers,
    const std::vector<SoldierCategory>& targetCategories, float delta) {
  // 更新攻击冷却时间
  if (_attackCooldown > 0.0f) {
    _attackCooldown -= delta;
  }

  // 如果建筑已死亡，无法攻击
  if (!isAlive()) {
    _currentTarget = nullptr;
    return false;
  }

  // 获取建筑中心位置
  Vec2 buildingPos = Vec2(_centerX, _centerY);
  // CCLOG("buildingPos: %f, %f", buildingPos.x, buildingPos.y);
  // 过滤出符合条件的士兵：存活、在攻击范围内、类别匹配
  std::vector<BasicSoldier*> validTargets;
  for (auto* soldier : soldiers) {
    if (!soldier || !soldier->isAlive()) {
      continue;
    }
    // CCLOG("soldier位置: %f, %f", soldier->getPosition().x,
    // soldier->getPosition().y); 检查类别是否匹配
    bool categoryMatch =
        targetCategories.empty();  // 如果类别列表为空，攻击所有类别
    if (!categoryMatch) {
      for (auto category : targetCategories) {
        if (soldier->getSoldierCategory() == category) {
          categoryMatch = true;
          break;
        }
      }
    }

    if (!categoryMatch) {
      continue;
    }

    // 检查是否在攻击范围内
    Vec2 soldierPos = soldier->getPosition();
    float distance = buildingPos.distance(soldierPos);
    // TODO: 临时修改攻击范围为200像素
    if (distance <= 200.0f) {
      validTargets.push_back(soldier);
    }
  }

  // 如果没有有效目标，清除当前目标
  if (validTargets.empty()) {
    _currentTarget = nullptr;
    return false;
  }

  // 找到最近的目标
  BasicSoldier* nearestTarget = nullptr;
  float nearestDistance = FLT_MAX;
  for (auto* target : validTargets) {
    Vec2 targetPos = target->getPosition();
    float distance = buildingPos.distance(targetPos);
    if (distance < nearestDistance) {
      nearestDistance = distance;
      nearestTarget = target;
    }
  }

  // 更新当前目标
  _currentTarget = nearestTarget;

  // 如果攻击冷却时间已过，进行攻击
  if (_attackCooldown <= 0.0f && _currentTarget) {
    // 对目标造成伤害
    _currentTarget->takeDamage(static_cast<float>(_damage));

    // 重置攻击冷却时间（攻击速度是每秒攻击次数，所以冷却时间是 1/攻击速度）
    _attackCooldown = (_attackSpeed > 0.0f) ? (1.0f / _attackSpeed) : 1.0f;

    return true;
  }

  return false;
}

bool DefenseBuilding::attackSoldiers(const std::vector<BasicSoldier*>& soldiers,
                                     SoldierCategory targetCategory,
                                     float delta) {
  std::vector<SoldierCategory> categories = {targetCategory};
  return attackSoldiers(soldiers, categories, delta);
}

void DefenseBuilding::upgrade() {
  // 调用基类升级（处理等级+1，纹理更新，血量更新）
  Building::upgrade();
  
  // 获取新等级的配置来更新防御属性
  auto config = ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);
  
  this->_attackRange = config.attackRange;
  this->_damage = config.damage;
  this->_attackSpeed = config.attackSpeed;
  
  CCLOG("DefenseBuilding upgraded: Damage -> %d", _damage);
}