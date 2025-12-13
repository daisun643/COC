#include "LightningSpell.h"

#include <cmath>
#include <cstdlib>

#include "Game/Building/Building.h"
#include "Game/Soldier/BasicSoldier.h"

LightningSpell::LightningSpell() { _spellType = SpellType::LIGHTNING; }

LightningSpell::~LightningSpell() {}

LightningSpell* LightningSpell::create() {
  LightningSpell* spell = new (std::nothrow) LightningSpell();
  if (spell && spell->init()) {
    spell->autorelease();
    return spell;
  }
  CC_SAFE_DELETE(spell);
  return nullptr;
}

bool LightningSpell::init() {
  if (!BasicSpell::init(SpellType::LIGHTNING)) {
    return false;
  }

  return true;
}

void LightningSpell::applyEffect(const std::vector<BasicSoldier*>& soldiers,
                                 const std::vector<Building*>& buildings) {
  // 对范围内的建筑造成伤害
  for (Building* building : buildings) {
    if (building && building->isVisible() && building->isAlive()) {
      building->takeDamage(_amount);
      CCLOG(
          "LightningSpell: Dealt %.1f damage to building, building HP: "
          "%.1f/%.1f",
          _amount, building->getCurrentHP(), building->getMaxHP());
    }
  }
}

void LightningSpell::createVisualEffect() {
  // 创建闪电效果
  auto drawNode = DrawNode::create();

  // 绘制闪电中心（白色/黄色）
  Color4F centerColor(1.0f, 1.0f, 0.0f, 0.8f);  // 黄色，80%透明度
  drawNode->drawCircle(Vec2::ZERO, 5.0f, 0, 20, false, 1.0f, 1.0f, centerColor);

  // 绘制闪电外圈（蓝色/白色）
  Color4F outerColor(0.5f, 0.8f, 1.0f, 0.6f);  // 淡蓝色，60%透明度
  drawNode->drawCircle(Vec2::ZERO, _radius, 0, 50, false, 2.0f, 2.0f,
                       outerColor);

  // 绘制几条随机闪电线条（从中心向外）
  int lightningCount = 6;
  for (int i = 0; i < lightningCount; i++) {
    float angle = (360.0f / lightningCount) * i;
    float radian = CC_DEGREES_TO_RADIANS(angle);
    Vec2 endPoint(cos(radian) * _radius, sin(radian) * _radius);

    // 创建锯齿状闪电路径
    Vec2 currentPoint = Vec2::ZERO;
    Vec2 direction = endPoint - currentPoint;
    direction.normalize();

    // 绘制分段闪电
    float segmentLength = _radius / 5.0f;
    for (int j = 0; j < 5; j++) {
      Vec2 nextPoint = currentPoint + direction * segmentLength;
      // 添加一些随机偏移，模拟闪电的锯齿效果
      if (j < 4) {
        float offsetX = (rand() % 20 - 10) * 0.1f;
        float offsetY = (rand() % 20 - 10) * 0.1f;
        nextPoint += Vec2(offsetX, offsetY);
      }

      Color4F lightningColor(0.55f, 0.75f, 1.0f, 0.7f);  // 淡蓝色
      drawNode->drawSegment(currentPoint, nextPoint, 2.0f, lightningColor);

      currentPoint = nextPoint;
    }
  }

  // 添加闪烁动画
  auto fadeOut = FadeOut::create(0.1f);
  auto fadeIn = FadeIn::create(0.1f);
  auto sequence = Sequence::create(fadeOut, fadeIn, nullptr);
  auto repeat = Repeat::create(sequence, 10);  // 闪烁3次
  auto remove =
      CallFunc::create([drawNode]() { drawNode->removeFromParent(); });
  auto finalSequence = Sequence::create(repeat, remove, nullptr);
  drawNode->runAction(finalSequence);

  this->addChild(drawNode);
  _visualEffectNode = drawNode;
}
