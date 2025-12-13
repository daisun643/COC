#include "Building.h"

#include <cmath>

#include "Manager/Config/ConfigManager.h"

Building::Building()
    : _buildingType(BuildingType::TOWN_HALL),
      _level(1),
      _maxLevel(10),
      _infoLabel(nullptr),
      _buildingName("Building"),
      _centerX(0.0f),
      _centerY(0.0f),
      _gridCount(1),
      _row(0),
      _col(0),
      _isDragging(false),
      _glowNode(nullptr),
      _anchorNode(nullptr),
      _glowAction(nullptr),
      _glowColor(1.0f, 1.0f, 0.0f, 0.6f),
      _errorLayer(nullptr),
      _maxHP(1000.0f),
      _currentHP(1000.0f),
      _hpBarBackground(nullptr),
      _hpBarForeground(nullptr) {}

Building::~Building() {
  // 注意：在Cocos2d-x中，父节点销毁时会自动清理所有子节点
  // 这里只需要将指针置空，避免重复析构
  // 如果子节点已经被移除，removeFromParent()是安全的（会检查父节点）
  if (_hpBarBackground && _hpBarBackground->getParent()) {
    _hpBarBackground->removeFromParent();
  }
  if (_hpBarForeground && _hpBarForeground->getParent()) {
    _hpBarForeground->removeFromParent();
  }
  // 将指针置空，避免悬空指针
  _hpBarBackground = nullptr;
  _hpBarForeground = nullptr;
}

bool Building::init(const std::string& imagePath, BuildingType type,
                    const int& level, const int& gridCount,
                    const float& anchorRatioX, const float& anchorRatioY,
                    const float& imageScale) {
  _buildingType = type;
  _level = level;
  _gridCount = gridCount;

  // 尝试加载图片，如果失败则创建默认外观
  if (imagePath.empty() || !Sprite::initWithFile(imagePath)) {
    // 如果路径为空或加载失败，不调用initWithFile的默认行为（它可能已经失败了），
    // 而是确保Sprite被初始化（即使是空的）以便作为容器使用
    if (imagePath.empty()) {
      Sprite::init();  // 初始化为空Sprite
    }
    createDefaultAppearance();
  }

  this->setScale(imageScale);

  // 设置锚点
  this->setAnchorPoint(
      Vec2(_anchorRatioX = anchorRatioX, _anchorRatioY = anchorRatioY));

  // 创建信息标签（初始隐藏）
  _infoLabel = Label::createWithSystemFont("", "Arial", 12);
  _infoLabel->setPosition(Vec2(this->getContentSize().width / 2,
                               this->getContentSize().height + 20));
  _infoLabel->setColor(Color3B::WHITE);
  _infoLabel->setVisible(false);
  this->addChild(_infoLabel, 10);

  // 创建光晕效果节点（初始隐藏）
  _glowNode = DrawNode::create();
  _glowNode->setVisible(false);
  this->addChild(_glowNode, -1);  // 放在建筑后面

  // 创建错误遮罩层（红色半透明，初始隐藏）
  _errorLayer = LayerColor::create(Color4B(255, 0, 0, 128));
  _errorLayer->setContentSize(this->getContentSize());
  _errorLayer->setPosition(Vec2::ZERO);
  _errorLayer->setVisible(false);
  this->addChild(_errorLayer, 5);  // 放在内容之上，标签之下

  // 创建锚点标记节点（红点）
  _anchorNode = DrawNode::create();
  // 绘制一个红色圆点（在锚点位置，即本地坐标系原点）
  auto width = this->getContentSize().width;
  auto height = this->getContentSize().height;
  _anchorNode->drawDot(Vec2(anchorRatioX * width, anchorRatioY * height), 5.0f,
                       Color4F(1.0f, 0.0f, 0.0f, 1.0f));  // 红色，半径5像素
  this->addChild(_anchorNode, 10);  // 放在最前面，确保可见

  // 创建生命值条
  _hpBarBackground = DrawNode::create();
  _hpBarForeground = DrawNode::create();
  this->addChild(_hpBarBackground, 10);
  this->addChild(_hpBarForeground, 11);
  updateHPBar();

  return true;
}

void Building::createDefaultAppearance() {
  // 根据建筑类型创建不同颜色的默认外观
  Color4B color = Color4B::WHITE;
  switch (_buildingType) {
    case BuildingType::TOWN_HALL:
      color = Color4B(139, 69, 19, 255);  // 棕色
      break;
    case BuildingType::DEFENSE:
      color = Color4B(255, 0, 0, 255);  // 红色
      break;
    case BuildingType::RESOURCE:
      color = Color4B(255, 215, 0, 255);  // 金色
      break;
    case BuildingType::STORAGE:
      color = Color4B(0, 0, 255, 255);  // 蓝色
      break;
    case BuildingType::BARRACKS:
      color = Color4B(0, 255, 0, 255);  // 绿色
      break;
    case BuildingType::WALL:
      color = Color4B(128, 128, 128, 255);  // 灰色
      break;
  }

  // 创建彩色矩形作为默认外观
  auto layer = LayerColor::create(color, 80, 80);
  layer->setPosition(Vec2::ZERO);
  this->addChild(layer);

  // 设置内容大小
  this->setContentSize(Size(80, 80));
}

bool Building::isOutOfBounds(int gridSize) const {
  float topRow, topCol, rightRow, rightCol, bottomRow, bottomCol, leftRow,
      leftCol;

  float halfGridCount = _gridCount / 2.0f;
  topRow = _row + halfGridCount;
  topCol = _col - halfGridCount;
  rightRow = _row + halfGridCount;
  rightCol = _col + halfGridCount;
  bottomRow = _row - halfGridCount;
  bottomCol = _col + halfGridCount;
  leftRow = _row - halfGridCount;
  leftCol = _col - halfGridCount;

  // 检查四个角是否都在有效范围内 [0, gridSize]
  if (topRow < 0 || topRow >= gridSize || topCol < 0 || topCol >= gridSize)
    return true;
  if (rightRow < 0 || rightRow >= gridSize || rightCol < 0 ||
      rightCol >= gridSize)
    return true;
  if (bottomRow < 0 || bottomRow >= gridSize || bottomCol < 0 ||
      bottomCol >= gridSize)
    return true;
  if (leftRow < 0 || leftRow >= gridSize || leftCol < 0 || leftCol >= gridSize)
    return true;

  return false;
}

void Building::showGlow() {
  if (!_glowNode) {
    return;
  }

  _glowNode->setVisible(true);

  // 获取配置管理器中的GlowDelay
  auto configManager = ConfigManager::getInstance();
  float glowDelay = 0.5f;
  if (configManager) {
    auto constantConfig = configManager->getConstantConfig();
    glowDelay = constantConfig.glowDelay;
  }

  // 先绘制一次光晕
  updateGlowDrawing();

  // 停止之前的动画
  if (_glowAction) {
    _glowNode->stopAction(_glowAction);
  }

  // 创建明暗交替动画
  // 使用FadeTo实现明暗交替（从0.3到0.8透明度，0-255范围）
  auto fadeToBright =
      FadeTo::create(glowDelay, static_cast<GLubyte>(0.8f * 255.0f));
  auto fadeToDark =
      FadeTo::create(glowDelay, static_cast<GLubyte>(0.3f * 255.0f));
  auto sequence = Sequence::create(fadeToBright, fadeToDark, nullptr);
  auto repeat = RepeatForever::create(sequence);

  // 设置初始透明度（0-255范围）
  _glowNode->setOpacity(static_cast<GLubyte>(0.6f * 255.0f));

  // 开始动画
  _glowAction = repeat;
  _glowNode->runAction(_glowAction);
}

void Building::updateGlowDrawing() {
  if (!_glowNode) {
    return;
  }

  _glowNode->clear();

  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return;
  }
  auto constantConfig = configManager->getConstantConfig();
  float deltaX = constantConfig.deltaX;
  float deltaY = constantConfig.deltaY;

  // 绘制光晕边框（使用黄色半透明）
  float glowWidth = 3.0f;  // 光晕宽度

  int gridCount = _gridCount;  // 使用getter方法获取宽度
  Vec2 center(this->getContentSize().width * _anchorRatioX,
              this->getContentSize().height * _anchorRatioY);
  Vec2 top = center + Vec2(0, gridCount * deltaY);
  Vec2 right = center + Vec2(gridCount * deltaX, 0);
  Vec2 bottom = center + Vec2(0, -gridCount * deltaY);
  Vec2 left = center + Vec2(-gridCount * deltaX, 0);
  // Vec2 top(_gridCount * deltaX, 2 * _gridCount * deltaY);
  // Vec2 right(2 * _gridCount * deltaX, _gridCount * deltaY);
  // Vec2 bottom(_gridCount * deltaX, 0);
  // Vec2 left(0, _gridCount * deltaY);
  // 绘制四条边
  _glowNode->drawSegment(top, right, glowWidth, _glowColor);
  _glowNode->drawSegment(right, bottom, glowWidth, _glowColor);
  _glowNode->drawSegment(bottom, left, glowWidth, _glowColor);
  _glowNode->drawSegment(left, top, glowWidth, _glowColor);
}

void Building::setPlacementValid(bool isValid) {
  if (isValid) {
    // 有效：黄色光晕，正常颜色
    _glowColor = Color4F(1.0f, 1.0f, 0.0f, 0.6f);
    if (_errorLayer) {
      _errorLayer->setVisible(false);
    }
  } else {
    // 无效：红色光晕，红色叠加
    _glowColor = Color4F(1.0f, 0.0f, 0.0f, 0.8f);
    if (_errorLayer) {
      _errorLayer->setVisible(true);
    }
  }
  // 立即更新光晕
  updateGlowDrawing();
}

void Building::hideGlow() {
  if (_glowNode) {
    _glowNode->setVisible(false);
    _glowNode->clear();
    if (_glowAction) {
      _glowNode->stopAction(_glowAction);
      _glowAction = nullptr;
    }
  }
}
// TODO bug
bool Building::inDiamond(const Vec2& pos) const {
  // Layer 坐标系下
  auto constantConfig = ConfigManager::getInstance()->getConstantConfig();
  float deltaX = constantConfig.deltaX;
  float deltaY = constantConfig.deltaY;
  // 将点转换为相对于锚点的坐标
  Vec2 relativePos = pos - this->getPosition();

  // 菱形的边界：以锚点为中心，gridSize为边长的菱形
  // 在等距投影中，菱形的边界条件：
  // |x / deltaX| + |y / deltaY| <= gridCount

  if (deltaX <= 0 || deltaY <= 0) {
    return false;
  }

  float dx = relativePos.x / deltaX;
  float dy = relativePos.y / deltaY;

  // 曼哈顿距离
  float manhattanDist = std::abs(dx) + std::abs(dy);

  return manhattanDist <= _gridCount;
}

// 建筑升级：包含重新设置纹理和刷新血量
void Building::upgrade() {
  if (_level >= _maxLevel) {
    CCLOG("Building %s reached max level %d", _buildingName.c_str(), _maxLevel);
    return;
  }

  _level++;

  // 使用新的 ConfigManager 接口
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);

  if (!config.image.empty()) {
    // 使用 TextureCache 获取纹理，确保资源存在
    auto texture = Director::getInstance()->getTextureCache()->addImage(config.image);
    if (texture) {
        // 切换纹理
        this->setTexture(texture);
        // 设置纹理矩形（这对 Sprite 显示完整图片很重要）
        this->setTextureRect(Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
        // [修改 2] 显式更新 ContentSize，因为新等级图片大小可能不同
        this->setContentSize(texture->getContentSize());
    } else {
        CCLOG("Failed to load image for upgrade: %s", config.image.c_str());
    }
  }

  // 由于 ContentSize 可能改变，需要刷新依赖尺寸的子节点位置

  // 1. 刷新信息标签位置
  if (_infoLabel) {
    _infoLabel->setPosition(Vec2(this->getContentSize().width / 2,
                                 this->getContentSize().height + 20));
  }

  // 2. 刷新选中光晕（Glow），因为光晕中心依赖于 ContentSize 和 anchor
  updateGlowDrawing();

  // 3. 刷新调试锚点显示
  if (_anchorNode) {
      _anchorNode->clear();
      float width = this->getContentSize().width;
      float height = this->getContentSize().height;
      _anchorNode->drawDot(Vec2(_anchorRatioX * width, _anchorRatioY * height), 5.0f,
                       Color4F(1.0f, 0.0f, 0.0f, 1.0f));
  }
  
  // 4. 刷新错误遮罩大小
  if (_errorLayer) {
      _errorLayer->setContentSize(this->getContentSize());
  }

  // 更新血量
  _maxHP = config.maxHP;
  // 升级通常会将血量回满
  _currentHP = _maxHP;

  // 5. 刷新血条（updateHPBar 内部会使用新的 ContentSize 重新计算位置）
  updateHPBar();

  CCLOG("Upgraded %s to level %d. Image: %s", _buildingName.c_str(), _level, config.image.c_str());
}

void Building::updateHPBar() {
  if (!_hpBarBackground || !_hpBarForeground) {
    return;
  }

  // 生命值条尺寸（根据建筑大小调整）
  float barWidth =
      this->getContentSize().width * 0.8f;  // 血条宽度为建筑宽度的80%
  if (barWidth < 40.0f) {
    barWidth = 40.0f;  // 最小宽度
  }
  float barHeight = 5.0f;  // 血条高度

  // 在Cocos2d-x中，设置锚点后，本地坐标系的原点(0,0)就是锚点位置
  float barY = this->getContentSize()
                   .height;  // 血条的Y坐标（相对于锚点，即本地坐标系原点）
  float anchorX = this->getContentSize().width * _anchorRatioX;
  float anchorY = this->getContentSize().height * _anchorRatioY;
  // 清除之前的绘制
  _hpBarBackground->clear();
  _hpBarForeground->clear();

  // 绘制背景（深红色/暗红色）
  // 血条中心在(barX, barY)，所以起点和终点相对于中心
  Vec2 bgStart(anchorX - barWidth / 2, barY);
  Vec2 bgEnd(anchorX + barWidth / 2, barY);
  _hpBarBackground->drawSegment(bgStart, bgEnd, barHeight,
                                Color4F(0.3f, 0.0f, 0.0f, 1.0f));

  // 绘制前景（红色，根据生命值比例）
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

      // 使用红色血条
      Color4F barColor = Color4F(1.0f, 0.0f, 0.0f, 1.0f);  // 红色

      _hpBarForeground->drawSegment(fgStart, fgEnd, barHeight, barColor);
    }
  }
}

void Building::setCurrentHPAndUpdate(float hp) {
  _currentHP = hp;
  if (_currentHP < 0) {
    _currentHP = 0;
  }
  if (_currentHP > _maxHP) {
    _currentHP = _maxHP;
  }
  updateHPBar();
}

void Building::setHealthBarVisible(bool visible) {
  if (_hpBarBackground) {
    _hpBarBackground->setVisible(visible);
  }
  if (_hpBarForeground) {
    _hpBarForeground->setVisible(visible);
  }
}

void Building::takeDamage(float damage) {
  if (!isAlive()) {
    return;
  }

  _currentHP -= damage;
  if (_currentHP < 0) {
    _currentHP = 0;
  }
  updateHPBar();

  // 如果HP为0，隐藏建筑
  if (_currentHP <= 0) {
    this->setVisible(false);
  }
}

bool Building::isAlive() const { return _currentHP > 0; }
