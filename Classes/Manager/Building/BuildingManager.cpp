#include "BuildingManager.h"

#include <io.h>  // for _access
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

// 包含所有具体的建筑头文件
#include "Game/Building/BarracksBuilding.h"
#include "Game/Building/DefenseBuilding.h"
#include "Game/Building/ResourceBuilding.h"
#include "Game/Building/StorageBuilding.h"
#include "Game/Building/TownHall.h"
#include "Game/Building/Wall.h"
#include "Game/Building/TrapBuilding.h"
#include "Manager/Config/ConfigManager.h"
#include "Manager/PlayerManager.h"
#include "Utils/API/Clans/ClansWar.h"
#include "Utils/PathUtils.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

BuildingManager::BuildingManager(const std::string& jsonFilePath,
                                 const Vec2& p00)
    : _jsonFilePath(jsonFilePath), _p00(p00), _isLoading(false) {
  // 初始化网格地图为可通行 (0)
  for (int i = 0; i < MAP_GRID_SIZE; ++i) {
    for (int j = 0; j < MAP_GRID_SIZE; ++j) {
      _gridMap[i][j] = 0;
    }
  }
}

BuildingManager::~BuildingManager() { clearAllBuildings(); }

bool BuildingManager::isWalkable(int row, int col) const {
  if (!isValidGrid(row, col)) {
    return false;
  }
  return _gridMap[row][col] == 0;
}

bool BuildingManager::isValidGrid(int row, int col) const {
  return row >= 0 && row < MAP_GRID_SIZE && col >= 0 && col < MAP_GRID_SIZE;
}

void BuildingManager::updateGridState(int row, int col, int size,
                                      bool blocked) {
  // 假设 row, col 是中心点
  // 计算左下角起始点
  // 对于奇数 size (1, 3, 5): 中心在网格中心。范围 [c - s/2, c + s/2]
  // 对于偶数 size (2, 4): 中心在顶点。范围 [c - s/2, c + s/2 - 1]

  // 修正：根据 createBuilding 中的逻辑
  // if (gridCount % 2 != 0) anchorPos.x += _deltaX;
  // 这意味着奇数尺寸时，传入的 row/col
  // 对应的是网格坐标的整数部分，但实际中心偏移了。 让我们统一使用：row/col
  // 是中心网格坐标（或中心顶点坐标）。

  int startRow = row - size / 2;
  int startCol = col - size / 2;

  for (int r = startRow; r < startRow + size; ++r) {
    for (int c = startCol; c < startCol + size; ++c) {
      if (isValidGrid(r, c)) {
        _gridMap[r][c] = blocked ? 1 : 0;
      }
    }
  }
}

bool BuildingManager::init() {
  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("Failed to get ConfigManager in BuildingManager!");
    return false;
  }

  auto constantConfig = configManager->getConstantConfig();
  _deltaX = constantConfig.deltaX;
  _deltaY = constantConfig.deltaY;
  _gridSize = constantConfig.gridSize;

  // 加载建筑地图
  if (!loadBuildingMap()) {
    CCLOG("Failed to load building map!");
    return false;
  }

  // 初始计算一次资源统计
  updatePlayerResourcesStats();

  return true;
}

bool BuildingManager::loadBuildingMap() {
  _isLoading = true;
  rapidjson::Document doc;

  std::string content;
  bool isSaveFile = (_jsonFilePath == "develop/map.json");

  // 使用 PathUtils 获取真实路径
  std::string fullPath = PathUtils::getRealFilePath(_jsonFilePath, false);

  bool needSaveDefault = false;

  if (!fullPath.empty() && FileUtils::getInstance()->isFileExist(fullPath)) {
    content = FileUtils::getInstance()->getStringFromFile(fullPath);
    CCLOG("BuildingManager: Loading map from config: %s", fullPath.c_str());
  } else {
    if (_jsonFilePath == "develop/map.json") {
      CCLOG("Map file not found, creating default map.");
      content = R"({
    "TownHall": [ { "row": 22, "col": 22, "level": 1, "HP": 1500 } ],
    "GoldStorage": [ { "row": 18, "col": 22, "level": 1, "HP": 800 } ],
    "ElixirBottle": [ { "row": 26, "col": 22, "level": 1, "HP": 800 } ],
    "GoldMine": [ { "row": 22, "col": 18, "level": 1, "HP": 300 } ],
    "ElixirPump": [ { "row": 22, "col": 26, "level": 1, "HP": 300 } ]
})";
      needSaveDefault = true;
    } else {
      CCLOG("Config file not found: %s", _jsonFilePath.c_str());
      _isLoading = false;
      return false;
    }
  }

  if (content.empty()) {
    _isLoading = false;
    return false;
  }

  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    CCLOG("JSON parse error");
    _isLoading = false;
    return false;
  }

  for (auto& m : doc.GetObject()) {
    std::string buildingName = m.name.GetString();
    if (buildingName == "tips") continue;

    if (m.value.IsArray()) {
      const rapidjson::Value& array = m.value;
      for (rapidjson::SizeType i = 0; i < array.Size(); ++i) {
        const rapidjson::Value& item = array[i];
        if (item.IsObject() && item.HasMember("row") && item.HasMember("col")) {
          float row = item["row"].GetFloat();
          float col = item["col"].GetFloat();
          int level = item.HasMember("level") ? item["level"].GetInt() : 1;
          float hp = item.HasMember("HP") ? item["HP"].GetFloat() : -1.0f;

          // 读取存档中的 storedResource 和 lastTimestamp
          float storedResource = 0.0f;
          long long lastTimestamp = 0;
          if (item.HasMember("storedResource")) {
            storedResource = item["storedResource"].GetFloat();
          }
          if (item.HasMember("lastTimestamp")) {
            lastTimestamp = item["lastTimestamp"].GetInt64();
          }

          Building* building =
              createBuilding(buildingName, row, col, level, hp);
          if (building) {
            // 如果是资源建筑，设置暂存量并计算离线产出
            auto resBuilding = dynamic_cast<ResourceBuilding*>(building);
            if (resBuilding) {
              // 如果是从存档加载，计算离线产出
              if (isSaveFile && lastTimestamp > 0) {
                // updateOfflineProduction 会设置 storedResource 并加上离线产出
                resBuilding->updateOfflineProduction(lastTimestamp,
                                                     storedResource);
              }
            }

            registerBuilding(building);
          }
        }
      }
    }
  }

  // 更新玩家资源上限
  auto playerManager = PlayerManager::getInstance();
  if (playerManager) {
    // 资源上限已在 registerBuilding -> updatePlayerResourcesStats 中更新
    int currentMaxGold = playerManager->getMaxGold();
    int currentMaxElixir = playerManager->getMaxElixir();

    CCLOG("BuildingManager: Total Capacity Calculated - Gold: %d, Elixir: %d",
          currentMaxGold, currentMaxElixir);

    // 如果是新游戏（首次运行），将资源填满
    // [修复] 只有当资源确实为0时才填满，防止覆盖已加载的存档
    // 即使 isNewGame 为 true (可能是因为 user_data.json 丢失)，
    // 但如果 PlayerManager 内存中已经有数据（虽然不太可能），也不应该覆盖
    // 更重要的是，如果 loadUserData 成功，isNewGame 应该是 false
    if (playerManager->isNewGame()) {
      playerManager->setGold(currentMaxGold);
      playerManager->setElixir(currentMaxElixir);
      CCLOG("New Game: Resources filled to max capacity (Gold: %d, Elixir: %d)",
            currentMaxGold, currentMaxElixir);

      // 立即标记为非新游戏，防止后续重复重置
      playerManager->setIsNewGame(false);
    }

    // 强制保存一次，确保 user_data.json 被创建
    playerManager->saveUserData();
  }

  if (needSaveDefault) {
    saveBuildingMap();
    _isLoading = false;
  }

  return true;
}

// 实现保存地图功能
void BuildingManager::saveBuildingMap() {
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  // 将建筑按名称分组
  std::map<std::string, rapidjson::Value> buildingMap;

  for (auto building : _buildings) {
    if (!building) continue;

    std::string name = building->getBuildingName();
    if (buildingMap.find(name) == buildingMap.end()) {
      rapidjson::Value arr(rapidjson::kArrayType);
      buildingMap[name] = arr;
    }

    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("row", building->getRow(), allocator);
    obj.AddMember("col", building->getCol(), allocator);
    obj.AddMember("level", building->getLevel(), allocator);
    obj.AddMember("HP", building->getCurrentHP(), allocator);

    // 保存资源建筑的状态
    auto resBuilding = dynamic_cast<ResourceBuilding*>(building);
    if (resBuilding) {
      // 保存当前未收集的资源
      obj.AddMember("storedResource", resBuilding->getStoredResource(),
                    allocator);
      // 保存当前时间戳
      obj.AddMember("lastTimestamp", ResourceBuilding::getCurrentTimestamp(),
                    allocator);
    }

    buildingMap[name].PushBack(obj, allocator);
  }

  for (auto& pair : buildingMap) {
    rapidjson::Value k(pair.first.c_str(), allocator);
    doc.AddMember(k, pair.second, allocator);
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  // 使用 PathUtils 获取真实写入路径
  std::string path = PathUtils::getRealFilePath(_jsonFilePath, true);

  // [新增] 确保目录存在
  PathUtils::ensureDirectoryExists(path);

  // 使用 FileUtils::writeStringToFile 统一写入
  // 注意：PathUtils 在 Windows DevMode 下返回的是绝对路径，FileUtils 可以处理
  // 在非 Windows 平台，返回的是相对路径，FileUtils 会写入到 WritablePath
  // (如果设置了) 或者我们直接使用 std::ofstream 来确保绝对路径写入成功
  // (FileUtils 有时会强制前缀)

  // 为了确保 Windows DevMode 下能写入源码目录，我们优先使用 std::ofstream
  // 因为 FileUtils::writeStringToFile 可能会强制加上 WritablePath 前缀

  std::ofstream outFile(path.c_str());
  if (outFile.is_open()) {
    outFile << buffer.GetString();
    outFile.close();
    CCLOG("BuildingManager: Map saved to %s", path.c_str());
  } else {
    CCLOG("BuildingManager: Failed to save map to %s", path.c_str());
    // 尝试使用 MessageBox 提示错误 (仅限 Windows)
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    std::string msg = "Failed to save map to: " + path;
    MessageBoxA(NULL, msg.c_str(), "Save Error", MB_OK | MB_ICONERROR);
#endif
  }
}

Building* BuildingManager::createBuilding(const std::string& buildingName,
                                          float row, float col, int level,
                                          float hp) {
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return nullptr;
  }

  // 1. 获取该建筑的配置
  auto config = configManager->getBuildingConfig(buildingName);

  // 2. 获取类型 (TOWN_HALL, DEFENSE, RESOURCE...)
  std::string type = config.type;

  Building* building = nullptr;

  // 3. 根据类型分发创建
  if (type == "TOWN_HALL") {
    building = TownHall::create(level);
  } else if (type == "DEFENSE") {
    // 传入 buildingName (例如 "Cannon")，以便 DefenseBuilding
    // 内部再次读取伤害、范围等参数
    building = DefenseBuilding::create(level, buildingName);
  } else if (type == "RESOURCE") {
    building = ResourceBuilding::create(level, buildingName);
  } else if (type == "STORAGE") {
    building = StorageBuilding::create(level, buildingName);
  } else if (type == "BARRACKS") {
    building = BarracksBuilding::create(level, buildingName);
  } else if (type == "WALL") {
    building = Wall::create(level, buildingName);
  } else if (type == "TRAP") { 
    building = TrapBuilding::create(level, buildingName);
  }else {
    CCLOG("Unknown building type '%s' for building '%s'", type.c_str(),
          buildingName.c_str());
    return nullptr;
  }

  // 4. 设置位置 (通用逻辑)
  if (building) {
    Vec2 anchorPos = GridUtils::gridToScene(row, col, _p00);

    // 如果建筑占用的网格数是奇数（如3x3），其中心应该在网格中心
    // 但如果坐标系是基于网格顶点的（整数坐标），那么位置会偏离中心半个网格
    // 需要进行偏移修正，使其对齐到网格线
    // 偶数尺寸（如2x2, 4x4）的中心正好在顶点上，不需要修正
    if (building->getGridCount() % 2 != 0) {
      // 在等距视角下，向右移动 deltaX 相当于在网格坐标上移动 (0.5, 0.5)
      // 即从顶点移动到网格中心
      anchorPos.x += _deltaX;
    }

    building->setPosition(anchorPos);
    building->setCenterX(anchorPos.x);
    building->setCenterY(anchorPos.y);
    building->setRow(row);
    building->setCol(col);

    // 5. 设置生命值（如果指定了 HP，使用指定值；否则使用 MaxHP）
    if (hp >= 0.0f) {
      building->setCurrentHPAndUpdate(hp);
    } else {
      // 如果没有指定 HP，默认使用 MaxHP
      building->setCurrentHPAndUpdate(building->getMaxHP());
    }
  }

  return building;
}

Building* BuildingManager::getBuildingAtPosition(const Vec2& pos) const {
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return nullptr;
  }
  // 从后往前遍历（后添加的建筑在上层）
  for (auto it = _buildings.rbegin(); it != _buildings.rend(); ++it) {
    Building* building = *it;
    if (!building) {
      continue;
    }
    // TODO
    // 判断点是否在建筑的菱形区域内
    if (building->inDiamond(pos)) {
      return building;
    }
  }

  return nullptr;
}

Building* BuildingManager::getBuildingAtGrid(int row, int col) const {
  for (Building* building : _buildings) {
    if (building && building->isAlive()) {
      // 建筑中心坐标
      float bRow = building->getRow();
      float bCol = building->getCol();
      int gridSize = building->getGridCount();

      // 建筑占据的范围是从 center - size/2 到 center + size/2
      float halfSize = gridSize / 2.0f;

      // 检查点 (row, col) 是否在建筑范围内
      // 注意：这里使用简单的矩形范围检查，因为网格坐标系本身是正交的（虽然渲染是菱形）
      // 建筑占据的网格是 [bRow - halfSize, bRow + halfSize)
      if (row >= bRow - halfSize && row < bRow + halfSize &&
          col >= bCol - halfSize && col < bCol + halfSize) {
        return building;
      }
    }
  }
  return nullptr;
}

void BuildingManager::addBuildingsToLayer(Layer* layer) {
  if (!layer) {
    return;
  }

  for (auto building : _buildings) {
    if (building) {
      layer->addChild(building, 1);
    }
  }
}

void BuildingManager::clearAllBuildings() {
  for (auto building : _buildings) {
    if (building) {
      // 先从父节点移除（如果还在父节点中）
      // removeFromParent() 会自动调用 release()，将引用计数从 2 减到 1
      building->removeFromParent();
      // 由于 Building 是通过 create() 创建的，使用了 autorelease()
      // 初始引用计数为 1，addChild() 后变为 2
      // removeFromParent() 后变为 1，需要再 release 一次使其变为 0
      building->release();
    }
  }
  _buildings.clear();
}

void BuildingManager::registerBuilding(Building* building) {
  if (!building) {
    return;
  }

  // 避免重复注册
  auto it = std::find(_buildings.begin(), _buildings.end(), building);
  if (it != _buildings.end()) {
    return;
  }

  building->retain();
  _buildings.push_back(building);

  // 设置死亡回调
  building->setOnDeathCallback([this](Building* b) {
    // 建筑被摧毁时，更新网格状态为可通行
    this->updateGridState(static_cast<int>(b->getRow()),
                          static_cast<int>(b->getCol()), b->getGridCount(),
                          false);
    // 注意：这里不立即移除建筑，避免迭代器失效或悬空指针
    // 建筑对象会被保留在 _buildings 中直到场景销毁
    // 但 isAlive() 会返回 false，所以逻辑上已经死亡
  });

  // 更新网格状态
  updateGridState(static_cast<int>(building->getRow()),
                  static_cast<int>(building->getCol()),
                  building->getGridCount(), true);

  // 更新资源统计
  updatePlayerResourcesStats();

  if (!_isLoading) {
    saveBuildingMap();
  }
}

void BuildingManager::updatePlayerResourcesStats() {
  auto playerManager = PlayerManager::getInstance();
  if (!playerManager) {
    return;
  }

  int maxGold = 0;
  int maxElixir = 0;
  int goldProd = 0;
  int elixirProd = 0;

  // 基础容量
  maxGold = 0;
  maxElixir = 0;

  for (auto building : _buildings) {
    if (!building) continue;

    if (building->getBuildingType() == BuildingType::STORAGE) {
      auto storage = dynamic_cast<StorageBuilding*>(building);
      if (storage) {
        if (storage->getResourceType() == "Gold") {
          maxGold += storage->getCapacity();
        } else if (storage->getResourceType() == "Elixir") {
          maxElixir += storage->getCapacity();
        }
      }
    } else if (building->getBuildingType() == BuildingType::RESOURCE) {
      auto resource = dynamic_cast<ResourceBuilding*>(building);
      if (resource) {
        if (resource->getResourceType() == "Gold") {
          // Resource buildings store their own resources, do not add to global
          // capacity
          goldProd += resource->getProductionRate();
        } else if (resource->getResourceType() == "Elixir") {
          // Resource buildings store their own resources, do not add to global
          // capacity
          elixirProd += resource->getProductionRate();
        }
      }
    }
  }

  playerManager->setMaxGold(maxGold);
  playerManager->setMaxElixir(maxElixir);
  playerManager->setGoldProduction(goldProd);
  playerManager->setElixirProduction(elixirProd);
}

void BuildingManager::removeBuilding(Building* building) {
  if (!building) return;
  auto it = std::find(_buildings.begin(), _buildings.end(), building);
  if (it != _buildings.end()) {
    // 更新网格状态
    updateGridState(static_cast<int>(building->getRow()),
                    static_cast<int>(building->getCol()),
                    building->getGridCount(), false);

    _buildings.erase(it);
    updatePlayerResourcesStats();
    saveBuildingMap();
  }
}

bool BuildingManager::getBuildingResult(int& stars, float& ratio, bool& win) {
  stars = 0;
  ratio = 0.0f;
  win = false;

  int totalCount = 0;
  int destroyedCount = 0;
  bool townHallDestroyed = false;

  for (auto building : _buildings) {
    if (!building) continue;
    if (dynamic_cast<Wall*>(building)) continue;

    totalCount++;

    if (building->getCurrentHP() <= 0) {
      destroyedCount++;

      if (dynamic_cast<TownHall*>(building)) {
        townHallDestroyed = true;
      }
    }
  }

  if (totalCount == 0) {
    return false;
  }

  ratio = static_cast<float>(destroyedCount) / totalCount;

  // 星级判定
  if (ratio >= 1.0f) {
    stars = 3;
  } else if (ratio >= 0.5f) {
    stars = townHallDestroyed ? 2 : 1;
  } else {
    stars = townHallDestroyed ? 1 : 0;
  }

  win = (stars >= 1);

  // 同步成员变量
  _ratio = ratio;
  _stars = stars;
  _win = win;

  return true;
}

void BuildingManager::updateClansWar(const std::string& clans_id,
                                     const std::string& map_id) {
  // 构造当前地图的 JSON 字符串（与 saveBuildingMap 类似，但不写入文件）
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  // 将建筑按名称分组
  std::map<std::string, rapidjson::Value> buildingMap;

  for (auto building : _buildings) {
    if (!building || building->getCurrentHP() < 0.1f) continue;

    std::string name = building->getBuildingName();
    if (buildingMap.find(name) == buildingMap.end()) {
      rapidjson::Value arr(rapidjson::kArrayType);
      buildingMap[name] = arr;
    }

    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("row", building->getRow(), allocator);
    obj.AddMember("col", building->getCol(), allocator);
    obj.AddMember("level", building->getLevel(), allocator);
    obj.AddMember("HP", building->getCurrentHP(), allocator);

    // 保存资源建筑的状态
    auto resBuilding = dynamic_cast<ResourceBuilding*>(building);
    if (resBuilding) {
      obj.AddMember("storedResource", resBuilding->getStoredResource(),
                    allocator);
      obj.AddMember("lastTimestamp", ResourceBuilding::getCurrentTimestamp(),
                    allocator);
    }

    buildingMap[name].PushBack(obj, allocator);
  }

  for (auto& pair : buildingMap) {
    rapidjson::Value k(pair.first.c_str(), allocator);
    doc.AddMember(k, pair.second, allocator);
  }
  int stars;
  float ratio;
  bool win;
  getBuildingResult(stars, ratio, win);
  doc.AddMember("stars", stars, allocator);
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  std::string jsonString = buffer.GetString();

  // 使用 ClansWar API 上传/保存地图数据
  ClansWar::saveWarMap(
      clans_id, map_id, jsonString,
      [clans_id, map_id](bool success, const std::string& message) {
        if (success) {
          CCLOG("ClansWar: saveWarMap succeeded for %s %s", clans_id.c_str(),
                map_id.c_str());
        } else {
          CCLOG("ClansWar: saveWarMap failed: %s", message.c_str());
        }
      });
}