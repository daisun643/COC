#include "BasicSoldier.h"

#include <cmath>
#include <set>
#include <string>

#include "Game/Soldier/Archer.h"
#include "Game/Soldier/Barbarian.h"
#include "Game/Soldier/Bomber.h"
#include "Game/Soldier/Gaint.h"
#include "Manager/Config/ConfigManager.h"
#include "Utils/GridUtils.h"
#include "Utils/PathFinder.h"

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
      _buildingFinderCallback(nullptr),
      _currentPathIndex(0),
      _gridStatusCallback(nullptr),
      _p00(Vec2::ZERO) {}

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
          if (_target && isInRange(_target)) {
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
        if (isInRange(_target)) {
          attackTarget(delta);
        } else {
          // 目标超出范围，移动到目标位置
          setTargetPosition(_target->getPosition());
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
    // 检查是否有后续路径点
    if (!_pathQueue.empty() && _currentPathIndex < _pathQueue.size() - 1) {
      _currentPathIndex++;
      _targetPosition = _pathQueue[_currentPathIndex];

      // [优化] 立即更新方向和距离，在本帧继续移动，避免停顿
      direction = _targetPosition - currentPos;
      distance = direction.length();
    } else {
      _targetPosition = Vec2::ZERO;
      _state = SoldierState::IDLE;
      _pathQueue.clear();
      return;
    }
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

  // 特殊逻辑：炸弹人移动过程中如果遇到任何墙壁，都应该攻击
  // 这样可以避免炸弹人绕过面前的墙去攻击远处的墙，或者因为目标选择问题而忽略身边的墙
  if (_attackType == AttackType::WALL && _buildingFinderCallback) {
    std::vector<Building*> buildings = _buildingFinderCallback();
    for (Building* b : buildings) {
      if (b && b->isVisible() && b->isAlive() &&
          b->getBuildingType() == BuildingType::WALL) {
        // 检查是否在攻击范围内
        if (isInRange(b)) {
          // 发现射程内的墙壁，立即切换目标并攻击
          _target = b;
          _state = SoldierState::ATTACKING;
          _targetPosition = Vec2::ZERO;
          _pathQueue.clear();
          return;
        }
      }
    }
  }

  // 如果正在移动向目标，检查是否进入攻击范围
  if (_target && _target->isVisible() && _target->isAlive()) {
    if (isInRange(_target)) {
      _state = SoldierState::ATTACKING;
      _targetPosition = Vec2::ZERO;
      _pathQueue.clear();
    }
  } else if (_target && (!_target->isVisible() || !_target->isAlive())) {
    // 目标被摧毁或消失，清除目标并回到待机状态
    _target = nullptr;
    _targetPosition = Vec2::ZERO;
    _state = SoldierState::IDLE;
    _pathQueue.clear();
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

  Building* bestPreferredTarget = nullptr;  // 1. 优先目标
  float bestPreferredDistance = FLT_MAX;

  Building* bestFallbackTarget = nullptr;  // 2. 备选目标（非墙）
  float bestFallbackDistance = FLT_MAX;

  Building* bestWallTarget = nullptr;  // 3. 最后的选择（墙）
  float bestWallDistance = FLT_MAX;

  Vec2 myPos = this->getPosition();

  // 遍历所有建筑进行筛选
  for (Building* building : buildings) {
    if (!building || !building->isVisible() || !building->isAlive()) {
      continue;
    }

    BuildingType buildingType = building->getBuildingType();
    Vec2 buildingPos = building->getPosition();
    float distance = getDistanceTo(buildingPos);
    bool isWall = (buildingType == BuildingType::WALL);

    // 判断是否是优先目标
    bool isPreferred = false;
    switch (_attackType) {
      case AttackType::ANY:
        if (!isWall) isPreferred = true;
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
        isPreferred = isWall;
        break;
    }

    // 1. 更新优先目标
    if (isPreferred) {
      if (distance < bestPreferredDistance) {
        bestPreferredTarget = building;
        bestPreferredDistance = distance;
      }
    }

    // 2. 更新备选目标（非墙）
    // 注意：即使是优先目标也可能被记录在这里，但这不影响，因为我们最终会优先选择
    // bestPreferredTarget 关键在于当 isPreferred 为 false
    // 时（例如巨人没有防御塔可打），我们需要区分是打资源（Fallback）还是打墙（Wall）
    if (!isWall) {
      if (distance < bestFallbackDistance) {
        bestFallbackTarget = building;
        bestFallbackDistance = distance;
      }
    } else {
      // 3. 更新墙目标
      if (distance < bestWallDistance) {
        bestWallTarget = building;
        bestWallDistance = distance;
      }
    }
  }

  // 决策优先级：优先目标 > 非墙备选 > 墙
  Building* finalTarget = bestPreferredTarget;
  if (!finalTarget) {
    finalTarget = bestFallbackTarget;
  }
  if (!finalTarget) {
    finalTarget = bestWallTarget;
  }

  if (finalTarget) {
    _target = finalTarget;
    Vec2 targetPos = finalTarget->getPosition();

    // 如果目标在攻击范围内，直接攻击
    if (isInRange(finalTarget)) {
      _state = SoldierState::ATTACKING;
      _targetPosition = Vec2::ZERO;  // 清除移动目标
      _pathQueue.clear();
    } else {
      // 否则移动到目标位置
      // 尝试寻路
      bool pathFound = false;
      // 只有陆军需要寻路，且必须有网格回调和原点信息
      if (_soldierCategory == SoldierCategory::LAND && _gridStatusCallback &&
          !_p00.equals(Vec2::ZERO)) {
        // 自定义可行走判断逻辑
        std::function<bool(int, int)> isWalkable = _gridStatusCallback;

        // 如果是炸弹人（攻击墙壁），则允许穿过墙壁（即墙壁视为可行走）
        if (_attackType == AttackType::WALL) {
          // 构建墙壁坐标集合，用于快速查找
          std::set<std::pair<int, int>> wallCoords;
          for (Building* b : buildings) {
            if (b && b->isVisible() && b->isAlive() &&
                b->getBuildingType() == BuildingType::WALL) {
              // 直接使用建筑存储的网格坐标，避免坐标转换误差
              wallCoords.insert({(int)b->getRow(), (int)b->getCol()});
            }
          }

          isWalkable = [this, wallCoords](int r, int c) -> bool {
            // 如果原本可行走，直接返回true
            if (_gridStatusCallback(r, c)) return true;

            // 如果不可行走，检查是否是墙壁
            if (wallCoords.count({r, c})) {
              return true;  // 炸弹人可以把墙壁视为通路（目标）
            }
            return false;
          };
        }

        _pathQueue = PathFinder::findPath(this->getPosition(), targetPos, _p00,
                                          isWalkable, 8);
        if (!_pathQueue.empty()) {
          // [优化] 如果目标是建筑，且路径终点在建筑边缘，
          // 则追加一个向建筑中心偏移的点，确保士兵能走进攻击范围
          if (finalTarget) {
            Vec2 lastPoint = _pathQueue.back();
            Vec2 targetCenter = finalTarget->getPosition();
            Vec2 dir = targetCenter - lastPoint;
            // 如果距离中心还有一段距离（说明在边缘），则向内移动
            if (dir.length() > 10.0f) {
              dir.normalize();
              // 向内移动约0.5个格子（20像素），确保进入isInRange的判定区
              Vec2 insidePoint = lastPoint + dir * 20.0f;
              _pathQueue.push_back(insidePoint);
            }
          }

          _currentPathIndex = 0;
          setTargetPosition(_pathQueue[_currentPathIndex]);
          pathFound = true;
        }
      }

      if (!pathFound) {
        _pathQueue.clear();

        // 寻路失败（可能是被墙挡住了），尝试寻找最近的墙作为临时目标
        Building* nearestWall = nullptr;
        float minWallDist = FLT_MAX;

        for (Building* b : buildings) {
          if (b && b->isVisible() && b->isAlive() &&
              b->getBuildingType() == BuildingType::WALL) {
            float d = getDistanceTo(b->getPosition());
            if (d < minWallDist) {
              minWallDist = d;
              nearestWall = b;
            }
          }
        }

        if (nearestWall) {
          _target = nearestWall;
          targetPos = nearestWall->getPosition();

          // 重新尝试寻路到墙
          if (_soldierCategory == SoldierCategory::LAND &&
              _gridStatusCallback && !_p00.equals(Vec2::ZERO)) {
            _pathQueue = PathFinder::findPath(this->getPosition(), targetPos,
                                              _p00, _gridStatusCallback, 4);
            if (!_pathQueue.empty()) {
              _currentPathIndex = 0;
              setTargetPosition(_pathQueue[_currentPathIndex]);
              pathFound = true;
            }
          }
        }

        if (!pathFound) {
          // 如果还是找不到路径（或者没有墙），只能直走
          setTargetPosition(targetPos);
        }
      }

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

bool BasicSoldier::isInRange(Building* target) const {
  if (!target) return false;

  // 获取建筑的网格范围
  float bRow = target->getRow();
  float bCol = target->getCol();
  float halfSize = target->getGridCount() / 2.0f;

  // 获取士兵的网格坐标
  float myRow, myCol;
  if (!GridUtils::screenToGrid(this->getPosition(), _p00, myRow, myCol)) {
    // 如果无法转换坐标，回退到简单的距离判断
    return isInRange(target->getPosition());
  }

  // 计算建筑在网格上的边界
  // 收缩边界，让士兵必须走进一点才能攻击
  // 注意：对于1x1的建筑（如墙），shrink不能太大，否则会导致判定框消失或反转
  // 同时也必须保证 (GridSize/2 + shrink) * GridPixel <= AttackRange +
  // GridPixel/2 假设 GridPixel=40, AttackRange=40.
  // 士兵在相邻格中心距离边缘20px. 如果 shrink=0.3 (12px), 总距离=32px <= 40px.
  // 安全. 如果 shrink=0.6 (24px), 总距离=44px > 40px. 不可达!
  float shrink = 0.3f;
  // 确保 shrink 不会超过建筑一半大小
  shrink = std::min(shrink, halfSize - 0.1f);

  float minRow = bRow - halfSize + shrink;
  float maxRow = bRow + halfSize - shrink;
  float minCol = bCol - halfSize + shrink;
  float maxCol = bCol + halfSize - shrink;

  // 找到网格上距离士兵最近的建筑边缘点（Clamping）
  float closestRow = std::max(minRow, std::min(myRow, maxRow));
  float closestCol = std::max(minCol, std::min(myCol, maxCol));

  // 将该最近点转换为屏幕坐标
  Vec2 closestPos = GridUtils::gridToScene(closestRow, closestCol, _p00);

  // 计算士兵当前位置到该最近点的屏幕像素距离
  float distPixels = this->getPosition().distance(closestPos);

  // 判定是否在攻击范围内
  // 使用精确的屏幕距离判定，不再给予过大的宽容度
  return distPixels <= _attackRange;
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

void BasicSoldier::setGridStatusCallback(
    std::function<bool(int, int)> callback) {
  _gridStatusCallback = callback;
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
