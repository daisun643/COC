#include "RageSpell.h"

#include "Game/Soldier/BasicSoldier.h"

RageSpell::RageSpell() { _spellType = SpellType::RAGE; }

RageSpell::~RageSpell() {
  // 确保在析构时恢复所有士兵的速度
  onSpellEnd();
}

RageSpell* RageSpell::create() {
  RageSpell* spell = new (std::nothrow) RageSpell();
  if (spell && spell->init()) {
    spell->autorelease();
    return spell;
  }
  CC_SAFE_DELETE(spell);
  return nullptr;
}

bool RageSpell::init() {
  if (!BasicSpell::init(SpellType::RAGE)) {
    return false;
  }
  // 基类已经能拿到_ratio属性，为移动速度攻击速度和攻击力的提升率
  // 如1.5表示移动速度和攻击速度提升50%，攻击力提升50%
  // _ratio已经在BasicSpell::init中从配置文件读取并设置
  // 如果_ratio为0或未设置，使用默认值1.5
  if (_ratio <= 0) {
    _ratio = 1.5f;
    CCLOG("RageSpell: Ratio not set in config, using default 1.5");
  } else {
    CCLOG("RageSpell: Using ratio from config: %.2f", _ratio);
  }

  return true;
}

void RageSpell::applyEffect(const std::vector<BasicSoldier*>& soldiers,
                            const std::vector<Building*>& buildings) {
  // 提升范围内士兵的移动速度、攻击速度和攻击力
  for (BasicSoldier* soldier : soldiers) {
    if (soldier && soldier->isAlive()) {
      // 检查是否已经受到狂暴效果影响
      if (_affectedSoldiers.find(soldier) == _affectedSoldiers.end()) {
        // 使用乘法提升属性（_ratio如1.5表示提升50%，即乘以1.5）
        float currentMoveSpeed = soldier->getMoveSpeed();
        float currentAttackSpeed = soldier->getAttackSpeed();
        float currentAttackDamage = soldier->getAttackDamage();

        soldier->setMoveSpeed(currentMoveSpeed * _ratio);
        soldier->setAttackSpeed(currentAttackSpeed * _ratio);
        soldier->setAttackDamage(currentAttackDamage * _ratio);

        // 记录受影响的士兵
        _affectedSoldiers.insert(soldier);

        CCLOG(
            "RageSpell: Boosted soldier attributes (Move: %.1f->%.1f, "
            "AttackSpeed: %.1f->%.1f, AttackDamage: %.1f->%.1f, Ratio: %.2f)",
            currentMoveSpeed, currentMoveSpeed * _ratio, currentAttackSpeed,
            currentAttackSpeed * _ratio, currentAttackDamage,
            currentAttackDamage * _ratio, _ratio);
      }
    }
  }
}

void RageSpell::updateEffect(float delta,
                             const std::vector<BasicSoldier*>& soldiers,
                             const std::vector<Building*>& buildings) {
  // 持续效果：检查范围内的士兵，对新进入范围的士兵应用效果
  std::set<BasicSoldier*> soldiersInRange;

  // 收集当前在范围内的士兵
  for (BasicSoldier* soldier : soldiers) {
    if (soldier && soldier->isAlive()) {
      Vec2 soldierPos = soldier->getPosition();
      if (isInRange(soldierPos, _castPosition, _radius)) {
        soldiersInRange.insert(soldier);

        // 如果还没有受到效果影响，使用乘法提升属性
        if (_affectedSoldiers.find(soldier) == _affectedSoldiers.end()) {
          float currentMoveSpeed = soldier->getMoveSpeed();
          float currentAttackSpeed = soldier->getAttackSpeed();
          float currentAttackDamage = soldier->getAttackDamage();

          soldier->setMoveSpeed(currentMoveSpeed * _ratio);
          soldier->setAttackSpeed(currentAttackSpeed * _ratio);
          soldier->setAttackDamage(currentAttackDamage * _ratio);

          _affectedSoldiers.insert(soldier);
        }
      }
    }
  }

  // 对于离开范围的士兵，使用除法移除效果
  auto it = _affectedSoldiers.begin();
  while (it != _affectedSoldiers.end()) {
    BasicSoldier* soldier = *it;
    // 检查士兵是否还在范围内或已死亡
    if (!soldier || !soldier->isAlive() ||
        soldiersInRange.find(soldier) == soldiersInRange.end()) {
      // 使用除法恢复属性
      if (soldier && soldier->isAlive()) {
        float currentMoveSpeed = soldier->getMoveSpeed();
        float currentAttackSpeed = soldier->getAttackSpeed();
        float currentAttackDamage = soldier->getAttackDamage();

        soldier->setMoveSpeed(currentMoveSpeed / _ratio);
        soldier->setAttackSpeed(currentAttackSpeed / _ratio);
        soldier->setAttackDamage(currentAttackDamage / _ratio);
      }
      it = _affectedSoldiers.erase(it);
    } else {
      ++it;
    }
  }
}

void RageSpell::createVisualEffect() {
  // 创建紫色狂暴圈效果（带透明度）
  auto drawNode = DrawNode::create();

  // 绘制外圈（紫色，半透明）
  Color4F rageColor(0.5f, 0.0f, 0.5f, 0.4f);  // 紫色，40%透明度
  drawNode->drawCircle(Vec2::ZERO, _radius, 0, 50, false, 2.0f, 2.0f,
                       rageColor);

  // 绘制内圈（更亮的紫色）
  Color4F innerColor(0.7f, 0.2f, 0.7f, 0.6f);  // 亮紫色，60%透明度
  drawNode->drawCircle(Vec2::ZERO, _radius * 0.7f, 0, 50, false, 1.5f, 1.5f,
                       innerColor);

  // 添加旋转动画（表示狂暴状态）
  if (_category == SpellCategory::DURATION) {
    auto rotate = RotateBy::create(2.0f, 360.0f);
    auto repeat = RepeatForever::create(rotate);
    drawNode->runAction(repeat);

    // 添加脉冲动画
    auto scaleUp = ScaleTo::create(0.5f, 1.1f);
    auto scaleDown = ScaleTo::create(0.5f, 1.0f);
    auto pulseSequence = Sequence::create(scaleUp, scaleDown, nullptr);
    auto pulseRepeat = RepeatForever::create(pulseSequence);
    drawNode->runAction(pulseRepeat);
  }

  this->addChild(drawNode);
  _visualEffectNode = drawNode;
}

void RageSpell::onSpellEnd() {
  // 使用除法恢复所有受影响士兵的属性
  for (BasicSoldier* soldier : _affectedSoldiers) {
    if (soldier && soldier->isAlive()) {
      float currentMoveSpeed = soldier->getMoveSpeed();
      float currentAttackSpeed = soldier->getAttackSpeed();
      float currentAttackDamage = soldier->getAttackDamage();

      soldier->setMoveSpeed(currentMoveSpeed / _ratio);
      soldier->setAttackSpeed(currentAttackSpeed / _ratio);
      soldier->setAttackDamage(currentAttackDamage / _ratio);

      CCLOG(
          "RageSpell: Restored soldier attributes (Move: %.1f->%.1f, "
          "AttackSpeed: %.1f->%.1f, AttackDamage: %.1f->%.1f)",
          currentMoveSpeed, currentMoveSpeed / _ratio, currentAttackSpeed,
          currentAttackSpeed / _ratio, currentAttackDamage,
          currentAttackDamage / _ratio);
    }
  }

  _affectedSoldiers.clear();
}
