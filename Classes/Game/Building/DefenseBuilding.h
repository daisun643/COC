#ifndef __DEFENSE_BUILDING_H__
#define __DEFENSE_BUILDING_H__

#include "Building.h"

class DefenseBuilding : public Building {
 public:
  static DefenseBuilding* create(int level, const std::string& buildingName);
  bool init(int level, const std::string& buildingName);

  // 重写升级方法
  virtual void upgrade() override;

  CC_SYNTHESIZE(float, _attackRange, AttackRange);
  CC_SYNTHESIZE(int, _damage, Damage);
  CC_SYNTHESIZE(float, _attackSpeed, AttackSpeed);

 protected:
  DefenseBuilding();
  virtual ~DefenseBuilding();
};

#endif