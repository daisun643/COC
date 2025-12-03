#include "GameScene.h"

#include <float.h>

#include "Container/Layer/AttackLayer.h"
#include "Container/Layer/ReplayLayer.h"
#include "Game/Building/PlaceholderBuilding.h"
#include "Manager/Config/ConfigManager.h"

Scene* GameScene::createScene() { return GameScene::create(); }

bool GameScene::init() {
  // 先调用父类的初始化
  if (!BasicScene::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建 UI 层 (ZOrder 设为 100，保证在最上层)
  _uiLayer = MainUILayer::create();
  this->addChild(_uiLayer, 100);

  // 设置 UI 按钮回调
  _uiLayer->setOnShopClickCallback([this]() { this->openShop(); });

  _uiLayer->setOnAttackClickCallback([this]() {
    if (this->getChildByName("AttackLayerUI")) {
      return;
    }
    auto attackLayer = AttackLayer::create();
    if (attackLayer) {
      attackLayer->setName("AttackLayerUI");
      attackLayer->setOnSearchOpponentCallback([]() {
        CCLOG("Search Opponent Clicked!");
        // TODO: Implement search logic
      });
      attackLayer->setOnLevelSelectedCallback([](int levelId) {
        CCLOG("Level %d Selected!", levelId);
        // TODO: Implement level loading
      });
      this->addChild(attackLayer, 200);
    }
  });

  _uiLayer->setOnReplayClickCallback([this]() {
    if (this->getChildByName("ReplayLayerUI")) {
      return;
    }
    auto replayLayer = ReplayLayer::create();
    if (replayLayer) {
      replayLayer->setName("ReplayLayerUI");
      replayLayer->setOnReplaySelectedCallback([](int replayId) {
        CCLOG("Replay %d Selected!", replayId);
        // TODO: Implement replay playback
      });
      this->addChild(replayLayer, 200);
    }
  });

  // 初始化 GameScene 特有的放置模式相关变量
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

  // 创建放置提示标签
  _placementHintLabel = Label::createWithSystemFont("", "Arial", 20);
  if (_placementHintLabel) {
    _placementHintLabel->setColor(Color3B::YELLOW);
    _placementHintLabel->setVisible(false);
    _placementHintLabel->setPosition(
        Vec2(origin.x + visibleSize.width / 2.0f, origin.y + 80.0f));
    this->addChild(_placementHintLabel, 150);
  }

  return true;
}

void GameScene::onMouseScroll(Event* event) {
  if (isPopupOpen()) {
    return;
  }
  // 调用父类方法处理缩放
  BasicScene::onMouseScroll(event);
}

void GameScene::onMouseDown(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  if (isPopupOpen()) {
    return;
  }

  if (_isPlacingBuilding) {
    // 放置模式下：
    // 左键点击：不做处理，等待 MouseUp
    // 右键点击：取消放置
    if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
      cancelPlacementMode(true);
      return;
    }
    return;
  }

  // 调用父类方法处理常规鼠标按下事件
  BasicScene::onMouseDown(event);
}

void GameScene::onMouseMove(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  Vec2 currentPos = mouseEvent->getLocationInView();
  _currentMousePos = currentPos;

  if (isPopupOpen()) {
    return;
  }

  if (_isPlacingBuilding) {
    // 放置模式下禁用地图拖动，仅更新建筑预览位置
    if (_placementBuilding) {
      updatePlacementPreview(currentPos);
    }
    return;
  }

  // 调用父类方法处理常规鼠标移动事件
  BasicScene::onMouseMove(event);

  // 额外处理：如果正在拖动已有建筑，进行重叠检测并更新视觉状态
  if (_draggingBuilding) {
    if (isPlacementValid(_draggingBuilding)) {
      _draggingBuilding->setPlacementValid(true);
    } else {
      _draggingBuilding->setPlacementValid(false);
    }
  }
}

void GameScene::onMouseUp(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  if (isPopupOpen()) {
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
        return;
      }

      // 再次检查重叠（防止快速点击时的竞态条件）
      if (checkBuildingOverlap(_placementBuilding)) {
        return;
      }

      _placementBuilding->setOpacity(255);
      _placementBuilding->hideGlow();
      _placementBuilding->setPlacementValid(true);  // 恢复正常颜色

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
      return;
    }

    return;
  }

  // 调用父类方法处理常规鼠标抬起事件
  BasicScene::onMouseUp(event);
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

bool GameScene::isPopupOpen() const {
  return this->getChildByName("ShopLayerUI") != nullptr ||
         this->getChildByName("AttackLayerUI") != nullptr ||
         this->getChildByName("ReplayLayerUI") != nullptr;
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

    // 检查重叠
    if (checkBuildingOverlap(_placementBuilding)) {
      _placementPreviewValid = false;
      showPlacementHint("位置已被占用");
      _placementBuilding->setPlacementValid(false);
    } else {
      _placementBuilding->setPlacementValid(true);
    }
  } else {
    // 未吸附时，直接跟随鼠标
    _placementBuilding->setPosition(mapPos);
    // 确保中心点也同步更新，防止逻辑位置偏差
    _placementBuilding->setCenterX(mapPos.x);
    _placementBuilding->setCenterY(mapPos.y);

    showPlacementHint("无法吸附到网格");
    _placementPreviewValid = false;
    _placementPreviewAnchor = mapPos;
    _placementBuilding->setPlacementValid(false);
  }
}

bool GameScene::checkBuildingOverlap(const Building* building) const {
  if (!_buildingManager || !building) {
    return false;
  }

  const auto& buildings = _buildingManager->getAllBuildings();
  int r1 = building->getRow();
  int c1 = building->getCol();
  int g1 = building->getGridCount();

  for (const auto& other : buildings) {
    if (other == building) {
      continue;
    }

    int r2 = other->getRow();
    int c2 = other->getCol();
    int g2 = other->getGridCount();

    // 检查两个正方形区域是否重叠
    // 区域中心为 (r, c)，边长为 g
    // 重叠条件：|r1 - r2| < (g1 + g2) / 2.0 && |c1 - c2| < (g1 + g2) / 2.0
    // 注意：这里使用浮点数比较，且阈值必须严格小于
    // 例如 2x2 和 2x2，阈值是 2。距离为 1 时重叠，距离为 2 时不重叠。
    if (std::abs(r1 - r2) < (g1 + g2) / 2.0f &&
        std::abs(c1 - c2) < (g1 + g2) / 2.0f) {
      return true;
    }
  }

  return false;
}

bool GameScene::isPlacementValid(Building* building) const {
  if (!BasicScene::isPlacementValid(building)) {
    return false;
  }
  // 额外检查重叠
  return !checkBuildingOverlap(building);
}

GameScene::~GameScene() {
  // 取消放置模式（不退回资源，因为析构时资源已经不需要了）
  cancelPlacementMode(false);
  // 注意：父类 BasicScene 的析构函数会自动调用，不需要显式调用
}
