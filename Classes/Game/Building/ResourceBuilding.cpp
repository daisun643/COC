#include "ResourceBuilding.h"
#include "Manager/Config/ConfigManager.h"

ResourceBuilding::ResourceBuilding() 
: _productionRate(0), _capacity(0) {
  _buildingType = BuildingType::RESOURCE;
}

ResourceBuilding::~ResourceBuilding() {}

ResourceBuilding* ResourceBuilding::create(int level, const std::string& buildingName) {
  ResourceBuilding* p = new (std::nothrow) ResourceBuilding();
  if (p && p->init(level, buildingName)) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool ResourceBuilding::init(int level, const std::string& buildingName) {
  auto config = ConfigManager::getInstance()->getBuildingConfig(buildingName);
  _buildingName = buildingName;

  if (!Building::init(config.image, BuildingType::RESOURCE, level, 
                      config.gridCount, config.anchorRatioX, config.anchorRatioY, config.imageScale)) {
    return false;
  }

  this->_productionRate = config.productionRate;
  this->_capacity = config.capacity;
  this->_resourceType = config.resourceType;

  return true;
}