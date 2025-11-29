#include "BuildingManager.h"

#include <cmath>
#include <fstream>
#include <sstream>

#include "Game/Building/TownHall.h"
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

  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return false;
  }
  // TODO 这里 进一步适配
  std::vector<std::string> buildingTypes = {"TownHall"};

  // 解析TownHall配置
  if (doc.HasMember("TownHall") && doc["TownHall"].IsArray()) {
    const rapidjson::Value& townHallArray = doc["TownHall"];
    for (rapidjson::SizeType i = 0; i < townHallArray.Size(); ++i) {
      const rapidjson::Value& item = townHallArray[i];
      if (item.IsObject() && item.HasMember("row") && item.HasMember("col") &&
          item.HasMember("level")) {
        int row = item["row"].GetInt();
        int col = item["col"].GetInt();
        int level = item["level"].GetInt();

        Building* building = createBuilding("TownHall", row, col, level);
        if (building) {
          building->retain();
          _buildings.push_back(building);
        }
      }
    }
  }

  return true;
}

Building* BuildingManager::createBuilding(const std::string& buildingType,
                                          int row, int col, int level) {
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return nullptr;
  }

  Building* building = nullptr;

  if (buildingType == "TownHall") {
    auto townHallConfig = configManager->getTownHallConfig();
    auto constantConfig = configManager->getConstantConfig();

    // 使用TownHall的create方法，传入gridSize和anchorRatio
    TownHall* townHall = TownHall::create(
        level, townHallConfig.gridSize, townHallConfig.anchorRatioX,
        townHallConfig.anchorRatioY, constantConfig.deltaX,
        constantConfig.grassWidth, townHallConfig.imageScale);

    if (townHall) {
      building = townHall;

      // 计算锚点位置
      Vec2 anchorPos = GridUtils::gridToScreen(row, col, _p00);

      // 设置位置（根据anchor和gridSize计算中心坐标）
      building->setPositionFromAnchor(anchorPos.x, anchorPos.y, _deltaX, row,
                                      col);
    }
  }
  // 可以在这里添加其他建筑类型的创建逻辑

  return building;
}

Building* BuildingManager::getBuildingAtPosition(const Vec2& pos) const {
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return nullptr;
  }
  auto constantConfig = configManager->getConstantConfig();

  // 从后往前遍历（后添加的建筑在上层）
  for (auto it = _buildings.rbegin(); it != _buildings.rend(); ++it) {
    Building* building = *it;
    if (!building) {
      continue;
    }

    // 获取建筑的锚点位置
    Vec2 anchorPos = building->getPosition();

    // 判断点是否在建筑的菱形区域内
    if (building->isPointInDiamond(pos, anchorPos, building->getWidth(),
                                   constantConfig.deltaX,
                                   constantConfig.deltaY)) {
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
