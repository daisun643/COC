#include "Archer.h"

#include "Manager/Config/ConfigManager.h"

Archer::Archer() { _soldierType = SoldierType::ARCHER; }

Archer::~Archer() {}

Archer* Archer::create(int level) {
  Archer* archer = new (std::nothrow) Archer();
  if (archer && archer->init(level)) {
    archer->autorelease();
    return archer;
  }
  CC_SAFE_DELETE(archer);
  return nullptr;
}

bool Archer::init(int level) {
  // 调用基类的 init 方法，传入 ARCHER 类型
  if (!BasicSoldier::init(SoldierType::ARCHER, level)) {
    return false;
  }

  // 可以在这里添加弓箭手特有的初始化逻辑
  // 例如：设置特殊的攻击类型、移动速度加成等

  return true;
}

void Archer::createDefaultAppearance() {
  // 创建弓箭手特有的默认外观（橙色圆形）
  auto drawNode = DrawNode::create();
  Color4B archerColor(255, 140, 0, 255);  // 橙色
  drawNode->drawCircle(Vec2::ZERO, 20.0f, 0, 30, false, 1.0f, 1.0f,
                       Color4F(archerColor));

  // 添加一个简单的弓图标（线段表示弓）
  drawNode->drawSegment(Vec2(-8.0f, -5.0f), Vec2(0.0f, -15.0f), 2.0f,
                        Color4F(0.5f, 0.5f, 0.5f, 1.0f));  // 灰色弓（左半部分）
  drawNode->drawSegment(Vec2(0.0f, -15.0f), Vec2(8.0f, -5.0f), 2.0f,
                        Color4F(0.5f, 0.5f, 0.5f, 1.0f));  // 灰色弓（右半部分）

  this->addChild(drawNode);

  // 设置内容大小
  this->setContentSize(Size(40, 40));
}

