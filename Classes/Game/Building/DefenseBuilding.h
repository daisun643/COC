#ifndef __DEFENSE_BUILDING_H__
#define __DEFENSE_BUILDING_H__

#include "Building.h"
#include "Game/Soldier/BasicSoldier.h"
#include <vector>

class DefenseBuilding : public Building {
 public:
  static DefenseBuilding* create(int level, const std::string& buildingName);
  bool init(int level, const std::string& buildingName);

  CC_SYNTHESIZE(float, _attackRange, AttackRange);
  CC_SYNTHESIZE(int, _damage, Damage);
  CC_SYNTHESIZE(float, _attackSpeed, AttackSpeed);
  
  BasicSoldier* _currentTarget;  // 当前攻击目标
  float _attackCooldown;          // 攻击冷却时间

  /**
   * 攻击范围内的士兵
   * @param soldiers 士兵列表
   * @param targetCategories 可攻击的士兵类别列表（如果为空，则攻击所有类别）
   * @param delta 时间间隔
   * @return 是否成功攻击了目标
   */
  bool attackSoldiers(const std::vector<BasicSoldier*>& soldiers,
                      const std::vector<SoldierCategory>& targetCategories,
                      float delta);

  /**
   * 攻击范围内的士兵（重载版本，攻击指定类别）
   * @param soldiers 士兵列表
   * @param targetCategory 可攻击的士兵类别
   * @param delta 时间间隔
   * @return 是否成功攻击了目标
   */
  bool attackSoldiers(const std::vector<BasicSoldier*>& soldiers,
                      SoldierCategory targetCategory,
                      float delta);

 protected:
  DefenseBuilding();
  virtual ~DefenseBuilding();
};

#endif