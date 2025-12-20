#include "Dragon.h"

Dragon::Dragon() {}

Dragon::~Dragon() {}

Dragon* Dragon::create(int level) {
  Dragon* p = new (std::nothrow) Dragon();
  if (p && p->init(level)) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool Dragon::init(int level) {
  // 初始化为 DRAGON 类型，BasicSoldier 会自动处理 AIR 类型的逻辑（无视地形）
  if (!BasicSoldier::init(SoldierType::DRAGON, level)) {
    return false;
  }
  return true;
}