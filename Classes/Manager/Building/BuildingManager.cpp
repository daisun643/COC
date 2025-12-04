#include "BuildingManager.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

// 包含所有具体的建筑头文件
#include "Game/Building/TownHall.h"
#include "Game/Building/DefenseBuilding.h"
#include "Game/Building/ResourceBuilding.h"
#include "Game/Building/StorageBuilding.h"
#include "Game/Building/BarracksBuilding.h"

#include "Manager/Config/ConfigManager.h"
#include "Utils/GridUtils.h"
#include "json/document.h"

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

  return true;
}

bool BuildingManager::loadBuildingMap() {
  rapidjson::Document doc;

  // 使用构造函数传入的JSON文件路径
  FileUtils* fileUtils = FileUtils::getInstance();
  std::string fullPath = fileUtils->fullPathForFilename(_jsonFilePath);

  if (fullPath.empty()) {
    CCLOG("Config file not found: %s", _jsonFilePath.c_str());
    return false;
  }

  // 读取文件内容
  std::string content = fileUtils->getStringFromFile(fullPath);
  if (content.empty()) {
    CCLOG("Config file is empty: %s", _jsonFilePath.c_str());
    return false;
  }

  // 解析JSON
  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    CCLOG("JSON parse error in file %s at offset %u (error code: %d)",
          _jsonFilePath.c_str(), (unsigned)doc.GetErrorOffset(),
          (int)doc.GetParseError());
    return false;
  }

  // 遍历 JSON 中的所有键 (TownHall, Cannon, etc.)
  for (auto& m : doc.GetObject()) {
    std::string buildingName = m.name.GetString();
    
    // 跳过可能的非建筑字段 (如 tips)
    if (buildingName == "tips") continue;

    // 对应的值必须是一个数组，包含多个坐标对象
    if (m.value.IsArray()) {
      const rapidjson::Value& array = m.value;
      for (rapidjson::SizeType i = 0; i < array.Size(); ++i) {
        const rapidjson::Value& item = array[i];
        if (item.IsObject() && item.HasMember("row") && item.HasMember("col")) {
          int row = item["row"].GetInt();
          int col = item["col"].GetInt();
          int level = item.HasMember("level") ? item["level"].GetInt() : 1;

          // 调用工厂方法创建建筑
          Building* building = createBuilding(buildingName, row, col, level);
          if (building) {
            registerBuilding(building);
          }
        }
      }
    }
  }

  return true;
}

Building* BuildingManager::createBuilding(const std::string& buildingName,
                                          int row, int col, int level) {
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
  } 
  else if (type == "DEFENSE") {
    // 传入 buildingName (例如 "Cannon")，以便 DefenseBuilding 内部再次读取伤害、范围等参数
    building = DefenseBuilding::create(level, buildingName);
  }
  else if (type == "RESOURCE") {
    building = ResourceBuilding::create(level, buildingName);
  }
  else if (type == "STORAGE") {
    building = StorageBuilding::create(level, buildingName);
  }
  else if (type == "BARRACKS") {
    building = BarracksBuilding::create(level, buildingName);
  }
  else {
    CCLOG("Unknown building type '%s' for building '%s'", type.c_str(), buildingName.c_str());
    return nullptr;
  }

  // 4. 设置位置 (通用逻辑)
  if (building) {
    Vec2 anchorPos = GridUtils::gridToScene(row, col, _p00);
    building->setPosition(anchorPos);
    building->setCenterX(anchorPos.x);
    building->setCenterY(anchorPos.y);
    building->setRow(row);
    building->setCol(col);
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
}
