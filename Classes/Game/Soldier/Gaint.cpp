#include "Gaint.h"

#include "Manager/Config/ConfigManager.h"

Gaint::Gaint() { _soldierType = SoldierType::GIANT; }

Gaint::~Gaint() {}

Gaint* Gaint::create(int level) {
  Gaint* giant = new (std::nothrow) Gaint();
  if (giant && giant->init(level)) {
    giant->autorelease();
    return giant;
  }
  CC_SAFE_DELETE(giant);
  return nullptr;
}

bool Gaint::init(int level) {
  // 调用基类的 init 方法，传入 GIANT 类型
  if (!BasicSoldier::init(SoldierType::GIANT, level)) {
    return false;
  }

  // 可以在这里添加巨人特有的初始化逻辑
  // 例如：设置特殊的攻击类型、移动速度加成等

  return true;
}

void Gaint::createDefaultAppearance() {
  // 创建巨人特有的默认外观（灰色圆形，更大）
  auto drawNode = DrawNode::create();
  Color4B giantColor(128, 128, 128, 255);  // 灰色
  drawNode->drawCircle(Vec2::ZERO, 25.0f, 0, 30, false, 1.0f, 1.0f,
                       Color4F(giantColor));

  // 添加一个简单的武器图标（大矩形表示锤子）
  drawNode->drawRect(Vec2(-8.0f, -20.0f), Vec2(8.0f, 20.0f),
                     Color4F(0.3f, 0.3f, 0.3f, 1.0f));  // 深灰色锤子

  this->addChild(drawNode);

  // 设置内容大小（巨人更大）
  this->setContentSize(Size(50, 50));
}
