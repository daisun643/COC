#ifndef __HEAL_SPELL_H__
#define __HEAL_SPELL_H__

#include "Game/Spell/BasicSpell.h"

USING_NS_CC;

/**
 * 治疗法术
 * 治疗范围内的友方士兵
 */
class HealSpell : public BasicSpell {
 public:
  /**
   * 创建治疗法术
   * @return 法术实例
   */
  static HealSpell* create();

  /**
   * 初始化治疗法术
   * @return 是否初始化成功
   */
  bool init() override;

 protected:
  /**
   * 应用治疗效果
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表（治疗法术不影响建筑）
   */
  void applyEffect(const std::vector<BasicSoldier*>& soldiers,
                   const std::vector<Building*>& buildings) override;

  /**
   * 更新持续治疗效果
   * @param delta 时间间隔
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表
   */
  void updateEffect(float delta, const std::vector<BasicSoldier*>& soldiers,
                    const std::vector<Building*>& buildings) override;

  /**
   * 创建视觉效果（绿色治疗圈）
   */
  void createVisualEffect() override;

  HealSpell();
  virtual ~HealSpell();

 private:
  float _healPerSecond;  // 每秒治疗量
};

#endif  // __HEAL_SPELL_H__
