#ifndef __RESOURCE_BUILDING_H__
#define __RESOURCE_BUILDING_H__

#include "Building.h"

class ResourceBuilding : public Building {
 public:
  static ResourceBuilding* create(int level, const std::string& buildingName);
  bool init(int level, const std::string& buildingName);

  CC_SYNTHESIZE(int, _productionRate, ProductionRate);
  CC_SYNTHESIZE(int, _capacity, Capacity);
  CC_SYNTHESIZE(std::string, _resourceType, ResourceType);

  // 当前暂存的资源量
  CC_SYNTHESIZE_READONLY(float, _storedResource, StoredResource);

  // 收集资源，返回收集到的数量，并清空暂存
  int collect();

  // 每帧更新生产
  virtual void update(float dt) override;

 protected:
  ResourceBuilding();
  virtual ~ResourceBuilding();
};

#endif