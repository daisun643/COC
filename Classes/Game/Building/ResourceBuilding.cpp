#include "ResourceBuilding.h"

ResourceBuilding::ResourceBuilding()
    : _resourceType(ResourceType::GOLD), _productionRate(100), _capacity(1000),
      _accumulatedTime(0.0f), _storedResource(0) {
  _buildingType = BuildingType::RESOURCE;
  _maxLevel = 10;
}

ResourceBuilding::~ResourceBuilding() {}

ResourceBuilding *ResourceBuilding::create(ResourceType resourceType,
                                           int level) {
  ResourceBuilding *building = new (std::nothrow) ResourceBuilding();
  if (building && building->init(resourceType, level)) {
    building->autorelease();
    return building;
  }
  CC_SAFE_DELETE(building);
  return nullptr;
}

bool ResourceBuilding::init(ResourceType resourceType, int level) {
  _resourceType = resourceType;

  // 根据资源类型设置名称
  switch (resourceType) {
  case ResourceType::GOLD:
    _buildingName = "Gold Mine";
    break;
  case ResourceType::ELIXIR:
    _buildingName = "Elixir Collector";
    break;
  case ResourceType::DARK_ELIXIR:
    _buildingName = "Dark Elixir Drill";
    break;
  }

  if (!Building::init("", BuildingType::RESOURCE, level)) {
    return false;
  }

  initResourceProperties();

  // 开始生产
  startProduction();

  // 启用更新
  this->scheduleUpdate();

  return true;
}

void ResourceBuilding::initResourceProperties() {
  // 根据资源类型和等级初始化属性
  switch (_resourceType) {
  case ResourceType::GOLD:
    _productionRate = 100 * _level; // 每小时产量
    _capacity = 1000 * _level;      // 储存容量
    break;
  case ResourceType::ELIXIR:
    _productionRate = 80 * _level;
    _capacity = 800 * _level;
    break;
  case ResourceType::DARK_ELIXIR:
    _productionRate = 50 * _level;
    _capacity = 500 * _level;
    break;
  }
}

void ResourceBuilding::startProduction() {
  _accumulatedTime = 0.0f;
  _storedResource = 0;
}

void ResourceBuilding::stopProduction() {
  // 停止生产
}

int ResourceBuilding::collectResource() {
  int collected = _storedResource;
  _storedResource = 0;
  return collected;
}

void ResourceBuilding::update(float delta) {
  _accumulatedTime += delta;

  // 每秒生产资源（简化计算）
  // 实际游戏中应该按小时计算
  float productionPerSecond = _productionRate / 3600.0f;
  int newResource = static_cast<int>(_accumulatedTime * productionPerSecond);

  if (newResource > 0) {
    _storedResource += newResource;
    _accumulatedTime = 0.0f;

    // 限制不超过容量
    if (_storedResource > _capacity) {
      _storedResource = _capacity;
    }
  }
}

std::string ResourceBuilding::getBuildingInfo() const {
  char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "%s\nLevel: %d/%d\nResource: %s\nProduction: %d/h\nCapacity: "
           "%d\nStored: %d/%d\nUpgrade Cost: %d",
           _buildingName.c_str(), _level, _maxLevel, getResourceName().c_str(),
           _productionRate, _capacity, _storedResource, _capacity,
           getUpgradeCost());
  return std::string(buffer);
}

std::string ResourceBuilding::getResourceName() const {
  switch (_resourceType) {
  case ResourceType::GOLD:
    return "Gold";
  case ResourceType::ELIXIR:
    return "Elixir";
  case ResourceType::DARK_ELIXIR:
    return "Dark Elixir";
  }
  return "Unknown";
}
