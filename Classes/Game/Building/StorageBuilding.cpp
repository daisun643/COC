#include "StorageBuilding.h"

#include "Manager/Config/ConfigManager.h"

StorageBuilding::StorageBuilding() : _capacity(0) {
  _buildingType = BuildingType::STORAGE;
}

StorageBuilding::~StorageBuilding() {}

StorageBuilding* StorageBuilding::create(int level,
                                         const std::string& buildingName) {
  StorageBuilding* p = new (std::nothrow) StorageBuilding();
  if (p && p->init(level, buildingName)) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool StorageBuilding::init(int level, const std::string& buildingName) {
  auto config = ConfigManager::getInstance()->getBuildingConfig(buildingName);
  _buildingName = buildingName;

  if (!Building::init(config.image, BuildingType::STORAGE, level,
                      config.gridCount, config.anchorRatioX,
                      config.anchorRatioY, config.imageScale)) {
    return false;
  }

  this->_capacity = config.capacity;
  this->_resourceType = config.resourceType;

  // 设置最大生命值（当前生命值将在 BuildingManager 中设置，默认为 MaxHP）
  this->_maxHP = config.maxHP;

  return true;
}