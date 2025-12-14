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
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(buildingName, level);
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

void ResourceBuilding::upgrade() {
  Building::upgrade();
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);
  this->_productionRate = config.productionRate;
  this->_capacity = config.capacity;
  CCLOG("ResourceBuilding upgraded: Rate -> %d", _productionRate);
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

// 实现离线产出计算
void ResourceBuilding::updateOfflineProduction(long long lastTimestamp, float savedStoredAmount) {
  // 1. 恢复上次保存的库存
  _storedResource = savedStoredAmount;

  if (lastTimestamp <= 0) {
      return; // 如果没有有效的时间戳，直接返回
  }

  // 2. 获取当前时间
  long long now = getCurrentTimestamp();

  // 3. 计算时间差（秒）
  long long diff = now - lastTimestamp;

  if (diff > 0) {
      // 4. 计算离线期间的产出
      // 注意：这里的产率计算需与 update 中的逻辑保持一致 (_productionRate / 60.0f)
      float productionPerSecond = _productionRate / 60.0f;
      float offlineProduction = diff * productionPerSecond;

      _storedResource += offlineProduction;
      
      CCLOG("ResourceBuilding: Offline for %llds, produced %.2f resources.", diff, offlineProduction);
  }

  // 5. 确保不超过容量上限
  if (_storedResource > _capacity) {
      _storedResource = _capacity;
  }
}

// 获取当前时间戳的辅助函数
long long ResourceBuilding::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

int ResourceBuilding::collect() {
  int amount = static_cast<int>(_storedResource);
  if (amount > 0) {
    _storedResource -= amount;
  }
  return amount;
}