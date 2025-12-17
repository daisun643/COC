#ifndef __TRAP_BUILDING_H__
#define __TRAP_BUILDING_H__

#include "Building.h"
#include "Game/Soldier/BasicSoldier.h"
#include <vector>

/**
 * 陷阱建筑类
 * 敌人进入范围后触发，造成范围伤害
 */
class TrapBuilding : public Building {
public:
    static TrapBuilding* create(int level, const std::string& buildingName);
    virtual bool init(int level, const std::string& buildingName);

    // 重写升级方法
    virtual void upgrade() override;

    // 陷阱属性
    CC_SYNTHESIZE(float, _triggerRange, TriggerRange); // 触发范围
    CC_SYNTHESIZE(int, _damage, Damage);               // 爆炸伤害
    CC_SYNTHESIZE(bool, _isArmed, IsArmed);            // 是否已布防

    /**
     * 检查并尝试触发陷阱
     * @param soldiers 附近的敌军列表
     * @return 是否成功触发
     */
    bool checkTrigger(const std::vector<BasicSoldier*>& soldiers);

    /**
     * 重置陷阱（例如战斗结束后重新布防）
     */
    void rearm();

protected:
    TrapBuilding();
    virtual ~TrapBuilding();

    void explode(); // 执行爆炸逻辑（特效、伤害计算等）
};

#endif // __TRAP_BUILDING_H__