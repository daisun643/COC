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

 protected:
  ResourceBuilding();
  virtual ~ResourceBuilding();
};

#endif