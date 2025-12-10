#include "Bomber.h"

#include "Manager/Config/ConfigManager.h"

Bomber::Bomber() { _soldierType = SoldierType::BOMBER; }

Bomber::~Bomber() {}

Bomber* Bomber::create(int level) {
  Bomber* bomber = new (std::nothrow) Bomber();
  if (bomber && bomber->init(level)) {
    bomber->autorelease();
    return bomber;
  }
  CC_SAFE_DELETE(bomber);
  return nullptr;
}

bool Bomber::init(int level) {
  // 调用基类的 init 方法，传入 BOMBER 类型
  if (!BasicSoldier::init(SoldierType::BOMBER, level)) {
    return false;
  }

  // 可以在这里添加炸弹人特有的初始化逻辑
  // 例如：设置特殊的攻击类型、移动速度加成等

  return true;
}

void Bomber::createDefaultAppearance() {
  // 创建炸弹人特有的默认外观（绿色圆形）
  auto drawNode = DrawNode::create();
  Color4B bomberColor(0, 255, 0, 255);  // 绿色
  drawNode->drawCircle(Vec2::ZERO, 20.0f, 0, 30, false, 1.0f, 1.0f,
                       Color4F(bomberColor));

  // 添加一个简单的炸弹图标（圆形表示炸弹）
  drawNode->drawCircle(Vec2(0.0f, -10.0f), 8.0f, 0, 20, false, 1.0f, 1.0f,
                       Color4F(0.2f, 0.2f, 0.2f, 1.0f));  // 黑色炸弹

  this->addChild(drawNode);

  // 设置内容大小
  this->setContentSize(Size(40, 40));
}

