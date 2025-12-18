#ifndef __RESOURCE_BUILDING_H__
#define __RESOURCE_BUILDING_H__

#include "Building.h"
#include <chrono>

class ResourceBuilding : public Building {
 public:
  static ResourceBuilding* create(int level, const std::string& buildingName);
  bool init(int level, const std::string& buildingName);

  // 重写升级方法
  virtual void upgrade() override;
  // 新增重写 completeUpgrade
  virtual void completeUpgrade() override;

  CC_SYNTHESIZE(int, _productionRate, ProductionRate);
  CC_SYNTHESIZE(int, _capacity, Capacity);
  CC_SYNTHESIZE(std::string, _resourceType, ResourceType);

  // 当前暂存的资源量
  CC_SYNTHESIZE_READONLY(float, _storedResource, StoredResource);

  // 收集资源，返回收集到的数量，并清空暂存
  int collect();

  // 每帧更新生产
  virtual void update(float dt) override;

  // 计算离线产出
  // 参数: lastTimestamp (上次存档/退出的时间戳，秒), savedStoredAmount (上次存档时已有的资源量)
  void updateOfflineProduction(long long lastTimestamp, float savedStoredAmount);

  // 获取当前时间戳（秒），用于保存存档时调用
  static long long getCurrentTimestamp();

 protected:
  ResourceBuilding();
  virtual ~ResourceBuilding();
};

#endif