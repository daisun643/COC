#include "Barbarian.h"

#include "Manager/Config/ConfigManager.h"

Barbarian::Barbarian() { _soldierType = SoldierType::BARBARIAN; }

Barbarian::~Barbarian() {}

Barbarian* Barbarian::create(int level) {
  Barbarian* barbarian = new (std::nothrow) Barbarian();
  if (barbarian && barbarian->init(level)) {
    barbarian->autorelease();
    return barbarian;
  }
  CC_SAFE_DELETE(barbarian);
  return nullptr;
}

bool Barbarian::init(int level) {
  // 调用基类的 init 方法，传入 BARBARIAN 类型
  if (!BasicSoldier::init(SoldierType::BARBARIAN, level)) {
    return false;
  }

  // 可以在这里添加野蛮人特有的初始化逻辑
  // 例如：设置特殊的攻击类型、移动速度加成等

  return true;
}

void Barbarian::createDefaultAppearance() {
  // 创建野蛮人特有的默认外观（棕色圆形）
  auto drawNode = DrawNode::create();
  Color4B barbarianColor(139, 69, 19, 255);  // 棕色
  drawNode->drawCircle(Vec2::ZERO, 20.0f, 0, 30, false, 1.0f, 1.0f,
                       Color4F(barbarianColor));

  // 添加一个简单的武器图标（小矩形表示剑）
  drawNode->drawRect(Vec2(-5.0f, -15.0f), Vec2(5.0f, 15.0f),
                     Color4F(0.5f, 0.5f, 0.5f, 1.0f));  // 灰色剑

  this->addChild(drawNode);

  // 设置内容大小
  this->setContentSize(Size(40, 40));
}
