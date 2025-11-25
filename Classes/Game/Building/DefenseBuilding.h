#ifndef __DEFENSE_BUILDING_H__
#define __DEFENSE_BUILDING_H__

#include "Building.h"

/**
 * 防御建筑类型
 */
enum class DefenseType {
    CANNON,         // 加农炮
    ARCHER_TOWER,   // 箭塔
    WALL,           // 城墙
    MORTAR          // 迫击炮
};

/**
 * 防御建筑基类
 */
class DefenseBuilding : public Building {
public:
    static DefenseBuilding* create(DefenseType defenseType, int level = 1);
    
    virtual bool init(DefenseType defenseType, int level);
    
    // 防御属性
    CC_SYNTHESIZE(int, _damage, Damage);
    CC_SYNTHESIZE(float, _range, Range);
    CC_SYNTHESIZE(float, _attackSpeed, AttackSpeed);
    CC_SYNTHESIZE(DefenseType, _defenseType, DefenseType);
    
    /**
     * 攻击目标
     */
    virtual void attack(Node* target);
    
    /**
     * 获取防御建筑信息
     */
    virtual std::string getBuildingInfo() const override;
    
protected:
    DefenseBuilding();
    virtual ~DefenseBuilding();
    
    /**
     * 根据防御类型初始化属性
     */
    virtual void initDefenseProperties();
};

#endif // __DEFENSE_BUILDING_H__

