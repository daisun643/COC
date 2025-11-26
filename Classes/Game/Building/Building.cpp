#include "Building.h"

Building::Building()
    : _buildingType(BuildingType::TOWN_HALL), _level(1), _maxLevel(10),
      _infoLabel(nullptr), _buildingName("Building"),
      _centerX(0.0f), _centerY(0.0f), _width(1), _row(0), _col(0),
      _isDragging(false), _glowNode(nullptr), _anchorNode(nullptr) {}

Building::~Building() {}

Building *Building::create(const std::string &imagePath, BuildingType type,
                           int level) {
  Building *building = new (std::nothrow) Building();
  if (building && building->init(imagePath, type, level)) {
    building->autorelease();
    return building;
  }
  CC_SAFE_DELETE(building);
  return nullptr;
}

bool Building::init(const std::string &imagePath, BuildingType type,
                    int level) {
  _buildingType = type;
  _level = level;

  // 尝试加载图片，如果失败则创建默认外观
  if (!Sprite::initWithFile(imagePath)) {
    createDefaultAppearance();
  }

  // 设置锚点为左侧中点（与grass一致）
  this->setAnchorPoint(Vec2(0.0f, 0.5f));

  // 创建信息标签（初始隐藏）
  _infoLabel = Label::createWithSystemFont("", "Arial", 12);
  _infoLabel->setPosition(Vec2(this->getContentSize().width / 2,
                               this->getContentSize().height + 20));
  _infoLabel->setColor(Color3B::WHITE);
  _infoLabel->setVisible(false);
  this->addChild(_infoLabel, 10);

  // 触摸事件由GameScene统一处理（使用鼠标事件）
  // 这里保留点击显示信息的功能（如果需要）
  
  // 创建光晕效果节点（初始隐藏）
  _glowNode = DrawNode::create();
  _glowNode->setVisible(false);
  this->addChild(_glowNode, -1); // 放在建筑后面
  
  // 创建锚点标记节点（红点）
  _anchorNode = DrawNode::create();
  // 锚点位置是(0, 0.5)，即左侧中点
  // 绘制一个红色圆点
  _anchorNode->drawDot(Vec2(0, 0), 5.0f, Color4F(1.0f, 0.0f, 0.0f, 1.0f)); // 红色，半径5像素
  this->addChild(_anchorNode, 10); // 放在最前面，确保可见

  return true;
}

void Building::createDefaultAppearance() {
  // 根据建筑类型创建不同颜色的默认外观
  Color4B color = Color4B::WHITE;
  switch (_buildingType) {
  case BuildingType::TOWN_HALL:
    color = Color4B(139, 69, 19, 255); // 棕色
    break;
  case BuildingType::DEFENSE:
    color = Color4B(255, 0, 0, 255); // 红色
    break;
  case BuildingType::RESOURCE:
    color = Color4B(255, 215, 0, 255); // 金色
    break;
  case BuildingType::STORAGE:
    color = Color4B(0, 0, 255, 255); // 蓝色
    break;
  case BuildingType::BARRACKS:
    color = Color4B(0, 255, 0, 255); // 绿色
    break;
  }

  // 创建彩色矩形作为默认外观
  auto layer = LayerColor::create(color, 80, 80);
  layer->setPosition(Vec2::ZERO);
  this->addChild(layer);

  // 设置内容大小
  this->setContentSize(Size(80, 80));
}

bool Building::upgrade() {
  if (_level >= _maxLevel) {
    return false; // 已达到最高等级
  }

  _level++;
  // 可以在这里添加升级后的视觉效果
  return true;
}

int Building::getUpgradeCost() const {
  // 基础升级成本 * 等级
  return 100 * _level;
}

std::string Building::getBuildingInfo() const {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%s\nLevel: %d/%d\nUpgrade: %d",
           _buildingName.c_str(), _level, _maxLevel, getUpgradeCost());
  return std::string(buffer);
}

void Building::showInfo() {
  if (_infoLabel) {
    _infoLabel->setString(getBuildingInfo());
    _infoLabel->setVisible(true);
  }
}

void Building::hideInfo() {
  if (_infoLabel) {
    _infoLabel->setVisible(false);
  }
}

void Building::setCenterPosition(float x, float y, int row, int col) {
  _centerX = x;
  _centerY = y;
  _row = row;
  _col = col;
  // 更新显示位置（锚点在左侧中点）
  this->setPosition(Vec2(x, y));
}

void Building::getCornerCoordinates(int &topRow, int &topCol, 
                                    int &rightRow, int &rightCol,
                                    int &bottomRow, int &bottomCol,
                                    int &leftRow, int &leftCol) const {
  // Building是宽度*宽度的菱形
  // 在等距投影坐标系中，菱形的四个角：
  // 顶部：向上移动width/2步 (row - width/2, col)
  // 右侧：向右下移动width/2步 (row, col + width/2)
  // 底部：向下移动width/2步 (row + width/2, col)
  // 左侧：向左上移动width/2步 (row, col - width/2)
  int halfWidth = _width / 2;
  
  topRow = _row - halfWidth;
  topCol = _col;
  
  rightRow = _row;
  rightCol = _col + halfWidth;
  
  bottomRow = _row + halfWidth;
  bottomCol = _col;
  
  leftRow = _row;
  leftCol = _col - halfWidth;
}

bool Building::isOutOfBounds(int gridSize) const {
  int topRow, topCol, rightRow, rightCol, bottomRow, bottomCol, leftRow, leftCol;
  getCornerCoordinates(topRow, topCol, rightRow, rightCol, 
                       bottomRow, bottomCol, leftRow, leftCol);
  
  // 检查四个角是否都在有效范围内 [0, gridSize-1]
  if (topRow < 0 || topRow >= gridSize || topCol < 0 || topCol >= gridSize)
    return true;
  if (rightRow < 0 || rightRow >= gridSize || rightCol < 0 || rightCol >= gridSize)
    return true;
  if (bottomRow < 0 || bottomRow >= gridSize || bottomCol < 0 || bottomCol >= gridSize)
    return true;
  if (leftRow < 0 || leftRow >= gridSize || leftCol < 0 || leftCol >= gridSize)
    return true;
  
  return false;
}

void Building::showGlow() {
  if (!_glowNode) {
    return;
  }
  
  _glowNode->clear();
  _glowNode->setVisible(true);
  
  // 获取建筑尺寸
  Size buildingSize = this->getContentSize();
  float width = buildingSize.width;
  float height = buildingSize.height;
  
  // 绘制光晕边框（使用黄色半透明）
  Color4F glowColor(1.0f, 1.0f, 0.0f, 0.6f); // 黄色，60%透明度
  float glowWidth = 3.0f; // 光晕宽度
  
  // 绘制菱形的光晕边框
  // 由于建筑锚点是左侧中点(0, 0.5)，需要调整坐标
  // 菱形的中心在(width/2, height/2)，但相对于锚点需要偏移
  float halfW = width / 2;
  float halfH = height / 2;
  
  // 菱形的四个顶点（相对于锚点(0, 0.5)）
  // 锚点在左侧中点，所以中心在(width/2, 0)
  Vec2 center(halfW, 0);
  Vec2 top(center.x, center.y + halfH);
  Vec2 right(center.x + halfW, center.y);
  Vec2 bottom(center.x, center.y - halfH);
  Vec2 left(center.x - halfW, center.y);
  
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
  }
}
