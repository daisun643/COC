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
  // 只调用父类开始倒计时，不要在这里更新属性
  // 属性更新应该在倒计时结束后的 completeUpgrade 中进行
  Building::upgrade();
}

// 新增 completeUpgrade 实现
void ResourceBuilding::completeUpgrade() {
    // 1. 先调用父类完成通用逻辑（等级+1，外观更新，血量更新等）
    Building::completeUpgrade();

    // 2. 更新资源建筑特有的属性（产能、容量）
    auto config = ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);
    this->_productionRate = config.productionRate;
    this->_capacity = config.capacity;

    CCLOG("ResourceBuilding upgrade completed! New Rate: %d, New Capacity: %d", _productionRate, _capacity);
}

void ResourceBuilding::update(float dt) {
  // 必须调用父类 update 以执行倒计时逻辑
  Building::update(dt);

  // 可选：如果希望升级时停止生产，可以打开下面这行注释
  // if (isUpgrading()) return;

  if (_productionRate <= 0 || _capacity <= 0) return;

  // 生产逻辑
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