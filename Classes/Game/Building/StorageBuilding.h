#ifndef __STORAGE_BUILDING_H__
#define __STORAGE_BUILDING_H__

#include "Building.h"

class StorageBuilding : public Building {
 public:
  static StorageBuilding* create(int level, const std::string& buildingName);
  bool init(int level, const std::string& buildingName);

  CC_SYNTHESIZE(int, _capacity, Capacity);
  CC_SYNTHESIZE(std::string, _resourceType, ResourceType);

 protected:
  StorageBuilding();
  virtual ~StorageBuilding();
};

#endif