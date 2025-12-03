#ifndef __RAGE_SPELL_H__
#define __RAGE_SPELL_H__

#include <set>

#include "Game/Spell/BasicSpell.h"

USING_NS_CC;

/**
 * 狂暴法术
 * 提升范围内士兵的攻击和移动速度
 */
class RageSpell : public BasicSpell {
 public:
  /**
   * 创建狂暴法术
   * @return 法术实例
   */
  static RageSpell* create();

  /**
   * 初始化狂暴法术
   * @return 是否初始化成功
   */
  bool init() override;

 protected:
  /**
   * 应用狂暴效果
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表（狂暴法术不影响建筑）
   */
  void applyEffect(const std::vector<BasicSoldier*>& soldiers,
                   const std::vector<Building*>& buildings) override;

  /**
   * 更新持续狂暴效果
   * @param delta 时间间隔
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表
   */
  void updateEffect(float delta, const std::vector<BasicSoldier*>& soldiers,
                    const std::vector<Building*>& buildings) override;

  /**
   * 创建视觉效果（红色狂暴圈）
   */
  void createVisualEffect() override;

  /**
   * 法术结束时的处理（恢复原始速度）
   */
  void onSpellEnd() override;

  RageSpell();
  virtual ~RageSpell();

 private:
  std::set<BasicSoldier*> _affectedSoldiers;  // 当前在范围内的受影响的士兵集合
};

#endif  // __RAGE_SPELL_H__
