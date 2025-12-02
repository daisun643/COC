#include "GameScene.h"

#include <float.h>

#include "Game/Building/PlaceholderBuilding.h"
#include "Manager/Config/ConfigManager.h"

Scene* GameScene::createScene() { return GameScene::create(); }

bool GameScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("Failed to get ConfigManager in GameScene!");
    return false;
  }

  auto constantConfig = configManager->getConstantConfig();

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建地图容器层
  _mapLayer = Layer::create();
  this->addChild(_mapLayer, 0);

  // 创建 UI 层 (ZOrder 设为 100，保证在最上层)
  _uiLayer = MainUILayer::create();
  this->addChild(_uiLayer, 100);

  // 设置 UI 按钮回调
  _uiLayer->setOnShopClickCallback([this]() { this->openShop(); });

  _uiLayer->setOnAttackClickCallback([]() {
    CCLOG("Attack Button Clicked!");
    // TODO: 切换到进攻场景
  });

  _currentScale = 1.0f;
  _isDragging = false;
  _isMouseDown = false;
  _selectedBuilding = nullptr;
  _draggingBuilding = nullptr;
  _isPlacingBuilding = false;
  _placementBuilding = nullptr;
  _placementHintLabel = nullptr;
  _isPlacementMouseDown = false;
  _placementDraggingMap = false;
  _placementMouseDownPos = Vec2::ZERO;
  _placementLastMousePos = Vec2::ZERO;
  _placementPreviewValid = false;
  _placementPreviewRow = 0;
  _placementPreviewCol = 0;
  _placementPreviewAnchor = Vec2::ZERO;
  _currentMousePos = Vec2::ZERO;
  _ignoreNextMouseUp = false;
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

  _placementHintLabel = Label::createWithSystemFont("", "Arial", 20);
  if (_placementHintLabel) {
    _placementHintLabel->setColor(Color3B::YELLOW);
    _placementHintLabel->setVisible(false);
    _placementHintLabel->setPosition(
        Vec2(origin.x + visibleSize.width / 2.0f, origin.y + 80.0f));
    this->addChild(_placementHintLabel, 150);
  }

  // 初始化鼠标事件监听器
  initMouseEventListeners();
  return true;
}

void GameScene::calculateP00() {
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

void GameScene::initGrassBackground() {
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

void GameScene::initMouseEventListeners() {
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

void GameScene::onMouseScroll(Event* event) {
  if (isShopOpen()) {
    return;
  }

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
    // 获取鼠标在OpenGL坐标系中的位置
    // 注意：在当前环境下，getLocationInView似乎直接返回了GL坐标（左下角原点）
    // 因此不需要convertToGL，否则会导致Y轴翻转
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

void GameScene::onMouseDown(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  if (isShopOpen()) {
    return;
  }

  if (_isPlacingBuilding) {
    // 放置模式下：
    // 左键点击：不做处理，等待 MouseUp
    // 进行放置（或者在这里放置也可以，但为了统一逻辑，通常在 Up 处理）
    // 右键点击：取消放置
    if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
      cancelPlacementMode(true);
      showPopupDialog("提示", "已取消放置，资源已退回");
      return;
    }
    return;
  }

  // 检查是否按下左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    Vec2 mousePos = mouseEvent->getLocationInView();
    // mousePos = Director::getInstance()->convertToGL(mousePos);
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

void GameScene::onMouseMove(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  Vec2 currentPos = mouseEvent->getLocationInView();
  // currentPos = Director::getInstance()->convertToGL(currentPos);
  _currentMousePos = currentPos;

  if (isShopOpen()) {
    return;
  }

  if (_isPlacingBuilding) {
    // 放置模式下禁用地图拖动，仅更新建筑预览位置
    if (_placementBuilding) {
      updatePlacementPreview(currentPos);
    }
    return;
  }

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
      // 直接使用 mapPos，不需要翻转
      _draggingBuilding->_dragOffset = mapPos - _buildingStartPos;
    }
  }

  // 只有按住鼠标才能拖动建筑
  if (_isMouseDown && _draggingBuilding) {
    // 拖动建筑：实时更新位置并显示吸附预览
    Vec2 mapPos = _mapLayer->convertToNodeSpace(currentPos);

    // 直接使用 mapPos，不需要翻转
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
    delta.x = delta.x;   // 水平分量保持原样
    // delta.y = -delta.y;  // 垂直分量取相反数
    _mapLayer->setPosition(_mapLayer->getPosition() + delta);
    _lastMousePos = currentPos;
  }
}

void GameScene::onMouseUp(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  if (isShopOpen()) {
    return;
  }

  if (_ignoreNextMouseUp) {
    _ignoreNextMouseUp = false;
    return;
  }

  if (_isPlacingBuilding) {
    Vec2 mousePos = mouseEvent->getLocationInView();
    // Vec2 worldPos = Director::getInstance()->convertToGL(mousePos);
    Vec2 worldPos = mousePos;

    if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
      // 放置模式下点击左键：尝试放置建筑
      updatePlacementPreview(worldPos);

      if (!_placementBuilding) {
        return;
      }

      if (!_placementPreviewValid) {
        showPopupDialog("提示", "请选择有效的草地网格");
        return;
      }

      _placementBuilding->setPosition(_placementPreviewAnchor);

      auto configManager = ConfigManager::getInstance();
      if (configManager) {
        auto constantConfig = configManager->getConstantConfig();
        float deltaX = constantConfig.deltaX;
        _placementBuilding->setCenterX(_placementPreviewAnchor.x +
                                       deltaX *
                                           _placementBuilding->getGridCount());
      } else {
        _placementBuilding->setCenterX(_placementPreviewAnchor.x);
      }
      _placementBuilding->setCenterY(_placementPreviewAnchor.y);
      _placementBuilding->setRow(_placementPreviewRow);
      _placementBuilding->setCol(_placementPreviewCol);

      if (_placementBuilding->isOutOfBounds(_gridSize)) {
        showPopupDialog("提示", "超出地图范围，无法放置");
        return;
      }

      _placementBuilding->setOpacity(255);
      _placementBuilding->hideGlow();

      if (_buildingManager) {
        _buildingManager->registerBuilding(_placementBuilding);
      }

      _selectedBuilding = _placementBuilding;
      _placementBuilding = nullptr;
      _isPlacingBuilding = false;
      _placementPreviewValid = false;
      _isPlacementMouseDown = false;
      _placementMouseDownPos = Vec2::ZERO;
      showPlacementHint("建筑已放置");

      if (_placementHintLabel) {
        _placementHintLabel->stopAllActions();
        auto delay = DelayTime::create(1.5f);
        auto clear = CallFunc::create([this]() { showPlacementHint(""); });
        _placementHintLabel->runAction(Sequence::create(delay, clear, nullptr));
      }
      return;
    }

    if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
      cancelPlacementMode(true);
      showPopupDialog("提示", "已取消放置，资源已退回");
      return;
    }

    return;
  }

  // 检查是否释放左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    if (_draggingBuilding) {
      // 如果正在拖动建筑，进行最终吸附
      Vec2 mousePos = mouseEvent->getLocationInView();
      // mousePos = Director::getInstance()->convertToGL(mousePos);

      // 转换为地图层坐标系
      Vec2 mapPos = _mapLayer->convertToNodeSpace(mousePos);

      // 直接使用 mapPos，不需要翻转
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
        // 检查是否越界
        if (_draggingBuilding->isOutOfBounds(_gridSize)) {
          // 如果越界，恢复到之前的位置
          _draggingBuilding->setPosition(_buildingStartPos);
          // 显示错误提示
          showPopupDialog("错误", "建筑移动越界，已恢复到原位置");
        } else {
          // 位置有效，显示更新后的建筑格点坐标
          std::string coordMsg = "建筑已移动到坐标: (" + std::to_string(row) +
                                 ", " + std::to_string(col) + ")";
          showPopupDialog("建筑位置", coordMsg);
        }
      } else {
        // 找不到有效网格点，恢复到之前的位置
        _draggingBuilding->setPosition(_buildingStartPos);
        showPopupDialog("错误", "无法找到有效网格点，已恢复到原位置");
      }

      _draggingBuilding->_isDragging = false;
      _draggingBuilding = nullptr;
      _isMouseDown = false;  // 标记鼠标已释放
      // 注意：不隐藏光晕，保持选中状态
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

void GameScene::showPopupDialog(const std::string& title,
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

void GameScene::openShop() {
  if (this->getChildByName("ShopLayerUI")) {
    return;
  }

  auto items = buildShopCatalog();
  if (items.empty()) {
    showPopupDialog("提示", "暂时没有可购买的建筑");
    return;
  }

  auto shopLayer = ShopLayer::createWithItems(items);
  if (!shopLayer) {
    return;
  }

  shopLayer->setName("ShopLayerUI");
  shopLayer->setPurchaseCallback(
      [this](const ShopItem& item) { return this->handleShopPurchase(item); });
  this->addChild(shopLayer, 200);
}

bool GameScene::isShopOpen() const {
  return this->getChildByName("ShopLayerUI") != nullptr;
}

std::vector<ShopItem> GameScene::buildShopCatalog() const {
  std::vector<ShopItem> items;

  auto configManager = ConfigManager::getInstance();
  if (configManager) {
    auto townHallConfig = configManager->getTownHallConfig();
    ShopItem townHall;
    townHall.id = "TownHall";
    townHall.displayName = "大本营";
    townHall.description = "村庄的核心建筑，解锁更多功能";
    townHall.costGold = 1000;
    townHall.costElixir = 0;
    townHall.defaultLevel = townHallConfig.defaultLevel;
    townHall.category = BuildingType::TOWN_HALL;
    townHall.placeholderColor = Color4B(139, 69, 19, 255);
    townHall.gridCount = townHallConfig.gridCount;
    townHall.anchorRatioX = townHallConfig.anchorRatioX;
    townHall.anchorRatioY = townHallConfig.anchorRatioY;
    townHall.imageScale = townHallConfig.imageScale;
    items.push_back(townHall);
  }

  ShopItem goldMine;
  goldMine.id = "GoldMine";
  goldMine.displayName = "金矿";
  goldMine.description = "持续生产金币的基础建筑";
  goldMine.costGold = 300;
  goldMine.costElixir = 0;
  goldMine.defaultLevel = 1;
  goldMine.category = BuildingType::RESOURCE;
  goldMine.placeholderColor = Color4B(212, 175, 55, 255);
  goldMine.gridCount = 2;
  goldMine.anchorRatioX = 0.5f;
  goldMine.anchorRatioY = 0.25f;
  goldMine.imageScale = 0.7f;
  items.push_back(goldMine);

  ShopItem elixirCollector;
  elixirCollector.id = "ElixirCollector";
  elixirCollector.displayName = "圣水采集器";
  elixirCollector.description = "产出圣水的基础设施";
  elixirCollector.costGold = 0;
  elixirCollector.costElixir = 350;
  elixirCollector.defaultLevel = 1;
  elixirCollector.category = BuildingType::RESOURCE;
  elixirCollector.placeholderColor = Color4B(140, 90, 200, 255);
  elixirCollector.gridCount = 2;
  elixirCollector.anchorRatioX = 0.5f;
  elixirCollector.anchorRatioY = 0.3f;
  elixirCollector.imageScale = 0.7f;
  items.push_back(elixirCollector);

  ShopItem cannon;
  cannon.id = "Cannon";
  cannon.displayName = "加农炮";
  cannon.description = "基础防御建筑，守护村庄安全";
  cannon.costGold = 800;
  cannon.costElixir = 200;
  cannon.defaultLevel = 1;
  cannon.category = BuildingType::DEFENSE;
  cannon.placeholderColor = Color4B(120, 120, 120, 255);
  cannon.gridCount = 2;
  cannon.anchorRatioX = 0.5f;
  cannon.anchorRatioY = 0.35f;
  cannon.imageScale = 0.65f;
  items.push_back(cannon);

  return items;
}

bool GameScene::handleShopPurchase(const ShopItem& item) {
  auto playerManager = PlayerManager::getInstance();
  if (!playerManager) {
    showPopupDialog("错误", "未找到玩家数据，无法购买");
    return false;
  }

  if (_isPlacingBuilding) {
    cancelPlacementMode(true);
  }

  if (item.costGold > 0 && playerManager->getGold() < item.costGold) {
    showPopupDialog("提示", "金币不足，无法购买该建筑");
    return false;
  }

  if (item.costElixir > 0 && playerManager->getElixir() < item.costElixir) {
    showPopupDialog("提示", "圣水不足，无法购买该建筑");
    return false;
  }

  if (!playerManager->consumeGold(item.costGold)) {
    showPopupDialog("提示", "金币不足，无法购买该建筑");
    return false;
  }

  if (!playerManager->consumeElixir(item.costElixir)) {
    // 退回金币
    playerManager->addGold(item.costGold);
    showPopupDialog("提示", "圣水不足，无法购买该建筑");
    return false;
  }

  enterPlacementMode(item);
  return true;
}

void GameScene::enterPlacementMode(const ShopItem& item) {
  cancelPlacementMode(false);

  // 强制重置地图拖动状态，防止购买后地图跟随鼠标移动
  _isDragging = false;
  _isMouseDown = false;
  // 忽略紧接着的鼠标抬起事件（防止点击购买按钮后直接触发放置）
  _ignoreNextMouseUp = true;

  Building* building = createBuildingForItem(item);
  if (!building) {
    showPopupDialog("提示", "该建筑尚未完成，实现暂不可用");
    auto playerManager = PlayerManager::getInstance();
    if (playerManager) {
      playerManager->addGold(item.costGold);
      playerManager->addElixir(item.costElixir);
    }
    return;
  }

  _placementBuilding = building;
  _placementItem = item;
  _isPlacingBuilding = true;
  _placementPreviewValid = false;
  _placementPreviewRow = 0;
  _placementPreviewCol = 0;
  _placementPreviewAnchor = Vec2::ZERO;

  _mapLayer->addChild(building, 2);
  building->setOpacity(180);
  building->showGlow();

  // 使用当前鼠标位置作为初始位置
  Vec2 initialPos;
  if (_currentMousePos.equals(Vec2::ZERO)) {
    // 如果没有鼠标位置记录（极少情况），使用屏幕中心
    auto director = Director::getInstance();
    auto visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    initialPos = Vec2(origin.x + visibleSize.width / 2.0f,
                      origin.y + visibleSize.height / 2.0f);
  } else {
    initialPos = _currentMousePos;
  }

  // 立即更新一次位置
  updatePlacementPreview(initialPos);

  showPlacementHint("拖动鼠标调整位置，左键确认放置，右键取消");
}

void GameScene::cancelPlacementMode(bool refundResources) {
  if (!_isPlacingBuilding) {
    return;
  }

  if (refundResources) {
    auto playerManager = PlayerManager::getInstance();
    if (playerManager) {
      playerManager->addGold(_placementItem.costGold);
      playerManager->addElixir(_placementItem.costElixir);
    }
  }

  if (_placementBuilding) {
    _placementBuilding->removeFromParent();
    _placementBuilding = nullptr;
  }

  _isPlacingBuilding = false;
  _placementItem = ShopItem();
  showPlacementHint("");
  _isPlacementMouseDown = false;
  _placementDraggingMap = false;
  _placementMouseDownPos = Vec2::ZERO;
  _placementLastMousePos = Vec2::ZERO;
  _placementPreviewValid = false;
  _placementPreviewRow = 0;
  _placementPreviewCol = 0;
  _placementPreviewAnchor = Vec2::ZERO;
}

Building* GameScene::createBuildingForItem(const ShopItem& item) {
  if (item.id == "TownHall") {
    return TownHall::create(item.defaultLevel);
  }

  return PlaceholderBuilding::create(
      item.displayName, item.category, item.placeholderColor, item.defaultLevel,
      item.gridCount, item.anchorRatioX, item.anchorRatioY, item.imageScale);
}

void GameScene::showPlacementHint(const std::string& text) {
  if (!_placementHintLabel) {
    return;
  }

  if (text.empty()) {
    _placementHintLabel->setVisible(false);
    _placementHintLabel->setString("");
  } else {
    _placementHintLabel->setString(text);
    _placementHintLabel->setVisible(true);
  }
}

void GameScene::updatePlacementPreview(const Vec2& worldPos) {
  if (!_placementBuilding) {
    return;
  }

  Vec2 mapPos = _mapLayer->convertToNodeSpace(worldPos);

  int row = 0;
  int col = 0;
  Vec2 nearestPos;
  if (GridUtils::findNearestGrassVertex(mapPos, _p00, row, col, nearestPos)) {
    float deltaX = 0.0f;
    auto configManager = ConfigManager::getInstance();
    if (configManager) {
      deltaX = configManager->getConstantConfig().deltaX;
    }

    _placementBuilding->setPosition(nearestPos);
    if (deltaX > 0.0f) {
      _placementBuilding->setCenterX(
          nearestPos.x + deltaX * _placementBuilding->getGridCount());
    } else {
      _placementBuilding->setCenterX(nearestPos.x);
    }
    _placementBuilding->setCenterY(nearestPos.y);
    _placementBuilding->setRow(row);
    _placementBuilding->setCol(col);

    showPlacementHint(StringUtils::format("放置坐标: (%d, %d)", row, col));
    _placementPreviewValid = true;
    _placementPreviewRow = row;
    _placementPreviewCol = col;
    _placementPreviewAnchor = nearestPos;
  } else {
    // 未吸附时，直接跟随鼠标
    _placementBuilding->setPosition(mapPos);
    // 确保中心点也同步更新，防止逻辑位置偏差
    _placementBuilding->setCenterX(mapPos.x);
    _placementBuilding->setCenterY(mapPos.y);

    showPlacementHint("无法吸附到网格");
    _placementPreviewValid = false;
    _placementPreviewAnchor = mapPos;
  }
}

GameScene::~GameScene() {
  cancelPlacementMode(false);
  if (_buildingManager) {
    delete _buildingManager;
    _buildingManager = nullptr;
  }
}
