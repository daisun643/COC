#include "AttackScene.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "Utils/PathUtils.h"

#ifdef _WIN32
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <direct.h>
#include <io.h>
#endif
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "Container/Scene/SenceHelper.h"
#include "Game/Building/DefenseBuilding.h"
#include "Game/Building/TrapBuilding.h"
#include "Game/Soldier/BasicSoldier.h"
#include "Game/Spell/HealSpell.h"
#include "Game/Spell/LightningSpell.h"
#include "Game/Spell/RageSpell.h"
#include "Manager/Record/RecordManager.h"
#include "Manager/Troop/TroopManager.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "platform/CCFileUtils.h"
#include "ui/CocosGUI.h"

Scene* AttackScene::createScene(const std::string& levelFilePath,
                                const std::string& levelName) {
  if (levelFilePath.empty()) {
  }
  AttackScene* scene = new (std::nothrow) AttackScene();
  if (scene) {
    // 如果指定了关卡文件路径，使用它；否则使用默认路径
    std::string jsonFilePath =
        levelFilePath.empty() ? "Resources/develop/map.json" : levelFilePath;
    scene->_levelName = levelName;         // 保存关卡名称
    scene->_levelFilePath = jsonFilePath;  // 保存关卡文件路径
    if (scene->init(jsonFilePath)) {
      scene->autorelease();
      return scene;
    }
  }
  CC_SAFE_DELETE(scene);
  return nullptr;
}

bool AttackScene::init(const std::string& jsonFilePath) {
  // 防止敌人的地图数据（可能没有金库或金库等级低）覆盖了玩家真实的资源上限
  // 在加载地图前，锁定资源上限更新
  PlayerManager::getInstance()->setAllowUpdateMaxLimit(false);

  // 先调用父类的初始化，传入文件路径
  if (!BasicScene::init(jsonFilePath)) {
    return false;
  }

  // 攻击场景不允许选中或拖动建筑（移动端触摸会走父类的触摸处理）
  this->_allowBuildingDrag = false;

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 初始化变量
  _isPlacingTroop = false;
  _isPlacingSpell = false;
  _isTroopSelected = false;
  _isSpellSelected = false;
  _placementPreview = nullptr;
  _selectedTroopIconBg = nullptr;
  _selectedSpellIconBg = nullptr;
  _selectedTroopIndex = -1;
  _selectedSpellIndex = -1;
  _statusBarLayer = nullptr;
  _troopManager = nullptr;
  _troopIconBgs.clear();
  _spellIconBgs.clear();
  _isAttackStarted = false;
  _isEnd = false;
  _countdownSeconds = ATTACK_DURATION;
  _startAttackButton = nullptr;
  _endAttackButton = nullptr;
  _exitButton = nullptr;
  _countdownLabel = nullptr;
  // _levelName 在 createScene() 中设置，这里不要重置

  // 初始化时将所有陷阱设为不可见（隐形）
  if (_buildingManager) {
    const auto& allBuildings = _buildingManager->getAllBuildings();
    for (Building* building : allBuildings) {
      if (building && building->getBuildingType() == BuildingType::TRAP) {
        TrapBuilding* trap = dynamic_cast<TrapBuilding*>(building);
        if (trap) {
          trap->hide();  // 隐藏陷阱
        }
      }
    }
  }

  // 创建并初始化 TroopManager
  _troopManager = new (std::nothrow) TroopManager();
  if (!_troopManager || !_troopManager->init()) {
    CC_SAFE_DELETE(_troopManager);
    return false;
  }

  _troopItems = _troopManager->getTroopItems();
  _spellItems = _troopManager->getSpellItems();

  // 预加载图标纹理，避免在选择或重建状态栏时出现卡顿
  auto texCache = Director::getInstance()->getTextureCache();
  for (const auto& t : _troopItems) {
    if (!t.panelImage.empty()) {
      texCache->addImage(t.panelImage);
    }
  }
  for (const auto& s : _spellItems) {
    if (!s.panelImage.empty()) {
      texCache->addImage(s.panelImage);
    }
  }

  // 创建并初始化 RecordManager
  _recordManager = new (std::nothrow) RecordManager();
  if (!_recordManager || !_recordManager->init()) {
    CC_SAFE_DELETE(_recordManager);
    return false;
  }

  // 清空图标背景列表
  _troopIconBgs.clear();
  _spellIconBgs.clear();

  // 创建底部状态栏
  createStatusBar();

  // 创建进攻控制按钮
  createAttackButtons();

  // 在移动端添加地图触摸监听，桌面使用鼠标事件处理以避免重复放置
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || \
    (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  auto mapTouchListener = EventListenerTouchOneByOne::create();
  mapTouchListener->setSwallowTouches(false);  // 不吞噬，避免干扰父类触摸状态
  mapTouchListener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
    if (this->_isEnd) return false;
    Vec2 touchPos = touch->getLocation();
    // 忽略状态栏区域的触摸
    if (touchPos.y < 80) return false;
    // 点击兵种后，直接点击地图放置（不进入拖动预览模式）
    if (this->_isTroopSelected || this->_isSpellSelected) {
      return true;  // 我们会处理后续 onTouchEnded
    }
    return false;
  };

  mapTouchListener->onTouchMoved = [this](Touch* touch, Event* event) {
    // 不做跟随预览，保持点击-放置模式的一致性
  };

  mapTouchListener->onTouchEnded = [this](Touch* touch, Event* event) {
    if (this->_isTroopSelected) {
      Vec2 mapPos = this->screenToMapPosition(touch->getLocation());
      if (this->isValidPlacementPosition(mapPos)) {
        this->placeSoldier(mapPos, this->_currentTroopItem);

        // 更新数量标签并决定是否保持选中
        if (_troopManager) {
          _troopItems = _troopManager->getTroopItems();
          for (size_t idx = 0;
               idx < _troopCountLabels.size() && idx < _troopItems.size();
               ++idx) {
            if (_troopCountLabels[idx] && _troopCountLabels[idx]->getParent()) {
              _troopCountLabels[idx]->setString(
                  std::to_string(_troopItems[idx].count));
            }
          }
          int foundIndex = -1;
          for (size_t k = 0; k < _troopItems.size(); ++k) {
            if (_troopItems[k].soldierType == _currentTroopItem.soldierType &&
                _troopItems[k].level == _currentTroopItem.level) {
              foundIndex = static_cast<int>(k);
              break;
            }
          }
          if (foundIndex >= 0 && _troopItems[foundIndex].count > 0) {
            _selectedTroopIndex = foundIndex;
            if (foundIndex < (int)_troopIconBgs.size()) {
              LayerColor* bg = _troopIconBgs[foundIndex].first;
              if (bg && bg->getParent()) {
                bg->setColor(Color3B(255, 255, 0));
                _selectedTroopIconBg = bg;
              }
            }
            _isTroopSelected = true;
          } else {
            cancelPlacementMode();
          }
        } else {
          cancelPlacementMode();
        }
      }
    } else if (this->_isSpellSelected) {
      Vec2 mapPos = this->screenToMapPosition(touch->getLocation());
      if (this->isValidPlacementPosition(mapPos)) {
        this->castSpell(mapPos, this->_currentSpellItem);
        this->cancelPlacementMode();
      }
    }

    // 重置触摸状态
    this->_isTouchDragging = false;
    this->_isMouseDown = false;
    this->_isPinching = false;
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(mapTouchListener,
                                                           _mapLayer);
#endif

  return true;
}

void AttackScene::createStatusBar() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 清空图标背景列表（重新创建时需要清空）
  _troopIconBgs.clear();
  _spellIconBgs.clear();
  _troopCountLabels.clear();
  _spellCountLabels.clear();

  // 创建状态栏层（固定在底部）
  if (!_statusBarLayer) {
    _statusBarLayer = Layer::create();
    _statusBarLayer->setPosition(origin);
    this->addChild(_statusBarLayer, 100);
  } else {
    // 如果已存在，清除所有子节点
    _statusBarLayer->removeAllChildren();
  }

  // 创建状态栏背景
  auto statusBarBg =
      LayerColor::create(Color4B(50, 50, 50, 200), visibleSize.width, 80);
  statusBarBg->setPosition(Vec2(0, 0));
  _statusBarLayer->addChild(statusBarBg);

  // 创建军队图标
  createTroopIcons();

  // 创建法术图标
  createSpellIcons();
}

void AttackScene::createTroopIcons() {
  if (_troopItems.empty()) {
    return;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  float startX = 20.0f;
  float iconSize = 60.0f;
  float iconSpacing = 10.0f;
  float yPos = 40.0f;  // 状态栏中心

  for (size_t i = 0; i < _troopItems.size(); i++) {
    const TroopItem& item = _troopItems[i];
    float xPos = startX + i * (iconSize + iconSpacing);

    // 创建图标背景
    auto iconBg =
        LayerColor::create(Color4B(100, 100, 100, 255), iconSize, iconSize);
    iconBg->setPosition(Vec2(xPos, yPos - iconSize / 2));
    iconBg->setAnchorPoint(Vec2(0, 0));  // 确保锚点在左下角
    _statusBarLayer->addChild(iconBg);

    // 保存图标背景和对应的item，用于鼠标点击检测
    _troopIconBgs.push_back(std::make_pair(iconBg, item));

    // 创建图标精灵
    Sprite* iconSprite = nullptr;
    if (!item.panelImage.empty()) {
      iconSprite = Sprite::create(item.panelImage);
      if (!iconSprite) {
      } else {
      }
    } else {
    }

    if (!iconSprite) {
      // 如果图片加载失败，创建默认图标（彩色矩形）
      iconSprite = Sprite::create();
      auto drawNode = DrawNode::create();
      drawNode->drawSolidRect(Vec2(-iconSize / 2, -iconSize / 2),
                              Vec2(iconSize / 2, iconSize / 2),
                              Color4F(0.5f, 0.5f, 1.0f, 1.0f));
      iconSprite->addChild(drawNode);
      // 设置默认内容大小，避免除零错误
      iconSprite->setContentSize(Size(iconSize, iconSize));
    }

    iconSprite->setPosition(Vec2(xPos + iconSize / 2, yPos));
    // 修复除零错误：检查内容大小是否有效
    Size contentSize = iconSprite->getContentSize();
    if (contentSize.width > 0) {
      iconSprite->setScale(iconSize / contentSize.width);
    } else {
      // 如果内容大小为0，设置固定大小
      iconSprite->setContentSize(Size(iconSize, iconSize));
    }
    _statusBarLayer->addChild(iconSprite);

    // 创建数量标签，保存指针以便后续更新
    auto countLabel =
        Label::createWithSystemFont(std::to_string(item.count), "Arial", 16);
    countLabel->setPosition(
        Vec2(xPos + iconSize - 10, yPos + iconSize / 2 - 10));
    _statusBarLayer->addChild(countLabel);
    _troopCountLabels.push_back(countLabel);

    // 添加触摸事件（点击选中panel，等待在地图上点击布置）
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    // 捕获当前索引 i，避免闭包引用错误
    touchListener->onTouchBegan = [this, item, iconSprite, iconBg, i](
                                      Touch* touch, Event* event) {
      if (this->_isEnd) return false;
      Vec2 touchPos = touch->getLocation();
      // 把触摸点转换到 iconSprite 的本地坐标系，按 contentSize 和 anchorPoint
      // 判定命中
      Vec2 localPos = iconSprite->convertToNodeSpace(touchPos);
      Size sz = iconSprite->getContentSize();
      Vec2 anchor = iconSprite->getAnchorPoint();
      Vec2 origin(-anchor.x * sz.width, -anchor.y * sz.height);
      Rect localRect(origin.x, origin.y, sz.width, sz.height);

      if (localRect.containsPoint(localPos)) {
        if (item.count > 0) {
          // 取消之前选中的士兵panel
          if (_selectedTroopIconBg && _selectedTroopIconBg != iconBg) {
            _selectedTroopIconBg->setColor(Color3B(100, 100, 100));
          }
          // 取消选中的法术panel
          if (_selectedSpellIconBg) {
            // _selectedSpellIconBg->setColor(Color3B(100, 100, 100));
            _selectedSpellIconBg = nullptr;
            _isSpellSelected = false;
            cancelPlacementMode();
          }

          // 选中当前panel（高亮显示）
          _selectedTroopIndex = static_cast<int>(i);
          _selectedTroopIconBg = iconBg;
          _selectedTroopIconBg->setColor(Color3B(255, 255, 0));  // 黄色高亮
          _isTroopSelected = true;
          _currentTroopItem = item;

          // 取消之前的预览
          if (_placementPreview) {
            _placementPreview->removeFromParent();
            _placementPreview = nullptr;
          }

          return true;
        }
      }
      return false;
    };
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
        touchListener, iconSprite);

    // 如果当前索引被标记为已选中，应用高亮（用于重建状态栏时保持选中）
    if (_selectedTroopIndex == static_cast<int>(i)) {
      iconBg->setColor(Color3B(255, 255, 0));
      _selectedTroopIconBg = iconBg;
      _isTroopSelected = true;
      _currentTroopItem = item;
    }
  }
}

void AttackScene::createSpellIcons() {
  if (_spellItems.empty()) {
    return;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  float startX = visibleSize.width - 200.0f;  // 从右侧开始
  float iconSize = 60.0f;
  float iconSpacing = 10.0f;
  float yPos = 40.0f;

  for (size_t i = 0; i < _spellItems.size(); i++) {
    const SpellItem& item = _spellItems[i];
    float xPos = startX + i * (iconSize + iconSpacing);

    // 创建图标背景
    auto iconBg =
        LayerColor::create(Color4B(100, 100, 100, 255), iconSize, iconSize);
    iconBg->setPosition(Vec2(xPos, yPos - iconSize / 2));
    iconBg->setAnchorPoint(Vec2(0, 0));  // 确保锚点在左下角
    _statusBarLayer->addChild(iconBg);

    // 保存图标背景和对应的item，用于鼠标点击检测
    _spellIconBgs.push_back(std::make_pair(iconBg, item));

    // 创建图标精灵
    Sprite* iconSprite = nullptr;
    if (!item.panelImage.empty()) {
      iconSprite = Sprite::create(item.panelImage);
      if (!iconSprite) {
      } else {
      }
    } else {
      CCLOG("Spell item panelImage path is empty for spellType: %s",
            item.spellType.c_str());
    }

    if (!iconSprite) {
      // 如果图片加载失败，创建默认图标（彩色矩形）
      iconSprite = Sprite::create();
      auto drawNode = DrawNode::create();
      drawNode->drawSolidRect(Vec2(-iconSize / 2, -iconSize / 2),
                              Vec2(iconSize / 2, iconSize / 2),
                              Color4F(1.0f, 0.5f, 0.5f, 1.0f));
      iconSprite->addChild(drawNode);
      // 设置默认内容大小，避免除零错误
      iconSprite->setContentSize(Size(iconSize, iconSize));
    }

    iconSprite->setPosition(Vec2(xPos + iconSize / 2, yPos));
    // 修复除零错误：检查内容大小是否有效
    Size contentSize = iconSprite->getContentSize();
    if (contentSize.width > 0) {
      iconSprite->setScale(iconSize / contentSize.width);
    } else {
      // 如果内容大小为0，设置固定大小
      iconSprite->setContentSize(Size(iconSize, iconSize));
    }
    _statusBarLayer->addChild(iconSprite);
    // TODO 标签不明显
    // 创建数量标签，保存指针以便后续更新
    auto countLabel =
        Label::createWithSystemFont(std::to_string(item.count), "Arial", 16);
    countLabel->setPosition(
        Vec2(xPos + iconSize - 10, yPos + iconSize / 2 - 10));
    _statusBarLayer->addChild(countLabel);
    _spellCountLabels.push_back(countLabel);

    // 添加触摸事件（点击选中panel，等待在地图上点击施法）
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    // 捕获索引 i，便于重建时恢复选中
    touchListener->onTouchBegan = [this, item, iconSprite, iconBg, i](
                                      Touch* touch, Event* event) {
      if (this->_isEnd) return false;
      Vec2 touchPos = touch->getLocation();
      // 把触摸点转换到 iconSprite 的本地坐标系，按 contentSize 和 anchorPoint
      // 判定命中
      Vec2 localPos = iconSprite->convertToNodeSpace(touchPos);
      Size sz = iconSprite->getContentSize();
      Vec2 anchor = iconSprite->getAnchorPoint();
      Vec2 origin(-anchor.x * sz.width, -anchor.y * sz.height);
      Rect localRect(origin.x, origin.y, sz.width, sz.height);

      if (localRect.containsPoint(localPos)) {
        if (item.count > 0) {
          // 取消之前选中的法术panel
          if (_selectedSpellIconBg && _selectedSpellIconBg != iconBg) {
            _selectedSpellIconBg->setColor(Color3B(100, 100, 100));
          }
          // 取消选中的士兵panel
          if (_selectedTroopIconBg) {
            _selectedTroopIconBg = nullptr;
            _isTroopSelected = false;
            cancelPlacementMode();
          }

          // 选中当前panel（高亮显示）
          _selectedSpellIndex = static_cast<int>(i);
          _selectedSpellIconBg = iconBg;
          _selectedSpellIconBg->setColor(Color3B(255, 255, 0));  // 黄色高亮
          _isSpellSelected = true;
          _currentSpellItem = item;

          // 取消之前的预览
          if (_placementPreview) {
            _placementPreview->removeFromParent();
            _placementPreview = nullptr;
          }

          return true;
        }
      }
      return false;
    };
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
        touchListener, iconSprite);
    // 重建时恢复选中样式
    if (_selectedSpellIndex == static_cast<int>(i)) {
      iconBg->setColor(Color3B(255, 255, 0));
      _selectedSpellIconBg = iconBg;
      _isSpellSelected = true;
      _currentSpellItem = item;
    }
  }
}

void AttackScene::enterTroopPlacementMode(const TroopItem& item) {
  if (_isEnd) return;
  _isPlacingTroop = true;
  _currentTroopItem = item;

  // 创建预览精灵
  if (!item.panelImage.empty()) {
    _placementPreview = Sprite::create(item.panelImage);
  }

  if (!_placementPreview) {
    // 默认预览
    _placementPreview = Sprite::create();
    auto drawNode = DrawNode::create();
    drawNode->drawCircle(Vec2::ZERO, 20.0f, 0, 20, false, 1.0f, 1.0f,
                         Color4F(0.5f, 0.5f, 1.0f, 0.7f));
    _placementPreview->addChild(drawNode);
  }

  _placementPreview->setOpacity(180);
  _mapLayer->addChild(_placementPreview, 10);
}

void AttackScene::enterSpellPlacementMode(const SpellItem& item) {
  if (_isEnd) return;
  _isPlacingSpell = true;
  _currentSpellItem = item;

  // 创建预览（法术范围圈）
  _placementPreview = Sprite::create();
  auto drawNode = DrawNode::create();
  // 根据法术类型设置不同颜色的预览
  Color4F previewColor;
  if (item.spellType == "Heal") {
    previewColor = Color4F(1.0f, 1.0f, 0.0f, 0.5f);  // 黄色
  } else if (item.spellType == "Rage") {
    previewColor = Color4F(0.5f, 0.0f, 0.5f, 0.5f);  // 紫色
  } else {
    previewColor = Color4F(1.0f, 0.0f, 0.0f, 0.5f);  // 红色（默认）
  }
  drawNode->drawCircle(Vec2::ZERO, 50.0f, 0, 50, false, 2.0f, 2.0f,
                       previewColor);
  _placementPreview->addChild(drawNode);
  _placementPreview->setOpacity(180);
  _mapLayer->addChild(_placementPreview, 10);
}

void AttackScene::cancelPlacementMode() {
  _isPlacingTroop = false;
  _isPlacingSpell = false;
  _isTroopSelected = false;
  _isSpellSelected = false;

  if (_placementPreview) {
    _placementPreview->removeFromParent();
    _placementPreview = nullptr;
  }

  // 取消panel高亮（使用索引安全处理）
  if (_selectedTroopIndex >= 0 &&
      _selectedTroopIndex < (int)_troopIconBgs.size()) {
    LayerColor* bg = _troopIconBgs[_selectedTroopIndex].first;
    if (bg && bg->getParent()) bg->setColor(Color3B(100, 100, 100));
  }
  _selectedTroopIndex = -1;
  _selectedTroopIconBg = nullptr;

  if (_selectedSpellIndex >= 0 &&
      _selectedSpellIndex < (int)_spellIconBgs.size()) {
    LayerColor* bg = _spellIconBgs[_selectedSpellIndex].first;
    if (bg && bg->getParent()) bg->setColor(Color3B(100, 100, 100));
  }
  _selectedSpellIndex = -1;
  _selectedSpellIconBg = nullptr;
}

void AttackScene::placeSoldier(const Vec2& worldPos, const TroopItem& item) {
  if (_isEnd) return;
  // 转换士兵类型字符串为枚举
  SoldierType soldierType = SoldierType::BARBARIAN;
  if (item.soldierType == "barbarian") {
    soldierType = SoldierType::BARBARIAN;
  } else if (item.soldierType == "archer") {
    soldierType = SoldierType::ARCHER;
  } else if (item.soldierType == "giant") {
    soldierType = SoldierType::GIANT;
  } else if (item.soldierType == "bomber") {
    soldierType = SoldierType::BOMBER;
  } else if (item.soldierType == "dragon") {
    soldierType = SoldierType::DRAGON;
  }

  // 创建士兵
  auto soldier = BasicSoldier::create(soldierType, item.level);
  if (soldier) {
    soldier->setPosition(worldPos);
    _mapLayer->addChild(soldier, 5);
    _placedSoldiers.push_back(soldier);

    // 设置建筑查找回调
    soldier->setBuildingFinderCallback([this]() {
      std::vector<Building*> buildings;
      if (_buildingManager) {
        const auto& allBuildings = _buildingManager->getAllBuildings();
        // 只返回存活的建筑
        for (Building* building : allBuildings) {
          if (building && building->isVisible() && building->isAlive()) {
            buildings.push_back(building);
          }
        }
      }
      return buildings;
    });

    // 设置网格状态回调
    soldier->setP00(_p00);
    soldier->setGridStatusCallback([this](int row, int col) {
      if (_buildingManager) {
        return _buildingManager->isWalkable(row, col);
      }
      return true;  // 默认可通行
    });

    // 通过 TroopManager 减少数量
    if (_troopManager) {
      if (!_troopManager->consumeTroop(item.soldierType, item.level)) {
        cancelPlacementMode();
      }

      // 更新本地列表
      _troopItems = _troopManager->getTroopItems();

      // 更新数量标签（避免重建整个状态栏，提高性能）
      for (size_t idx = 0;
           idx < _troopCountLabels.size() && idx < _troopItems.size(); ++idx) {
        if (_troopCountLabels[idx] && _troopCountLabels[idx]->getParent()) {
          _troopCountLabels[idx]->setString(
              std::to_string(_troopItems[idx].count));
        }
      }
    }

    // 如果进攻尚未开始，则自动开始
    if (!_isAttackStarted) {
      startAttack();
    }

    // 记录兵种布置
    if (_recordManager) {
      int timestamp = _recordManager->getCurrentTimestamp();
      _recordManager->recordTroopPlacement(item.soldierType, item.level,
                                           worldPos.x, worldPos.y, timestamp);
    }
  }
}

void AttackScene::castSpell(const Vec2& worldPos, const SpellItem& item) {
  if (_isEnd) return;
  // 转换法术类型字符串为枚举
  SpellType spellType = SpellType::HEAL;
  if (item.spellType == "Heal") {
    spellType = SpellType::HEAL;
  } else if (item.spellType == "Lightning") {
    spellType = SpellType::LIGHTNING;
  } else if (item.spellType == "Rage") {
    spellType = SpellType::RAGE;
  }

  // 创建法术
  BasicSpell* spell = nullptr;
  switch (spellType) {
    case SpellType::HEAL:
      spell = HealSpell::create();
      break;
    case SpellType::LIGHTNING:
      spell = LightningSpell::create();
      break;
    case SpellType::RAGE:
      spell = RageSpell::create();
      break;
  }

  if (spell) {
    // 设置目标查找回调
    spell->setSoldierFinderCallback([this]() { return _placedSoldiers; });
    spell->setBuildingFinderCallback([this]() {
      // 从 BuildingManager 获取建筑列表
      std::vector<Building*> buildings;
      if (_buildingManager) {
        const auto& allBuildings = _buildingManager->getAllBuildings();
        // 只返回存活的建筑
        for (Building* building : allBuildings) {
          if (building && building->isVisible() && building->isAlive()) {
            buildings.push_back(building);
          }
        }
      }
      return buildings;
    });

    // 获取所有建筑（用于cast方法）
    std::vector<Building*> buildings;
    if (_buildingManager) {
      const auto& allBuildings = _buildingManager->getAllBuildings();
      // 只返回存活的建筑
      for (Building* building : allBuildings) {
        if (building && building->isVisible() && building->isAlive()) {
          buildings.push_back(building);
        }
      }
    }

    // 施放法术
    if (spell->cast(worldPos, _placedSoldiers, buildings)) {
      _mapLayer->addChild(spell, 8);
      _activeSpells.push_back(spell);

      // 通过 TroopManager 减少数量
      if (_troopManager) {
        if (!_troopManager->consumeSpell(item.spellType)) {
          cancelPlacementMode();
        }

        // 更新本地列表
        _spellItems = _troopManager->getSpellItems();

        // 更新数量标签（避免重建整个状态栏）
        for (size_t idx = 0;
             idx < _spellCountLabels.size() && idx < _spellItems.size();
             ++idx) {
          if (_spellCountLabels[idx] && _spellCountLabels[idx]->getParent()) {
            _spellCountLabels[idx]->setString(
                std::to_string(_spellItems[idx].count));
          }
        }
      }

      // 如果进攻尚未开始，则自动开始
      if (!_isAttackStarted) {
        startAttack();
      }

      // 记录法术布置
      if (_recordManager) {
        int timestamp = _recordManager->getCurrentTimestamp();
        _recordManager->recordSpellPlacement(item.spellType, worldPos.x,
                                             worldPos.y, timestamp);
      }

    } else {
      CC_SAFE_DELETE(spell);
    }
  }
}

Vec2 AttackScene::screenToMapPosition(const Vec2& screenPos) const {
  // 将屏幕坐标转换为地图层坐标
  return _mapLayer->convertToNodeSpace(screenPos);
}

bool AttackScene::isValidPlacementPosition(const Vec2& mapPos) const {
  // TODO
  // 检查位置是否在地图范围内
  // 这里可以添加更复杂的检查逻辑
  return true;
}

void AttackScene::onMouseDown(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);

  // 检查是否按下左键
  if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
    Vec2 mousePos = mouseEvent->getLocationInView();
    _mouseDownPos = mousePos;  // 记录按下位置

    // 检查是否点击在状态栏上
    auto visibleSize = Director::getInstance()->getVisibleSize();
    if (mousePos.y < 80) {  // 状态栏高度
      // 检查是否点击了士兵panel
      for (auto& pair : _troopIconBgs) {
        LayerColor* iconBg = pair.first;
        const TroopItem& item = pair.second;
        Rect rect = iconBg->getBoundingBox();
        if (rect.containsPoint(mousePos)) {
          if (item.count > 0) {
            // 取消之前选中的士兵panel
            if (_selectedTroopIconBg && _selectedTroopIconBg != iconBg) {
              _selectedTroopIconBg->setColor(Color3B(100, 100, 100));
            }
            // 取消选中的法术panel
            if (_selectedSpellIconBg) {
              // _selectedSpellIconBg->setColor(Color3B(100, 100, 100));
              _selectedSpellIconBg = nullptr;
              _isSpellSelected = false;
              cancelPlacementMode();
            }

            // 选中当前panel（高亮显示）
            _selectedTroopIconBg = iconBg;
            _selectedTroopIconBg->setColor(Color3B(255, 255, 0));  // 黄色高亮
            _isTroopSelected = true;
            _currentTroopItem = item;

            // 取消之前的预览
            if (_placementPreview) {
              _placementPreview->removeFromParent();
              _placementPreview = nullptr;
            }
            return;
          }
        }
      }

      // 检查是否点击了法术panel
      for (auto& pair : _spellIconBgs) {
        LayerColor* iconBg = pair.first;
        const SpellItem& item = pair.second;
        Rect rect = iconBg->getBoundingBox();
        if (rect.containsPoint(mousePos)) {
          if (item.count > 0) {
            // 取消之前选中的法术panel
            if (_selectedSpellIconBg && _selectedSpellIconBg != iconBg) {
              _selectedSpellIconBg->setColor(Color3B(100, 100, 100));
            }
            // 取消选中的士兵panel
            if (_selectedTroopIconBg) {
              // _selectedTroopIconBg->setColor(Color3B(100, 100, 100));
              _selectedTroopIconBg = nullptr;
              _isTroopSelected = false;
              cancelPlacementMode();
            }

            // 选中当前panel（高亮显示）
            _selectedSpellIconBg = iconBg;
            _selectedSpellIconBg->setColor(Color3B(255, 255, 0));  // 黄色高亮
            _isSpellSelected = true;
            _currentSpellItem = item;

            // 取消之前的预览
            if (_placementPreview) {
              _placementPreview->removeFromParent();
              _placementPreview = nullptr;
            }
            return;
          }
        }
      }

      // 点击状态栏但没有点击panel，取消选中
      cancelPlacementMode();
      return;
    }

    // 如果选中了士兵或法术，记录鼠标按下位置，等待鼠标抬起时执行操作
    if (_isTroopSelected || _isSpellSelected) {
      // 不在这里执行操作，等待 onMouseUp
      _isMouseDown = true;
      return;
    }

    // 如果没有选中任何panel，允许拖动地图
    // 取消任何选中的建筑
    if (_selectedBuilding) {
      _selectedBuilding->hideGlow();
      _selectedBuilding = nullptr;
    }

    _isDragging = true;
    _lastMousePos = mousePos;
    _isMouseDown = true;  // 标记鼠标已按下
  }
}

void AttackScene::onMouseMove(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  Vec2 mousePos = mouseEvent->getLocationInView();

  // 如果选中了士兵或法术，等待鼠标抬起以完成放置（不进入拖动预览）
  if (_isTroopSelected || _isSpellSelected) {
    return;
  }

  if (_isPlacingTroop || _isPlacingSpell) {
    // 更新预览位置
    if (_placementPreview) {
      Vec2 mapPos = screenToMapPosition(mousePos);
      _placementPreview->setPosition(mapPos);
    }
    return;
  }

  // 禁止建筑拖动，只允许地图拖动
  if (_isMouseDown && _isDragging) {
    // 拖动地图
    Vec2 delta = mousePos - _lastMousePos;
    Vec2 currentPos = _mapLayer->getPosition();
    _mapLayer->setPosition(currentPos + delta);
    _lastMousePos = mousePos;
  }
}

void AttackScene::onMouseUp(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  Vec2 mousePos = mouseEvent->getLocationInView();

  // 检查是否点击在状态栏上
  auto visibleSize = Director::getInstance()->getVisibleSize();
  if (mousePos.y < 80) {  // 状态栏高度
    // 点击状态栏，不执行任何操作（已经在onMouseDown中处理取消选中）
    if (_isMouseDown && _isDragging) {
      _isDragging = false;
      _isMouseDown = false;
    }
    return;
  }

  // 如果选中了士兵，在地图上点击时放置
  if (_isTroopSelected) {
    Vec2 mapPos = screenToMapPosition(mousePos);
    if (isValidPlacementPosition(mapPos)) {
      placeSoldier(mapPos, _currentTroopItem);

      // 更新本地列表并刷新数量标签；保持选中以支持连续放置
      if (_troopManager) {
        _troopItems = _troopManager->getTroopItems();
        for (size_t idx = 0;
             idx < _troopCountLabels.size() && idx < _troopItems.size();
             ++idx) {
          if (_troopCountLabels[idx] && _troopCountLabels[idx]->getParent()) {
            _troopCountLabels[idx]->setString(
                std::to_string(_troopItems[idx].count));
          }
        }

        // 查找当前选中兵种的新索引并决定是否保持选中
        int foundIndex = -1;
        for (size_t k = 0; k < _troopItems.size(); ++k) {
          if (_troopItems[k].soldierType == _currentTroopItem.soldierType &&
              _troopItems[k].level == _currentTroopItem.level) {
            foundIndex = static_cast<int>(k);
            break;
          }
        }
        if (foundIndex >= 0 && _troopItems[foundIndex].count > 0) {
          _selectedTroopIndex = foundIndex;
          if (foundIndex < (int)_troopIconBgs.size()) {
            LayerColor* bg = _troopIconBgs[foundIndex].first;
            if (bg && bg->getParent()) {
              bg->setColor(Color3B(255, 255, 0));
              _selectedTroopIconBg = bg;
            }
          }
          _isTroopSelected = true;
        } else {
          // 数量耗尽或不存在，取消选中
          cancelPlacementMode();
        }
      } else {
        cancelPlacementMode();
      }
    }
    _isMouseDown = false;
    return;
  }

  // 如果选中了法术，在地图上点击时施法
  if (_isSpellSelected) {
    Vec2 mapPos = screenToMapPosition(mousePos);
    if (isValidPlacementPosition(mapPos)) {
      CCLOG("Casting spell at map position: (%.1f, %.1f)", mapPos.x, mapPos.y);
      castSpell(mapPos, _currentSpellItem);
      // 施法后取消选中
      cancelPlacementMode();
    }
    _isMouseDown = false;
    return;
  }

  // 禁止建筑拖动，只处理地图拖动结束
  if (_isMouseDown && _isDragging) {
    _isDragging = false;
    _isMouseDown = false;
  }
}

// 辅助函数：创建橙色圆角背景
static DrawNode* createOrangeRoundedBackground(const Size& size,
                                               float radius = 8.0f) {
  auto drawNode = DrawNode::create();
  // 橙色背景 (255, 165, 0)
  Color4F orangeColor(255.0f / 255.0f, 165.0f / 255.0f, 0.0f / 255.0f, 1.0f);

  float width = size.width;
  float height = size.height;

  // 绘制圆角矩形（通过绘制多个部分来模拟圆角）
  // 绘制中心矩形
  drawNode->drawSolidRect(Vec2(radius, radius),
                          Vec2(width - radius, height - radius), orangeColor);

  // 绘制四个圆角
  drawNode->drawSolidCircle(Vec2(radius, radius), radius, 0, 20, orangeColor);
  drawNode->drawSolidCircle(Vec2(width - radius, radius), radius, 0, 20,
                            orangeColor);
  drawNode->drawSolidCircle(Vec2(radius, height - radius), radius, 0, 20,
                            orangeColor);
  drawNode->drawSolidCircle(Vec2(width - radius, height - radius), radius, 0,
                            20, orangeColor);

  // 绘制连接圆角的矩形
  drawNode->drawSolidRect(Vec2(radius, 0), Vec2(width - radius, height),
                          orangeColor);
  drawNode->drawSolidRect(Vec2(0, radius), Vec2(width, height - radius),
                          orangeColor);

  return drawNode;
}

void AttackScene::createAttackButtons() {
  // 如果按钮已存在，不重复创建
  if (_startAttackButton && _endAttackButton && _countdownLabel) {
    return;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 按钮尺寸和间距
  const float buttonWidth = 120.0f;
  const float buttonHeight = 40.0f;
  const float buttonSpacing = 10.0f;
  const float margin = 20.0f;
  const float radius = 8.0f;

  // 创建开始进攻按钮（右上角，垂直排列）
  if (!_startAttackButton) {
    Vec2 buttonPos(origin.x + visibleSize.width - margin - buttonWidth / 2,
                   origin.y + visibleSize.height - margin - buttonHeight / 2);

    // 创建橙色圆角背景
    auto bgDrawNode =
        createOrangeRoundedBackground(Size(buttonWidth, buttonHeight), radius);
    bgDrawNode->setPosition(
        Vec2(buttonPos.x - buttonWidth / 2, buttonPos.y - buttonHeight / 2));
    this->addChild(bgDrawNode, 199);

    _startAttackButton = ui::Button::create();
    _startAttackButton->setTitleText("开始进攻");
    _startAttackButton->setTitleFontSize(20);
    _startAttackButton->setTitleColor(Color3B::WHITE);
    _startAttackButton->setContentSize(Size(buttonWidth, buttonHeight));
    _startAttackButton->setPosition(buttonPos);
    _startAttackButton->addClickEventListener([this](Ref* sender) {
      if (_isEnd) return;
      this->startAttack();
    });
    this->addChild(_startAttackButton, 200);
  }

  // 创建结束进攻按钮（在开始按钮下方）
  if (!_endAttackButton) {
    Vec2 buttonPos(origin.x + visibleSize.width - margin - buttonWidth / 2,
                   origin.y + visibleSize.height - margin - buttonHeight / 2 -
                       buttonHeight - buttonSpacing);

    // 创建橙色圆角背景
    auto bgDrawNode =
        createOrangeRoundedBackground(Size(buttonWidth, buttonHeight), radius);
    bgDrawNode->setPosition(
        Vec2(buttonPos.x - buttonWidth / 2, buttonPos.y - buttonHeight / 2));
    this->addChild(bgDrawNode, 199);

    _endAttackButton = ui::Button::create();
    _endAttackButton->setTitleText("结束进攻");
    _endAttackButton->setTitleFontSize(20);
    _endAttackButton->setTitleColor(Color3B::WHITE);
    _endAttackButton->setContentSize(Size(buttonWidth, buttonHeight));
    _endAttackButton->setPosition(buttonPos);
    _endAttackButton->setEnabled(false);
    _endAttackButton->setBright(false);
    _endAttackButton->addClickEventListener(
        [this](Ref* sender) { this->endAttack(); });
    this->addChild(_endAttackButton, 200);
  }

  // 创建退出按钮（左上角）
  if (!_exitButton) {
    Vec2 buttonPos(origin.x + margin + buttonWidth / 2,
                   origin.y + visibleSize.height - margin - buttonHeight / 2);

    // 创建橙色圆角背景
    auto bgDrawNode =
        createOrangeRoundedBackground(Size(buttonWidth, buttonHeight), radius);
    bgDrawNode->setPosition(
        Vec2(buttonPos.x - buttonWidth / 2, buttonPos.y - buttonHeight / 2));
    this->addChild(bgDrawNode, 199);

    _exitButton = ui::Button::create();
    _exitButton->setTitleText("退出");
    _exitButton->setTitleFontSize(20);
    _exitButton->setTitleColor(Color3B::WHITE);
    _exitButton->setContentSize(Size(buttonWidth, buttonHeight));
    _exitButton->setPosition(buttonPos);
    _exitButton->addClickEventListener(
        [this](Ref* sender) { this->exitScene(); });
    this->addChild(_exitButton, 200);
  }

  // 创建倒计时标签（右上角，在结束按钮下方）
  if (!_countdownLabel) {
    _countdownLabel = Label::createWithSystemFont("", "Arial", 24);
    _countdownLabel->setColor(Color3B::YELLOW);
    _countdownLabel->setPosition(
        Vec2(origin.x + visibleSize.width - margin - buttonWidth / 2,
             origin.y + visibleSize.height - margin - buttonHeight / 2 -
                 (buttonHeight + buttonSpacing) * 2));
    _countdownLabel->setString(formatTime(ATTACK_DURATION));
    this->addChild(_countdownLabel, 200);
  }
}

void AttackScene::startAttack() {
  if (_isAttackStarted) {
    return;
  }

  _isAttackStarted = true;
  _countdownSeconds = ATTACK_DURATION;

  // 更新按钮状态
  if (_startAttackButton) {
    _startAttackButton->setEnabled(false);
    _startAttackButton->setBright(false);
  }
  if (_endAttackButton) {
    _endAttackButton->setEnabled(true);
    _endAttackButton->setBright(true);
  }

  // 开始记录
  if (_recordManager) {
    _recordManager->startAttack();
  }

  // 启动倒计时更新
  this->schedule([this](float dt) { this->updateCountdown(dt); }, 1.0f,
                 "updateCountdown");

  // 启动防御建筑攻击更新（每帧更新，使用 0.0f 表示每帧调用）
  this->schedule([this](float dt) { this->updateDefenseBuildings(dt); }, 0.0f,
                 "updateDefenseBuildings");

  // 启动陷阱检测更新 (每 0.1 秒检查一次)
  this->schedule([this](float dt) { this->updateTraps(dt); }, 0.1f,
                 "updateTraps");

  CCLOG("Attack started, countdown: %d seconds", _countdownSeconds);
}

void AttackScene::endAttack() {
  if (!_isAttackStarted) {
    return;
  }
  if (_isClansWar) {
    _buildingManager->updateClansWar(_clans_id, _map_id);
  }
  _isEnd = true;
  // 停止倒计时
  this->unschedule("updateCountdown");

  // 停止防御建筑攻击更新
  this->unschedule("updateDefenseBuildings");

  // 停止陷阱检测
  this->unschedule("updateTraps");

  // 保存记录
  if (_recordManager) {
    // 使用关卡名称构建记录文件路径
    std::string recordFileName = _levelName.empty() ? "default" : _levelName;
    // 清理文件名，移除不允许的字符
    std::string cleanName = recordFileName;
    for (char& c : cleanName) {
      if (c == ' ' || c == '/' || c == '\\') {
        c = '_';
      }
    }
    // 获取当前时间戳（格式：YYYYMMDD_HHMMSS）
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream timeStream;
    timeStream << std::setfill('0') << std::setw(4)
               << (1900 + localTime->tm_year) << std::setw(2)
               << (localTime->tm_mon + 1) << std::setw(2) << localTime->tm_mday
               << "_" << std::setw(2) << localTime->tm_hour << std::setw(2)
               << localTime->tm_min << std::setw(2) << localTime->tm_sec;
    std::string timeStr = timeStream.str();

    // recordPath需要加上时间
    std::string recordPath = "record/" + cleanName + "_" + timeStr + ".json";
    // 保存记录文件
    if (_recordManager->endAttackAndSave(recordPath)) {
      CCLOG("AttackScene: Record saved to %s", recordPath.c_str());

      // 更新 record/summary.json
      updateRecordSummary(cleanName, recordPath, timeStr);
    } else {
      CCLOG("AttackScene: Failed to save record to %s", recordPath.c_str());
    }
  }

  // 重置状态
  _isAttackStarted = false;
  _countdownSeconds = ATTACK_DURATION;

  // [修复] 停止所有士兵和法术的行动
  for (auto soldier : _placedSoldiers) {
    if (soldier) {
      // 停止更新，使其冻结在原地
      soldier->unscheduleUpdate();
      // 可选：播放待机动画或移除
    }
  }

  // 停止所有法术效果
  for (auto spell : _activeSpells) {
    if (spell) {
      spell->unscheduleUpdate();
      spell->removeFromParent();  // 法术通常是瞬时或持续效果，结束后应移除
    }
  }
  _activeSpells.clear();

  // 更新按钮状态
  if (_startAttackButton) {
    _startAttackButton->setEnabled(true);
    _startAttackButton->setBright(true);
  }
  if (_endAttackButton) {
    _endAttackButton->setEnabled(false);
    _endAttackButton->setBright(false);
  }
  if (_countdownLabel) {
    _countdownLabel->setString(formatTime(ATTACK_DURATION));
  }
  showResult();
  CCLOG("Attack ended, records saved");
}

void AttackScene::updateCountdown(float dt) {
  if (!_isAttackStarted) {
    return;
  }

  _countdownSeconds--;
  if (_countdownSeconds < 0) {
    _countdownSeconds = 0;
  }

  // 更新倒计时标签
  if (_countdownLabel) {
    _countdownLabel->setString(formatTime(_countdownSeconds));
    if (_countdownSeconds <= 10) {
      _countdownLabel->setColor(Color3B::RED);
    } else {
      _countdownLabel->setColor(Color3B::YELLOW);
    }
  }

  // 倒计时结束，自动结束进攻
  if (_countdownSeconds <= 0) {
    endAttack();
  }
}

std::string AttackScene::formatTime(int seconds) const {
  int minutes = seconds / 60;
  int secs = seconds % 60;
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
  return std::string(buffer);
}

void AttackScene::updateDefenseBuildings(float delta) {
  // 如果攻击未开始，不更新防御建筑
  if (!_isAttackStarted) {
    return;
  }

  // 如果没有建筑管理器或没有士兵，直接返回
  if (!_buildingManager || _placedSoldiers.empty()) {
    return;
  }

  // 获取所有建筑
  const auto& allBuildings = _buildingManager->getAllBuildings();

  // 遍历所有建筑，找到防御建筑并更新攻击
  for (Building* building : allBuildings) {
    if (!building || !building->isVisible() || !building->isAlive()) {
      continue;
    }

    // 检查是否为防御建筑
    if (building->getBuildingType() != BuildingType::DEFENSE) {
      continue;
    }

    // 转换为防御建筑
    DefenseBuilding* defenseBuilding = dynamic_cast<DefenseBuilding*>(building);
    if (!defenseBuilding) {
      continue;
    }

    // 根据建筑名称决定攻击类别
    // 默认攻击所有类别，可以根据需要扩展
    std::vector<SoldierCategory> targetCategories = {SoldierCategory::LAND,
                                                     SoldierCategory::AIR};

    // 可以根据建筑名称设置不同的攻击类别
    // 例如：防空火箭只攻击空军，加农炮只攻击陆军等
    // 这里先实现攻击所有类别，后续可以根据配置扩展
    // 如果 targetCategories 为空，attackSoldiers 会攻击所有类别

    // 调用防御建筑的攻击方法
    defenseBuilding->attackSoldiers(_placedSoldiers, targetCategories, delta);
  }
}

// 陷阱检测更新函数
void AttackScene::updateTraps(float delta) {
  if (!_isAttackStarted || !_buildingManager || _placedSoldiers.empty()) {
    return;
  }

  const auto& allBuildings = _buildingManager->getAllBuildings();

  // 遍历所有建筑
  for (Building* building : allBuildings) {
    // 筛选出存活的陷阱建筑
    if (building && building->getBuildingType() == BuildingType::TRAP &&
        building->isAlive()) {
      TrapBuilding* trap = dynamic_cast<TrapBuilding*>(building);
      // 只有处于布防状态（未爆炸）的陷阱才需要检查
      if (trap && trap->getIsArmed()) {
        // 如果检测到触发，trap 内部会处理伤害、显形和特效
        trap->checkTrigger(_placedSoldiers);
      }
    }
  }
}

AttackScene::~AttackScene() {
  cancelPlacementMode();

  // 停止倒计时
  if (_isAttackStarted) {
    this->unschedule("updateCountdown");
    this->unschedule("updateDefenseBuildings");
  }

  // 清理士兵和法术
  // for (auto soldier : _placedSoldiers) {
  //   if (soldier) {
  //     soldier->removeFromParent();
  //   }
  // }
  // _placedSoldiers.clear();

  // for (auto spell : _activeSpells) {
  //   if (spell) {
  //     spell->removeFromParent();
  //   }
  // }
  // _activeSpells.clear();

  // 清理管理器
  CC_SAFE_DELETE(_troopManager);
  CC_SAFE_DELETE(_recordManager);
}

void AttackScene::updateRecordSummary(const std::string& recordName,
                                      const std::string& recordPath,
                                      const std::string& timeStr) {
  FileUtils* fileUtils = FileUtils::getInstance();
  std::string summaryPath = "record/summary.json";
  rapidjson::Document doc;
  bool fileExists = false;

  // 使用 PathUtils 获取真实路径 (用于读取)
  std::string fullPath = PathUtils::getRealFilePath(summaryPath, false);

  // 尝试读取现有文件（如果存在）
  if (!fullPath.empty() && fileUtils->isFileExist(fullPath)) {
    std::string content = fileUtils->getStringFromFile(fullPath);
    if (!content.empty()) {
      doc.Parse(content.c_str());
      if (!doc.HasParseError() && doc.IsObject()) {
        fileExists = true;
      }
    }
  }

  // 如果文件不存在或解析失败，创建新文档
  if (!fileExists) {
    doc.SetObject();
  }

  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  // 获取或创建 records 数组
  rapidjson::Value* recordsArray = nullptr;
  if (doc.HasMember("records") && doc["records"].IsArray()) {
    recordsArray = &doc["records"];
  } else {
    // 如果不存在，创建一个新的数组并添加到文档中
    // 注意：如果存在但不是数组，这里没有处理移除，直接覆盖可能会有问题，
    // 但通常 summary.json 格式是固定的。为了安全，可以先移除。
    if (doc.HasMember("records")) {
      doc.RemoveMember("records");
    }
    rapidjson::Value newArray(rapidjson::kArrayType);
    doc.AddMember("records", newArray, allocator);
    recordsArray = &doc["records"];
  }

  // 添加新记录（每次进攻都创建新记录，不更新旧记录）
  rapidjson::Value recordObj(rapidjson::kObjectType);

  // name: 对手名称（关卡名称）
  rapidjson::Value nameValue;
  nameValue.SetString(recordName.c_str(), allocator);
  recordObj.AddMember("name", nameValue, allocator);

  // mapPath: 地图 json 路径
  rapidjson::Value mapPathValue;
  mapPathValue.SetString(_levelFilePath.c_str(), allocator);
  recordObj.AddMember("mapPath", mapPathValue, allocator);

  // recordPath: 布兵 json 路径
  rapidjson::Value recordPathValue;
  recordPathValue.SetString(recordPath.c_str(), allocator);
  recordObj.AddMember("recordPath", recordPathValue, allocator);

  // time: 时间
  rapidjson::Value timeValue;
  timeValue.SetString(timeStr.c_str(), allocator);
  recordObj.AddMember("time", timeValue, allocator);

  // win(bool): 是否获胜
  recordObj.AddMember("win", _buildingManager->getWin(), allocator);

  // stars(int): 取得星星数
  recordObj.AddMember("stars", _buildingManager->getStars(), allocator);

  // ratio(float): 摧毁比例
  recordObj.AddMember("ratio", _buildingManager->getRatio(), allocator);

  // 将新记录添加到数组中
  recordsArray->PushBack(recordObj, allocator);

  // 将 JSON 转换为字符串
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  std::string jsonString = buffer.GetString();

  // 使用 PathUtils 获取真实写入路径
  std::string writePath = PathUtils::getRealFilePath(summaryPath, true);

  // 确保目录存在
  PathUtils::ensureDirectoryExists(writePath);

  // 写入文件
  std::ofstream outFile(writePath, std::ios::out | std::ios::trunc);
  if (outFile.is_open()) {
    outFile << jsonString;
    outFile.close();
    CCLOG("AttackScene: Updated record summary at %s", writePath.c_str());
  } else {
    CCLOG("AttackScene: Failed to write record summary to %s",
          writePath.c_str());
  }
}

void AttackScene::exitScene() {
  // 如果正在进攻，先结束进攻
  if (_isAttackStarted) {
    endAttack();
  }
  // 退出时恢复允许更新
  PlayerManager::getInstance()->setAllowUpdateMaxLimit(true);

  // 从场景管理器获取原来的 GameScene
  auto sceneHelper = SceneHelper::getInstance();
  if (sceneHelper) {
    auto gameScene = sceneHelper->getGameScene();
    if (gameScene) {
      // 返回到原来的 GameScene
      auto director = Director::getInstance();
      director->replaceScene(gameScene);
      return;
    }
  }
  CCLOG("AttackScene: exitScene failed, no game scene found");
  // 如果获取失败，则直接退出
  auto director = Director::getInstance();
  director->end();
}

void AttackScene::showResult() {
  int stars = 0;
  float ratio = 0.0f;
  bool win = false;
  if (!_buildingManager->getBuildingResult(stars, ratio, win)) {
    return;
  }

  std::string text =
      "Attack Result:" + (win ? std::string("Victory!") : std::string("Lose")) +
      "\n"
      "Stars: " +
      std::to_string(stars) +
      "\n"
      "Destroy: " +
      std::to_string((int)(ratio * 100)) + "%";

  auto label = Label::createWithSystemFont(text, "Arial", 28);

  label->setPosition(Director::getInstance()->getVisibleSize() / 2);
  this->addChild(label, 100);
}

void AttackScene::setClansWarInfo(const std::string& clans_id,
                                  const std::string& map_id) {
  _clans_id = clans_id;
  _map_id = map_id;
  _isClansWar = true;
}