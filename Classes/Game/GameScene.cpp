#include "GameScene.h"
#include <float.h>

Scene *GameScene::createScene() { return GameScene::create(); }

bool GameScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建地图容器层
  _mapLayer = Layer::create();
  this->addChild(_mapLayer, 0);
  _currentScale = 1.0f;
  _isDragging = false;
  _draggingBuilding = nullptr;
  _gridSize = 44;

  // 创建44x44网格地图背景，使用grass.png进行菱形密铺
  initGrassBackground();

  // 添加标题（标题不随地图移动）
  auto titleLabel = Label::createWithSystemFont("COC - Village", "Arial", 36);
  titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height - 30));
  titleLabel->setColor(Color3B::WHITE);
  this->addChild(titleLabel, 1);

  // 初始化鼠标事件监听器
  initMouseEventListeners();

  // 初始化TownHall
  initTownHall();

  return true;
}

void GameScene::initTownHall() {
  // 创建大本营，放在地图中心位置
  int centerRow = _gridSize / 2;
  int centerCol = _gridSize / 2;
  Vec2 centerPos = gridToScreen(centerRow, centerCol);
  
  _townHall = TownHall::create(1);
  _townHall->setWidth(4);  // 宽度4
  
  // 根据2*deltaX进行等比例缩放
  // 原始图片尺寸是158*120，需要缩放到2*deltaX
  float targetSize = 2.0f * _deltaX;
  float scaleFactor = targetSize / 158.0f;  // 158是原始图片长度
  _townHall->setScale(scaleFactor);
  
  // 设置锚点为左侧中点（与grass一致）
  _townHall->setAnchorPoint(Vec2(0.0f, 0.5f));
  
  // 设置中心位置和坐标编码
  _townHall->setCenterPosition(centerPos.x, centerPos.y, centerRow, centerCol);
  
  _mapLayer->addChild(_townHall, 1);
}

void GameScene::initDefenseBuildings() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建不同类型的防御建筑
  auto cannon1 = DefenseBuilding::create(DefenseType::CANNON, 1);
  addBuilding(cannon1, Vec2(origin.x + 150, origin.y + 200));

  auto cannon2 = DefenseBuilding::create(DefenseType::CANNON, 2);
  addBuilding(cannon2,
              Vec2(origin.x + visibleSize.width - 150, origin.y + 200));

  auto archerTower = DefenseBuilding::create(DefenseType::ARCHER_TOWER, 1);
  addBuilding(archerTower,
              Vec2(origin.x + visibleSize.width / 2, origin.y + 100));

  auto mortar = DefenseBuilding::create(DefenseType::MORTAR, 1);
  addBuilding(mortar, Vec2(origin.x + visibleSize.width / 2,
                           origin.y + visibleSize.height - 150));
}

void GameScene::initResourceBuildings() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建资源建筑
  auto goldMine1 = ResourceBuilding::create(ResourceType::GOLD, 1);
  addBuilding(goldMine1, Vec2(origin.x + 100, origin.y + 400));

  auto goldMine2 = ResourceBuilding::create(ResourceType::GOLD, 2);
  addBuilding(goldMine2, Vec2(origin.x + 250, origin.y + 400));

  auto elixirCollector1 = ResourceBuilding::create(ResourceType::ELIXIR, 1);
  addBuilding(elixirCollector1,
              Vec2(origin.x + visibleSize.width - 100, origin.y + 400));

  auto elixirCollector2 = ResourceBuilding::create(ResourceType::ELIXIR, 1);
  addBuilding(elixirCollector2,
              Vec2(origin.x + visibleSize.width - 250, origin.y + 400));
}

void GameScene::initStorageBuildings() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建储存建筑
  auto goldStorage = StorageBuilding::create(ResourceType::GOLD, 1);
  addBuilding(goldStorage, Vec2(origin.x + 150, origin.y + 300));

  auto elixirStorage = StorageBuilding::create(ResourceType::ELIXIR, 1);
  addBuilding(elixirStorage,
              Vec2(origin.x + visibleSize.width - 150, origin.y + 300));
}

void GameScene::initBarracks() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建兵营
  auto barracks = Barracks::create(1);
  addBuilding(barracks, Vec2(origin.x + visibleSize.width / 2, origin.y + 250));
}

void GameScene::addBuilding(Building *building, const Vec2 &position) {
  if (building) {
    building->setPosition(position);
    this->addChild(building, 1);
  }
}

void GameScene::initGrassBackground() {
  const int GRID_SIZE = 44;  // 44x44的网格
  const float L = 158.0f;    // 图片长度：158像素
  const float W = 120.0f;    // 图片宽度：120像素
  const float H = W;         // 高度等于宽度：120像素
  const std::string GRASS_PATH = "images/backgrond/grass.png";

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 根据用户给定的公式计算四个顶点
  // p[i][j] 表示第i行第j列的图片坐标（图片左侧的中点）
  
  // 确定p[0][0]的位置（最左端，居中显示）
  // 根据公式计算总尺寸：
  // p[43][43].x = p[0][0].x + W/2 * 84
  // p[43][43].y = p[0][0].y
  // p[43][0].y = p[0][0].y - H/2 * 43
  // p[0][43].y = p[0][0].y + H/2 * 43
  
  float totalWidth = (W / 2.0f) * 84 + L;  // 最右端x坐标 + 图片长度
  float minY = -(H / 2.0f) * 43;           // 最上端相对y坐标
  float maxY = (H / 2.0f) * 43;            // 最下端相对y坐标
  float totalHeight = maxY - minY + H;
  
  Vec2 p00(origin.x + (visibleSize.width - totalWidth) / 2,
           origin.y + visibleSize.height / 2);

  // 验证四个顶点（用于调试）
  Vec2 p43_0, p0_43, p43_43;
  p43_0.x = p00.x + (W / 2.0f) * 43;
  p43_0.y = p00.y - (H / 2.0f) * 43;
  
  p0_43.x = p00.x + (W / 2.0f) * 43;
  p0_43.y = p00.y + (H / 2.0f) * 43;
  
  p43_43.x = p00.x + (W / 2.0f) * 84;
  p43_43.y = p00.y;
  _deltaX = (W / 2.0f) + 16.0f;
  _deltaY = (H / 2.0f);
  _p00 = p00;
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
        grassSprite->setContentSize(Size(L, W));
        // 注意：pos是图片左侧的中点，需要设置锚点
        grassSprite->setAnchorPoint(Vec2(0.0f, 0.5f));  // 左侧中点
        grassSprite->setPosition(pos);
        _mapLayer->addChild(grassSprite, 0);
      }
    }
  }
}

void GameScene::initMouseEventListeners() {
  // 创建鼠标事件监听器
  auto mouseListener = EventListenerMouse::create();
  
  // 滚轮缩放事件
  mouseListener->onMouseScroll = [this](Event *event) {
    this->onMouseScroll(event);
  };
  
  // 鼠标按下事件
  mouseListener->onMouseDown = [this](Event *event) {
    this->onMouseDown(event);
  };
  
  // 鼠标移动事件
  mouseListener->onMouseMove = [this](Event *event) {
    this->onMouseMove(event);
  };
  
  // 鼠标抬起事件
  mouseListener->onMouseUp = [this](Event *event) {
    this->onMouseUp(event);
  };
  
  _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void GameScene::onMouseScroll(Event *event) {
  EventMouse *mouseEvent = static_cast<EventMouse *>(event);
  float scrollY = mouseEvent->getScrollY();
  
  // 缩放因子
  const float SCALE_FACTOR = 0.1f;
  const float MIN_SCALE = 0.5f;
  const float MAX_SCALE = 3.0f;
  
  // 计算新的缩放比例
  float deltaScale = scrollY > 0 ? (1.0f + SCALE_FACTOR) : (1.0f - SCALE_FACTOR);
  float newScale = _currentScale * deltaScale;
  
  // 限制缩放范围
  if (newScale < MIN_SCALE) {
    newScale = MIN_SCALE;
  } else if (newScale > MAX_SCALE) {
    newScale = MAX_SCALE;
  }
  
  if (newScale != _currentScale) {
    // 获取鼠标在屏幕坐标系中的位置
    Vec2 mouseScreenPos = mouseEvent->getLocationInView();
    
    // 转换为OpenGL坐标系
    Vec2 mouseGLPos = Director::getInstance()->convertToGL(mouseScreenPos);
    
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

void GameScene::onMouseDown(Event *event) {
  EventMouse *mouseEvent = static_cast<EventMouse *>(event);
  
  // 检查是否按下左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    Vec2 mousePos = mouseEvent->getLocationInView();
    mousePos = Director::getInstance()->convertToGL(mousePos);
    
    // 转换为地图层坐标系
    Vec2 mapPos = _mapLayer->convertToNodeSpace(mousePos);
    
    // 检查是否点击了建筑
    _draggingBuilding = nullptr;
    Vector<Node*> children = _mapLayer->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      Building *building = dynamic_cast<Building*>(*it);
      if (building) {
        Vec2 buildingPos = building->getPosition();
        Rect buildingRect = Rect(buildingPos.x - building->getContentSize().width / 2,
                                 buildingPos.y - building->getContentSize().height / 2,
                                 building->getContentSize().width,
                                 building->getContentSize().height);
        if (buildingRect.containsPoint(mapPos)) {
          _draggingBuilding = building;
          building->_isDragging = true;
          building->_dragOffset = mapPos - buildingPos;
          building->showGlow(); // 显示选中光晕
          break;
        }
      }
    }
    
    if (!_draggingBuilding) {
      // 如果没有点击建筑，则拖动地图
      _isDragging = true;
      _lastMousePos = mousePos;
    }
  }
}

void GameScene::onMouseMove(Event *event) {
  EventMouse *mouseEvent = static_cast<EventMouse *>(event);
  Vec2 currentPos = mouseEvent->getLocationInView();
  currentPos = Director::getInstance()->convertToGL(currentPos);
  
  if (_draggingBuilding) {
    // 拖动建筑
    Vec2 mapPos = _mapLayer->convertToNodeSpace(currentPos);
    Vec2 newPos = mapPos - _draggingBuilding->_dragOffset;
    _draggingBuilding->setPosition(newPos);
    
    // 更新中心坐标（临时）
    _draggingBuilding->setCenterX(newPos.x);
    _draggingBuilding->setCenterY(newPos.y);
  } else if (_isDragging) {
    // 拖动地图（鼠标移动方向与地图移动方向相反，就像拖动物体）
    // 水平分量取相反数，垂直分量保持原样
    Vec2 delta = currentPos - _lastMousePos;
    delta.x = delta.x; // 水平分量取相反数
    delta.y = -delta.y;
    _mapLayer->setPosition(_mapLayer->getPosition() + delta);
    _lastMousePos = currentPos;
  }
}

void GameScene::onMouseUp(Event *event) {
  EventMouse *mouseEvent = static_cast<EventMouse *>(event);
  
  // 检查是否释放左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    if (_draggingBuilding) {
      // 如果正在拖动建筑，进行吸附
      Vec2 mousePos = mouseEvent->getLocationInView();
      mousePos = Director::getInstance()->convertToGL(mousePos);
      
      // 转换为地图层坐标系
      Vec2 mapPos = _mapLayer->convertToNodeSpace(mousePos);
      
      int row, col;
      Vec2 nearestPos;
      if (findNearestGrassVertex(mapPos, row, col, nearestPos)) {
        // 设置新位置
        _draggingBuilding->setCenterPosition(nearestPos.x, nearestPos.y, row, col);
        
        // 检查是否越界
        if (_draggingBuilding->isOutOfBounds(_gridSize)) {
          // 如果越界，恢复到之前的位置（这里可以添加提示）
          // 暂时允许越界，但可以添加视觉提示
        }
      }
      
      _draggingBuilding->hideGlow(); // 隐藏光晕
      _draggingBuilding->_isDragging = false;
      _draggingBuilding = nullptr;
    } else {
      _isDragging = false;
    }
  }
}

Vec2 GameScene::gridToScreen(int row, int col) const {
  Vec2 pos;
  pos.x = _p00.x + (row + col) * _deltaX;
  pos.y = _p00.y + (col - row) * _deltaY;
  return pos;
}

bool GameScene::screenToGrid(const Vec2 &screenPos, int &row, int &col) const {
  // 从屏幕坐标转换为网格坐标
  // 使用逆变换公式
  float dx = screenPos.x - _p00.x;
  float dy = screenPos.y - _p00.y;
  
  // 解方程组：
  // dx = (row + col) * deltaX
  // dy = (col - row) * deltaY
  // 
  // 解得：
  // col = (dx/deltaX + dy/deltaY) / 2
  // row = (dx/deltaX - dy/deltaY) / 2
  
  float col_f = (dx / _deltaX + dy / _deltaY) / 2.0f;
  float row_f = (dx / _deltaX - dy / _deltaY) / 2.0f;
  
  // 四舍五入到最近的整数
  col = (int)(col_f + 0.5f);
  row = (int)(row_f + 0.5f);
  
  // 检查是否在有效范围内
  return (row >= 0 && row < _gridSize && col >= 0 && col < _gridSize);
}

bool GameScene::findNearestGrassVertex(const Vec2 &screenPos, int &row, int &col, Vec2 &nearestPos) const {
  // 先转换为网格坐标
  int tempRow, tempCol;
  if (!screenToGrid(screenPos, tempRow, tempCol)) {
    return false;
  }
  
  // 检查周围的几个点，找到最近的
  float minDist = FLT_MAX;
  int bestRow = tempRow, bestCol = tempCol;
  
  for (int r = tempRow - 1; r <= tempRow + 1; ++r) {
    for (int c = tempCol - 1; c <= tempCol + 1; ++c) {
      if (r >= 0 && r < _gridSize && c >= 0 && c < _gridSize) {
        Vec2 gridPos = gridToScreen(r, c);
        float dist = screenPos.distance(gridPos);
        if (dist < minDist) {
          minDist = dist;
          bestRow = r;
          bestCol = c;
        }
      }
    }
  }
  
  row = bestRow;
  col = bestCol;
  nearestPos = gridToScreen(row, col);
  return true;
}
