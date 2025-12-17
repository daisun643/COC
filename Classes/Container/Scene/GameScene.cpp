#include "GameScene.h"

#include <float.h>

#include "Container/Layer/AttackLayer.h"
#include "Container/Layer/ReplayLayer.h"
#include "Container/Scene/SenceHelper.h"
#include "Game/Building/AllBuildings.h"
#include "Game/Building/BarracksBuilding.h"
#include "Game/Building/BuildingFactory.h"
#include "Game/Building/DefenseBuilding.h"
#include "Game/Building/PlaceholderBuilding.h"
#include "Game/Building/ResourceBuilding.h"
#include "Game/Building/StorageBuilding.h"
#include "Game/Building/TownHall.h"
#include "Game/Building/Wall.h"
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

  _uiLayer->setOnMapEditClickCallback([this]() { this->enterMapEditMode(); });

  // 初始化 GameScene 特有的放置模式相关变量
  _isMapEditMode = false;
  _isPlacementFromInventory = false;
  _mapEditLayer = nullptr;
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
  _autoSaveOnExit = true;

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
  _buildingMenuLayer->setOnUpgradeCallback([this](Building* b) {
    if (!b) return;

    CCLOG("Upgrade clicked for building: %s, level: %d",
          b->getBuildingName().c_str(), b->getLevel());

    // 调用建筑的升级方法，这将触发 Building::upgrade()
    // 从而加载新配置并切换图片
    b->upgrade();

    if (_buildingManager) {
      _buildingManager->saveBuildingMap();
    }

    // 升级成功后，隐藏操作菜单
    _buildingMenuLayer->hideOptions();

    // 显示提示信息
    showPlacementHint("升级成功！");
    if (_placementHintLabel) {
      _placementHintLabel->stopAllActions();
      _placementHintLabel->setVisible(true);
      auto delay = DelayTime::create(1.5f);
      auto clear = CallFunc::create([this]() { showPlacementHint(""); });
      _placementHintLabel->runAction(Sequence::create(delay, clear, nullptr));
    }
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

  // 设置自动保存回调
  // 当 PlayerManager 触发自动保存时，同时保存建筑地图
  auto playerManager = PlayerManager::getInstance();
  if (playerManager) {
    playerManager->setAutoSaveCallback([this]() {
      if (_buildingManager) {
        _buildingManager->saveBuildingMap();
        // 可选：添加日志
        // CCLOG("GameScene: Auto-saved building map.");
      }
    });
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
        _placementBuilding->setCenterX(_placementPreviewAnchor.x);
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

        // [修复] 强制保存一次，确保商店购买的建筑立即写入文件
        // 虽然 registerBuilding
        // 内部也会尝试保存，但为了保险起见，这里显式调用一次
        _buildingManager->saveBuildingMap();
        CCLOG("GameScene: Building placed and saved.");
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
      if (_isMapEditMode) {
        _buildingMenuLayer->showRemoveOption(_selectedBuilding);
      } else {
        _buildingMenuLayer->showBuildingOptions(_selectedBuilding);
      }
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
  if (!configManager) {
    return items;
  }

  std::vector<std::string> buildingNames = configManager->getAllBuildingNames();

  // 定义建筑的显示名称、描述和造价（这些信息目前不在 building.json
  // 中，暂时硬编码映射） 理想情况下，这些信息也应该移入 building.json
  struct BuildingMeta {
    std::string displayName;
    std::string description;
    int costGold;
    int costElixir;
    Color4B placeholderColor;
  };

  std::map<std::string, BuildingMeta> metaMap = {
      {"TownHall",
       {"大本营", "村庄的核心建筑，解锁更多功能", 1000, 0,
        Color4B(139, 69, 19, 255)}},
      {"GoldMine",
       {"金矿", "持续生产金币的基础建筑", 0, 300, Color4B(212, 175, 55, 255)}},
      {"ElixirPump",
       {"圣水采集器", "产出圣水的基础设施", 350, 0,
        Color4B(140, 90, 200, 255)}},
      {"Cannon",
       {"加农炮", "基础防御建筑，守护村庄安全", 800, 200,
        Color4B(120, 120, 120, 255)}},
      {"ArcherTower",
       {"箭塔", "远程防御建筑，可攻击空中和地面目标", 1000, 0,
        Color4B(100, 100, 100, 255)}},
      {"GoldStorage",
       {"储金罐", "储存大量金币", 0, 500, Color4B(255, 215, 0, 255)}},
      {"ElixirBottle",
       {"圣水瓶", "储存大量圣水", 500, 0, Color4B(186, 85, 211, 255)}},
      {"Barracks", {"兵营", "训练军队的地方", 200, 0, Color4B(139, 0, 0, 255)}},
      {"Wall", {"城墙", "阻挡敌人进攻", 50, 0, Color4B(200, 200, 200, 255)}}};

  for (const auto& name : buildingNames) {
    // 跳过没有元数据的建筑（或者使用默认值）
    if (metaMap.find(name) == metaMap.end()) {
      continue;
    }

    auto config = configManager->getBuildingConfig(name);
    const auto& meta = metaMap[name];

    ShopItem item;
    item.id = name;
    item.displayName = meta.displayName;
    item.description = meta.description;
    item.costGold = meta.costGold;
    item.costElixir = meta.costElixir;
    item.defaultLevel = 1;  // 默认建造等级为1

    // 映射类型字符串到枚举
    if (config.type == "TOWN_HALL")
      item.category = BuildingType::TOWN_HALL;
    else if (config.type == "DEFENSE")
      item.category = BuildingType::DEFENSE;
    else if (config.type == "RESOURCE")
      item.category = BuildingType::RESOURCE;
    else if (config.type == "STORAGE")
      item.category = BuildingType::STORAGE;
    else if (config.type == "BARRACKS")
      item.category = BuildingType::BARRACKS;
    else if (config.type == "WALL")
      item.category = BuildingType::WALL;
    else
      continue;  // 未知类型跳过

    item.imagePath = config.image;
    item.placeholderColor = meta.placeholderColor;
    item.gridCount = config.gridCount;
    item.anchorRatioX = config.anchorRatioX;
    item.anchorRatioY = config.anchorRatioY;
    item.imageScale = config.imageScale;

    items.push_back(item);
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

  if (_isPlacementFromInventory) {
    // Return to inventory
    _tempInventory[_placementItem.id]++;
    if (_mapEditLayer) {
      _mapEditLayer->updateInventory(_tempInventory);
    }
    _isPlacementFromInventory = false;
  } else if (refundResources) {
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
  return BuildingFactory::getInstance()->createBuilding(item);
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

    // 如果建筑占用的网格数是奇数（如3x3），其中心应该在网格中心
    // 需要进行偏移修正，使其对齐到网格线
    if (_placementBuilding->getGridCount() % 2 != 0) {
      nearestPos.x += deltaX;
    }

    _placementBuilding->setPosition(nearestPos);
    if (deltaX > 0.0f) {
      _placementBuilding->setCenterX(nearestPos.x);
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
  // 退出场景时保存所有数据
  if (_autoSaveOnExit) {
    if (_buildingManager) {
      _buildingManager->saveBuildingMap();
    }
    auto playerManager = PlayerManager::getInstance();
    if (playerManager) {
      playerManager->saveUserData();
    }
  }
  // 取消放置模式（不退回资源，因为析构时资源已经不需要了）
  cancelPlacementMode(false);
  // 注意：父类 BasicScene 的析构函数会自动调用，不需要显式调用
}

void GameScene::enterMapEditMode() {
  if (_isMapEditMode) return;
  _isMapEditMode = true;
  _isPlacementFromInventory = false;

  // Hide main UI
  if (_uiLayer) _uiLayer->setVisible(false);
  if (_buildingMenuLayer) _buildingMenuLayer->hideOptions();

  // Clear temp inventory
  _tempInventory.clear();

  // Build shop catalog for reference
  _shopCatalog = buildShopCatalog();

  // Create MapEditLayer
  _mapEditLayer = MapEditLayer::createWithItems(_shopCatalog);
  if (_mapEditLayer) {
    _mapEditLayer->setOnRemoveAllCallback(
        [this]() { this->onRemoveAllBuildings(); });
    _mapEditLayer->setOnSaveCallback([this]() { this->exitMapEditMode(true); });
    _mapEditLayer->setOnCancelCallback(
        [this]() { this->exitMapEditMode(false); });
    _mapEditLayer->setOnItemClickCallback([this](const std::string& id) {
      this->onPlaceBuildingFromInventory(id);
    });
    this->addChild(_mapEditLayer, 200);
  }

  // Set remove callback for building menu
  if (_buildingMenuLayer) {
    _buildingMenuLayer->setOnRemoveCallback(
        [this](Building* b) { this->onRemoveBuilding(b); });
  }
}

void GameScene::exitMapEditMode(bool save) {
  if (!_isMapEditMode) return;

  if (!save) {
    // Reload scene to revert changes
    _autoSaveOnExit = false;  // 禁止析构时保存，防止覆盖存档
    Director::getInstance()->replaceScene(GameScene::createScene());
    return;
  }

  // 编辑模式保存时，同时保存建筑和玩家数据
  if (_buildingManager) {
    _buildingManager->saveBuildingMap();
  }
  auto playerManager = PlayerManager::getInstance();
  if (playerManager) {
    playerManager->saveUserData();
  }

  _isMapEditMode = false;
  if (_mapEditLayer) {
    _mapEditLayer->removeFromParent();
    _mapEditLayer = nullptr;
  }
  if (_uiLayer) _uiLayer->setVisible(true);

  // Clear inventory (items left are lost or should be handled)
  _tempInventory.clear();
}

void GameScene::onRemoveAllBuildings() {
  if (!_buildingManager) return;

  // Copy list to avoid iterator invalidation
  auto buildings = _buildingManager->getAllBuildings();
  for (auto b : buildings) {
    onRemoveBuilding(b);
  }
}

void GameScene::onRemoveBuilding(Building* building) {
  if (!building) return;

  std::string id = building->getBuildingName();
  // Check if this ID exists in catalog (to ensure it's a valid shop item)
  bool isValid = false;
  for (const auto& item : _shopCatalog) {
    if (item.id == id) {
      isValid = true;
      break;
    }
  }

  if (isValid) {
    _tempInventory[id]++;
    if (_mapEditLayer) {
      _mapEditLayer->updateInventory(_tempInventory);
    }
  }

  // Remove from manager and map
  if (_buildingManager) {
    _buildingManager->removeBuilding(building);
  }

  if (_selectedBuilding == building) {
    _selectedBuilding = nullptr;
  }

  building->removeFromParent();

  if (_buildingMenuLayer) _buildingMenuLayer->hideOptions();
}

void GameScene::onPlaceBuildingFromInventory(const std::string& id) {
  if (_tempInventory[id] <= 0) return;

  // Find item
  ShopItem targetItem;
  bool found = false;
  for (const auto& item : _shopCatalog) {
    if (item.id == id) {
      targetItem = item;
      found = true;
      break;
    }
  }

  if (!found) return;

  _tempInventory[id]--;
  if (_mapEditLayer) _mapEditLayer->updateInventory(_tempInventory);

  _isPlacementFromInventory = true;
  enterPlacementMode(targetItem);
}
