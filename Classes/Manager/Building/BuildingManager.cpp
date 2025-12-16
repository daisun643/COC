#include "BuildingManager.h"

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
#include "Manager/Config/ConfigManager.h"
#include "Manager/PlayerManager.h"
#include "Utils/GridUtils.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

BuildingManager::BuildingManager(const std::string& jsonFilePath,
                                 const Vec2& p00)
    : _jsonFilePath(jsonFilePath), _p00(p00) {}

BuildingManager::~BuildingManager() { clearAllBuildings(); }

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
  rapidjson::Document doc;

  // 优先从 WritablePath (存档) 加载
  std::string writablePath =
      FileUtils::getInstance()->getWritablePath() + "map.json";
  std::string content;
  bool isSaveFile = false;

  if (FileUtils::getInstance()->isFileExist(writablePath)) {
    content = FileUtils::getInstance()->getStringFromFile(writablePath);
    CCLOG("BuildingManager: Loading map from save file: %s",
          writablePath.c_str());
    isSaveFile = true;
  } else {
    // 如果没有存档，从 Resources 加载默认配置
    std::string fullPath =
        FileUtils::getInstance()->fullPathForFilename(_jsonFilePath);
    if (fullPath.empty()) {
      CCLOG("Config file not found: %s", _jsonFilePath.c_str());
      return false;
    }
    content = FileUtils::getInstance()->getStringFromFile(fullPath);
    CCLOG("BuildingManager: Loading map from default config: %s",
          fullPath.c_str());
  }

  if (content.empty()) {
    return false;
  }

  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    CCLOG("JSON parse error");
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
              } else {
                // 如果是初始配置或没有存档，重置为0 (ResourceBuilding
                // 构造函数已做，但这里确保一下) 注意：初始配置没有
                // lastTimestamp，逻辑会自动跳过计算
              }
            }

            registerBuilding(building);
          }
        }
      }
    }
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

  // 保存到 WritablePath 下的 map.json
  std::string path = FileUtils::getInstance()->getWritablePath() + "map.json";

  std::ofstream outFile(path.c_str());
  if (outFile.is_open()) {
    outFile << buffer.GetString();
    outFile.close();
    CCLOG("BuildingManager: Map saved to %s", path.c_str());
  } else {
    CCLOG("BuildingManager: Failed to save map.");
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
  } else {
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

  // 更新资源统计
  updatePlayerResourcesStats();
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

  // 基础容量（例如大本营自带的，或者系统默认的）
  // 这里假设默认有1000容量，避免初始为0
  maxGold = 1000;
  maxElixir = 1000;

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
    _buildings.erase(it);
    updatePlayerResourcesStats();
  }
}
