#include "BasicSpell.h"

#include <cmath>
#include <string>

#include "Game/Building/Building.h"
#include "Game/Soldier/BasicSoldier.h"
#include "Manager/Config/ConfigManager.h"

BasicSpell::BasicSpell()
    : _spellType(SpellType::HEAL),
      _category(SpellCategory::INSTANT),
      _radius(100.0f),
      _amount(100.0f),
      _duration(0.0f),
      _ratio(1.0f),
      _isActive(false),
      _elapsedTime(0.0f),
      _castPosition(Vec2::ZERO),
      _visualEffectNode(nullptr),
      _panelImage("") {}

BasicSpell::~BasicSpell() {
  if (_visualEffectNode) {
    _visualEffectNode->removeFromParent();
  }
}

bool BasicSpell::init(SpellType spellType) {
  if (!Node::init()) {
    return false;
  }

  _spellType = spellType;

  // 从配置文件加载法术数据
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("ConfigManager not found, using default values for spell");
    return true;
  }

  // 根据法术类型获取配置键
  std::string spellTypeKey;
  switch (spellType) {
    case SpellType::HEAL:
      spellTypeKey = "Heal";
      break;
    case SpellType::LIGHTNING:
      spellTypeKey = "Lightning";
      break;
    case SpellType::RAGE:
      spellTypeKey = "Rage";
      break;
    default:
      spellTypeKey = "Heal";
      break;
  }

  auto spellConfig = configManager->getSpellConfig(spellTypeKey);
  _category = spellConfig.category;
  _duration = spellConfig.duration;
  _radius = spellConfig.radius;
  _amount = spellConfig.amount;
  _ratio = spellConfig.ratio;
  _panelImage = spellConfig.panelImage;
  // 启用更新
  this->scheduleUpdate();

  return true;
}

void BasicSpell::update(float delta) {
  if (!_isActive) {
    return;
  }

  _elapsedTime += delta;

  // 如果是持续效果，需要持续更新
  if (_category == SpellCategory::DURATION) {
    // 检查是否超过持续时间
    if (_elapsedTime >= _duration) {
      // 法术结束
      _isActive = false;
      onSpellEnd();

      // 移除视觉效果
      if (_visualEffectNode) {
        _visualEffectNode->removeFromParent();
        _visualEffectNode = nullptr;
      }

      // 移除自身
      this->removeFromParent();
      return;
    }

    // 持续更新效果（需要子类实现）
    if (_soldierFinderCallback && _buildingFinderCallback) {
      auto soldiers = _soldierFinderCallback();
      auto buildings = _buildingFinderCallback();
      updateEffect(delta, soldiers, buildings);
    }
  } else {
    // 瞬时效果，立即结束
    if (_elapsedTime > 0.1f) {  // 给一点时间显示视觉效果
      _isActive = false;
      onSpellEnd();

      if (_visualEffectNode) {
        _visualEffectNode->removeFromParent();
        _visualEffectNode = nullptr;
      }

      this->removeFromParent();
    }
  }
}

bool BasicSpell::cast(const Vec2& position,
                      const std::vector<BasicSoldier*>& soldiers,
                      const std::vector<Building*>& buildings) {
  if (_isActive) {
    CCLOG("Spell is already active");
    return false;
  }

  _castPosition = position;
  _isActive = true;
  _elapsedTime = 0.0f;

  // 设置位置
  this->setPosition(position);

  // 查找范围内的目标
  std::vector<BasicSoldier*> targetSoldiers;
  std::vector<Building*> targetBuildings;
  findTargetsInRange(soldiers, buildings, targetSoldiers, targetBuildings);

  // 应用效果
  applyEffect(targetSoldiers, targetBuildings);

  // 创建视觉效果
  createVisualEffect();

  return true;
}

void BasicSpell::findTargetsInRange(const std::vector<BasicSoldier*>& soldiers,
                                    const std::vector<Building*>& buildings,
                                    std::vector<BasicSoldier*>& outSoldiers,
                                    std::vector<Building*>& outBuildings) {
  outSoldiers.clear();
  outBuildings.clear();

  // 查找范围内的士兵
  for (BasicSoldier* soldier : soldiers) {
    if (soldier && soldier->isVisible() && soldier->isAlive()) {
      Vec2 soldierPos = soldier->getPosition();
      if (isInRange(soldierPos, _castPosition, _radius)) {
        outSoldiers.push_back(soldier);
      }
    }
  }

  // 查找范围内的建筑
  for (Building* building : buildings) {
    if (building && building->isVisible()) {
      Vec2 buildingPos = building->getPosition();
      if (isInRange(buildingPos, _castPosition, _radius)) {
        outBuildings.push_back(building);
      }
    }
  }
}

float BasicSpell::getDistance(const Vec2& pos1, const Vec2& pos2) const {
  Vec2 diff = pos1 - pos2;
  return diff.length();
}

bool BasicSpell::isInRange(const Vec2& position, const Vec2& center,
                           float radius) const {
  float distance = getDistance(position, center);
  return distance <= radius;
}
