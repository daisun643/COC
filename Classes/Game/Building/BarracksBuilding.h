#ifndef __BARRACKS_BUILDING_H__
#define __BARRACKS_BUILDING_H__

#include "Building.h"

class BarracksBuilding : public Building {
 public:
  static BarracksBuilding* create(int level, const std::string& buildingName);
  bool init(int level, const std::string& buildingName);

  // 重写升级方法
  virtual void upgrade() override;

  CC_SYNTHESIZE(int, _queueSize, QueueSize);

 protected:
  BarracksBuilding();
  virtual ~BarracksBuilding();
};

#endif