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
      _glowAction(nullptr) {}

Building::~Building() {}


bool Building::init(const std::string& imagePath, BuildingType type, const int& level,
                    const int& gridCount, const float& anchorRatioX, const float& anchorRatioY,
                    const float& imageScale) {
  _buildingType = type;
  _level = level;
  _gridCount = gridCount;

  // 尝试加载图片，如果失败则创建默认外观
  if (!Sprite::initWithFile(imagePath)) {
    createDefaultAppearance();
  }


  this->setScale(imageScale);

  // 设置锚点
  this->setAnchorPoint(Vec2(_anchorRatioX = anchorRatioX, _anchorRatioY = anchorRatioY));

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

  // 创建锚点标记节点（红点）
  _anchorNode = DrawNode::create();
  // 绘制一个红色圆点（在锚点位置，即本地坐标系原点）
  auto width = this->getContentSize().width;
  auto height = this->getContentSize().height;
  _anchorNode->drawDot(Vec2(anchorRatioX * width, anchorRatioY * height), 5.0f,
                       Color4F(1.0f, 0.0f, 0.0f, 1.0f));  // 红色，半径5像素
  this->addChild(_anchorNode, 10);  // 放在最前面，确保可见

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
  }

  // 创建彩色矩形作为默认外观
  auto layer = LayerColor::create(color, 80, 80);
  layer->setPosition(Vec2::ZERO);
  this->addChild(layer);

  // 设置内容大小
  this->setContentSize(Size(80, 80));
}

bool Building::isOutOfBounds(int gridSize) const {
  int topRow, topCol, rightRow, rightCol, bottomRow, bottomCol, leftRow,leftCol;

  int halfGridCount = _gridCount / 2;
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
  if (leftRow < 0 || leftRow >= gridSize || leftCol < 0 || leftCol >= gridSize  )
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
  Color4F glowColor(1.0f, 1.0f, 0.0f, 0.6f);  // 黄色，60%透明度
  float glowWidth = 3.0f;                     // 光晕宽度

  int gridCount = _gridCount;  // 使用getter方法获取宽度
  Vec2 center(this->getContentSize().width * _anchorRatioX, 
        this->getContentSize().height * _anchorRatioY);
  Vec2 top = center + Vec2(0, gridCount * deltaY);
  Vec2 right = center + Vec2(gridCount * deltaX, 0);
  Vec2 bottom = center + Vec2(0, - gridCount * deltaY);
  Vec2 left = center + Vec2(- gridCount * deltaX, 0);
  // Vec2 top(_gridCount * deltaX, 2 * _gridCount * deltaY);
  // Vec2 right(2 * _gridCount * deltaX, _gridCount * deltaY);
  // Vec2 bottom(_gridCount * deltaX, 0);
  // Vec2 left(0, _gridCount * deltaY);
  // 绘制四条边
  _glowNode->drawSegment(top, right, glowWidth, glowColor);
  _glowNode->drawSegment(right, bottom, glowWidth, glowColor);
  _glowNode->drawSegment(bottom, left, glowWidth, glowColor);
  _glowNode->drawSegment(left, top, glowWidth, glowColor);
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
