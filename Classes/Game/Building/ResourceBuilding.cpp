#include "ResourceBuilding.h"

#include "Manager/Config/ConfigManager.h"

ResourceBuilding::ResourceBuilding()
    : _productionRate(0), _capacity(0), _storedResource(0.0f) {
  _buildingType = BuildingType::RESOURCE;
}

ResourceBuilding::~ResourceBuilding() {}

ResourceBuilding* ResourceBuilding::create(int level,
                                           const std::string& buildingName) {
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
                      config.gridCount, config.anchorRatioX,
                      config.anchorRatioY, config.imageScale)) {
    return false;
  }

  this->_productionRate = config.productionRate;
  this->_capacity = config.capacity;
  this->_resourceType = config.resourceType;
  // 设置最大生命值（当前生命值将在 BuildingManager 中设置，默认为 MaxHP）
  this->_maxHP = config.maxHP;
  // 开启update
  this->scheduleUpdate();

  return true;
}

void ResourceBuilding::update(float dt) {
  if (_productionRate <= 0 || _capacity <= 0) return;

  // 生产逻辑：为了加快测试速度，将配置的产量视为“每分钟产量”
  // 原逻辑是 / 3600.0f (每小时)，现在改为 / 60.0f (每分钟)
  float productionPerSecond = _productionRate / 60.0f;

  _storedResource += productionPerSecond * dt;

  if (_storedResource > _capacity) {
    _storedResource = _capacity;
  }
}
int ResourceBuilding::collect() {
  int amount = static_cast<int>(_storedResource);
  if (amount > 0) {
    _storedResource -= amount;
  }
  return amount;
}