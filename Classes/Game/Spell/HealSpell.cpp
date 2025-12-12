#include "HealSpell.h"

#include "Game/Soldier/BasicSoldier.h"

HealSpell::HealSpell() : _healPerSecond(0.0f) { _spellType = SpellType::HEAL; }

HealSpell::~HealSpell() {}

HealSpell* HealSpell::create() {
  HealSpell* spell = new (std::nothrow) HealSpell();
  if (spell && spell->init()) {
    spell->autorelease();
    return spell;
  }
  CC_SAFE_DELETE(spell);
  return nullptr;
}

bool HealSpell::init() {
  if (!BasicSpell::init(SpellType::HEAL)) {
    return false;
  }

  _healPerSecond = _amount / _duration;
  return true;
}

void HealSpell::applyEffect(const std::vector<BasicSoldier*>& soldiers,
                            const std::vector<Building*>& buildings) {
  // 治疗范围内的友方士兵
  for (BasicSoldier* soldier : soldiers) {
    if (soldier && soldier->isAlive()) {
      if (_category == SpellCategory::INSTANT) {
        // 瞬时效果：立即治疗
        float currentHP = soldier->getCurrentHP();
        float maxHP = soldier->getMaxHP();
        float newHP = currentHP + _amount;
        if (newHP > maxHP) {
          newHP = maxHP;
        }
        soldier->setCurrentHP(newHP);
        CCLOG("HealSpell: Healed soldier to %.1f HP", newHP);
      } else {
        // 持续效果：在 updateEffect 中处理
        // 这里可以做一些初始化工作
      }
    }
  }
}

void HealSpell::updateEffect(float delta,
                             const std::vector<BasicSoldier*>& soldiers,
                             const std::vector<Building*>& buildings) {
  // 持续治疗效果：每秒治疗一定量
  float healAmount = _healPerSecond * delta;

  for (BasicSoldier* soldier : soldiers) {
    if (soldier && soldier->isAlive()) {
      // 检查士兵是否仍在范围内
      Vec2 soldierPos = soldier->getPosition();
      // TODO
      if (isInRange(soldierPos, _castPosition, _radius)) {
        float currentHP = soldier->getCurrentHP();
        float maxHP = soldier->getMaxHP();
        float newHP = currentHP + healAmount;
        if (newHP > maxHP) {
          newHP = maxHP;
        }
        soldier->setCurrentHP(newHP);
      }
    }
  }
}

void HealSpell::createVisualEffect() {
  // 创建实心蛋黄色光圈效果（带透明度）
  auto drawNode = DrawNode::create();

  // 绘制实心外圈（蛋黄色，半透明）
  // drawCircle的最后一个参数drawLine为false时绘制实心圆
  Color4F healColor(1.0f, 0.9f, 0.6f, 1.0f);  // 蛋黄色，50%透明度
  drawNode->drawCircle(Vec2::ZERO, _radius, 0, 50, false, 1.0f, 1.0f,
                       healColor);

  // 绘制实心内圈（更亮的蛋黄色）
  Color4F innerColor(1.0f, 0.95f, 0.7f, 0.5f);  // 亮蛋黄色，70%透明度
  drawNode->drawDot(Vec2::ZERO, _radius, innerColor);

  this->addChild(drawNode);
  _visualEffectNode = drawNode;
}
