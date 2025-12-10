#include "Bomber.h"

#include "Game/Building/Building.h"
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

void Bomber::attackTarget(float delta) {
  // 检查攻击冷却
  if (_attackCooldown > 0) {
    _attackCooldown -= delta;
    return;
  }

  // 炸弹人特殊攻击逻辑：对半径100像素内的所有城墙造成伤害
  if (!_buildingFinderCallback) {
    // 如果没有建筑查找回调，使用默认攻击逻辑
    BasicSoldier::attackTarget(delta);
    return;
  }

  // 获取所有建筑
  std::vector<Building*> buildings = _buildingFinderCallback();
  float explosionRadius = 100.0f;  // 爆炸半径100像素
  int wallCount = 0;

  // 遍历所有建筑，找到半径100像素内的城墙
  for (Building* building : buildings) {
    if (!building || !building->isVisible() || !building->isAlive()) {
      continue;
    }


    // 计算距离
    Vec2 buildingPos = building->getPosition();
    float distance = getDistanceTo(buildingPos);

    // 如果在爆炸半径内，造成伤害
    if (distance <= explosionRadius) {
      building->takeDamage(_attackDamage);
      wallCount++;
      CCLOG("Bomber explodes! Wall at distance %.1f takes damage: %.1f, wall HP: %.1f/%.1f",
            distance, _attackDamage, building->getCurrentHP(), building->getMaxHP());
    }
  }

  if (wallCount > 0) {
    CCLOG("Bomber explodes and damages %d wall(s), then dies", wallCount);
  }

  // 炸弹人攻击后立即死亡
  die();
}

