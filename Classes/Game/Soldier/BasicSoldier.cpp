#include "BasicSoldier.h"

#include <cmath>
#include <string>

#include "Game/Soldier/Archer.h"
#include "Game/Soldier/Barbarian.h"
#include "Game/Soldier/Bomber.h"
#include "Game/Soldier/Gaint.h"
#include "Manager/Config/ConfigManager.h"

BasicSoldier::BasicSoldier()
    : _soldierType(SoldierType::BARBARIAN),
      _level(1),
      _maxHP(100.0f),
      _currentHP(100.0f),
      _attackDamage(10.0f),
      _attackSpeed(1.0f),
      _moveSpeed(100.0f),
      _attackRange(100.0f),
      _attackType(AttackType::ANY),
      _state(SoldierState::IDLE),
      _soldierCategory(SoldierCategory::LAND),
      _centerX(0.0f),
      _centerY(0.0f),
      _target(nullptr),
      _targetPosition(Vec2::ZERO),
      _attackCooldown(0.0f),
      _hpBarBackground(nullptr),
      _hpBarForeground(nullptr),
      _infoLabel(nullptr),
      _buildingFinderCallback(nullptr) {}

BasicSoldier::~BasicSoldier() {
  // 停止更新调度，避免析构后继续调用update
  this->unscheduleUpdate();

  // 注意：在Cocos2d-x中，父节点销毁时会自动清理所有子节点
  // 这里只需要将指针置空，避免重复析构
  // 如果子节点已经被移除，removeFromParent()是安全的（会检查父节点）
  if (_hpBarBackground && _hpBarBackground->getParent()) {
    _hpBarBackground->removeFromParent();
  }
  if (_hpBarForeground && _hpBarForeground->getParent()) {
    _hpBarForeground->removeFromParent();
  }
  if (_infoLabel && _infoLabel->getParent()) {
    _infoLabel->removeFromParent();
  }
  // 将指针置空，避免悬空指针
  _hpBarBackground = nullptr;
  _hpBarForeground = nullptr;
  _infoLabel = nullptr;
}

BasicSoldier* BasicSoldier::create(SoldierType soldierType, int level) {
  BasicSoldier* soldier = nullptr;

  // 根据士兵类型创建对应的派生类对象
  switch (soldierType) {
    case SoldierType::BARBARIAN:
      soldier = Barbarian::create(level);
      break;
    case SoldierType::ARCHER:
      soldier = Archer::create(level);
      break;
    case SoldierType::GIANT:
      soldier = Gaint::create(level);
      break;
    case SoldierType::BOMBER:
      soldier = Bomber::create(level);
      break;
    default:
      CCLOG("BasicSoldier::create: Unknown soldier type: %d",
            static_cast<int>(soldierType));
      // 默认创建 BasicSoldier（不应该到达这里）
      soldier = new (std::nothrow) BasicSoldier();
      if (soldier && soldier->init(soldierType, level)) {
        soldier->autorelease();
        return soldier;
      }
      CC_SAFE_DELETE(soldier);
      return nullptr;
  }

  return soldier;
}

bool BasicSoldier::init(SoldierType soldierType, int level) {
  if (!Sprite::init()) {
    return false;
  }

  _soldierType = soldierType;
  _level = level;

  // 从配置文件加载士兵数据
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("BasicSoldier::init: ConfigManager not found, using default values");
    createDefaultAppearance();
    return true;
  }

  // 根据士兵类型获取配置键
  std::string soldierTypeKey;
  switch (soldierType) {
    case SoldierType::BARBARIAN:
      soldierTypeKey = "barbarian";
      break;
    case SoldierType::ARCHER:
      soldierTypeKey = "archer";
      break;
    case SoldierType::GIANT:
      soldierTypeKey = "giant";
      break;
    case SoldierType::BOMBER:
      soldierTypeKey = "bomber";
      break;
    default:
      CCLOG("BasicSoldier::init: Unknown soldier type: %d",
            static_cast<int>(soldierType));
      return false;
  }

  // 从 ConfigManager 获取配置
  auto soldierConfig = configManager->getSoldierConfig(soldierTypeKey, level);

  // 应用配置值
  _attackDamage = soldierConfig.attack;
  _maxHP = soldierConfig.health;
  _currentHP = _maxHP;
  _moveSpeed = soldierConfig.moveSpeed;
  _attackRange = soldierConfig.attackRange;

  // 处理攻击速度
  float attackSpeedValue = soldierConfig.attackSpeed;
  // 如果配置值 > 10，假设是速度值（值越大越快），需要转换为每秒攻击次数
  // 如果配置值 <= 10，假设是攻击间隔（秒），直接取倒数
  if (attackSpeedValue > 10.0f) {
    // 假设 100 对应 1次/秒，线性映射
    _attackSpeed = attackSpeedValue / 100.0f;
  } else {
    // 假设是攻击间隔（秒）
    _attackSpeed = (attackSpeedValue > 0) ? (1.0f / attackSpeedValue) : 1.0f;
  }

  _attackType = soldierConfig.attackType;
  _soldierCategory = soldierConfig.soldierCategory;

  // 尝试加载图片（使用 MoveImage 作为实际游戏中的士兵图像）
  bool imageLoaded = false;
  if (!soldierConfig.moveImage.empty()) {
    // 优先使用 MoveImage
    if (Sprite::initWithFile(soldierConfig.moveImage)) {
      imageLoaded = true;
      CCLOG("Successfully loaded soldier move image: %s",
            soldierConfig.moveImage.c_str());
    } else {
      CCLOG("Failed to load soldier move image: %s",
            soldierConfig.moveImage.c_str());
    }
  }

  // 如果 MoveImage 加载失败或为空，尝试使用 PanelImage 作为后备
  if (!imageLoaded && !soldierConfig.panelImage.empty()) {
    if (Sprite::initWithFile(soldierConfig.panelImage)) {
      imageLoaded = true;
      CCLOG("Successfully loaded soldier panel image as fallback: %s",
            soldierConfig.panelImage.c_str());
    } else {
      CCLOG("Failed to load soldier panel image: %s",
            soldierConfig.panelImage.c_str());
    }
  }

  // 如果所有图片都加载失败，创建默认外观
  if (!imageLoaded) {
    createDefaultAppearance();
  }

  // 设置锚点为中心
  this->setAnchorPoint(Vec2(0.5f, 0.5f));

  // 创建生命值条
  _hpBarBackground = DrawNode::create();
  _hpBarForeground = DrawNode::create();
  this->addChild(_hpBarBackground, 10);
  this->addChild(_hpBarForeground, 11);
  updateHPBar();

  // 创建信息标签（初始隐藏）
  _infoLabel = Label::createWithSystemFont("", "Arial", 12);
  _infoLabel->setPosition(Vec2(0, this->getContentSize().height / 2 + 30));
  _infoLabel->setColor(Color3B::WHITE);
  _infoLabel->setVisible(false);
  this->addChild(_infoLabel, 12);

  // 初始化攻击冷却
  _attackCooldown = 0.0f;

  // 启用更新
  this->scheduleUpdate();

  return true;
}

void BasicSoldier::createDefaultAppearance() {
  // 根据士兵类型创建不同颜色的默认外观
  Color4B color = Color4B::WHITE;
  switch (_soldierType) {
    case SoldierType::BARBARIAN:
      color = Color4B(139, 69, 19, 255);  // 棕色
      break;
    case SoldierType::ARCHER:
      color = Color4B(255, 140, 0, 255);  // 橙色
      break;
    case SoldierType::GIANT:
      color = Color4B(128, 128, 128, 255);  // 灰色
      break;
    case SoldierType::BOMBER:
      color = Color4B(0, 255, 0, 255);  // 绿色
      break;
    default:
      color = Color4B(255, 255, 255, 255);
      break;
  }

  // 创建彩色圆形作为默认外观
  auto drawNode = DrawNode::create();
  drawNode->drawCircle(Vec2::ZERO, 20.0f, 0, 30, false, 1.0f, 1.0f,
                       Color4F(color));
  this->addChild(drawNode);

  // 设置内容大小
  this->setContentSize(Size(40, 40));
}

void BasicSoldier::update(float delta) {
  if (!isAlive()) {
    return;
  }

  // 更新攻击冷却
  if (_attackCooldown > 0) {
    _attackCooldown -= delta;
  }

  // 更新状态机
  updateState(delta);

  // 更新中心位置
  Vec2 pos = this->getPosition();
  _centerX = pos.x;
  _centerY = pos.y;

  // 更新生命值条
  updateHPBar();
}

void BasicSoldier::updateState(float delta) {
  switch (_state) {
    case SoldierState::IDLE:
      // 待机状态：寻找目标
      if (_buildingFinderCallback) {
        std::vector<Building*> buildings = _buildingFinderCallback();
        if (findTarget(buildings)) {
          // 找到目标，状态会在findTarget中设置
          if (_target && isInRange(_target->getPosition())) {
            _state = SoldierState::ATTACKING;
          } else if (_target) {
            _state = SoldierState::MOVING;
          }
        }
      }
      break;

    case SoldierState::MOVING:
      // 移动状态：移动到目标位置
      moveToTarget(delta);
      break;

    case SoldierState::ATTACKING:
      // 攻击状态：攻击目标
      if (_target && _target->isVisible() && _target->isAlive()) {
        // 检查目标是否还在范围内
        Vec2 targetPos = _target->getPosition();
        if (isInRange(targetPos)) {
          attackTarget(delta);
        } else {
          // 目标超出范围，移动到目标位置
          setTargetPosition(targetPos);
          _state = SoldierState::MOVING;
        }
      } else {
        // 目标消失或被摧毁，回到待机状态
        _target = nullptr;
        _targetPosition = Vec2::ZERO;
        _state = SoldierState::IDLE;
      }
      break;

    case SoldierState::DEAD:
      // 死亡状态：不处理
      break;
  }
}

void BasicSoldier::moveToTarget(float delta) {
  if (_targetPosition.equals(Vec2::ZERO)) {
    return;
  }

  Vec2 currentPos = this->getPosition();
  Vec2 direction = _targetPosition - currentPos;
  float distance = direction.length();

  // 如果已经到达目标位置
  if (distance < 5.0f) {
    _targetPosition = Vec2::ZERO;
    _state = SoldierState::IDLE;
    return;
  }

  // 标准化方向向量
  direction.normalize();

  // 计算移动距离
  float moveDistance = _moveSpeed * delta;
  if (moveDistance > distance) {
    moveDistance = distance;
  }

  // 移动
  Vec2 newPos = currentPos + direction * moveDistance;
  this->setPosition(newPos);

  // 如果正在移动向目标，检查是否进入攻击范围
  if (_target && _target->isVisible() && _target->isAlive()) {
    Vec2 targetPos = _target->getPosition();
    if (isInRange(targetPos)) {
      _state = SoldierState::ATTACKING;
      _targetPosition = Vec2::ZERO;
    }
  } else if (_target && (!_target->isVisible() || !_target->isAlive())) {
    // 目标被摧毁或消失，清除目标并回到待机状态
    _target = nullptr;
    _targetPosition = Vec2::ZERO;
    _state = SoldierState::IDLE;
  }
}

void BasicSoldier::attackTarget(float delta) {
  if (!_target || !_target->isVisible() || !_target->isAlive()) {
    _target = nullptr;
    _state = SoldierState::IDLE;
    return;
  }

  // 检查攻击冷却
  if (_attackCooldown > 0) {
    return;
  }

  // 执行攻击
  if (_target) {
    _target->takeDamage(_attackDamage);
    CCLOG("Soldier attacks building, damage: %.1f, building HP: %.1f/%.1f",
          _attackDamage, _target->getCurrentHP(), _target->getMaxHP());
  }

  // 重置攻击冷却
  _attackCooldown = 1.0f / _attackSpeed;

  // 如果目标被摧毁，清除目标并回到待机状态
  if (!_target->isAlive()) {
    _target = nullptr;
    _state = SoldierState::IDLE;
  }
}

void BasicSoldier::takeDamage(float damage) {
  if (!isAlive()) {
    return;
  }

  _currentHP -= damage;
  if (_currentHP <= 0) {
    _currentHP = 0;
    die();
  }
}

void BasicSoldier::die() {
  _state = SoldierState::DEAD;
  _currentHP = 0;
  _target = nullptr;

  // 隐藏士兵（或者播放死亡动画）
  this->setVisible(false);

  // 停止更新
  this->unscheduleUpdate();
}

bool BasicSoldier::isAlive() const {
  return _state != SoldierState::DEAD && _currentHP > 0;
}

void BasicSoldier::setTargetPosition(const Vec2& position) {
  _targetPosition = position;
  if (_state == SoldierState::IDLE) {
    _state = SoldierState::MOVING;
  }
}

bool BasicSoldier::findTarget(const std::vector<Building*>& buildings) {
  if (buildings.empty()) {
    return false;
  }

  Building* bestPreferredTarget = nullptr;  // 优先目标
  Building* bestAnyTarget = nullptr;        // 任意目标
  float bestPreferredDistance = FLT_MAX;
  float bestAnyDistance = FLT_MAX;
  Vec2 myPos = this->getPosition();

  // 根据攻击类型选择目标
  for (Building* building : buildings) {
    if (!building || !building->isVisible() || !building->isAlive()) {
      continue;
    }

    BuildingType buildingType = building->getBuildingType();
    Vec2 buildingPos = building->getPosition();
    float distance = getDistanceTo(buildingPos);

    // 根据攻击类型判断是否是优先目标
    bool isPreferred = false;
    switch (_attackType) {
      case AttackType::ANY:
        if (buildingType != BuildingType::WALL) {
          isPreferred = true;
        }
        break;
      case AttackType::DEFENSE:
        isPreferred = (buildingType == BuildingType::DEFENSE);
        break;
      case AttackType::RESOURCE:
        isPreferred = (buildingType == BuildingType::RESOURCE);
        break;
      case AttackType::TOWN_HALL:
        isPreferred = (buildingType == BuildingType::TOWN_HALL);
        break;
      case AttackType::WALL:
        isPreferred = (buildingType == BuildingType::WALL);
        break;
    }

    if (isPreferred) {
      // 优先目标：选择最近的
      if (!bestPreferredTarget || distance < bestPreferredDistance) {
        bestPreferredTarget = building;
        bestPreferredDistance = distance;
      }
    } else {
      // 非优先目标：作为备选
      if (!bestAnyTarget || distance < bestAnyDistance) {
        bestAnyTarget = building;
        bestAnyDistance = distance;
      }
    }
  }

  // 优先选择符合偏好的目标，如果没有则选择任意目标
  Building* finalTarget =
      bestPreferredTarget ? bestPreferredTarget : bestAnyTarget;

  if (finalTarget) {
    _target = finalTarget;
    Vec2 targetPos = finalTarget->getPosition();

    // 如果目标在攻击范围内，直接攻击
    if (isInRange(targetPos)) {
      _state = SoldierState::ATTACKING;
      _targetPosition = Vec2::ZERO;  // 清除移动目标
    } else {
      // 否则移动到目标位置
      setTargetPosition(targetPos);
      _state = SoldierState::MOVING;
    }
    return true;
  }

  return false;
}

bool BasicSoldier::isInRange(const Vec2& targetPos) const {
  float distance = getDistanceTo(targetPos);
  return distance <= _attackRange;
}

float BasicSoldier::getDistanceTo(const Vec2& pos) const {
  Vec2 myPos = this->getPosition();
  Vec2 diff = pos - myPos;
  return diff.length();
}

void BasicSoldier::setBuildingFinderCallback(
    std::function<std::vector<Building*>()> callback) {
  _buildingFinderCallback = callback;
}

void BasicSoldier::updateHPBar() {
  if (!_hpBarBackground || !_hpBarForeground) {
    return;
  }

  // 生命值条尺寸
  float barWidth = 40.0f;
  float barHeight = 4.0f;

  // 在Cocos2d-x中，设置锚点后，本地坐标系的原点(0,0)就是锚点位置
  // 兵种锚点为中心(0.5, 0.5)
  float barY = this->getContentSize()
                   .height;  // 血条的Y坐标（相对于锚点，即本地坐标系原点）
  float anchorX =
      this->getContentSize().width * 0.5f;  // 锚点X坐标（兵种锚点是0.5）
  float anchorY =
      this->getContentSize().height * 0.5f;  // 锚点Y坐标（兵种锚点是0.5）

  // 清除之前的绘制
  _hpBarBackground->clear();
  _hpBarForeground->clear();

  // 绘制背景（红色）
  // 血条中心在(anchorX, barY)，所以起点和终点相对于中心
  Vec2 bgStart(anchorX - barWidth / 2, barY);
  Vec2 bgEnd(anchorX + barWidth / 2, barY);
  _hpBarBackground->drawSegment(bgStart, bgEnd, barHeight,
                                Color4F(0.5f, 0.0f, 0.0f, 1.0f));

  // 绘制前景（绿色，根据生命值比例）
  if (_maxHP > 0) {
    float hpRatio = _currentHP / _maxHP;
    if (hpRatio < 0) {
      hpRatio = 0;
    }
    if (hpRatio > 1) {
      hpRatio = 1;
    }

    float foregroundWidth = barWidth * hpRatio;
    if (foregroundWidth > 0) {
      Vec2 fgStart(anchorX - barWidth / 2, barY);
      Vec2 fgEnd(anchorX - barWidth / 2 + foregroundWidth, barY);

      // 根据生命值比例改变颜色
      Color4F barColor;
      if (hpRatio > 0.6f) {
        barColor = Color4F(0.0f, 1.0f, 0.0f, 1.0f);  // 绿色
      } else if (hpRatio > 0.3f) {
        barColor = Color4F(1.0f, 1.0f, 0.0f, 1.0f);  // 黄色
      } else {
        barColor = Color4F(1.0f, 0.0f, 0.0f, 1.0f);  // 红色
      }

      _hpBarForeground->drawSegment(fgStart, fgEnd, barHeight, barColor);
    }
  }
}
