#include "GameScene.h"

#include <float.h>

#include "Container/Layer/AttackLayer.h"
#include "Container/Layer/ReplayLayer.h"
#include "Container/Scene/SenceHelper.h"
#include "Game/Building/AllBuildings.h"
#include "Manager/Config/ConfigManager.h"

Scene* GameScene::createScene(const std::string& jsonFilePath) {
  GameScene* scene = new (std::nothrow) GameScene();
  if (scene) {
    if (scene->init(jsonFilePath)) {
      scene->autorelease();
      // 保存到场景管理器（retain 一次以避免被释放）
      auto sceneHelper = SceneHelper::getInstance();
      if (sceneHelper) {
        scene->retain();  // 增加引用计数，防止场景被过早释放
        sceneHelper->setGameScene(scene);
      }
      return scene;
    }
  }
  CC_SAFE_DELETE(scene);
  return nullptr;
}

bool GameScene::init(const std::string& jsonFilePath) {
  // 先调用父类的初始化，使用默认路径
  if (!BasicScene::init(jsonFilePath)) {
    return false;
  }

  // 在 GameScene 中隐藏所有建筑的血条
  if (_buildingManager) {
    const auto& buildings = _buildingManager->getAllBuildings();
    for (auto building : buildings) {
      if (building) {
        building->setHealthBarVisible(false);
      }
    }
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
      replayLayer->setOnReplaySelectedCallback(
          [](const std::string& recordPath) {
            CCLOG("Replay selected: %s", recordPath.c_str());
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
    // 将提示信息位置上移，避免被底部的建筑菜单按钮遮挡
    _placementHintLabel->setPosition(
        Vec2(origin.x + visibleSize.width / 2.0f, origin.y + 250.0f));
    this->addChild(_placementHintLabel, 150);
  }

  // 创建建筑菜单层 (ZOrder 150, above map but below popups)
  _buildingMenuLayer = BuildingMenuLayer::create();
  this->addChild(_buildingMenuLayer, 150);

  // 设置回调
  _buildingMenuLayer->setOnInfoCallback([](Building* b) {
    CCLOG("Info clicked for building type: %d", (int)b->getBuildingType());
    // TODO: Show info dialog
  });
  _buildingMenuLayer->setOnUpgradeCallback([](Building* b) {
    CCLOG("Upgrade clicked for building level: %d", b->getLevel());
    // TODO: Implement upgrade logic
  });
  _buildingMenuLayer->setOnCollectCallback([this](Building* b) {
    CCLOG("Collect clicked");
    auto resourceBuilding = dynamic_cast<ResourceBuilding*>(b);
    if (resourceBuilding) {
      auto playerManager = PlayerManager::getInstance();
      if (playerManager) {
        // 使用新的 collect 方法获取当前暂存的资源
        int amount = resourceBuilding->collect();

        if (amount > 0) {
          if (resourceBuilding->getResourceType() == "Gold") {
            playerManager->addGold(amount);
            showPlacementHint("收集了 " + std::to_string(amount) + " 金币");
          } else if (resourceBuilding->getResourceType() == "Elixir") {
            playerManager->addElixir(amount);
            showPlacementHint("收集了 " + std::to_string(amount) + " 圣水");
          }
        } else {
          showPlacementHint("没有资源可收集");
        }

        // 提示动画
        if (_placementHintLabel) {
          _placementHintLabel->stopAllActions();
          _placementHintLabel->setVisible(true);
          auto delay = DelayTime::create(1.5f);
          auto clear = CallFunc::create([this]() { showPlacementHint(""); });
          _placementHintLabel->runAction(
              Sequence::create(delay, clear, nullptr));
        }
      }
    }
  });

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

  // 检查是否点击了建筑菜单
  if (_buildingMenuLayer &&
      _buildingMenuLayer->isPointInMenu(mouseEvent->getLocationInView())) {
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
    // 拖动时隐藏菜单
    if (_buildingMenuLayer) {
      _buildingMenuLayer->hideOptions();
    }

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
        // 隐藏新建筑的血条（GameScene 中不显示血条）
        _placementBuilding->setHealthBarVisible(false);
      }

      _selectedBuilding = _placementBuilding;
      _placementBuilding = nullptr;
      _isPlacingBuilding = false;
      _placementPreviewValid = false;
      _isPlacementMouseDown = false;
      _placementMouseDownPos = Vec2::ZERO;
      showPlacementHint("建筑已放置");

      // 放置新建筑时不自动显示菜单，需要用户再次点击选中
      _selectedBuilding = nullptr;

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

  // 检查是否正在拖动（在父类处理前检查）
  bool wasDragging = _draggingBuilding != nullptr;

  // 检查是否点击了建筑菜单
  if (_buildingMenuLayer &&
      _buildingMenuLayer->isPointInMenu(mouseEvent->getLocationInView())) {
    return;
  }

  // 调用父类方法处理常规鼠标抬起事件
  BasicScene::onMouseUp(event);

  // 更新建筑菜单显示状态
  if (_buildingMenuLayer) {
    // 如果刚才在拖动，或者没有选中建筑，则隐藏菜单
    if (_selectedBuilding && !wasDragging) {
      _buildingMenuLayer->showBuildingOptions(_selectedBuilding);
    } else {
      _buildingMenuLayer->hideOptions();
    }
  }
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
    auto townHallConfig = configManager->getBuildingConfig("TownHall");
    ShopItem townHall;
    townHall.id = "TownHall";
    townHall.displayName = "大本营";
    townHall.description = "村庄的核心建筑，解锁更多功能";
    townHall.costGold = 1000;
    townHall.costElixir = 0;
    townHall.defaultLevel = townHallConfig.defaultLevel;
    townHall.category = BuildingType::TOWN_HALL;
    townHall.imagePath = townHallConfig.image;
    townHall.placeholderColor = Color4B(139, 69, 19, 255);
    townHall.gridCount = townHallConfig.gridCount;
    townHall.anchorRatioX = townHallConfig.anchorRatioX;
    townHall.anchorRatioY = townHallConfig.anchorRatioY;
    townHall.imageScale = townHallConfig.imageScale;
    items.push_back(townHall);

    auto goldMineConfig = configManager->getBuildingConfig("GoldMine");
    ShopItem goldMine;
    goldMine.id = "GoldMine";
    goldMine.displayName = "金矿";
    goldMine.description = "持续生产金币的基础建筑";
    goldMine.costGold = 300;
    goldMine.costElixir = 0;
    goldMine.defaultLevel = 1;
    goldMine.category = BuildingType::RESOURCE;
    goldMine.imagePath = goldMineConfig.image;
    goldMine.placeholderColor = Color4B(212, 175, 55, 255);
    goldMine.gridCount = goldMineConfig.gridCount;
    goldMine.anchorRatioX = goldMineConfig.anchorRatioX;
    goldMine.anchorRatioY = goldMineConfig.anchorRatioY;
    goldMine.imageScale = goldMineConfig.imageScale;
    items.push_back(goldMine);

    auto elixirPumpConfig = configManager->getBuildingConfig("ElixirPump");
    ShopItem elixirPump;
    elixirPump.id = "ElixirPump";
    elixirPump.displayName = "圣水采集器";
    elixirPump.description = "产出圣水的基础设施";
    elixirPump.costGold = 0;
    elixirPump.costElixir = 350;
    elixirPump.defaultLevel = 1;
    elixirPump.category = BuildingType::RESOURCE;
    elixirPump.imagePath = elixirPumpConfig.image;
    elixirPump.placeholderColor = Color4B(140, 90, 200, 255);
    elixirPump.gridCount = elixirPumpConfig.gridCount;
    elixirPump.anchorRatioX = elixirPumpConfig.anchorRatioX;
    elixirPump.anchorRatioY = elixirPumpConfig.anchorRatioY;
    elixirPump.imageScale = elixirPumpConfig.imageScale;
    items.push_back(elixirPump);

    auto cannonConfig = configManager->getBuildingConfig("Cannon");
    ShopItem cannon;
    cannon.id = "Cannon";
    cannon.displayName = "加农炮";
    cannon.description = "基础防御建筑，守护村庄安全";
    cannon.costGold = 800;
    cannon.costElixir = 200;
    cannon.defaultLevel = 1;
    cannon.category = BuildingType::DEFENSE;
    cannon.imagePath = cannonConfig.image;
    cannon.placeholderColor = Color4B(120, 120, 120, 255);
    cannon.gridCount = cannonConfig.gridCount;
    cannon.anchorRatioX = cannonConfig.anchorRatioX;
    cannon.anchorRatioY = cannonConfig.anchorRatioY;
    cannon.imageScale = cannonConfig.imageScale;
    items.push_back(cannon);

    auto archerTowerConfig = configManager->getBuildingConfig("ArcherTower");
    ShopItem archerTower;
    archerTower.id = "ArcherTower";
    archerTower.displayName = "箭塔";
    archerTower.description = "远程防御建筑，可攻击空中和地面目标";
    archerTower.costGold = 1000;
    archerTower.costElixir = 0;
    archerTower.defaultLevel = 1;
    archerTower.category = BuildingType::DEFENSE;
    archerTower.imagePath = archerTowerConfig.image;
    archerTower.placeholderColor = Color4B(100, 100, 100, 255);
    archerTower.gridCount = archerTowerConfig.gridCount;
    archerTower.anchorRatioX = archerTowerConfig.anchorRatioX;
    archerTower.anchorRatioY = archerTowerConfig.anchorRatioY;
    archerTower.imageScale = archerTowerConfig.imageScale;
    items.push_back(archerTower);

    auto goldStorageConfig = configManager->getBuildingConfig("GoldStorage");
    ShopItem goldStorage;
    goldStorage.id = "GoldStorage";
    goldStorage.displayName = "储金罐";
    goldStorage.description = "储存大量金币";
    goldStorage.costGold = 500;
    goldStorage.costElixir = 0;
    goldStorage.defaultLevel = 1;
    goldStorage.category = BuildingType::STORAGE;
    goldStorage.imagePath = goldStorageConfig.image;
    goldStorage.placeholderColor = Color4B(255, 215, 0, 255);
    goldStorage.gridCount = goldStorageConfig.gridCount;
    goldStorage.anchorRatioX = goldStorageConfig.anchorRatioX;
    goldStorage.anchorRatioY = goldStorageConfig.anchorRatioY;
    goldStorage.imageScale = goldStorageConfig.imageScale;
    items.push_back(goldStorage);

    auto barracksConfig = configManager->getBuildingConfig("Barracks");
    ShopItem barracks;
    barracks.id = "Barracks";
    barracks.displayName = "兵营";
    barracks.description = "训练军队的地方";
    barracks.costGold = 200;
    barracks.costElixir = 0;
    barracks.defaultLevel = 1;
    barracks.category = BuildingType::BARRACKS;
    barracks.imagePath = barracksConfig.image;
    barracks.placeholderColor = Color4B(139, 0, 0, 255);
    barracks.gridCount = barracksConfig.gridCount;
    barracks.anchorRatioX = barracksConfig.anchorRatioX;
    barracks.anchorRatioY = barracksConfig.anchorRatioY;
    barracks.imageScale = barracksConfig.imageScale;
    items.push_back(barracks);
  }

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
  switch (item.category) {
    case BuildingType::TOWN_HALL:
      return TownHall::create(item.defaultLevel);
    case BuildingType::RESOURCE:
      return ResourceBuilding::create(item.defaultLevel, item.id);
    case BuildingType::DEFENSE:
      return DefenseBuilding::create(item.defaultLevel, item.id);
    case BuildingType::STORAGE:
      return StorageBuilding::create(item.defaultLevel, item.id);
    case BuildingType::BARRACKS:
      return BarracksBuilding::create(item.defaultLevel, item.id);
    default:
      return PlaceholderBuilding::create(
          item.displayName, item.category, item.placeholderColor,
          item.defaultLevel, item.gridCount, item.anchorRatioX,
          item.anchorRatioY, item.imageScale);
  }
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

  float row = 0.0f;
  float col = 0.0f;
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

    showPlacementHint(StringUtils::format("放置坐标: (%.1f, %.1f)", row, col));
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
  float r1 = building->getRow();
  float c1 = building->getCol();
  int g1 = building->getGridCount();

  for (const auto& other : buildings) {
    if (other == building) {
      continue;
    }

    float r2 = other->getRow();
    float c2 = other->getCol();
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
