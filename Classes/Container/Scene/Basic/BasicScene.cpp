#include "BasicScene.h"

#include <float.h>

#include "Manager/Config/ConfigManager.h"

Scene* BasicScene::createScene() { return BasicScene::create(); }

bool BasicScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("Failed to get ConfigManager in BasicScene!");
    return false;
  }

  auto constantConfig = configManager->getConstantConfig();

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建地图容器层
  _mapLayer = Layer::create();
  this->addChild(_mapLayer, 0);
  _currentScale = 1.0f;
  _isDragging = false;
  _isMouseDown = false;
  _selectedBuilding = nullptr;
  _draggingBuilding = nullptr;
  // TODO
  _gridSize = constantConfig.gridSize;
  _buildingManager = nullptr;

  // 先计算p00（initGrassBackground需要用到）
  calculateP00();

  // 创建44x44网格地图背景，使用grass.png进行菱形密铺
  initGrassBackground();

  // 创建BuildingManager
  std::string jsonFilePath = "config/building_map.json";
  _buildingManager = new (std::nothrow) BuildingManager(jsonFilePath, _p00);
  if (_buildingManager && _buildingManager->init()) {
    _buildingManager->addBuildingsToLayer(_mapLayer);
  } else {
    CC_SAFE_DELETE(_buildingManager);
    _buildingManager = nullptr;
    CCLOG("Failed to initialize BuildingManager!");
  }

  // 添加标题（标题不随地图移动）
  auto titleLabel = Label::createWithSystemFont("COC - Village", "Arial", 36);
  titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height - 30));
  titleLabel->setColor(Color3B::WHITE);
  this->addChild(titleLabel, 1);

  // 初始化鼠标事件监听器
  initMouseEventListeners();
  return true;
}

void BasicScene::calculateP00() {
  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("ConfigManager not initialized");
    return;
  }

  auto constantConfig = configManager->getConstantConfig();

  // 从配置文件读取参数
  const float W = constantConfig.grassWidth;   // 图片宽度
  const float H = constantConfig.grassHeight;  // 图片高度

  // 获取可见区域大小和原点（此时分辨率已设置，可以正确获取）
  auto director = Director::getInstance();
  auto visibleSize = director->getVisibleSize();
  Vec2 origin = director->getVisibleOrigin();

  float totalWidth = (W / 2.0f) * 84 + W;  // 最右端x坐标 + 图片长度
  float minY = -(H / 2.0f) * 43;           // 最上端相对y坐标
  float maxY = (H / 2.0f) * 43;            // 最下端相对y坐标
  float totalHeight = maxY - minY + H;

  // 计算 p00 位置（最左端，居中显示）
  _p00 = Vec2(origin.x + (visibleSize.width - totalWidth) / 2,
              origin.y + visibleSize.height / 2);

  // 使用配置文件中的deltaX和deltaY
  _deltaX = constantConfig.deltaX;
  _deltaY = constantConfig.deltaY;
}

void BasicScene::initGrassBackground() {
  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  auto constantConfig = configManager->getConstantConfig();

  // 从配置文件读取参数
  const int GRID_SIZE = constantConfig.gridSize;
  const float W = constantConfig.grassWidth;   // 图片宽度
  const float H = constantConfig.grassHeight;  // 图片高度
  const std::string GRASS_PATH = constantConfig.grassImagePath;

  // p00已在calculateP00中计算
  Vec2 p00 = _p00;
  // 填充44x44的网格
  // 根据公式：p[i][j] = p[0][0] + (向右下j步) + (向上i步)
  // 向右下一步：x += W/2, y += H/2
  // 向上一步：x += W/2, y -= H/2
  // 所以：p[i][j].x = p[0][0].x + (i + j) * W/2
  //      p[i][j].y = p[0][0].y + (j - i) * H/2
  for (int i = 0; i < GRID_SIZE; ++i) {
    for (int j = 0; j < GRID_SIZE; ++j) {
      Vec2 pos;
      pos.x = p00.x + (i + j) * _deltaX;
      pos.y = p00.y + (j - i) * _deltaY;

      auto grassSprite = Sprite::create(GRASS_PATH);
      if (grassSprite) {
        // 设置尺寸（保持原始菱形尺寸）
        grassSprite->setContentSize(Size(W, H));
        // 注意：pos是图片左侧的中点，需要设置锚点
        grassSprite->setAnchorPoint(Vec2(0.0f, 0.5f));  // 左侧中点 ratio
        grassSprite->setPosition(pos);
        _mapLayer->addChild(grassSprite, 0);

        // Color4F::BLACK); 创建DrawNode用于绘制锚点和边框
        auto drawNode = DrawNode::create();

        // 1. 绘制黑色锚点（在锚点位置，即左侧中点，相对于sprite的(0, 0)）
        drawNode->drawDot(Vec2(0, H / 2.0f), 3.0f,
                          Color4F(0.0f, 0.0f, 0.0f, 1.0f));  // 黑色，半径3像素
        // drawNode 是 绝对像素坐标 左下角为00
        // 2. 绘制灰色边框（菱形的四个顶点）
        // - 左侧中点：(0, H/2) - 锚点位置
        // - 上顶点：(W/2, H)
        // - 右侧中点：(W, H/2)
        // - 下顶点：(W/2, 0)
        Vec2 leftMid(0, H / 2.0f);
        Vec2 topVertex(W / 2.0f, H);
        Vec2 rightMid(W, H / 2.0f);
        Vec2 bottomVertex(W / 2.0f, 0);

        // 绘制菱形边框（灰色，线宽1像素）
        Color4F borderColor(0.5f, 0.5f, 0.5f, 1.0f);  // 灰色
        float borderWidth = 1.0f;
        drawNode->drawLine(leftMid, topVertex, borderColor);
        drawNode->drawLine(topVertex, rightMid, borderColor);
        drawNode->drawLine(rightMid, bottomVertex, borderColor);
        drawNode->drawLine(bottomVertex, leftMid, borderColor);

        // 将DrawNode添加到grassSprite上，这样它会跟随sprite移动和缩放
        grassSprite->addChild(drawNode, 1);  // 放在sprite上方
      }
    }
  }
}

void BasicScene::initMouseEventListeners() {
  // 创建鼠标事件监听器
  auto mouseListener = EventListenerMouse::create();

  // 滚轮缩放事件
  mouseListener->onMouseScroll = [this](Event* event) {
    this->onMouseScroll(event);
  };

  // 鼠标按下事件
  mouseListener->onMouseDown = [this](Event* event) {
    this->onMouseDown(event);
  };

  // 鼠标移动事件
  mouseListener->onMouseMove = [this](Event* event) {
    this->onMouseMove(event);
  };

  // 鼠标抬起事件
  mouseListener->onMouseUp = [this](Event* event) { this->onMouseUp(event); };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void BasicScene::onMouseScroll(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  float scrollY = mouseEvent->getScrollY();
  // TODO 视角高低 最好放到参数里
  // 缩放因子
  const float SCALE_FACTOR = 0.1f;
  const float MIN_SCALE = 0.5f;
  const float MAX_SCALE = 2.0f;

  // 计算新的缩放比例
  float deltaScale =
      scrollY > 0 ? (1.0f + SCALE_FACTOR) : (1.0f - SCALE_FACTOR);
  float newScale = _currentScale * deltaScale;

  // 限制缩放范围
  if (newScale < MIN_SCALE) {
    newScale = MIN_SCALE;
  } else if (newScale > MAX_SCALE) {
    newScale = MAX_SCALE;
  }

  if (newScale != _currentScale) {
    // 获取鼠标在屏幕坐标系中的位置
    Vec2 mouseGLPos = mouseEvent->getLocationInView();

    // 获取地图层当前的位置和缩放
    Vec2 mapLayerPos = _mapLayer->getPosition();
    float oldScale = _currentScale;

    // 计算鼠标在地图层本地坐标系中的位置（缩放前）
    Vec2 mouseInMapBefore = (mouseGLPos - mapLayerPos) / oldScale;

    // 应用新缩放
    _currentScale = newScale;
    _mapLayer->setScale(_currentScale);

    // 计算缩放后，鼠标应该在地图层中的位置
    Vec2 mouseInMapAfter = mouseInMapBefore * _currentScale;

    // 调整地图层位置，使鼠标指向的点保持不动
    Vec2 newMapLayerPos = mouseGLPos - mouseInMapAfter;
    _mapLayer->setPosition(newMapLayerPos);
  }
}

void BasicScene::onMouseDown(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  // 检查是否按下左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    Vec2 mousePos = mouseEvent->getLocationInView();
    _mouseDownPos = mousePos;  // 记录按下位置

    // 转换为地图层坐标系
    Vec2 mapPos = _mapLayer->convertToNodeSpace(mousePos);

    // 检查是否点击了建筑（使用BuildingManager）
    Building* clickedBuilding = nullptr;

    if (_buildingManager) {
      // mapPos是地图层坐标系，直接传递给BuildingManager
      clickedBuilding = _buildingManager->getBuildingAtPosition(mapPos);
    }

    if (clickedBuilding) {
      // 点击了建筑：立即开始闪烁边界
      if (_selectedBuilding && _selectedBuilding != clickedBuilding) {
        // 取消之前选中的建筑
        _selectedBuilding->hideGlow();
      }

      _selectedBuilding = clickedBuilding;
      _selectedBuilding->showGlow();                         // 立即显示闪烁边界
      _selectedBuilding->_isDragging = false;                // 还未开始拖动
      _buildingStartPos = _selectedBuilding->getPosition();  // 记录建筑初始位置
      _draggingBuilding = nullptr;                           // 还未开始拖动
      _isMouseDown = true;                                   // 标记鼠标已按下
    } else {
      // 没有点击建筑：取消选中，准备拖动地图
      if (_selectedBuilding) {
        _selectedBuilding->hideGlow();
        _selectedBuilding = nullptr;
      }
      _isDragging = true;
      _lastMousePos = mousePos;
      _isMouseDown = true;  // 标记鼠标已按下
    }
  }
}

void BasicScene::onMouseMove(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  Vec2 currentPos = mouseEvent->getLocationInView();

  // 只有按住鼠标才能拖动建筑
  if (_isMouseDown && _selectedBuilding && !_draggingBuilding) {
    // 如果选中了建筑但还未开始拖动，检查是否移动了足够距离
    float dragThreshold = 5.0f;  // 拖动阈值（像素）
    float moveDistance = _mouseDownPos.distance(currentPos);

    if (moveDistance > dragThreshold) {
      // 开始拖动建筑
      _draggingBuilding = _selectedBuilding;
      _draggingBuilding->_isDragging = true;

      // 计算拖动偏移量（鼠标位置相对于建筑锚点的偏移）
      Vec2 mapPos = _mapLayer->convertToNodeSpace(currentPos);
      _draggingBuilding->_dragOffset = mapPos - _buildingStartPos;
    }
  }

  // 只有按住鼠标才能拖动建筑
  if (_isMouseDown && _draggingBuilding) {
    // 拖动建筑：实时更新位置并显示吸附预览
    Vec2 mapPos = _mapLayer->convertToNodeSpace(currentPos);

    // 竖直分量取相反数（鼠标向下移动时，建筑向上移动）
    Vec2 targetAnchorPos = mapPos - _draggingBuilding->_dragOffset;

    // 找到最近的网格点（用于吸附预览）
    int row, col;
    Vec2 nearestPos;
    if (GridUtils::findNearestGrassVertex(targetAnchorPos, _p00, row, col,
                                          nearestPos)) {
      // 临时设置位置到吸附点（拖动时显示预览）
      _draggingBuilding->setPosition(nearestPos);

      // 临时更新中心坐标（用于预览）
      _draggingBuilding->setCenterX(
          nearestPos.x + _deltaX * _draggingBuilding->getGridCount());
      _draggingBuilding->setCenterY(nearestPos.y);

      // 关键：更新网格坐标，以便进行重叠检测
      _draggingBuilding->setRow(row);
      _draggingBuilding->setCol(col);
    } else {
      // 如果找不到有效网格点，使用原始位置
      _draggingBuilding->setPosition(targetAnchorPos);
      _draggingBuilding->setCenterX(
          targetAnchorPos.x + _deltaX * _draggingBuilding->getGridCount());
      _draggingBuilding->setCenterY(targetAnchorPos.y);
    }
  } else if (_isDragging) {
    // 拖动地图（鼠标移动方向与地图移动方向相反，就像拖动物体）
    Vec2 delta = currentPos - _lastMousePos;
    delta.x = delta.x;  // 水平分量保持原样
    _mapLayer->setPosition(_mapLayer->getPosition() + delta);
    _lastMousePos = currentPos;
  }
}

void BasicScene::onMouseUp(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  // 检查是否释放左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    if (_draggingBuilding) {
      // 如果正在拖动建筑，进行最终吸附
      Vec2 mousePos = mouseEvent->getLocationInView();

      // 转换为地图层坐标系
      Vec2 mapPos = _mapLayer->convertToNodeSpace(mousePos);
      Vec2 targetAnchorPos = mapPos - _draggingBuilding->_dragOffset;

      // 找到最近的网格点
      int row, col;
      Vec2 nearestPos;
      if (GridUtils::findNearestGrassVertex(targetAnchorPos, _p00, row, col,
                                            nearestPos)) {
        _draggingBuilding->setCenterX(
            nearestPos.x + _deltaX * _draggingBuilding->getGridCount());
        _draggingBuilding->setCenterY(nearestPos.y);
        _draggingBuilding->setRow(row);
        // 检查是否有效（边界 + 子类规则）
        if (!isPlacementValid(_draggingBuilding)) {
          // 如果无效，恢复到之前的位置
          _draggingBuilding->setPosition(_buildingStartPos);
          // 恢复原来的网格坐标（重要！否则逻辑位置会错乱）
          // 这里比较麻烦，因为我们没有保存原来的 row/col。
          // 但 _buildingStartPos 对应的应该是原来的 row/col。
          // 简单起见，我们假设 _buildingStartPos 是正确的。
          // 实际上，如果恢复位置，下次点击时会重新计算 row/col。
          // 但为了数据一致性，最好重新计算一下 startPos 的 row/col。
          int startRow, startCol;
          GridUtils::screenToGrid(_buildingStartPos, _p00, startRow, startCol);
          _draggingBuilding->setRow(startRow);
          _draggingBuilding->setCol(startCol);
        }
      } else {
        // 找不到有效网格点，恢复到之前的位置
        _draggingBuilding->setPosition(_buildingStartPos);
      }

      _draggingBuilding->setPlacementValid(true);  // 恢复正常颜色
      _draggingBuilding->_isDragging = false;
      _draggingBuilding = nullptr;
      _isMouseDown = false;  // 标记鼠标已释放

      // 拖动结束后自动取消选中状态
      if (_selectedBuilding) {
        _selectedBuilding->hideGlow();
        _selectedBuilding = nullptr;
      }
    } else if (_selectedBuilding) {
      // 如果只是选中了建筑但没有拖动，保持选中状态（光晕继续显示）
      // 可以在这里添加其他点击行为（如显示建筑信息）
      _isMouseDown = false;  // 标记鼠标已释放
    } else {
      // 没有选中或拖动建筑，停止拖动地图
      _isDragging = false;
      _isMouseDown = false;  // 标记鼠标已释放
    }
  }
}

void BasicScene::showPopupDialog(const std::string& title,
                                 const std::string& message) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建半透明背景层
  auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width,
                                    visibleSize.height);
  bgLayer->setPosition(origin);
  bgLayer->setName("PopupBackground");
  this->addChild(bgLayer, 1000);

  // 创建对话框背景
  float dialogWidth = 400.0f;
  float dialogHeight = 200.0f;
  auto dialogBg =
      LayerColor::create(Color4B(50, 50, 50, 255), dialogWidth, dialogHeight);
  dialogBg->setPosition(
      Vec2(origin.x + (visibleSize.width - dialogWidth) / 2,
           origin.y + (visibleSize.height - dialogHeight) / 2));
  dialogBg->setName("DialogBackground");
  this->addChild(dialogBg, 1001);

  // 创建标题标签
  auto titleLabel = Label::createWithSystemFont(title, "Arial", 24);
  titleLabel->setColor(Color3B::WHITE);
  titleLabel->setPosition(Vec2(dialogWidth / 2, dialogHeight - 40));
  dialogBg->addChild(titleLabel);

  // 创建消息标签
  auto messageLabel = Label::createWithSystemFont(message, "Arial", 16);
  messageLabel->setColor(Color3B::WHITE);
  messageLabel->setDimensions(dialogWidth - 40, 0);
  messageLabel->setAlignment(TextHAlignment::CENTER);
  messageLabel->setPosition(Vec2(dialogWidth / 2, dialogHeight / 2 - 20));
  dialogBg->addChild(messageLabel);

  // 创建确定按钮
  auto okButton = Label::createWithSystemFont("确定", "Arial", 18);
  okButton->setColor(Color3B::YELLOW);
  auto okButtonBg = LayerColor::create(Color4B(100, 100, 100, 255), 100, 40);
  okButtonBg->setPosition(Vec2((dialogWidth - 100) / 2, 20));
  okButton->setPosition(Vec2(50, 20));
  okButtonBg->addChild(okButton);
  dialogBg->addChild(okButtonBg);

  // 添加按钮点击事件
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this, bgLayer, dialogBg](Touch* touch,
                                                     Event* event) -> bool {
    Vec2 location = touch->getLocation();
    Vec2 dialogPos = dialogBg->getPosition();
    Size dialogSize = dialogBg->getContentSize();

    // 检查是否点击了确定按钮区域
    if (location.x >= dialogPos.x &&
        location.x <= dialogPos.x + dialogSize.width &&
        location.y >= dialogPos.y &&
        location.y <= dialogPos.y + dialogSize.height) {
      // 关闭弹窗
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      return true;
    }
    // 点击背景也关闭弹窗
    else if (location.x >= bgLayer->getPosition().x &&
             location.x <=
                 bgLayer->getPosition().x + bgLayer->getContentSize().width &&
             location.y >= bgLayer->getPosition().y &&
             location.y <=
                 bgLayer->getPosition().y + bgLayer->getContentSize().height) {
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      return true;
    }
    return false;
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, dialogBg);
}

bool BasicScene::isPlacementValid(Building* building) const {
  // 基类只检查边界
  if (!building) return false;
  return !building->isOutOfBounds(_gridSize);
}

BasicScene::~BasicScene() {
  if (_buildingManager) {
    delete _buildingManager;
    _buildingManager = nullptr;
  }
}